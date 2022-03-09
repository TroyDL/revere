
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(
    LIBXML2_INCLUDE_DIRS
    ../deps/windows/gstreamer/1.0/msvc_x86_64/include/libxml2
    ../deps/windows/gstreamer/1.0/msvc_x86_64/include
)

set(
    LIBXML2_LIB_DIRS
    ../deps/windows/gstreamer/1.0/msvc_x86_64/lib
)

set(
    LIBXML2_LIBRARIES
    xml2.lib
)

endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

find_package(LibXml2)

endif()
