#include "r_motion/utils.h"
#include "r_motion/r_motion_state.h"
#include "r_utils/r_file.h"
#include "r_utils/r_string_utils.h"
#include "r_mux/r_demuxer.h"
#include "r_codec/r_video_decoder.h"

#include <deque>
#include <GLFW/glfw3.h>
#include <unistd.h>

using namespace std;
using namespace r_utils;
using namespace r_mux;
using namespace r_motion;
using namespace r_codec;

r_motion_state motion_state;
r_demuxer demuxer;
r_video_decoder decoder;
int video_stream_index;

const uint16_t W = 640;
const uint16_t H = 480;

std::string get_env(const string& name)
{
    char* env = getenv(name.c_str());
    return (env)?string(env):string();
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
    {
        bool got_video = false;

        while(!got_video)
        {
            if(demuxer.read_frame())
            {
                auto fi = demuxer.get_frame_info();
                if(fi.index != video_stream_index)
                    continue;
                got_video = true;

                decoder.attach_buffer(fi.data, fi.size);

                auto decoder_state = decoder.decode();

                if(decoder_state == R_VIDEO_DECODER_STATE_HAS_OUTPUT)
                {
                    auto decoded = decoder.get(AV_PIX_FMT_ARGB, W, H);

                    r_image argb_input;
                    argb_input.type = R_MOTION_IMAGE_TYPE_ARGB;
                    argb_input.width = W;
                    argb_input.height = H;
                    argb_input.data = decoded;

                    motion_state.process(argb_input);
                }
            }
        }
    }
}

int main(int argc, char* argv[])
{
   GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(W, H, "motion_test", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    auto fakey_root = get_env("FAKEY_ROOT");

    auto path = fakey_root + r_utils::r_fs::PATH_SLASH + "video_8.mp4";
    demuxer = r_demuxer(path, true);
    video_stream_index = demuxer.get_video_stream_index();
    auto vsi = demuxer.get_stream_info(video_stream_index);

    decoder = r_video_decoder(vsi.codec_id);

    motion_state = r_motion_state(10000);

    //glfwSetKeyCallback(window, key_callback);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        if(demuxer.read_frame())
        {
            auto fi = demuxer.get_frame_info();

            if(fi.index == video_stream_index && fi.key)
            {
                decoder.attach_buffer(fi.data, fi.size);

                auto decoder_state = decoder.decode();

                if(decoder_state == R_VIDEO_DECODER_STATE_HAS_OUTPUT)
                {
                    auto decoded = decoder.get(AV_PIX_FMT_ARGB, W, H);

                    r_image argb_input;
                    argb_input.type = R_MOTION_IMAGE_TYPE_ARGB;
                    argb_input.width = W;
                    argb_input.height = H;
                    argb_input.data = decoded;

                    auto result = motion_state.process(argb_input);
                    usleep(1000000);

                    if(!result.is_null())
                    {
                        auto mi = result.value();

                        vector<uint8_t> pb(mi.motion_pixels.width * mi.motion_pixels.height);
                        uint8_t* dst = pb.data();
                        const uint8_t* src = mi.motion_pixels.data.data() + mi.motion_pixels.width * (mi.motion_pixels.height - 1);
                        for(int i = 0; i < mi.motion_pixels.height; ++i)
                        {
                            memcpy(dst, src, mi.motion_pixels.width);
                            dst += mi.motion_pixels.width;
                            src -= mi.motion_pixels.width;
                        }
                        glDrawPixels(mi.motion_pixels.width, mi.motion_pixels.height, GL_LUMINANCE, GL_UNSIGNED_BYTE, pb.data());

                        auto delta = (mi.motion > mi.avg_motion)?mi.motion - mi.avg_motion:0;

                        auto threshold = (uint64_t)((double)mi.stddev * .5);

                        unsigned int rgba = (delta > threshold)?0xff0000ff:0xffffffff;
                        auto width = std::max(((double)delta / (double)mi.stddev) * 2.0, 0.1);
                        glBegin(GL_QUADS);
                            glColor4f(((rgba>>24)&0xff)/255.0f,((rgba>>16)&0xff)/255.0f,((rgba>>8)&0xff)/255.0f,(rgba&0xff)/255.0f);
                            glVertex3f(-1, -0.9, .0);
                            glVertex3f(-1+width, -0.9, 0);
                            glVertex3f(-1+width, -1.0, 0);
                            glVertex3f(-1, -1.0, 0);
                        glEnd();
                        glColor4f(1, 1, 1, 1);

                        /* Swap front and back buffers */
                        glfwSwapBuffers(window);
                    }
                }
            }
        }

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
}
