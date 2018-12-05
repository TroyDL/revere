
#ifndef __r_rtsp_r_fake_camera_h
#define __r_rtsp_r_fake_camera_h

#include "r_rtsp/r_rtsp_server.h"
#include "r_rtsp/r_session_base.h"
#include "r_utils/r_string_utils.h"
#include "r_utils/r_file.h"
#include "r_av/r_demuxer.h"
#include "r_av/r_muxer.h"
#include "r_av/r_options.h"
#include <memory>
#include <future>
#include <utility>

namespace r_rtsp
{

class r_fake_camera;

class r_fake_camera_session : public r_session_base
{
private:
    void _entry_point()
    {
        _running = true;

        _dm = std::make_shared<r_av::r_demuxer>(_mediaPath);

        auto streamTypes = _dm->get_stream_types();

        if(streamTypes.size() == 1)
        {
            auto inputVideoStreamIndex = _dm->get_video_stream_index();

            printf("FIX HARD CODED 127.0.0.1!\n");
            fflush(stdout);
            _vm = std::make_shared<r_av::r_muxer>(r_av::r_muxer::OUTPUT_LOCATION_RTP,
                                                  r_utils::r_string_utils::format("rtp://%s:%s", "127.0.0.1", _videoRtpPort.c_str()));

            r_av::r_stream_options vsoptions;
            vsoptions.type = "video";
            vsoptions.codec_id = _dm->get_stream_codec_id(inputVideoStreamIndex);
            vsoptions.format = _dm->get_pix_format(inputVideoStreamIndex);
            auto resolution = _dm->get_resolution(inputVideoStreamIndex);
            vsoptions.width = resolution.first;
            vsoptions.height = resolution.second;
            vsoptions.time_base_num = 1;
            vsoptions.time_base_den = 90000;
            auto inputFrameRate = _dm->get_frame_rate(inputVideoStreamIndex);
            auto outputVideoStreamIndex = _vm->add_stream(vsoptions);

            auto sleepMicros = (int64_t)(1000000.f / ((double)inputFrameRate.first / inputFrameRate.second));

            std::chrono::steady_clock::time_point lastTP;
            bool lastTPValid = false;

            while(_running)
            {
                int inputStreamIndex = -1;
                bool more = _dm->read_frame(inputStreamIndex);
                if(!more)
                {
                    _dm = std::make_shared<r_av::r_demuxer>(_mediaPath);
                    continue;
                }

                auto p = _dm->get();

                _vm->write_packet(p, outputVideoStreamIndex, p.is_key());

                auto now = std::chrono::steady_clock::now();

                if(lastTPValid)
                {
                    int64_t overhead = std::chrono::duration_cast<std::chrono::microseconds>(now-lastTP).count();
                    auto sleepTime = (sleepMicros > overhead)?sleepMicros-overhead:0;
                    usleep(sleepTime);
                }

                lastTP = now;
                lastTPValid = true;
            }

        }
        else
        {
            auto inputVideoStreamIndex = _dm->get_video_stream_index();
            auto inputAudioStreamIndex = _dm->get_primary_audio_stream_index();

            printf("FIX HARD CODED 127.0.0.1!\n");
            fflush(stdout);


            _vm = std::make_shared<r_av::r_muxer>(r_av::r_muxer::OUTPUT_LOCATION_RTP,
                                                  r_utils::r_string_utils::format("rtp://%s:%s", "127.0.0.1", _videoRtpPort.c_str()));

            _am = std::make_shared<r_av::r_muxer>(r_av::r_muxer::OUTPUT_LOCATION_RTP,
                                                  r_utils::r_string_utils::format("rtp://%s:%s", "127.0.0.1", _audioRtpPort.c_str()));

            r_av::r_stream_options vsoptions;
            vsoptions.type = "video";
            vsoptions.codec_id = _dm->get_stream_codec_id(inputVideoStreamIndex);
            vsoptions.format = _dm->get_pix_format(inputVideoStreamIndex);
            auto resolution = _dm->get_resolution(inputVideoStreamIndex);
            vsoptions.width = resolution.first;
            vsoptions.height = resolution.second;
            auto inputTimeBase = _dm->get_time_base(inputVideoStreamIndex);
            vsoptions.time_base_num = inputTimeBase.first;
            vsoptions.time_base_den = inputTimeBase.second;
            auto inputFrameRate = _dm->get_frame_rate(inputVideoStreamIndex);
            vsoptions.frame_rate_num = inputFrameRate.first;
            vsoptions.frame_rate_den = inputFrameRate.second;
            auto outputVideoStreamIndex = _vm->add_stream(vsoptions);

            printf("FIX HARD CODED bits_per_raw_sample, channels and sample_rate.");
            fflush(stdout);
            r_av::r_stream_options asoptions;
            asoptions.type = "audio";
            asoptions.codec_id = _dm->get_stream_codec_id(inputAudioStreamIndex);
            asoptions.bits_per_raw_sample = 32;
            asoptions.channels = 2;
            asoptions.sample_rate = 44100;
            auto outputAudioStreamIndex = _am->add_stream(asoptions);

            auto sleepMicros = (int64_t)(1000000.f / ((double)inputFrameRate.first / inputFrameRate.second));
            printf("sleepMicros = %s\n",r_utils::r_string_utils::int64_to_s(sleepMicros).c_str());
            //auto sleepMicros = ((double)inputTimeBase.first / (double)inputTimeBase.second) * 1000000;

            auto runStartTime = std::chrono::steady_clock::now();
            int64_t elapsedStreamTicks = 0;

            while(_running)
            {
                int inputStreamIndex = -1;
                bool more = _dm->read_frame(inputStreamIndex);
                if(!more)
                {
                    _dm = std::make_shared<r_av::r_demuxer>(_mediaPath);
                    continue;
                }
                auto p = _dm->get();

                if(inputStreamIndex == inputVideoStreamIndex)
                    _vm->write_packet(p, outputVideoStreamIndex, p.is_key());
                else _am->write_packet(p, outputAudioStreamIndex, true);

                auto elapsedTime = std::chrono::steady_clock::now() - runStartTime;
                auto elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count();

                static bool lastTSValid = false;
                static uint32_t lastTS = 0;

                if(lastTSValid)
                {
                    elapsedStreamTicks += p.get_pts() - lastTS;

                    auto timeBase = p.get_time_base();

                    auto elapsedStreamMillis = (int64_t)(((double)elapsedStreamTicks)/(double)timeBase.second*1000);

                    if(elapsedStreamMillis > elapsedMillis)
                        usleep((elapsedStreamMillis - elapsedMillis)*1000);
                }

                lastTS = p.get_pts();
                lastTSValid = true;
            }
        }
    }

public:
    r_fake_camera_session(r_rtsp_server& server, const std::string& serverRoot) :
        r_session_base(server),
        _serverRoot(serverRoot),
        _dm(),
        _vm(),
        _am(),
        _worker(),
        _running(false),
        _videoRtpPort(),
        _videoRtcpPort(),
        _mediaPath(),
        _interleaved(false)
    {
    }

