
#include "motion_context.h"
#include "r_utils/r_time_utils.h"
#include "r_vss_client/r_query.h"
#include "r_vss_client/r_media_parser.h"

using namespace r_utils;
using namespace r_av;
using namespace std;
using namespace std::chrono;

void motion_context::process(r_video_decoder& dec)
{
    auto now = system_clock::now();
    auto qs = (_lastValid)?_lastFrameTS:now - seconds(120);
    auto qe = qs + seconds(10);

    if(qs < qe)
    {

        try
        {
            auto buffer = r_vss_client::query(_dataSourceID,
                                              "video",
                                              false,
                                              qs,
                                              qe,
                                              true);

            r_vss_client::r_media_parser parser(buffer);

            system_clock::time_point lastTP;
            bool lastTPValid = false;
            size_t numFrames = 0;
            parser.begin();
            while(parser.valid())
            {
                system_clock::time_point tp;
                size_t frameSize;
                const uint8_t* p = NULL;
                uint32_t flags;
                parser.current_data(tp, frameSize, p, flags);

                lastTP = tp;
                lastTPValid = true;

                try
                {
                    r_packet pkt(const_cast<uint8_t*>(p), frameSize, false);

                    auto decState = dec.decode(pkt);
                    if(decState == r_video_decoder::r_decoder_state_accepted)
                    {
                        dec.set_output_width(dec.get_input_width());
                        dec.set_output_height(dec.get_input_height());

                        auto decoded = dec.get();
                    }
                }
                catch(exception& ex)
                {
                    R_LOG_NOTICE("%s", ex.what());
                }
                numFrames++;

                parser.next();
            }


            printf("[%s   ->   %s nframes=%u]\n", r_time_utils::tp_to_iso_8601(qs, false).c_str(), r_time_utils::tp_to_iso_8601(qe, false).c_str(), numFrames);
            fflush(stdout);

            if(lastTPValid)
            {
                _lastFrameTS = lastTP + milliseconds(1);
                _lastValid = true;
            }
            else _lastFrameTS = qe;
        }
        catch(exception& ex)
        {
            _lastFrameTS = qe;
            printf("EXCEPTION\n");
            R_LOG_NOTICE("Unable to process analytics: %s", ex.what());
        }
    }
}
