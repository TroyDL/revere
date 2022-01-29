
#ifndef r_storage_r_storage_file_h__
#define r_storage_r_storage_file_h__

#include "r_storage/r_dumbdex.h"
#include "r_storage/r_ind_block.h"
#include "r_storage/r_rel_block.h"
#include "r_utils/r_memory_map.h"
#include "r_utils/r_file.h"
#include "r_utils/r_nullable.h"
#include <string>
#include <vector>
#include <deque>
#include <limits>
#include <climits>

namespace r_storage
{

enum r_storage_media_type
{
    R_STORAGE_MEDIA_TYPE_VIDEO,
    R_STORAGE_MEDIA_TYPE_AUDIO,
    R_STORAGE_MEDIA_TYPE_ALL,

    R_STORAGE_MEDIA_TYPE_MAX
};

struct r_storage_write_context
{
    std::string video_codec_name;
    std::string video_codec_parameters;
    std::string audio_codec_name;
    std::string audio_codec_parameters;
};

class r_storage_file final
{
public:
    r_storage_file();
    r_storage_file(const std::string& file_name);

    r_storage_file(const r_storage_file&) = delete;
    r_storage_file(r_storage_file&& other) noexcept;

    ~r_storage_file() noexcept;

    r_storage_file& operator=(const r_storage_file&) = delete;
    r_storage_file& operator=(r_storage_file&& other) noexcept;

    r_storage_write_context create_write_context(
        const std::string& video_codec_name,
        const r_utils::r_nullable<std::string>& video_codec_parameters,
        const r_utils::r_nullable<std::string>& audio_codec_name,
        const r_utils::r_nullable<std::string>& audio_codec_parameters
    );

    void write_frame(const r_storage_write_context& ctx, r_storage_media_type media_type, const uint8_t* p, size_t size, bool key, int64_t ts, int64_t pts);

    void finalize(const r_storage_write_context& ctx);

    // query() returns an r_blob_tree populated with the query results (see unit tests).
    std::vector<uint8_t> query(r_storage_media_type media_type, int64_t start_ts, int64_t end_ts);

    // key_frame_start_times() returns an array of the independent block ts's
    std::vector<int64_t> key_frame_start_times(r_storage_media_type media_type, int64_t start_ts = 0, int64_t end_ts = LLONG_MAX);

    static void allocate(const std::string& file_name, size_t block_size, size_t num_blocks);

    static std::pair<int64_t, int64_t> required_file_size_for_retention_hours(int64_t retention_hours, int64_t byte_rate);

private:
    struct _header
    {
        uint32_t num_blocks;
        uint32_t block_size;
    };

    struct _gop
    {
        bool complete {false};
        int64_t ts {0};
        std::vector<uint8_t> data;
        r_storage_media_type media_type;
    };

    enum r_storage_file_enum
    {
        R_STORAGE_FILE_HEADER_SIZE = 128
    };

    static _header _read_header(const std::string& file_name);

    template<typename CB>
    void _visit_ind_blocks(r_storage_media_type media_type, int64_t start_ts, int64_t end_ts, CB cb);

    r_utils::r_memory_map _map_block(uint16_t block);
    r_ind_block _get_index_block(const r_storage_write_context& ctx, uint16_t block, int64_t ts, size_t n_indexes);

    bool _buffer_full() const;

    r_utils::r_file _file;
    _header _h;
    r_utils::r_memory_map _dumbdex_map;
    r_dumbdex _block_index;
    std::deque<_gop> _gop_buffer;
    
    r_utils::r_memory_map _ind_map;
    r_utils::r_nullable<r_ind_block> _current_block;
};

}

#endif
