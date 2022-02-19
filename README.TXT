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
   - Exports
       - mov, mp4, mkv (some audio + container combinations may not be supported)
       - export to disk, then upload (to support files larger than available memory)
       - Use r_stoage::query()
       - Use http chunked transfer encoding to support large files
       - Support localtime or UTC Queries
   - Analytics
       - Add r_camera flag "do_motion"
       - Add r_camera "motion_sensitivity"
       - Need motion_processor thread
           - Decode keys
           - Run through r_motion
           - Write to new motion.db
             - create motion.db if not exists
             - schema upgrades?
             - write motion
       - Analytic Storage
           - We need 192 bytes to store 3 64 bit values (motion, avg_motion, stddev)
           - If we store one of those per second then we need 192*86400=1,658,8800 bytes to store one day
           - 1,658,000 * 30 (days) = 497,664,000
           - When we do camera setup, lets ask if they want to do motion detection. If they do:
               - divide the storage file size they choose by the byterate to get the number of seconds of retention they want
               - multiply that number by 192, that is your motion db file size (+header).
   - Viewer
       - Cross platform QT Viewer
       - CRAZY fast + tons of keyboard shortcuts + super minimal UI
       - Pop up analytic event notifications
   - r_codec
       - r_audio_encoder
       - r_audio_decoder

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
