
Project Goals
   - The "Apache" of open source video surveillance software: e.g. the default.
      - Software should run on almost anything and scale its capabilities automatically.
   - Total Solution
      - Recording
         - From professional grade cameras and protocols.
         - From consumer grade cameras and protocols.
      - Transcoding
         - Support automaticlally quality scaled transcoding for viewing.
      - Analytics
         - Configure particular cameras for anlytic analysys:
            -> Motion detection
               -> Blob Tracking
                  -> Person detection
      - Viewing
         - Browser based full motion viewing application
         - Thumbnail scrubbing
         - Motion/Event Timelines
   - Professional ONVIF camera discovery.
      - Hikvision - Ubiquiti - Pelco - Axis - Etc...
   - Commercial Grade
      - Nest Cam - Ring - Etc...

Current Progress
   - We have recording working pretty well.
   - We have thumbnail transcoding and mp4 creating from vts.
   - We have skeleton implementations of vas and commander.
   - So, we have a long way to go....

Building
   First build x264 and ffmpeg in the usual cmake way.

      mkdir x264/build
      pushd x264/build
      cmake .. && make
      popd
   
      mkdir ffmpeg/build
      pushd ffmpeg/build
      cmake .. && make
      popd

   Set LD_LIBRARY_PATH to point to ffmpeg/artifacts/lib and x264/artifacts/lib while cmake building the top level (this
   will allow this project to use the locally built ffmpeg and x264 as opposed to your system packages). Then build the top level:

      mkdir build
      pushd build
      cmake .. && make
      popd

TODO
   BUGS
      r_file_index::_fix_end_times() sets the end_time to the last_key() time! Not the last frame time!
         - refresh memory on r_storage (we're not only storing chunks starting with keys are we?)
         - looking at last_key() it looks like its returning the last thing in the index!
