
Building
   First build x264 and ffmpeg in the usual cmake way, then do a top level cmake build.

TODO
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
