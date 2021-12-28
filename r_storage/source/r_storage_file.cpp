
#include "r_storage/r_storage_file.h"
#include "r_storage/r_dumbdex.h"
#include "r_storage/r_rel_block.h"
#include "r_utils/r_file.h"
#include "r_utils/r_exception.h"
#include "r_utils/r_blob_tree.h"
#include <stdexcept>
#include <cmath>
#include <memory>
#include <algorithm>

using namespace r_utils;
using namespace r_storage;
using namespace std;

r_storage_file::r_storage_file() :
    _file(),
    _h(),
    _dumbdex_map(),
    _block_index(),
    _gop_buffer(),
    _ind_map(),
    _current_block()
{
}

r_storage_file::r_storage_file(const string& file_name) :
    _file(r_file::open(file_name, "r+")),
    _h(_read_header(file_name)),
    _dumbdex_map(_map_block(0)),
    _block_index((uint8_t*)_dumbdex_map.map() + R_STORAGE_FILE_HEADER_SIZE, _h.num_blocks),
    _gop_buffer(),
    _ind_map(),
    _current_block()
{
}

r_storage_file::r_storage_file(r_storage_file&& other) noexcept :
    _file(std::move(other._file)),
    _h(std::move(other._h)),
    _dumbdex_map(std::move(other._dumbdex_map)),
    _block_index(std::move(other._block_index)),
    _gop_buffer(std::move(other._gop_buffer)),
    _ind_map(std::move(other._ind_map)),
    _current_block(std::move(other._current_block))
{
}

r_storage_file::~r_storage_file() noexcept
{
}

r_storage_write_context r_storage_file::create_write_context(
    const string& video_codec_name,
    const r_nullable<string>& video_codec_parameters,
    const r_nullable<string>& audio_codec_name,
    const r_nullable<string>& audio_codec_parameters
)
{
    r_storage_write_context ctx;
    ctx.video_codec_name = video_codec_name;
    ctx.video_codec_parameters = (!video_codec_parameters.is_null())?video_codec_parameters.value():"";
    ctx.audio_codec_name = (!audio_codec_name.is_null())?audio_codec_name.value():"";
    ctx.audio_codec_parameters = (!audio_codec_parameters.is_null())?audio_codec_parameters.value():"";
    return ctx;
}

r_storage_file& r_storage_file::operator=(r_storage_file&& other) noexcept
{
    if(this != &other)
    {
        _file = std::move(other._file);
        _h = std::move(other._h);
        _dumbdex_map = std::move(other._dumbdex_map);
        _block_index = std::move(other._block_index);
        _gop_buffer = std::move(other._gop_buffer);
        _ind_map = std::move(other._ind_map);
        _current_block = std::move(other._current_block);
    }

    return *this;
}

void r_storage_file::write_frame(const r_storage_write_context& ctx, r_storage_media_type media_type, const uint8_t* p, size_t size, bool key, int64_t ts, int64_t pts)
{
    if(media_type >= R_STORAGE_MEDIA_TYPE_ALL)
        R_THROW(("Invalid storage media type."));

    if(key)
    {
        for(auto& gop : _gop_buffer)
        {
            // mark any existing incomplete gop's of the same type as complete
            if(gop.media_type == media_type && gop.complete == false)
                gop.complete = true;
        }

        _gop g;
        g.complete = false;
        g.ts = ts;
        g.data.resize(size + r_rel_block::PER_FRAME_OVERHEAD);
        g.media_type = media_type;
        r_rel_block::append(g.data.data(), p, size, pts, 1);
        _gop_buffer.insert(lower_bound(_gop_buffer.begin(), _gop_buffer.end(), g, [](const _gop& a, const _gop& b) { return a.ts < b.ts; }), g);
    }
    else
    {
        // find our incomplete gop of this media_type and append this frame to it.

        auto found = find_if(
            begin(_gop_buffer), 
            end(_gop_buffer), 
            [&](const _gop& g) {return g.media_type == media_type && g.complete == false;}
        );
        if(found == end(_gop_buffer))
            R_THROW(("No incomplete GOP found for media type."));

        auto current_size = found->data.size();
        auto new_size = current_size + size + r_rel_block::PER_FRAME_OVERHEAD;

        if(new_size > _h.block_size)
            R_THROW(("GOP is larger than our storage block size!"));

        if(found->data.capacity() < new_size)
            found->data.reserve(new_size * 2);

        found->data.resize(new_size);

        r_rel_block::append(&found->data[current_size], p, size, pts, 0);
    }

    while(_buffer_full())
    {
        _gop g = _gop_buffer.front();
        _gop_buffer.pop_front();

        if(_current_block.is_null() || !_current_block.value().fits(g.data.size()))
            _current_block.assign(_get_index_block(ctx, _block_index.insert(g.ts), g.ts, _h.block_size / g.data.size()));

        _current_block.raw().append(g.data.data(), g.data.size(), g.media_type, 0, g.ts);
    }
}

