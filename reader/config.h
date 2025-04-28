#pragma once

#define REFRESH_RATE 250 // ms
#define DEBUG 1

#define info_print(fmt, ...) fprintf(stderr, "[INFO] " fmt, __VA_ARGS__); 
#define debug_print(fmt, ...) if (DEBUG) fprintf(stderr, "[DEBUG] " fmt, __VA_ARGS__);
#define error_print(fmt, ...) fprintf(stderr, "[ERROR] " fmt, __VA_ARGS__);
#define errorm_print(fmt) fprintf(stderr, "[ERROR] " fmt);
#define output_print(fmt, ...) fprintf(stdout, fmt, __VA_ARGS__);