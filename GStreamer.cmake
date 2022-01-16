
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(
    GSTREAMER_INCLUDE_DIRS
    deps/windows/gstreamer/1.0/msvc_x86_64/include/gstreamer-1.0
    deps/windows/gstreamer/1.0/msvc_x86_64/include/glib-2.0
    deps/windows/gstreamer/1.0/msvc_x86_64/lib/glib-2.0/include
)

set(
    GSTREAMER_LIB_DIRS
    deps/windows/gstreamer/1.0/msvc_x86_64/lib
)

set(
    GSTREAMER_LIBS
    gstreamer-1.0
    gstapp-1.0
    gstsdp-1.0
    gstrtspserver-1.0
    gstcodecparsers-1.0
    gobject-2.0
    gmodule-2.0
    xml2
    gthread-2.0
    glib-2.0
)

endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

find_package(PkgConfig)
pkg_check_modules(GSTREAMER_CORE REQUIRED gstreamer-1.0)
pkg_check_modules(GST_APP REQUIRED gstreamer-app-1.0)
pkg_check_modules(GST_SDP REQUIRED gstreamer-sdp-1.0)
pkg_check_modules(GST_RTSP_SERVER REQUIRED gstreamer-rtsp-server-1.0)
pkg_check_modules(GST_CODECPARSERS REQUIRED gstreamer-codecparsers-1.0)
pkg_check_modules(GOBJECT REQUIRED gobject-2.0)
pkg_check_modules(GMODULE REQUIRED gmodule-2.0)
pkg_check_modules(GTHREAD REQUIRED gthread-2.0)
pkg_check_modules(GLIB REQUIRED glib-2.0)

set(
    GSTREAMER_INCLUDE_DIRS
    ${GSTREAMER_CORE_INCLUDE_DIRS}
    ${GST_APP_INCLUDE_DIRS}
    ${GST_SDP_INCLUDE_DIRS}
    ${GST_RTSP_SERVER_INCLUDE_DIRS}
    ${GST_CODECPARSERS_INCLUDE_DIRS}
    ${GOBJECT_INCLUDE_DIRS}
    ${GMODULE_INCLUDE_DIRS}
    ${GTHREAD_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
)

set(
    GSTREAMER_LIB_DIRS
    ${GSTREAMER_CORE_LIBRARY_DIRS}
    ${GST_APP_LIBRARY_DIRS}
    ${GST_SDP_LIBRARY_DIRS}
    ${GST_RTSP_SERVER_LIBRARY_DIRS}
    ${GST_CODECPARSERS_LIBRARY_DIRS}
    ${GOBJECT_LIBRARY_DIRS}
    ${GMODULE_LIBRARY_DIRS}
    ${GTHREAD_LIBRARY_DIRS}
    ${GLIB_LIBRARY_DIRS}
)

set(
    GSTREAMER_LIBS
    ${GSTREAMER_CORE_LIBRARIES}
    ${GST_APP_LIBRARIES}
    ${GST_SDP_LIBRARIES}
    ${GST_RTSP_SERVER_LIBRARIES}
    ${GST_CODECPARSERS_LIBRARIES}
    ${GOBJECT_LIBRARY_LIBRARIES}
    ${GMODULE_LIBRARY_LIBRARIES}
    ${GTHREAD_LIBRARY_LIBRARIES}
    ${GLIB_LIBRARY_LIBRARIES}
)

endif()
