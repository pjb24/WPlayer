#include "pch.h"

#include "FFmpegCore.h"

bool FFmpegCore::initialize(CALLBACK_INT32_UINT16_PTR_UINT16 cb, u32 scene_index)
{
    u32 packet_index = 0;
    u32 frame_index = 0;

    packet_index = initialize_packet_queue();
    if (packet_index != _packet_queue_size)
    {
        failed_free_packet_queue(packet_index);
        return false;
    }

    frame_index = initialize_frame_queue();
    if (frame_index != _frame_queue_size)
    {
        failed_free_frame_queue(frame_index);
        failed_free_packet_queue(packet_index);
        return false;
    }

    _format_ctx = avformat_alloc_context();

    _codec_ctx = avcodec_alloc_context3(nullptr);
    if (!_codec_ctx)
    {
        failed_free_frame_queue(frame_index);
        failed_free_packet_queue(packet_index);
        return false;
    }

    _first_decode = true;

    _callback_ffmpeg = cb;
    _scene_index = scene_index;

    return true;
}

void FFmpegCore::shutdown()
{
    avcodec_free_context(&_codec_ctx);

    avformat_close_input(&_format_ctx);

    if (!_frame_queue_free)
    {
        for (u32 i = 0; i < _frame_queue_size; i++)
        {
            av_frame_unref(_frame_queue[i]);
            av_frame_free(&_frame_queue[i]);
        }
        _frame_queue_free = true;
    }

    if (!_packet_queue_free)
    {
        for (u32 i = 0; i < _packet_queue_size; i++)
        {
            av_packet_unref(_packet_queue[i]);
            av_packet_free(&_packet_queue[i]);
        }
        _packet_queue_free = true;
    }
}

int FFmpegCore::open_file()
{
    if (_file_path.empty())
    {
        return (int)error_type::file_path_unsetted;
    }

    int result = 0;

    result = avformat_open_input(&_format_ctx, _file_path.c_str(), nullptr, nullptr /* dictionary */);
    if (result != 0)
    {
        return (int)error_type::file_not_exist;
    }

    /*
    if (dictionary)
    {
        av_dict_free(dictionary);
    }
    */

    _stream_index = av_find_best_stream(_format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

    _time_base = _format_ctx->streams[_stream_index]->time_base;
    _time_base_d = av_q2d(_time_base);
    _duration = _format_ctx->streams[_stream_index]->duration;
    _start_time = _format_ctx->streams[_stream_index]->start_time;

    return (int)error_type::ok;
}

void FFmpegCore::play_start(void* connection)
{
    std::lock_guard<std::mutex> play_lock(_play_mutex);

    _read_flag = true;
    _decode_flag = true;

    {
        std::lock_guard<std::mutex> pause_lock(_pause_mutex);

        _pause_flag = false;
    }

    _connection_play_start = connection;

    _read_thread = std::thread(&FFmpegCore::read, this);
    _decode_thread = std::thread(&FFmpegCore::decode, this);
}

void FFmpegCore::play_pause(void* connection)
{
    packet_result result = packet_result::ok;

    if (_pause_flag)
    {
        play_continue();
        result = packet_result::resume;
    }
    else
    {
        pause();
        result = packet_result::pause;
    }

    _callback_ffmpeg(_scene_index, (uint16_t)command_type::pause, connection, (uint16_t)result);
}

void FFmpegCore::play_stop(void* connection)
{
    if (!(_read_thread.joinable() && _decode_thread.joinable()))
    {
        return;
    }

    std::lock_guard<std::mutex> lock(_play_mutex);

    _read_flag = false;
    _decode_flag = false;

    if (_pause_flag)
    {
        play_continue();
    }

    if (_read_thread.joinable())
    {
        _read_thread.join();
    }
    if (_decode_thread.joinable())
    {
        _decode_thread.join();
    }

    _callback_ffmpeg(_scene_index, (uint16_t)command_type::stop, connection, (uint16_t)packet_result::ok);
}

s32 FFmpegCore::get_frame(AVFrame *& frame)
{
    error_type result = error_type::ok;
    s32 index = -1;

    // queue에서 frame 가져오기
    result = output_frame(frame, index);
    if (result != error_type::ok)
    {
        return -1;
    }

    return index;
}

s32 FFmpegCore::frame_to_next()
{
    std::lock_guard<std::mutex> lock(_frame_mutex);

    if (is_empty_frame_queue())
    {
        if (_pause_flag)
        {
            _time_started = 0.0f;
        }
        return error_type::queue_is_empty;
    }

    double time_now = av_gettime_relative() / 1'000.0;  // millisecond
    double next_frame_pts = _frame_queue[_output_frame_index]->pts * _time_base_d * 1'000.0;
    double next_frame_present_time = _time_started + (next_frame_pts);

    if (time_now < next_frame_present_time)
    {
        return error_type::use_previous_frame;
    }

    if (_time_started == 0.0f)
    {
        _time_started = time_now - next_frame_pts;
    }

    av_frame_unref(_frame_queue[_output_frame_index]);

    _output_frame_index = (_output_frame_index + 1) % _frame_queue_size;

    return error_type::ok;
}

void FFmpegCore::read()
{
    error_type result = error_type::ok;

    AVPacket* packet = nullptr;
    packet = av_packet_alloc();

    while (_read_flag)
    {
        while (_pause_flag || _seek_flag)
        {
            if (_seek_flag)
            {
                _seek_ready_flag_reader = true;
                std::unique_lock<std::mutex> lk(_seek_mutex_reader);
                _seek_condition_reader.wait(lk);
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
        }

        while (true)
        {
            result = read_internal(packet);
            if (result == error_type::ok)
            {
                open_codec();

                while (true)
                {
                    result = input_packet(packet);
                    if (result == error_type::ok)
                    {
                        break;
                    }
                    else
                    {
                        // queue_is_full
                        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
                        if (_seek_flag)
                        {
                            av_packet_unref(packet);
                            break;
                        }
                        if (!_read_flag)
                        {
                            break;
                        }
                    }
                }

                break;
            }
            else if (_eof)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
                break;
            }
        }
    }

    av_packet_unref(packet);
    av_packet_free(&packet);
}

error_type FFmpegCore::read_internal(AVPacket *& packet)
{
    if (!packet)
    {
        return error_type::input_unallocated_packet;
    }

    int result = 0;

    if (_eof)
    {
        return error_type::read_eof;
    }

    result = av_read_frame(_format_ctx, packet);
    if (result == AVERROR_EOF)
    {
        // TODO: EOF
        _eof = true;
    }

    if (packet->stream_index != _stream_index)
    {
        av_packet_unref(packet);
        return error_type::read_audio_packet;
    }

    return error_type::ok;
}

void FFmpegCore::open_codec()
{
    if (_codec_opened)
    {
        return;
    }

    int result = 0;

    result = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar);
    if (result < 0)
    {
        // TODO: error_type::open_codec_fail;
        return;
    }

    const AVCodec* codec = avcodec_find_decoder(_codec_ctx->codec_id);

    _codec_ctx->thread_count = 4;
    _codec_ctx->thread_type = FF_THREAD_SLICE;

    result = avcodec_open2(_codec_ctx, codec, nullptr);
    if (result != 0)
    {
        // TODO: error_type::open_codec_fail;
        return;
    }

    _codec_opened = true;
}

