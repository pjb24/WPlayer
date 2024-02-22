#pragma once

#define EXPORT extern "C" __declspec(dllexport)

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN             // 거의 사용되지 않는 내용을 Windows 헤더에서 제외합니다.
#include <Windows.h>

extern "C"
{
#include "libavutil/frame.h"
}
#pragma comment(lib, "avutil.lib")

// CallbackType
typedef void(*CALLBACK_UINT32)(uint32_t);

EXPORT void* cpp_ffmpeg_wrapper_create();

EXPORT void cpp_ffmpeg_wrapper_delete(void* instance);

EXPORT bool cpp_ffmpeg_wrapper_initialize(void* instance, CALLBACK_UINT32 cb, uint32_t scene_index);

EXPORT void cpp_ffmpeg_wrapper_shutdown(void* instance);

EXPORT void cpp_ffmpeg_wrapper_set_file_path(void* instance, char* url);

EXPORT int cpp_ffmpeg_wrapper_open_file(void* instance);

EXPORT void cpp_ffmpeg_wrapper_play_start(void* instance);

EXPORT void cpp_ffmpeg_wrapper_play_pause(void* instance);

EXPORT void cpp_ffmpeg_wrapper_play_stop(void* instance);

EXPORT int32_t cpp_ffmpeg_wrapper_get_frame(void* instance, AVFrame*& frame, int64_t pts);
