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

bool cpp_ffmpeg_wrapper_initialize(void * instance, CALLBACK_UINT32_UINT16_PTR_UINT16 cb, uint32_t scene_index)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    return core->initialize(cb, scene_index);
}

void cpp_ffmpeg_wrapper_shutdown(void * instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->shutdown();
}

void cpp_ffmpeg_wrapper_set_file_path(void * instance, char * url)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    std::string temp;
    temp.assign(url);
    
    core->file_path(temp);
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
