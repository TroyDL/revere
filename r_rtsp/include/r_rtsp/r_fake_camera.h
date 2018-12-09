
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
            auto outputVideoStreamIndex = _vm->add_stream(vsoptions);

            auto streamStartTime = std::chrono::steady_clock::now();
            int64_t videoStreamTime = 0;
            double videoStreamTimeSeconds = 0.0;

            std::chrono::steady_clock::time_point lastTP;
            bool lastTPValid = false;

            while(_running && _playVideo)
            {
                int inputStreamIndex = -1;
                bool more = _dm->read_frame(inputStreamIndex);
                if(!more)
                {
                    _dm = std::make_shared<r_av::r_demuxer>(_mediaPath);

                    _vm = std::make_shared<r_av::r_muxer>(r_av::r_muxer::OUTPUT_LOCATION_RTP,
                                                        r_utils::r_string_utils::format("rtp://%s:%s", "127.0.0.1", _videoRtpPort.c_str()));
                    outputVideoStreamIndex = _vm->add_stream(vsoptions);
                    continue;
                }

                auto p = _dm->get();

                videoStreamTime += p.get_duration();
                videoStreamTimeSeconds = (((double)videoStreamTime) / (double)p.get_time_base().second);
                _vm->write_packet(p, outputVideoStreamIndex, p.is_key());

                // A frame of video has a duration. "stream time" is the sum of the durations of all the frames we've emitted since we started streaming.
                // We also have the elaped wall clock time since we started streaming. Ideally, these should be the same thing... but the reality is that sometimes
                // we emit a frame and our "stream time" is ahead of our elapsed wall clock time. We can solve this by sleeping for the delta (correction in below code)
                // between these two times.
                // But if thats all we did, the video and audio would be choppy because this loop takes time to run... so we also time our loop and subtract that from
                // our sleep amount...

                auto now = std::chrono::steady_clock::now();

                auto secondsSincePlay = (double)std::chrono::duration_cast<std::chrono::milliseconds>(now - streamStartTime).count() / 1000;

                if((videoStreamTimeSeconds > secondsSincePlay))
                {
                    auto correction = videoStreamTimeSeconds - secondsSincePlay;

                    uint32_t sleepMicros = (uint32_t)(correction * 1000 * 1000);

                    if(lastTPValid)
                    {
                        int64_t overhead = std::chrono::duration_cast<std::chrono::microseconds>(now-lastTP).count();

                        if(overhead < sleepMicros)
                            sleepMicros -= overhead;
                        else sleepMicros = 0;
                    }

                    if(sleepMicros > 0)
                        usleep(sleepMicros);
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


            if(_playVideo)
                _vm = std::make_shared<r_av::r_muxer>(r_av::r_muxer::OUTPUT_LOCATION_RTP,
                                                    r_utils::r_string_utils::format("rtp://%s:%s", "127.0.0.1", _videoRtpPort.c_str()));

            if(_playAudio)
                _am = std::make_shared<r_av::r_muxer>(r_av::r_muxer::OUTPUT_LOCATION_RTP,
                                                    r_utils::r_string_utils::format("rtp://%s:%s", "127.0.0.1", _audioRtpPort.c_str()));

            r_av::r_stream_options vsoptions;
            vsoptions.type = "video";
            vsoptions.codec_id = _dm->get_stream_codec_id(inputVideoStreamIndex);
            vsoptions.format = _dm->get_pix_format(inputVideoStreamIndex);
            auto resolution = _dm->get_resolution(inputVideoStreamIndex);
            vsoptions.width = resolution.first;
            vsoptions.height = resolution.second;
            vsoptions.time_base_num = 1;
            vsoptions.time_base_den = 90000;
            int outputVideoStreamIndex = -1;
            if(_playVideo)
                outputVideoStreamIndex = _vm->add_stream(vsoptions);

            printf("FIX HARD CODED bits_per_raw_sample, channels and sample_rate.");
            fflush(stdout);
            r_av::r_stream_options asoptions;
            asoptions.type = "audio";
            asoptions.codec_id = _dm->get_stream_codec_id(inputAudioStreamIndex);
            asoptions.bits_per_raw_sample = 32;
            asoptions.channels = 2;
            asoptions.sample_rate = 44100;
            asoptions.time_base_num = 1;
            asoptions.time_base_den = 90000;
            int outputAudioStreamIndex = -1;
            if(_playAudio)
                outputAudioStreamIndex = _am->add_stream(asoptions);

            auto streamStartTime = std::chrono::steady_clock::now();
            int64_t videoStreamTime = 0;
            int64_t audioStreamTime = 0;
            double videoStreamTimeSeconds = 0.0;
            double audioStreamTimeSeconds = 0.0;

            std::chrono::steady_clock::time_point lastTP;
            bool lastTPValid = false;

            while(_running)
            {
                int inputStreamIndex = -1;
                bool more = _dm->read_frame(inputStreamIndex);
                if(!more)
                {
                    _dm = std::make_shared<r_av::r_demuxer>(_mediaPath);

                    if(_playVideo)
                    {
                        _vm = std::make_shared<r_av::r_muxer>(r_av::r_muxer::OUTPUT_LOCATION_RTP,
                                                            r_utils::r_string_utils::format("rtp://%s:%s", "127.0.0.1", _videoRtpPort.c_str()));
                        outputVideoStreamIndex = _vm->add_stream(vsoptions);
                    }

                    if(_playAudio)
                    {
                        _am = std::make_shared<r_av::r_muxer>(r_av::r_muxer::OUTPUT_LOCATION_RTP,
                                                            r_utils::r_string_utils::format("rtp://%s:%s", "127.0.0.1", _audioRtpPort.c_str()));
                        outputAudioStreamIndex = _am->add_stream(asoptions);
                    }

                    continue;
                }
                auto p = _dm->get();

                if(inputStreamIndex == inputVideoStreamIndex)
                {
                    videoStreamTime += p.get_duration();
                    videoStreamTimeSeconds = (((double)videoStreamTime) / (double)p.get_time_base().second);
                    if(_playVideo)
                        _vm->write_packet(p, outputVideoStreamIndex, p.is_key());
                }
                else
                {
                    audioStreamTime += p.get_duration();
                    audioStreamTimeSeconds = (((double)audioStreamTime) / (double)p.get_time_base().second);
                    if(_playAudio)
                        _am->write_packet(p, outputAudioStreamIndex, true);
                }

                // A frame of video has a duration. "stream time" is the sum of the durations of all the frames we've emitted since we started streaming.
                // We also have the elaped wall clock time since we started streaming. Ideally, these should be the same thing... but the reality is that sometimes
                // we emit a frame and our "stream time" is ahead of our elapsed wall clock time. We can solve this by sleeping for the delta (correction in below code)
                // between these two times.
                // But if thats all we did, the video and audio would be choppy because this loop takes time to run... so we also time our loop and subtract that from
                // our sleep amount... Finally, since there are two streams we need to make sure we're sleeping only if both of their stream times are ahead of the
                // elapsed wall clock time since we started streaming.

                auto now = std::chrono::steady_clock::now();

                auto secondsSincePlay = (double)std::chrono::duration_cast<std::chrono::milliseconds>(now - streamStartTime).count() / 1000;

                if((videoStreamTimeSeconds > secondsSincePlay) && (audioStreamTimeSeconds > secondsSincePlay))
                {
                    auto correction = (videoStreamTimeSeconds > audioStreamTimeSeconds)?videoStreamTimeSeconds - secondsSincePlay:audioStreamTimeSeconds - secondsSincePlay;

                    uint32_t sleepMicros = (uint32_t)(correction * 1000 * 1000);

                    if(lastTPValid)
                    {
                        int64_t overhead = std::chrono::duration_cast<std::chrono::microseconds>(now-lastTP).count();

                        if(overhead < sleepMicros)
                            sleepMicros -= overhead;
                        else sleepMicros = 0;
                    }

                    if(sleepMicros > 0)
                        usleep(sleepMicros);
                }

                lastTP = now;
                lastTPValid = true;
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
        _playVideo(false),
        _audioRtpPort(),
        _audioRtcpPort(),
        _playAudio(false),
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
                        _playVideo = true;
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
                        _playAudio = true;
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
    bool _playVideo;
    std::string _audioRtpPort;
    std::string _audioRtcpPort;
    bool _playAudio;
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
