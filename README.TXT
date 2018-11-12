first build 264 and ffmpeg in the usual cmake way.
then do a top level build.

TODO
   Some kind of streaming
   Some kind of transcoding.
   bento4 integratiooon
   

http://127.0.0.1:11002/data_sources
http://127.0.0.1:11003/jpg_before?data_source_id=c0f367c4-b360-4524-91df-d5e1fa52cd14&time=2018-11-10T13:34:00.000&width=320&height=240
http://127.0.0.1:11003/video?data_source_id=c0f367c4-b360-4524-91df-d5e1fa52cd14&start_time=2018-11-12T08:46:00.000&end_time=2018-11-12T08:47:00.000&file_path=/home/td/export.mp4

LD_LIBRARY_PATH=/home/td/code/dicroce/revere/ffmpeg/artifacts/lib:/home/td/code/dicroce/revere/x264/artifacts/lib ./ffmpeg -i out.mp4 -c:v copy -movflags empty_moov+omit_tfhd_offset+frag_keyframe+default_base_moof+isml out2.mp4