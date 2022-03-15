****************************************************************************************************
Project Goals
   - Simple open source video surveilance software.
   - Onvif, 4K, H.264, H.265, mu-law, AAC (latm & generic)
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

   Initial Setup

      1) Install Visual Studio 2019 (free community edition is fine)

      2) Install GIT & GIT Bash
         https://github.com/git-for-windows/git/releases/download/v2.35.1.windows.2/Git-2.35.1.2-64-bit.exe

      3) Install cmake (let it add cmake to your system PATH environment variable)
         https://github.com/Kitware/CMake/releases/download/v3.23.0-rc3/cmake-3.23.0-rc3-windows-x86_64.msi

      4) Install Qt 6.2.3 (open source edition is fine)

      5) Set QTDIR environment variable to path to qmake (example: C:\Qt\6.2.3\msvc2019_64\bin)
         
      6) Install gstreamer (vs 2019)
         Download and install the runtime installer:
         https://gstreamer.freedesktop.org/data/pkg/windows/1.20.0/msvc/gstreamer-1.0-msvc-x86_64-1.20.0.msi
         And the development installer:
         https://gstreamer.freedesktop.org/data/pkg/windows/1.20.0/msvc/gstreamer-1.0-devel-msvc-x86_64-1.20.0.msi
         
      7) Set GSTREAMER_DEV_ROOT to the path to the gstreamer installation directory (example: C:\gstreamer\1.0\msvc_x86_64)

      8) Add gstreamers "bin" dir to your system PATH environment variable: (example: C:\gstreamer\1.0\msvc_x86_64\bin)

      9) Install FFmpeg
         Download and unzip (you'll need 7zip to unzip this)
         https://www.gyan.dev/ffmpeg/builds/ffmpeg-release-full-shared.7z
      
      10) Set FFMPEG_DEV_ROOT to the path the FFmpeg installation directory (example: C:\dev\ffmpeg-5.0-full_build-shared)

      11) Add ffmpegs "bin" dir to your system PATH environment variable: (example C:\dev\ffmpeg-5.0-full_build-shared\bin)

      12) Use "GIT Bash" and create a development directory somewhere then clone the revere repo:

          git clone https://github.com/dicroce/revere

      Notes
         - The GSTREAMER_DEV_ROOT and FFMPEG_DEV_ROOT environment variables are required to build revere on windows. You may
         need to reopen git bash or other programs after you set them.
         - The gstreamer and ffmpeg "bin" directories added to your system path are required to run both the unit tests
         (revere_ut) and the vms backend (revere).

   Compilation

      1) mkdir build
      2) cd build
      3) cmake -G "Visual Studio 16 2019" ..
      4) cmake --build .

      To clean:
          cmake --build . --target clean

Visual Studio Code

   If you've done everything above and you install the "CMake Tools" extension in VSCode you should just be able to hit
   f7 to compile. If you add appropriate launch configurations you can also hit f5 to debug.

   On Windows my VSCode launch configuration looks like this:

      {
         // Use IntelliSense to learn about possible attributes.
         // Hover to view descriptions of existing attributes.
         // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
         "version": "0.2.0",
         "configurations": [
            {
                  "name": "revere_ut",
                  "type": "cppvsdbg",
                  "request": "launch",
                  "program": "${workspaceFolder}/build/ut/Debug/revere_ut.exe",
                  "args": [],
                  "stopAtEntry": false,
                  "cwd": "",
                  "environment": [],
                  "console": "externalTerminal",
            },
            {
                  "name": "revere",
                  "type": "cppvsdbg",
                  "request": "launch",
                  "program": "${workspaceFolder}/build/revere/Debug/revere.exe",
                  "args": [],
                  "stopAtEntry": false,
                  "cwd": "",
                  "environment": [],
                  "console": "externalTerminal",
            }
         ]
      }
   
   Linux looks very similar except for no /Debug in the path to the executables and of course no .exe on the end.