void FFmpegCore::decode()
{
    error_type result = error_type::ok;

    AVPacket* packet = nullptr;
    packet = av_packet_alloc();
    AVFrame* frame = nullptr;
    frame = av_frame_alloc();

    while (_decode_flag)
    {
        while (_pause_flag || _seek_flag)
        {
            if (_seek_flag)
            {
                _seek_ready_flag_decoder = true;
                std::unique_lock<std::mutex> lk(_seek_mutex_decoder);
                _seek_condition_decoder.wait(lk);
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
        }

        while (true)
        {
            while (true)
            {
                result = output_packet(packet);
                if (result == error_type::ok)
                {
                    break;
                }
                else
                {
                    // queue_is_empty
                    std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
                    if (_eof)
                    {
                        break;
                    }
                    else if (_seek_flag)
                    {
                        break;
                    }
                    else if (!_decode_flag)
                    {
                        break;
                    }
                }
            }

            result = decode_internal(packet, frame);
            if (result == error_type::ok)
            {
                //scale(frame);

                if (_first_decode)
                {
                    _first_decode = false;
                    _callback_ffmpeg(_scene_index, (uint16_t)command_type::play, _connection_play_start, (uint16_t)packet_result::ok);
                }

                while (true)
                {
                    result = input_frame(frame);
                    if (result == error_type::ok)
                    {
                        break;
                    }
                    else if (_seek_flag)
                    {
                        av_frame_unref(frame);
                        break;
                    }
                    else if (!_decode_flag)
                    {
                        break;
                    }
                    else
                    {
                        // queue_is_full
                        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
                    }
                }
                break;
            }
            else if (result == error_type::decode_eof)
            {
                break;
            }
        }
    }

    av_packet_unref(packet);
    av_packet_free(&packet);
    av_frame_unref(frame);
    av_frame_free(&frame);
}

error_type FFmpegCore::decode_internal(AVPacket * packet, AVFrame *& frame)
{
    int result = 0;

    result = avcodec_send_packet(_codec_ctx, packet);
    if (result == 0)
    {
        av_packet_unref(packet);
    }
    else if (result == AVERROR_EOF)
    {
        return error_type::decode_eof;
    }
    else if (result == AVERROR(EINVAL))
    {
        return error_type::codec_not_opened;
    }

    result = avcodec_receive_frame(_codec_ctx, frame);
    if (result == 0)
    {
        frame->pts = frame->best_effort_timestamp;
    }
    else if (result == AVERROR(EAGAIN))
    {
        return error_type::insufficient_input_packets;
    }
    else if (result == AVERROR_EOF)
    {
        return error_type::decode_eof;
    }

    return error_type::ok;
}

error_type FFmpegCore::flush_codec()
{
    if (_codec_ctx)
    {
        avcodec_flush_buffers(_codec_ctx);
    }

    return error_type::ok;
}

void FFmpegCore::pause()
{
    std::lock_guard<std::mutex> lock(_pause_mutex);

    _pause_flag = true;
}

void FFmpegCore::play_continue()
{
    std::lock_guard<std::mutex> lock(_pause_mutex);

    _pause_flag = false;
}

bool FFmpegCore::is_full_packet_queue()
{
    return (_output_packet_index == 0 && _input_packet_index == _packet_queue_size - 1) || (_output_packet_index == _input_packet_index + 1);
}

bool FFmpegCore::is_empty_packet_queue()
{
    return _output_packet_index == _input_packet_index;
}

bool FFmpegCore::is_full_frame_queue()
{
    return (_output_frame_index == 0 && _input_frame_index == _frame_queue_size - 1) || (_output_frame_index == _input_frame_index + 1);
}

bool FFmpegCore::is_empty_frame_queue()
{
    return _output_frame_index == _input_frame_index;
}

error_type FFmpegCore::input_packet(AVPacket * packet)
{
    std::lock_guard<std::mutex> lock(_packet_mutex);

    if (is_full_packet_queue())
    {
        return error_type::queue_is_full;
    }

    av_packet_move_ref(_packet_queue[_input_packet_index], packet);
    _input_packet_index = (_input_packet_index + 1) % _packet_queue_size;

    return error_type::ok;
}

error_type FFmpegCore::output_packet(AVPacket *& packet)
{
    std::lock_guard<std::mutex> lock(_packet_mutex);

    if (is_empty_packet_queue())
    {
        return error_type::queue_is_empty;
    }

    av_packet_move_ref(packet, _packet_queue[_output_packet_index]);

    _output_packet_index = (_output_packet_index + 1) % _packet_queue_size;

    return error_type::ok;
}

error_type FFmpegCore::input_frame(AVFrame * frame)
{
    std::lock_guard<std::mutex> lock(_frame_mutex);

    if (is_full_frame_queue())
    {
        return error_type::queue_is_full;
    }

    av_frame_move_ref(_frame_queue[_input_frame_index], frame);
    _input_frame_index = (_input_frame_index + 1) % _frame_queue_size;

    return error_type::ok;
}

error_type FFmpegCore::output_frame(AVFrame *& frame)
{
    std::lock_guard<std::mutex> lock(_frame_mutex);

    if (is_empty_frame_queue())
    {
        return error_type::queue_is_empty;
    }

    av_frame_move_ref(frame, _frame_queue[_output_frame_index]);

    _output_frame_index = (_output_frame_index + 1) % _frame_queue_size;

    return error_type::ok;
}

error_type FFmpegCore::output_frame(AVFrame *& frame, s32 & index)
{
    std::lock_guard<std::mutex> lock(_frame_mutex);

    if (is_empty_frame_queue())
    {
        return error_type::queue_is_empty;
    }

    index = _output_frame_index;

    av_frame_ref(frame, _frame_queue[_output_frame_index]);

    return error_type::ok;
}

u32 FFmpegCore::initialize_packet_queue()
{
    u32 packet_index = 0;
    for (; packet_index < _packet_queue_size; packet_index++)
    {
        _packet_queue[packet_index] = av_packet_alloc();
        if (!_packet_queue[packet_index])
        {
            break;
        }
    }

    return packet_index;
}

u32 FFmpegCore::initialize_frame_queue()
{
    u32 frame_index = 0;
    for (; frame_index < _frame_queue_size; frame_index++)
    {
        _frame_queue[frame_index] = av_frame_alloc();
        if (!_frame_queue[frame_index])
        {
            break;
        }
    }

    return frame_index;
}

void FFmpegCore::failed_free_packet_queue(u32 size)
{
    for (u32 i = 0; i < size; i++)
    {
        av_packet_free(&_packet_queue[i]);
    }
}

void FFmpegCore::failed_free_frame_queue(u32 size)
{
    for (u32 i = 0; i < size; i++)
    {
        av_frame_free(&_frame_queue[i]);
    }
}

void FFmpegCore::clear_packet_queue()
{
    error_type result = error_type::ok;
    AVPacket* packet = nullptr;
    packet = av_packet_alloc();

    while (true)
    {
        result = output_packet(packet);
        if (result == error_type::queue_is_empty)
        {
            break;
        }

        av_packet_unref(packet);
    }

    av_packet_free(&packet);
}

void FFmpegCore::clear_frame_queue()
{
    error_type result = error_type::ok;
    AVFrame* frame = nullptr;
    frame = av_frame_alloc();

    while (true)
    {
        result = output_frame(frame);
        if (result == error_type::queue_is_empty)
        {
            break;
        }

        av_frame_unref(frame);
    }

    av_frame_free(&frame);
}
