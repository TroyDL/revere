
Project Goals
   - Simple open source video surveilance software.
   - Onvif, 4K, H.264, H.265, mu-law, AAC (latm & generic), b-frames
   - Linux & Windows
   - Software should run on almost anything (desktop, server, raspberry pi, etc...) and scale its capabilities automatically.
   - Pre Allocated Storage
       - Select the number of days of retention you want for a camera and all the disk space is allocated up front (and never exceeded).
   

TODO
   - r_vss
       - Add api to stream keeper to query stats information about running streams:
         - number and type of streams (video & audio or just video)
         - bitrate
       - Add muxed file query API
       - Add jpg API
   - Add encode / decode lib
   - recording service
       - run in tray
       - context menu
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

   To prepare a fresh Ubuntu 21.10 VM for compilation:
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
