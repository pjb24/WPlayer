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
typedef void(*CALLBACK_PTR)(void*);

EXPORT void* cpp_ffmpeg_wrapper_create();

EXPORT void cpp_ffmpeg_wrapper_delete(void* instance);

EXPORT bool cpp_ffmpeg_wrapper_initialize(void* instance, CALLBACK_PTR cb);

EXPORT void cpp_ffmpeg_wrapper_shutdown(void* instance);

EXPORT void cpp_ffmpeg_wrapper_set_scene_index(void* instance, uint32_t scene_index);

EXPORT void cpp_ffmpeg_wrapper_set_rect(void* instance, RECT rect);

EXPORT void cpp_ffmpeg_wrapper_set_sync_group_index(void* instance, uint32_t sync_group_index);

EXPORT void cpp_ffmpeg_wrapper_set_sync_group_count(void* instance, uint16_t sync_group_count);

EXPORT void cpp_ffmpeg_wrapper_set_sync_group_time_started(void* instance);

EXPORT void cpp_ffmpeg_wrapper_set_sync_group_frame_numbering(void* instance);

EXPORT void cpp_ffmpeg_wrapper_set_scale(void* instance, bool scale);

EXPORT void cpp_ffmpeg_wrapper_set_file_path(void* instance, char* url);

EXPORT int cpp_ffmpeg_wrapper_open_file(void* instance);

EXPORT void cpp_ffmpeg_wrapper_play_start(void* instance, void* connection);

EXPORT void cpp_ffmpeg_wrapper_play_pause(void* instance, void* connection);

EXPORT void cpp_ffmpeg_wrapper_play_stop(void* instance, void* connection);

EXPORT int32_t cpp_ffmpeg_wrapper_get_frame(void* instance, AVFrame*& frame);

EXPORT int32_t cpp_ffmpeg_wrapper_frame_to_next(void* instance);

EXPORT int32_t cpp_ffmpeg_wrapper_frame_to_next_non_waiting(void* instance);

EXPORT void cpp_ffmpeg_wrapper_seek_pts(void * instance, int64_t pts);

EXPORT void cpp_ffmpeg_wrapper_jump_forward(void * instance, void* connection);

EXPORT void cpp_ffmpeg_wrapper_jump_backwards(void * instance, void* connection);

EXPORT void cpp_ffmpeg_wrapper_frame_numbering(void* instance);

EXPORT void cpp_ffmpeg_wrapper_set_hw_decode(void* instance);

EXPORT void cpp_ffmpeg_wrapper_set_hw_device_type(void* instance, int hw_device_type);

EXPORT void cpp_ffmpeg_wrapper_set_hw_decode_adapter_index(void* instance, int hw_decode_adapter_index);

EXPORT void cpp_ffmpeg_wrapper_set_repeat_flag(void* instance);

EXPORT void cpp_ffmpeg_wrapper_repeat_sync_group(void* instance);

EXPORT void cpp_ffmpeg_wrapper_unset_repeat_flag(void* instance);

EXPORT int32_t cpp_ffmpeg_wrapper_check_frame_to_next_sync_group(void* instance);

EXPORT void cpp_ffmpeg_wrapper_start_thread_repeat(void* instance);

EXPORT void cpp_ffmpeg_wrapper_get_timebase(void* instance, AVRational& timebase);

EXPORT void cpp_ffmpeg_wrapper_get_is_realtime(void* instance, bool& is_realtime);

EXPORT void cpp_ffmpeg_wrapper_get_flag_succeed_open_input(void* instance, bool& flag_succeed_open_input);

EXPORT void cpp_ffmpeg_wrapper_get_flag_play_started(void* instance, bool& flag_play_started);

EXPORT void cpp_ffmpeg_wrapper_initialize_small(void* instance, CALLBACK_PTR cb);

EXPORT void cpp_ffmpeg_wrapper_shutdown_small(void* instance);
