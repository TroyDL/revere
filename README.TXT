
Project Goals
   - The "Apache" of open source video surveillance software: e.g. the default.
      - Software should run on almost anything and scale its capabilities automatically.
      - 4K, H.264, H.265, AAC (latm & generic), mulaw, b-frames

TODO
   - Create r_vss library
       - r_vss is the video recorder.
       - Use r_disco to enumerate assigned cameras
       - Every few seconds:
           - Prune record contexts not receiving data or whose configuration has changed.
           - Query our assigned cameras
           - Enumerate already recording cameras
           - Start and Stop record contexts as necessary
       - "record context"
           - r_pipeline::r_gst_source to pull stream
           - r_storage::r_storage_file to record
           - To what file?
               - Camera database should also have recording configuration
               - Recording configuration
                   - What file to record to
               - Canned recording configuration for now BUT the final app should have a wizard for 
                 configuring a camera (stream briefly to estimate bitrate and a UI to configure recording retention / file size)
   - Port libonvif
       - Rewrite libonvif to be C++
           - Use our exceptions
           - Use RAII
           - Add libxml2 to windows deps
       - Use to implement r_disco provider
   - Create QT Revere VMS application
       - Cross platform QT tray service
       - UI for discovering cameras
           - Add camera wizard
       - UI for monitoring
           - Show camera status (discovered, assigned, current recording bitrate etc)
   - Viewer
       - Cross platform QT Viewer
       - CRAZY fast + tons of keyboard shortcuts + super minimal UI
       - Pop up analytic event notifications
   - Analytics
       - key frame motion detection
       - Triggers blob Tracking
       - Triggers object detection
       - Schedule aware analytic triggers (txt messages, UI notifications, etc)

Compiling on Linux

   To prepare a fresh Ubuntu 20.04 VM for compilation:
      sudo apt update
      sudo apt install -y build-essential linux-headers-$(uname -r)
      <install virtualbox guest additions> Note: Configure your VM to have a decent amount of vram.
      sudo apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev libavfilter-dev libavdevice-dev
      sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev gstreamer1.0-plugins-base gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio libgstrtspserver-1.0-* libxml2-dev

      git clone git@github.com:dicroce/revere.git
      cd revere
      mkdir build && cd build && cmake .. && make

Compiling on Windows

   <coming soon>