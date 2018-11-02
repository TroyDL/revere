
#ifndef _vts_video_h
#define _vts_video_h

#include "r_http/r_web_server.h"

r_http::r_server_response make_video(const r_http::r_web_server<r_utils::r_socket>& ws,
                                     r_utils::r_buffered_socket<r_utils::r_socket>& conn,
                                     const r_http::r_server_request& request);

#endif
