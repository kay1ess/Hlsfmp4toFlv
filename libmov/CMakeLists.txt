cmake_minimum_required(VERSION 3.10)

aux_source_directory(source libmov_src)
include_directories("./include")

add_library(mov STATIC ${libmov_src})