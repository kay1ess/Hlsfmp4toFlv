/*
 * @Author: kay 
 * @Date: 2021-09-29 17:20:17 
 * @Last Modified by: kay
 * @Last Modified time: 2021-10-19 20:52:25
 */


#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdarg.h>

#define log_info(format, ...) fprintf(stdout, "\033[0;37m" format "\n\033[0m", ##__VA_ARGS__)
#define log_warn(format, ...) fprintf(stdout, "\033[0;33m" format "\n\033[0m", ##__VA_ARGS__)
#define log_error(format, ...) fprintf(stderr, "\033[0;31m" format "\n\033[0m", ##__VA_ARGS__)

#endif