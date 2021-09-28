cmake_minimum_required(VERSION 3.10)

project(HLS2FLV VERSION 0.0.1)


add_subdirectory(libflv)
add_subdirectory(libmov)

add_executable(hls2flv main.cc)