
Building
   First build x264 and ffmpeg in the usual cmake way, then do a top level cmake build.

TODO
   - We need a "commander" process that runs the show. The "commander" should:
      - Discover cameras (and periodically rediscover them)
      - Implement API's
         - GET discovered cameras
         - GET assigned cameras
         - POST assign camera
         - DELETE camera
      - Periodically hit vss api's and make sure correct cameras are assigned.
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
   
Some random API'S
   http://127.0.0.1:11002/data_sources
   http://127.0.0.1:11003/jpg_before?data_source_id=c0f367c4-b360-4524-91df-d5e1fa52cd14&time=2018-11-10T13:34:00.000&width=320&height=240
   http://127.0.0.1:11003/video?data_source_id=c0f367c4-b360-4524-91df-d5e1fa52cd14&start_time=2018-11-12T08:46:00.000&end_time=2018-11-12T08:47:00.000&file_path=/home/td/export.mp4
