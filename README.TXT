
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

Dependencies

    The build script expects the following directories exist and are populated:

    Linux
        deps/
            linux/
                ffmpeg/
                gstreamer/
    Windows
        deps/
            windows/
                ffmpeg/
                gstreamer/
    
    Yes, this is a pain in the ass... but we're not really ready for others to be building us quite yet!

    gstreamer
        On Linux, gstreamer was built via the https://github.com/GStreamer/gst-build project with the following commands:

            meson --prefix=/path/to/install/prefix builddir
            ninja -C builddir
            meson install -C builddir

        builddir then becomes the above gstreamer deps folder.
        
        On Windows, download the official gstreamer binaries from

            https://gstreamer.freedesktop.org/download/#windows

        install them and then hand assemble the above directory layout with the installed files.

    ffmpeg
        Windows https://gstreamer.freedesktop.org/download/#windows
