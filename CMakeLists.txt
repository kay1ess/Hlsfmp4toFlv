cmake_minimum_required(VERSION 3.10)

project(HLS2FLV VERSION 0.0.1)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libflv)
add_subdirectory(libmov)
add_subdirectory(cpr)

include_directories("./cpr/include")

aux_source_directory(src src)
include_directories(src)
include_directories(${CMAKE_SOURCE_DIR}/libmov/include)
include_directories(${CMAKE_SOURCE_DIR}/libflv/include)

add_executable(hls2flv main.cc ${src})
target_link_libraries(hls2flv PRIVATE cpr mov flv)
