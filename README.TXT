
Building
   First build x264 and ffmpeg in the usual cmake way.

   Set LD_LIBRARY_PATH to point to ffmpeg/artifacts/lib and x264/artifacts/lib while cmake building the top level (this
   will allow this project to use the locally built ffmpeg and x264 as opposed to your system packages).

TODO
   BUGS
      r_file_index::_fix_end_times() sets the end_time to the last_key() time! Not the last frame time!
         - refresh memory on r_storage (we're not only storing chunks starting with keys are we?)
         - looking at last_key() it looks like its returning the last thing in the index!



   - We need analytics
      - Get h.264 keyframes from vss
         - Decode
            - Detect Motion
               - Database Motion
   - We need a viewing web app
      - We possibly need more advanced "player" javascript.
      - Features
         - View discovered cameras
         - Assign camera
         - View assigned cameras
         - View live thumbnails of all cameras (with periodic updates)
         - View live video
            - Timeline showing holes
   - Raspberry Pi
      - Use hardware encoder
      - Use hardware decoder

120+ decode fps (all keyframes)
    LD_LIBRARY_PATH=~/code/dicroce/revere/lame/artifacts/lib time ./ffmpeg -c:v h264_mmal -i out2.mp4 -f null -

58+ transcode fps (300% cpu! because decode is not being accelerated)
    LD_LIBRARY_PATH=~/code/dicroce/revere/lame/artifacts/lib time ./ffmpeg -i out2.mp4 -c:v h264_omx out3.mp4

10+ fps hw decode + sw encode
   LD_LIBRARY_PATH=~/code/dicroce/revere/lame/artifacts/lib time ./ffmpeg -c:v h264_mmal -i the_white_wolf_1280x720.mp4 -an -c:v libx264 out3.mp4

add "gpu_mem=256" to /boot/config
then

54 transcode fps + (50% cpu)
   LD_LIBRARY_PATH=~/code/dicroce/revere/lame/artifacts/lib time ./ffmpeg -c:v h264_mmal -i the_white_wolf_1280x720.mp4 -an -c:v h264_omx out3.mp4 

Big Unknown is motion detection performance. Need OpenCV build.