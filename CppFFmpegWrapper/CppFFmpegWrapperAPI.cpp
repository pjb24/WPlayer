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

bool cpp_ffmpeg_wrapper_initialize(void * instance, CALLBACK_UINT32 cb, uint32_t scene_index)
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

void cpp_ffmpeg_wrapper_play_start(void * instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->play_start();
}

void cpp_ffmpeg_wrapper_play_pause(void * instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->play_pause();
}

void cpp_ffmpeg_wrapper_play_stop(void * instance)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    core->play_stop();
}

int32_t cpp_ffmpeg_wrapper_get_frame(void* instance, AVFrame*& frame, int64_t pts)
{
    FFmpegCore* core = (FFmpegCore*)instance;
    return core->get_frame(frame);
}