    virtual ~r_fake_camera_session() noexcept
    {
        if(_running)
        {
            _running = false;
            _worker.join();
        }
    }

    std::shared_ptr<r_session_base> clone() const
    {
        return std::make_shared<r_fake_camera_session>(_server, _serverRoot);
    }

    virtual bool handles_this_presentation(const std::string& presentation)
    {
        auto urlParts = r_utils::r_string_utils::split(presentation, "/");

        if(urlParts[0] != "video")
            return false;

        auto mp = _serverRoot + r_utils::r_fs::PATH_SLASH + urlParts[1];

        if(!r_utils::r_fs::file_exists(mp))
            return false;

        return true;
    }

    virtual std::shared_ptr<r_server_response> handle_request( std::shared_ptr<r_server_request> request )
    {
        auto response = std::make_shared<r_server_response>();

        auto urlParts = r_utils::r_string_utils::split(request->get_uri(), "/");

        if(urlParts[0] != "video")
            R_THROW(("Invalid URL."));

        auto mp = _serverRoot + r_utils::r_fs::PATH_SLASH + urlParts[1];

        if(!r_utils::r_fs::file_exists(mp))
            R_THROW(("File does not exist."));

        auto method = request->get_method();
        if(method == M_DESCRIBE)
        {
            auto dm = std::make_shared<r_av::r_demuxer>(mp);
            response->set_body(dm->get_sdp());
        }
        else if(method == M_SETUP)
        {
            //Transport: RTP/AVP;unicast;client_port=9166-9167
            std::string transport;
            if(!request->get_header("Transport", transport))
                R_THROW(("Unable to find Transport header."));

            if(r_utils::r_string_utils::contains(urlParts[2], "streamid=0"))
            {
                auto outerParts = r_utils::r_string_utils::split(transport, ";");
                for(auto p : outerParts)
                {
                    if(r_utils::r_string_utils::contains(p, "client_port"))
                    {
                        auto innerParts = r_utils::r_string_utils::split(p, "=");
                    
                        auto portParts = r_utils::r_string_utils::split(innerParts[1], "-");

                        _videoRtpPort = portParts[0];
                        _videoRtcpPort = portParts[1];
                    }
                }
            }
            else
            {
                auto outerParts = r_utils::r_string_utils::split(transport, ";");
                for(auto p : outerParts)
                {
                    if(r_utils::r_string_utils::contains(p, "client_port"))
                    {
                        auto innerParts = r_utils::r_string_utils::split(p, "=");
                    
                        auto portParts = r_utils::r_string_utils::split(innerParts[1], "-");

                        _audioRtpPort = portParts[0];
                        _audioRtcpPort = portParts[1];
                    }
                }
            }
            _mediaPath = mp;

            if(transport.find("interleaved") != std::string::npos)
                _interleaved = true;

            response->set_header("Transport", transport);
        }
        else if(method == M_PLAY)
        {
            if(!_running)
            {
                _worker = std::thread(&r_fake_camera_session::_entry_point, this);
            }
        }
        else if(method == M_TEARDOWN)
        {
            if(_running)
            {
                _running = false;
                _worker.join();
            }
        }

        return response;
    }

private:
    std::string _serverRoot;
    std::shared_ptr<r_av::r_demuxer> _dm;
    std::shared_ptr<r_av::r_muxer> _vm;
    std::shared_ptr<r_av::r_muxer> _am;
    std::thread _worker;
    bool _running;
    std::string _videoRtpPort;
    std::string _videoRtcpPort;
    std::string _audioRtpPort;
    std::string _audioRtcpPort;
    std::string _mediaPath;
    bool _interleaved;
};

class r_fake_camera final
{
public:
    r_fake_camera(const std::string& serverRoot, int port, const std::string& inaddr = r_utils::ip4_addr_any) :
        _serverRoot(serverRoot),
        _port(port),
        _inaddr(inaddr),
        _rtspServer(std::make_shared<r_rtsp_server>(inaddr, port)),
        _running(false)
    {
        auto proto = std::make_shared<r_fake_camera_session>(*_rtspServer, _serverRoot);

        _rtspServer->attach_session_prototype(proto);
    }

    void start() { if(!_running) { _running = true; _rtspServer->start(); } }
    void stop() { if(_running) { _running = false; _rtspServer->stop(); } }

    ~r_fake_camera() noexcept
    {
        if(_running)
            stop();
    }

    std::string get_server_root() const { return _serverRoot; }

private:
    std::string _serverRoot;
    int _port;
    std::string _inaddr;
    std::shared_ptr<r_rtsp_server> _rtspServer;
    bool _running;
};


}

#endif
