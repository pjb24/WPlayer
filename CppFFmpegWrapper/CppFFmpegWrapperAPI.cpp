#include "pch.h"

#include "CppFFmpegWrapperAPI.h"

#include "FFmpegCore.h"

void * cpp_ffmpeg_wrapper_create()
{
    FFmpegCore* core = new FFmpegCore();
    return core;
}

void cpp_ffmpeg_wrapper_delete(void * instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    delete core;
}

bool cpp_ffmpeg_wrapper_initialize(void * instance, CALLBACK_PTR cb)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    return core->initialize(cb);
}

void cpp_ffmpeg_wrapper_shutdown(void * instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->shutdown();
}

void cpp_ffmpeg_wrapper_set_scene_index(void* instance, uint32_t scene_index)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->scene_index(scene_index);
}

void cpp_ffmpeg_wrapper_set_rect(void* instance, RECT rect)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->rect(rect);
}

void cpp_ffmpeg_wrapper_set_sync_group_index(void* instance, uint32_t sync_group_index)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->sync_group_index(sync_group_index);
}

void cpp_ffmpeg_wrapper_set_sync_group_count(void* instance, uint16_t sync_group_count)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->sync_group_count(sync_group_count);
}

void cpp_ffmpeg_wrapper_set_sync_group_time_started(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->sync_group_time_started();
}

void cpp_ffmpeg_wrapper_set_sync_group_frame_numbering(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->sync_group_frame_numbering();
}

void cpp_ffmpeg_wrapper_set_scale(void* instance, bool scale)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->set_scale(scale);
}

void cpp_ffmpeg_wrapper_set_file_path(void * instance, char * url)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    std::string temp;
    temp.assign(url);
    
    core->file_path(temp);

    core->is_realtime();
}

int cpp_ffmpeg_wrapper_open_file(void * instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    return core->open_file();
}

void cpp_ffmpeg_wrapper_play_start(void * instance, void* connection)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->play_start(connection);
}

void cpp_ffmpeg_wrapper_play_pause(void * instance, void* connection)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->play_pause(connection);
}

void cpp_ffmpeg_wrapper_play_stop(void * instance, void* connection)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->play_stop(connection);
}

int32_t cpp_ffmpeg_wrapper_get_frame(void* instance, AVFrame*& frame)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    return core->get_frame(frame);
}

int32_t cpp_ffmpeg_wrapper_frame_to_next(void * instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    return core->frame_to_next();
}

int32_t cpp_ffmpeg_wrapper_frame_to_next_non_waiting(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    return core->frame_to_next_non_waiting();
}

void cpp_ffmpeg_wrapper_seek_pts(void * instance, int64_t pts)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->seek_pts(pts);
}

void cpp_ffmpeg_wrapper_jump_forward(void * instance, void* connection)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->jump_forward(connection);
}

void cpp_ffmpeg_wrapper_jump_backwards(void * instance, void* connection)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->jump_backwards(connection);
}

void cpp_ffmpeg_wrapper_frame_numbering(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->frame_numbering();
}

void cpp_ffmpeg_wrapper_set_hw_decode(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->hw_decode(true);
}

void cpp_ffmpeg_wrapper_set_hw_device_type(void* instance, int hw_device_type)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->hw_device_type(hw_device_type);
}

void cpp_ffmpeg_wrapper_set_hw_decode_adapter_index(void* instance, int hw_decode_adapter_index)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->hw_decode_adapter_index(hw_decode_adapter_index);
}

void cpp_ffmpeg_wrapper_set_repeat_flag(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->set_repeat_flag();
}

void cpp_ffmpeg_wrapper_repeat_sync_group(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->repeat_sync_group();
}

void cpp_ffmpeg_wrapper_unset_repeat_flag(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->unset_repeat_flag();
}

int32_t cpp_ffmpeg_wrapper_check_frame_to_next_sync_group(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    return core->check_frame_to_next_sync_group();
}

void cpp_ffmpeg_wrapper_start_thread_repeat(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->start_thread_repeat();
}

void cpp_ffmpeg_wrapper_get_timebase(void* instance, AVRational& timebase)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->get_timebase(timebase);
}

void cpp_ffmpeg_wrapper_get_is_realtime(void* instance, bool& is_realtime)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->get_is_realtime(is_realtime);
}

void cpp_ffmpeg_wrapper_get_flag_succeed_open_input(void* instance, bool& flag_succeed_open_input)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->get_flag_succeed_open_input(flag_succeed_open_input);
}

void cpp_ffmpeg_wrapper_get_flag_play_started(void* instance, bool& flag_play_started)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->get_flag_play_started(flag_play_started);
}

void cpp_ffmpeg_wrapper_initialize_small(void* instance, CALLBACK_PTR cb)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->initialize_small(cb);
}

void cpp_ffmpeg_wrapper_shutdown_small(void* instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->shutdown_small();
}
