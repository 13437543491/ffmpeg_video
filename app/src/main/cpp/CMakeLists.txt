cmake_minimum_required(VERSION 3.10.2)

project("video")

include_directories(
        "include/ffmpeg"
        "include"
)

file(GLOB SOURCES *.cpp src/*.cpp)

add_library(
        video
        SHARED
        ${SOURCES}
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -L${CMAKE_SOURCE_DIR}/../jniLibs/${CMAKE_ANDROID_ARCH_ABI}")

find_library(
        log-lib
        log
)

target_link_libraries(
        video
        log
        -Wl,--start-group
        avcodec avfilter avformat avutil swresample swscale
        -Wl,--end-group
        z
        android
        OpenSLES
)

