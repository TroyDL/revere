#include "r_utils/r_logger.h"
#include "r_utils/r_time_utils.h"
#include "r_utils/r_functional.h"
#include "r_av/r_locky.h"
#include "r_av/r_video_decoder.h"
#include "r_av/r_options.h"
#include "r_vss_client/r_ds.h"
#include <range/v3/all.hpp>
#include <signal.h>
#include <unistd.h>

#include "motion_context.h"

using namespace r_utils;
using namespace r_vss_client;
using namespace r_av;
using namespace std;

bool running = true;

void handle_sigterm(int sig)
{
    if(sig == SIGTERM)
        running = false;
}

vector<data_source> fetch_and_parse_data_sources()
{
    vector<data_source> dataSources;

    try
    {
        dataSources = parse_data_sources(fetch_data_sources_json());
    }
    catch(exception& ex)
    {
        printf("%s\n",ex.what());
    }

    return dataSources;
}

int main(int argc, char* argv[])
{
    r_logger::install_terminate();

    signal(SIGTERM, handle_sigterm);

    r_locky::register_ffmpeg();

    string firstArgument = (argc > 1) ? argv[1] : string();

    if( firstArgument != "--interactive" )
        daemon( 1, 0 );

    vector<motion_context> processing;

    r_video_decoder dec(r_av::r_av_codec_id_h264, r_av::r_av_pix_fmt_yuv420p, r_av::get_decoder_options(2));

    while(running)
    {
        vector<data_source> dataSources = fetch_and_parse_data_sources();
        vector<data_source> recordingDataSources = dataSources | ranges::view::filter([](const data_source& ds){return ds.recording;});
        vector<string> recordingDataSourceIDS = recordingDataSources | ranges::view::transform([](const data_source& ds){return ds.id;});
        vector<string> processingIDS = processing | ranges::view::transform([](const motion_context& mc){return mc.data_source_id();});

        auto startingIDS = r_funky::set_diff(recordingDataSourceIDS, processingIDS);
        auto stoppingIDS = r_funky::set_diff(processingIDS, recordingDataSourceIDS);

        for(auto id : startingIDS)
        {
            printf("starting: %s\n", id.c_str());
            fflush(stdout);
            processing.push_back(motion_context(id));
        }

        for(auto& mc : processing)
            mc.process(dec);

        processing = processing | ranges::view::filter([&stoppingIDS](const motion_context& mc){bool keep = (find(begin(stoppingIDS), end(stoppingIDS), mc.data_source_id()) == end(stoppingIDS)); if(!keep) printf("STOPPING: %s\n",mc.data_source_id().c_str()); return keep;});

        usleep(5000000);
    }

    r_locky::unregister_ffmpeg();

    return 0;
}
