cmake_minimum_required(VERSION 3.10)

aux_source_directory(source libmov_src)
include_directories("./include")
include_directories(${CMAKE_SOURCE_DIR}/libflv/include)

add_library(mov STATIC ${libmov_src})

add_executable(fmp42flv "./test/fmp4-flv-test.cpp")
target_link_libraries(fmp42flv mov flv)