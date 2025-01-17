#ifndef r_pipeline_r_gst_buffer_h
#define r_pipeline_r_gst_buffer_h

#include "r_utils/r_exception.h"
#include "r_utils/r_nullable.h"

#ifdef IS_WINDOWS
#pragma warning( push )
#pragma warning( disable : 4244 )
#endif
#include <gst/gst.h>
#include <gst/rtsp/gstrtspmessage.h>
#include <gst/sdp/gstsdpmessage.h>
#include <gst/app/gstappsink.h>
#ifdef IS_WINDOWS
#pragma warning( pop )
#endif

namespace r_pipeline
{

class r_gst_buffer final
{
public:
    enum r_map_type
    {
        MT_READ,
        MT_WRITE
    };

    class r_map_info final
    {
        friend class r_gst_buffer;
    public:
        r_map_info() = delete;
        r_map_info(const r_gst_buffer* buffer, r_map_type type) :
            _buffer(buffer),
            _type(type),
            _info()
        {
            GstMapInfo info;
            if(!gst_buffer_map(_buffer->_buffer, &info, (type==MT_READ)?GST_MAP_READ:GST_MAP_WRITE))
                R_THROW(("Unable to map buffer!"));
            _info.set_value(info);
        }
        r_map_info(const r_map_info&) = delete;
        r_map_info(r_map_info&& obj) noexcept :
            _buffer(std::move(obj._buffer)),
            _type(std::move(obj._type)),
            _info(std::move(obj._info))
        {
        }
        ~r_map_info() noexcept
        {
            _clear();
        }

        r_map_info& operator=(const r_map_info&) = delete;
        r_map_info& operator=(r_map_info&& obj) noexcept
        {
            if(this != &obj)
            {
                _clear();

                _buffer = std::move(obj._buffer);
                _type = std::move(obj._type);
                _info = std::move(obj._info);
            }

            return *this;
        }

        uint8_t* data() const noexcept {return _info.value().data;}
        size_t size() const noexcept {return _info.value().size;}

    private:
        void _clear() noexcept
        {
            if(!_info.is_null())
            {
                auto info = _info.value();
                gst_buffer_unmap(_buffer->_buffer, &info);
                _info.clear();
            }
        }

        const r_gst_buffer* _buffer;
        r_map_type _type;
        r_utils::r_nullable<GstMapInfo> _info;
    };

    r_gst_buffer();
    r_gst_buffer(GstBuffer* buffer);
    r_gst_buffer(const r_gst_buffer& obj);
    r_gst_buffer(r_gst_buffer&& obj);
    ~r_gst_buffer() noexcept;

    r_gst_buffer& operator=(const r_gst_buffer& obj);
    r_gst_buffer& operator=(r_gst_buffer&& obj);

    r_map_info map(r_map_type type) const;

    GstBuffer* get() const noexcept {return _buffer;}

private:
    void _clear() noexcept;

    GstBuffer* _buffer;
};

}


#endif

