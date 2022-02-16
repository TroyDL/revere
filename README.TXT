****************************************************************************************************
Project Goals
   - Simple open source video surveilance software.
   - Onvif, 4K, H.264, H.265, mu-law, AAC (latm & generic), b-frames
   - Linux & Windows
   - Software should run on almost anything (desktop, server, raspberry pi, etc...) and scale its
     capabilities automatically.
   - Pre Allocated Storage
       - Select the number of days of retention you want for a camera and all the disk space is
         allocated up front (and never exceeded).

What exists
   - onvif discovery
   - gstreamer rtsp based stream ingestions (h.264, h.265, MuLaw, ALaw, AAC, G.726)
   - recording (and query)
   - rtsp restreaming
   - basic recording GUI

TODO SOON
   - r_codec
       - r_audio_encoder
       - r_audio_decoder
   - HTTP server infrastructure
       - Use r_http, setup server (in r_vss?)
   - Contents Queries
       - Use r_storage::key_frame_start_times() to query available chunks
       - Return JSON
       - Support video and/or audio Queries
       - Support localtime or UTC Queries
   - Thumbnail Queries
       - allow resolution argument
       - return jpg and possibly others
       - Support localtime or UTC Queries
   - Exports
       - mov, mp4, mkv (some audio + container combinations may not be supported)
       - export to disk, then upload (to support files larger than available memory)
       - Use r_stoage::query()
       - Use http chunked transfer encoding to support large files
       - Support localtime or UTC Queries
   - Analytics
       - Need motion_processor thread
           - Wake up, query last 5 seconds worth of key frames from all cameras we need analytics
           - Keep state for each camera (create for first encounter, expire state if no motion for
             a while)
           - Decode keys
           - Run through r_motion
           - Write to new motion.db
             - create motion.db if not exists
             - schema upgrades?
             - write motion
   - Viewer
       - Cross platform QT Viewer
       - CRAZY fast + tons of keyboard shortcuts + super minimal UI
       - Pop up analytic event notifications
   - Advanced Analytics
       - Triggers blob Tracking
       - Triggers object detection
       - Schedule aware analytic triggers (txt messages, UI notifications, etc)

Known Issues
    - Put in invalid password. Should error out.
    - We should have a warning for extremely high bitrate cameras
    - 401 Unauthorized when occurring with camera setup should be visible to user somehow.
    - It will currently let you configure a record file with more space than the disk has free
      right now.

Compiling on Linux

   To prepare a fresh Linux Mint 20.3 VM for compilation:
      sudo apt update
      sudo apt install -y build-essential linux-headers-$(uname -r)
      <install virtualbox guest additions> Note: Configure your VM to have a decent amount of vram.
      sudo apt install git cmake
      sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavfilter-dev libavdevice-dev
      sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio libgstrtspserver-1.0-* libxml2-dev
      sudo apt install qtbase5-dev qtmultimedia5-dev qttools5-dev
      git clone https://github.com/dicroce/revere
      cd revere
      mkdir build && cd build && cmake .. && make

Compiling on Windows

   <coming soon>
