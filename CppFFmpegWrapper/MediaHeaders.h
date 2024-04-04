#pragma once

#include "CommonHeaders.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/time.h"
}

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "swscale.lib")

constexpr u32 _packet_queue_size = 60;
constexpr u32 _frame_queue_size = frame_buffer_count;
constexpr u32 _jump_length = 5000;  // milliseconds

constexpr u32 _sleep_time = 10;

#include "ErrorTypes.h"

#include "CppFFmpegWrapperCallbackStruct.h"
