cmake_minimum_required(VERSION 3.10)

aux_source_directory(source libflv_src)
include_directories("./include")

add_library(flv STATIC ${libflv_src})