vector<uint8_t> _s_to_buffer(const string& s)
{
    vector<uint8_t> buffer(s.length());
    ::memcpy(buffer.data(), s.c_str(), s.length());
    return buffer;
}

vector<uint8_t> r_storage_file::query(r_storage_media_type media_type, int64_t start_ts, int64_t end_ts)
{
    r_blob_tree bt;

    size_t fi = 0;

    _visit_ind_blocks(
        media_type,
        start_ts,
        end_ts,
        [&](r_ind_block_info& ibii) {
            bt["video_codec_name"] = ibii.video_codec_name;
            bt["video_codec_parameters"] = ibii.video_codec_parameters;
            bt["audio_codec_name"] = ibii.audio_codec_name;
            bt["audio_codec_parameters"] = ibii.audio_codec_parameters;

            r_rel_block rel_block(ibii.block, ibii.block_size);

            auto rbi = rel_block.begin();

            while(rbi.valid())
            {
                auto frame_info = *rbi;

                vector<uint8_t> frame_buffer(frame_info.size);
                ::memcpy(frame_buffer.data(), frame_info.data, frame_info.size);

                bt["frames"][fi]["ind_block_ts"] = r_string_utils::int64_to_s(ibii.ts);
                bt["frames"][fi]["data"] = frame_buffer;
                bt["frames"][fi]["pts"] = r_string_utils::int64_to_s(frame_info.pts);
                bt["frames"][fi]["key"] = (frame_info.flags>0)?string("true"):string("false");
                bt["frames"][fi]["stream_id"] = r_string_utils::uint8_to_s(ibii.stream_id);

                rbi.next();
                ++fi;
            }
        }
    );

    return r_blob_tree::serialize(bt, 1);
}

vector<int64_t> r_storage_file::key_frame_start_times(r_storage_media_type media_type, int64_t start_ts, int64_t end_ts)
{
    vector<int64_t> gop_start_times;

    _visit_ind_blocks(
        media_type,
        start_ts,
        end_ts,
        [&](r_ind_block_info& ibii) {
            gop_start_times.push_back(ibii.ts);
        }
    );

    return gop_start_times;
}

static bool _is_power_of_2(uint32_t size)
{
    if(size == 0)
        return false;
    
    return (std::ceil(std::log2(size)) == std::floor(std::log2(size)));
}

void r_storage_file::allocate(const std::string& file_name, size_t block_size, size_t num_blocks)
{
    {
#ifdef IS_WINDOWS
        FILE* fp = nullptr;
        fopen_s(&fp, file_name.c_str(), "w+");
#endif
        std::unique_ptr<FILE, decltype(&fclose)> f(
#ifdef IS_WINDOWS
            fp,
#endif
#ifdef IS_LINUX
            fopen(file_name.c_str(), "w+"),
#endif
            &fclose
        );

        if(!f)
            R_THROW(("r_storage_file: unable to create r_storage_file file."));

        if(!_is_power_of_2((uint32_t)block_size))
            R_THROW(("r_storage_file: block_size must be power of 2."));

        if(r_fs::fallocate(f.get(), num_blocks * block_size) < 0)
            R_THROW(("r_storage_file: unable to allocate file."));
    }

    {
#ifdef IS_WINDOWS
        FILE* fp = nullptr;
        fopen_s(&fp, file_name.c_str(), "r+");
#endif
        std::unique_ptr<FILE, decltype(&fclose)> f(
#ifdef IS_WINDOWS
            fp,
#endif
#ifdef IS_LINUX
            fopen(file_name.c_str(), "r+"),
#endif
            &fclose
        );

        if(!f)
            throw std::runtime_error("dumbdex: unable to open dumbdex file.");

        r_memory_map mm(
#ifdef IS_LINUX
            fileno(f.get()),
#endif
#ifdef IS_WINDOWS
            _fileno(f.get()),
#endif
            0,
            (uint32_t)block_size,
            r_memory_map::RMM_PROT_READ | r_memory_map::RMM_PROT_WRITE,
            r_memory_map::RMM_TYPE_FILE | r_memory_map::RMM_SHARED
        );

        uint8_t* p = (uint8_t*)mm.map();
        ::memset(p, 0, R_STORAGE_FILE_HEADER_SIZE);

        *(uint32_t*)p = (uint32_t)(num_blocks-1);
        *(uint32_t*)(p+4) = (uint32_t)block_size;

        r_dumbdex::allocate(
            p + R_STORAGE_FILE_HEADER_SIZE,
            block_size - R_STORAGE_FILE_HEADER_SIZE,
            num_blocks - 1
        );
    }
}

