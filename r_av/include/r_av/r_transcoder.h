#ifndef __r_av_r_transcoder_h
#define __r_av_r_transcoder_h

#include "r_av/r_demuxer.h"
#include "r_av/r_muxer.h"
#include "r_av/r_video_decoder.h"
#include "r_av/r_video_encoder.h"

namespace r_av
{

class r_transcoder
{
public:
    r_transcoder(int inputTimeBaseNum,
                 int inputTimeBaseDen,
                 int outputTimeBaseNum,
                 int outputTimeBaseDen,
                 double speed = 1.0,
                 bool decodeSkipping = false) :
        _step(0.0),
        _speed(speed),
        _decodeSkipping(decodeSkipping)
    {
        // When the input and output framerates were specified with single integers, this was how we did it...
        // _outputFramesPerInputFrame( (((double)outputFrameRate) / (((double)inputFrameRate) * speed)) ),

        AVRational inputRational = { inputTimeBaseNum, inputTimeBaseDen };
        AVRational outputRational = { outputTimeBaseNum, outputTimeBaseDen };

        _outputFramesPerInputFrame = av_q2d( inputRational ) / (av_q2d( outputRational ) * _speed);
    }

    // The job of this method is to decode frames until it is time to make an output frame. If the output
    // framerate is greater than the input framerate this means that sometimes calls to this method will not
    // actually decode a frame (because we need to encode multiple outputs from the same input in that case).
    // Likewise, when the output framerate is less than the input framerate, we will sometimes decode multiple
    // times before we return. In either case, after this method returns, we are ready to make an output frame.
    template<class T>
    bool decode(T& avDeMuxer, int streamIndex, r_video_decoder& decoder)
    {
        if(_step < 1.0)
        {
            while(!avDeMuxer.end_of_file() && (_step < 1.0))
            {
                int si = 0;
                if(!avDeMuxer.read_frame(si))
                    return false;

                if(si == streamIndex)
                {
                    _step += _outputFramesPerInputFrame;

                    if(_step > 1.0 || !_decodeSkipping)
                        decoder.decode(avDeMuxer.get());
                }
            }
        }

        _step -= 1.0;

        return true;
    }

    template<class T>
    void encode_and_mux(T& encoder,
                        r_muxer& muxer,
                        int streamIndex,
                        const r_packet& pic,
                        r_video_encoder::r_encoder_packet_type type = r_video_encoder::r_encoder_packet_type_auto)
    {
        encoder.encode_image(pic, type);

        auto encodeBuffer = encoder.get();

        muxer.write_packet(encodeBuffer, streamIndex, encodeBuffer.is_key());
    }

    static int64_t compute_num_output_frames(int64_t numInputFrames,
                                             int inputTimeBaseNum, int inputTimeBaseDen,
                                             int outputTimeBaseNum, int outputTimeBaseDen,
                                             double speed = 1.0)
    {
        AVRational inputRational = { inputTimeBaseNum, inputTimeBaseDen };
        AVRational outputRational = { outputTimeBaseNum, outputTimeBaseDen };

        double outputFramesPerInputFrame = av_q2d( inputRational ) / (av_q2d( outputRational ) * speed);

        return (int64_t)(outputFramesPerInputFrame * (double)numInputFrames);
    }

private:
    double _outputFramesPerInputFrame;
    double _step;
    double _speed;
    bool _decodeSkipping;
};

}

#endif
