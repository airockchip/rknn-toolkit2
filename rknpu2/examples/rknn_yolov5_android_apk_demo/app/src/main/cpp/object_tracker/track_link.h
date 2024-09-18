#ifndef __TRACK_C_LINK_C_H
#define __TRACK_C_LINK_C_H
#include "stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif
#include <android/log.h>
#define LOG_TAG    "hpc -- JNILOG" // 这个是自定义的LOG的标识
// #define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG, __VA_ARGS__)

typedef struct
{
    int x;
    int y;
    int width;
    int height;
}
Rect_T;

typedef struct
{
    Rect_T r;
	int obj_class;
	float score;
	int id;
	int reserve[1];
}
object_T;

long create_tracker();

void destroy_tracker(long handle);

void track(long handle, int maxTrackLifetime, int track_input_num, float * c_track_input_locations, int * c_track_input_class, float * c_track_input_score,
	  		int * c_track_output_num, float * c_track_output_locations, int * c_track_output_class, float * c_track_output_score, int * c_track_output_id,
		   	int width, int height);
#ifdef __cplusplus
}
#endif
#endif