int64_t r_storage_file::required_file_size_for_retention_hours(int64_t retention_hours, int64_t byte_rate)
{
    // This may need to get more complicated in the future. The overhead should be linear however so we
    // we should be able to compute a constant overhead factor.
    return byte_rate * 60 * 60 * retention_hours;
}

r_storage_file::_header r_storage_file::_read_header(const std::string& file_name)
{
    auto tmp_f = r_file::open(file_name, "r+");
    _header h;
    h.num_blocks = 0;
    fread(&h.num_blocks, 1, sizeof(uint32_t), tmp_f);
    h.block_size = 0;
    fread(&h.block_size, 1, sizeof(uint32_t), tmp_f);

    return h;
}

template<typename CB>
void r_storage_file::_visit_ind_blocks(r_storage_media_type media_type, int64_t start_ts, int64_t end_ts, CB cb)
{
    if(media_type >= R_STORAGE_MEDIA_TYPE_MAX)
        R_THROW(("Invalid storage media type."));

    int64_t current_ts = start_ts;

    auto di = _block_index.find_lower_bound(current_ts);
    if(!di.valid())
        R_THROW(("Query start not found."));

    auto dumbdex_index_entry = *di;

    auto ind_block_map = _map_block(dumbdex_index_entry.second);
    r_ind_block ind_block((uint8_t*)ind_block_map.map(), _h.block_size);

    auto ibi = ind_block.find_lower_bound(current_ts);

    bool done = false;
    while(!done && current_ts < end_ts && di.valid())
    {
        if(ibi.valid())
        {
            auto ibii = *ibi;

            current_ts = ibii.ts;

            if(current_ts < end_ts)
            {
                if(ibii.stream_id == media_type || media_type == R_STORAGE_MEDIA_TYPE_ALL)
                    cb(ibii);
            }

            ibi.next();
        }
        else
        {
            di.next();
            if(!di.valid())
                done = true;
            else
            {
                dumbdex_index_entry = *di;
                ind_block_map = _map_block(dumbdex_index_entry.second);
                ind_block = r_ind_block((uint8_t*)ind_block_map.map(), _h.block_size);
                ibi = ind_block.begin();
            }
        }
    }
}

r_memory_map r_storage_file::_map_block(uint16_t block)
{
    return r_memory_map(
#ifdef IS_WINDOWS
        _fileno(_file),
#endif
#ifdef IS_LINUX
        fileno(_file),
#endif
        block * _h.block_size,
        _h.block_size,
        r_memory_map::RMM_PROT_READ | r_memory_map::RMM_PROT_WRITE,
        r_memory_map::RMM_TYPE_FILE | r_memory_map::RMM_SHARED
    );
}

r_ind_block r_storage_file::_get_index_block(const r_storage_write_context& ctx, uint16_t block, int64_t ts, size_t n_indexes)
{
    _ind_map = _map_block(block);

    auto p = (uint8_t*)_ind_map.map();

    r_ind_block::initialize_block(
        p,
        _h.block_size,
        (uint32_t)n_indexes,
        ts,
        ctx.video_codec_name,
        ctx.video_codec_parameters,
        ctx.audio_codec_name,
        ctx.audio_codec_parameters
    );

    return r_ind_block(p, _h.block_size);
}

bool r_storage_file::_buffer_full() const
{
    auto now = chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count();
    auto next = _gop_buffer.front().ts;

    return (now > next)?(now-next)>4000:false;
}