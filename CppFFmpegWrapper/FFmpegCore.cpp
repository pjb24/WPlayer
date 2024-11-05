#include "pch.h"

#include "FFmpegCore.h"

FFmpegCore::FFmpegCore()
{
    _logical_processor_count = 0;
    _logical_processor_count_half = 0;

    _connection_play_start = nullptr;

    _first_decode = false;
    _previous_frame_pts = 0;
    _time_started = 0.0;

    _sync_group_frame_numbering = false;

    _frame_numbering = 0;
    _frame_count = 0;

    _repeat_flag = false;

    _hw_decode = false;
    _hw_device_type = AVHWDeviceType::AV_HWDEVICE_TYPE_NONE;
    _hw_device_ctx = nullptr;
    _hw_decode_adapter_index = -1;

    _hw_frame = nullptr;

    _pause_flag = false;

    _seek_flag = false;

    _seek_ready_flag_reader = false;
    _seek_flag_reader = false;

    _seek_ready_flag_decoder = false;
    _seek_flag_decoder = false;

    _format_ctx = nullptr;
    _codec_ctx = nullptr;

    _stream_index = -1;

    _codec = nullptr;

    _option = nullptr;

    _time_base = { 0, 1 };
    _time_base_d = 0.0;
    _duration = 0;
    _duration_frame = 0;
    _duration_frame_half = 0.0;
    _start_time = 0;

    _read_flag = false;
    _decode_flag = false;

    _codec_opened = false;
    _eof_read = false;
    _eof_decode = false;
    _eof_read2 = false;

    _callback_ffmpeg = nullptr;

    _scene_index = u32_invalid_id;
    _rect = { 0, 0, 0, 0 };
    _sync_group_index = u32_invalid_id;
    _sync_group_count = 0;
    _url_size = 0;

    _scale_dest_format = AVPixelFormat::AV_PIX_FMT_NV12;
    _scale_frame = nullptr;
    _sws_ctx = nullptr;

    _scale_alloc_size = 0;
    _scale = true;

    _input_packet_index = 0;
    _output_packet_index = 0;
    _packet_queue_free = false;

    _input_frame_index = 0;
    _output_frame_index = 0;
    _frame_queue_free = false;
}
bool FFmpegCore::initialize(CALLBACK_PTR cb)
{
    UINT packet_index = 0;
    UINT frame_index = 0;

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

    _first_decode = true;

    _callback_ffmpeg = cb;

    SYSTEM_INFO _stSysInfo;
    GetSystemInfo(&_stSysInfo);
    _logical_processor_count = _stSysInfo.dwNumberOfProcessors;	// cpu 논리 프로세서 개수
    _logical_processor_count_half = _logical_processor_count / 2;

    _scale_frame = av_frame_alloc();

    _hw_frame = av_frame_alloc();

    return true;
}

void FFmpegCore::shutdown()
{
    if (_hw_frame)
    {
        av_frame_free(&_hw_frame);
        _hw_frame = nullptr;
    }

    if (_scale_frame)
    {
        av_frame_free(&_scale_frame);
        _scale_frame = nullptr;
    }

    if (_sws_ctx)
    {
        sws_freeContext(_sws_ctx);
        _sws_ctx = nullptr;
    }

    if (_codec_ctx != nullptr)
    {
        avcodec_free_context(&_codec_ctx);
    }

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

    _stream_index = av_find_best_stream(_format_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, &_codec, 0);

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

    _time_started = av_gettime_relative() / 1'000.0;

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

    ffmpeg_wrapper_callback_data* data = new ffmpeg_wrapper_callback_data();
    data->scene_index = _scene_index;
    data->command = (u16)command_type::pause;
    data->connection = connection;
    data->result = (u16)result;

    _callback_ffmpeg(data);

    delete data;
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

    ffmpeg_wrapper_callback_data* data = new ffmpeg_wrapper_callback_data();
    data->scene_index = _scene_index;

    if (_sync_group_index != u32_invalid_id)
    {
        data->command = (u16)command_type::stop_sync_group;
        data->sync_group_index = _sync_group_index;
        data->sync_group_count = _sync_group_count;
    }
    else
    {
        data->command = (u16)command_type::stop;
    }

    data->connection = connection;
    data->result = (u16)packet_result::ok;

    _callback_ffmpeg(data);

    delete data;
}

void FFmpegCore::jump_forward(void* connection)
{
    if (_seek_flag)
    {
        return;
    }

    s64 now_pts{ _previous_frame_pts - _start_time };
    s64 pts{ now_pts + s64(_jump_length / 1000 / av_q2d(_time_base)) };

    if (_duration - _start_time < pts)
    {
        pts = _duration - _start_time;
    }

    seek_pts(pts);

    ffmpeg_wrapper_callback_data* data = new ffmpeg_wrapper_callback_data();
    data->scene_index = _scene_index;
    data->command = (u16)command_type::jump_forward;
    data->connection = connection;
    data->result = (u16)packet_result::ok;

    _callback_ffmpeg(data);

    delete data;
}

void FFmpegCore::jump_backwards(void* connection)
{
    if (_seek_flag)
    {
        return;
    }

    s64 now_pts{ _previous_frame_pts - _start_time };
    s64 pts{ now_pts - s64(_jump_length / 1000 / av_q2d(_time_base)) };

    if (0 > pts)
    {
        pts = 0;
    }

    seek_pts(pts);

    ffmpeg_wrapper_callback_data* data = new ffmpeg_wrapper_callback_data();
    data->scene_index = _scene_index;
    data->command = (u16)command_type::jump_backwards;
    data->connection = connection;
    data->result = (u16)packet_result::ok;

    _callback_ffmpeg(data);

    delete data;
}

s32 FFmpegCore::get_frame(AVFrame*& frame)
{
    error_type result = error_type::ok;
    s32 index = -1;

    // queue에서 frame 가져오기
    result = output_frame(frame, index);
    if (result != error_type::ok)
    {
        //if (_eof_read == true)
        if (_eof_read2 == true)
        {
            if (_repeat_flag == true)
            {
                return -3;
            }

            return -2;
        }

        return -1;
    }

    _previous_frame_pts = frame->pts;

    return index;
}

s32 FFmpegCore::frame_to_next()
{
    if (_sync_group_frame_numbering == true && _sync_group_index != u32_invalid_id)
    {
        if (_frame_numbering < _frame_count)
        {
            return error_type::use_previous_frame;
        }
    }

    std::lock_guard<std::mutex> lock(_frame_mutex);

    if (is_empty_frame_queue())
    {
        if (_pause_flag)
        {
            _time_started = 0.0;
        }
        return error_type::queue_is_empty;
    }

    int frame_queue_size = get_frame_queue_size();

    double time_now = av_gettime_relative() / 1'000.0;  // millisecond

    double previous_frame_pts = 0;
    if (_frame_queue[_output_frame_index]->pts > 0)
    {
        previous_frame_pts = _frame_queue[_output_frame_index]->pts * _time_base_d * 1'000.0;
    }

    if (_time_started == 0.0)
    {
        _time_started = time_now - previous_frame_pts;
    }

    double previous_frame_present_time = _time_started + (previous_frame_pts);

    double time_delta = previous_frame_present_time - time_now;

    if (time_delta > -(_duration_frame_half))
    {
        return error_type::use_previous_frame;
    }

    if (frame_queue_size > 2)
    {
        s32 temp_index = (_output_frame_index + 1) % _frame_queue_size;
        // pts 역전이 있을 때
        if (_previous_frame_pts > _frame_queue[temp_index]->pts)
        {
            av_frame_unref(_frame_queue[_output_frame_index]);
            _output_frame_index = (_output_frame_index + 1) % _frame_queue_size;
        }
    }

    av_frame_unref(_frame_queue[_output_frame_index]);
    _output_frame_index = (_output_frame_index + 1) % _frame_queue_size;

    if (_sync_group_frame_numbering == true && _sync_group_index != u32_invalid_id)
    {
        _frame_count++;
    }

    return error_type::ok;
}

s32 FFmpegCore::frame_to_next_non_waiting()
{
    std::lock_guard<std::mutex> lock(_frame_mutex);

    if (is_empty_frame_queue())
    {
        return error_type::queue_is_empty;
    }

    av_frame_unref(_frame_queue[_output_frame_index]);
    _output_frame_index = (_output_frame_index + 1) % _frame_queue_size;

    return error_type::ok;
}

s32 FFmpegCore::check_frame_to_next_sync_group()
{
    if (_sync_group_frame_numbering == true && _sync_group_index != u32_invalid_id)
    {
        if (_frame_numbering < _frame_count)
        {
            return error_type::use_previous_frame;
        }
    }

    std::lock_guard<std::mutex> lock(_frame_mutex);

    if (is_empty_frame_queue())
    {
        if (_pause_flag)
        {
            _time_started = 0.0;
        }
        return error_type::queue_is_empty;
    }

    int frame_queue_size = get_frame_queue_size();

    double time_now = av_gettime_relative() / 1'000.0;  // millisecond

    double previous_frame_pts = 0;
    if (_frame_queue[_output_frame_index]->pts > 0)
    {
        previous_frame_pts = _frame_queue[_output_frame_index]->pts * _time_base_d * 1'000.0;
    }

    if (_time_started == 0.0)
    {
        _time_started = time_now - previous_frame_pts;
    }

    double previous_frame_present_time = _time_started + (previous_frame_pts);

    double time_delta = previous_frame_present_time - time_now;

    if (time_delta > -(_duration_frame_half))
    {
        return error_type::use_previous_frame;
    }

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
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));

            if (_seek_flag)
            {
                _seek_ready_flag_reader = true;
                std::unique_lock<std::mutex> lk(_seek_mutex_reader);
                if (_seek_flag_reader == false)
                {
                    _seek_condition_reader.wait(lk);
                }
                _seek_flag_reader = false;
                break;
            }
        }

        while (true)
        {
            open_codec();

            result = read_internal(packet);
            if (result == error_type::ok)
            {
                //open_codec();

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
            else if (_eof_read)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
                break;
            }
        }
    }

    av_packet_unref(packet);
    av_packet_free(&packet);
}

error_type FFmpegCore::read_internal(AVPacket*& packet)
{
    if (!packet)
    {
        return error_type::input_unallocated_packet;
    }

    int result = 0;

    if (_eof_read)
    {
        return error_type::read_eof;
    }

    result = av_read_frame(_format_ctx, packet);
    if (result == AVERROR_EOF)
    {
        // TODO: EOF
        _eof_read = true;
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

    if (_hw_decode == true)
    {
        for (int i = 0;; i++)
        {
            const AVCodecHWConfig* config = avcodec_get_hw_config(_codec, i);
            if (!config)
            {
                fprintf(stderr, "Decoder %s does not support device type %s.\n",
                    _codec->name, av_hwdevice_get_type_name(_hw_device_type));
                // return -1;
                return;
            }
            if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX &&
                config->device_type == _hw_device_type)
            {
                _hw_pix_fmt = config->pix_fmt;
                break;
            }
        }

        _codec_ctx = avcodec_alloc_context3(_codec);

        result = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar);
        if (result < 0)
        {
            // TODO: error_type::open_codec_fail;
            return;
        }

        _codec_ctx->get_format = get_hw_format;

        create_hw_codec();

        _codec_ctx->hw_device_ctx = av_buffer_ref(_hw_device_ctx);
        av_buffer_unref(&_hw_device_ctx);
    }
    else
    {
        _codec_ctx = avcodec_alloc_context3(nullptr);

        result = avcodec_parameters_to_context(_codec_ctx, _format_ctx->streams[_stream_index]->codecpar);
        if (result < 0)
        {
            // TODO: error_type::open_codec_fail;
            return;
        }

        _codec = avcodec_find_decoder(_codec_ctx->codec_id);

        if (_codec_ctx->width * _codec_ctx->height <= 1920 * 1080)
        {
            // FHD 사이즈 이하
            _codec_ctx->thread_count = _thread_count_fhd;
        }
        else if (_codec_ctx->width * _codec_ctx->height <= 3840 * 2160)
        {
            // 4K 사이즈 이하
            _codec_ctx->thread_count = _thread_count_4k;
        }
        else
        {
            // 4K 사이즈 초과
            _codec_ctx->thread_count = _thread_count_4k_higher;
        }

        if (_codec_ctx->thread_count > _logical_processor_count_half)
        {
            _codec_ctx->thread_count = _logical_processor_count_half;
        }

        _codec_ctx->thread_type = FF_THREAD_SLICE;
    }

    result = avcodec_open2(_codec_ctx, _codec, nullptr);
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

    bool repeat_sync_group_send = false;

    while (_decode_flag)
    {
        while (_pause_flag || _seek_flag)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));

            if (_seek_flag)
            {
                _seek_ready_flag_decoder = true;
                std::unique_lock<std::mutex> lk(_seek_mutex_decoder);
                if (_seek_flag_decoder == false)
                {
                    _seek_condition_decoder.wait(lk);
                }
                _seek_flag_decoder = false;
                break;
            }

            _time_started = 0.0;
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

                    if (_eof_read == true)
                    {
                        _eof_read2 = true;
                    }

                    if (_eof_decode)
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

            if (_hw_decode == true)
            {
                //result = decode_internal(packet, _hw_frame);
                result = decode_internal(packet, frame);
            }
            else
            {
                result = decode_internal(packet, frame);
            }
            if (result == error_type::ok)
            {
                if (_hw_decode == true)
                {
                    //av_hwframe_transfer_data(frame, _hw_frame, 0);
                    //av_frame_copy_props(frame, _hw_frame);
                    //av_frame_unref(_hw_frame);
                }

                if (_scale == true)
                {
                    scale(frame);
                }

                if (_first_decode)
                {
                    _first_decode = false;

                    ffmpeg_wrapper_callback_data* data = new ffmpeg_wrapper_callback_data();
                    data->scene_index = _scene_index;

                    if (_sync_group_count == 0)
                    {
                        data->command = (u16)command_type::play;
                    }
                    else
                    {
                        data->command = (u16)command_type::play_sync_group;
                    }

                    data->connection = _connection_play_start;
                    data->result = (u16)packet_result::ok;

                    data->left = _rect.left;
                    data->top = _rect.top;
                    data->width = _rect.right - _rect.left;
                    data->height = _rect.bottom - _rect.top;
                    memcpy(data->url, _file_path.c_str(), _file_path.size());
                    data->url_size = _file_path.size();

                    data->sync_group_index = _sync_group_index;
                    data->sync_group_count = _sync_group_count;

                    _callback_ffmpeg(data);

                    delete data;

                    _duration_frame = frame->duration;
                    _duration_frame_half = _duration_frame * _time_base_d * 1'000.0 / 2;
                }

                while (true)
                {
                    if (_repeat_flag == true)
                    {
                        if (repeat_sync_group_send == false)
                        {
                            repeat_sync_group_send = true;

                            ffmpeg_wrapper_callback_data* data = new ffmpeg_wrapper_callback_data();
                            data->scene_index = _scene_index;

                            data->command = (u16)command_type::seek_repeat_self_sync_group;

                            data->result = (u16)packet_result::ok;

                            data->sync_group_index = _sync_group_index;
                            data->sync_group_count = _sync_group_count;

                            _callback_ffmpeg(data);

                            delete data;
                        }

                        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
                        continue;
                    }
                    else
                    {
                        repeat_sync_group_send = false;
                    }

                    result = input_frame(frame);
                    if (result == error_type::ok)
                    {
                        if (_sync_group_frame_numbering == true && _sync_group_index != u32_invalid_id)
                        {
                            ffmpeg_wrapper_callback_data* data = new ffmpeg_wrapper_callback_data();
                            data->scene_index = _scene_index;
                            data->command = (u16)command_type::sync_group_frame_numbering;
                            data->connection = _connection_play_start;
                            data->result = (u16)packet_result::ok;

                            data->sync_group_index = _sync_group_index;
                            data->sync_group_count = _sync_group_count;

                            _callback_ffmpeg(data);

                            delete data;
                        }

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

error_type FFmpegCore::decode_internal(AVPacket* packet, AVFrame*& frame)
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

void FFmpegCore::sync_group_frame_numbering()
{
    _sync_group_frame_numbering = true;
}

void FFmpegCore::seek_pts(s64 pts)
{
    if (_read_flag == false && _decode_flag == false)
    {
        return;
    }

    if (_seek_flag == true)
    {
        return;
    }

    // 재생 일시정지
    _seek_flag = true;

    // 정지할 때까지 대기
    while (!(_seek_ready_flag_reader && _seek_ready_flag_decoder))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
    }

    _frame_count = 0;
    _frame_numbering = 0;

    _time_started = 0.0;

    // 코덱 버퍼 클리어
    flush_codec();

    // 큐 클리어
    clear_packet_queue();
    clear_frame_queue();

    // 위치 이동
    set_timestamp(pts);

    if (_eof_read == true)
    {
        _eof_read = false;
    }

    if (_eof_read2 == true)
    {
        _eof_read2 = false;
    }

    if (_eof_decode == true)
    {
        _eof_decode = false;
    }

    // 재생 시작
    _seek_flag = false;

    _seek_ready_flag_reader = false;
    _seek_ready_flag_decoder = false;

    _seek_condition_reader.notify_one();
    _seek_condition_decoder.notify_one();

    if (_repeat_flag == true)
    {
        _repeat_flag = false;
    }
}

void FFmpegCore::frame_numbering()
{
    _frame_numbering++;
}

void FFmpegCore::hw_decode_adapter_index(int hw_decode_adapter_index)
{
    if (hw_decode_adapter_index < 0)
    {
        return;
    }

    _hw_decode_adapter_index = hw_decode_adapter_index;
}

void FFmpegCore::set_repeat_flag()
{
    _repeat_flag = true;
}

void FFmpegCore::unset_repeat_flag()
{
    _repeat_flag = false;
}

void FFmpegCore::repeat_sync_group()
{
    if (_read_flag == false && _decode_flag == false)
    {
        return;
    }

    if (_seek_flag == true)
    {
        return;
    }

    // 재생 일시정지
    _seek_flag = true;

    // 정지할 때까지 대기
    while (!(_seek_ready_flag_reader && _seek_ready_flag_decoder))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
    }

    _frame_count = 0;
    _frame_numbering = 0;

    _time_started = 0.0;

    // 코덱 버퍼 클리어
    flush_codec();

    // 큐 클리어
    clear_packet_queue();
    clear_frame_queue();

    // 위치 이동
    set_timestamp(0);

    if (_eof_read == true)
    {
        _eof_read = false;
    }

    if (_eof_read2 == true)
    {
        _eof_read2 = false;
    }

    if (_eof_decode == true)
    {
        _eof_decode = false;
    }

    // 재생 시작
    _seek_flag = false;

    _seek_ready_flag_reader = false;
    _seek_ready_flag_decoder = false;

    _seek_mutex_reader.lock();
    _seek_condition_reader.notify_one();
    _seek_flag_reader = true;
    _seek_mutex_reader.unlock();

    _seek_mutex_decoder.lock();
    _seek_condition_decoder.notify_one();
    _seek_flag_decoder = true;
    _seek_mutex_decoder.unlock();
}

void FFmpegCore::set_scale(bool scale)
{
    _scale = scale;
}

void FFmpegCore::thread_repeat()
{
    if (_read_flag == false && _decode_flag == false)
    {
        return;
    }

    if (_seek_flag == true)
    {
        return;
    }

    // 재생 일시정지
    _seek_flag = true;

    // 정지할 때까지 대기
    while (!(_seek_ready_flag_reader && _seek_ready_flag_decoder))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(_sleep_time));
    }

    _frame_count = 0;
    _frame_numbering = 0;

    _time_started = 0.0;

    // 코덱 버퍼 클리어
    flush_codec();

    //if (_codec_ctx != nullptr)
    //{
    //    avcodec_free_context(&_codec_ctx);
    //}
    //
    //_codec_opened = false;

    // 큐 클리어
    clear_packet_queue();
    clear_frame_queue();

    // 위치 이동
    set_timestamp(0);

    if (_eof_read == true)
    {
        _eof_read = false;
    }

    if (_eof_read2 == true)
    {
        _eof_read2 = false;
    }

    if (_eof_decode == true)
    {
        _eof_decode = false;
    }

    // 재생 시작
    _seek_flag = false;

    _seek_ready_flag_reader = false;
    _seek_ready_flag_decoder = false;

    _seek_mutex_reader.lock();
    _seek_condition_reader.notify_one();
    _seek_flag_reader = true;
    _seek_mutex_reader.unlock();

    _seek_mutex_decoder.lock();
    _seek_condition_decoder.notify_one();
    _seek_flag_decoder = true;
    _seek_mutex_decoder.unlock();
}

void FFmpegCore::start_thread_repeat()
{
    std::thread thread = std::thread(&FFmpegCore::thread_repeat, this);

    thread.detach();
}

void FFmpegCore::get_timebase(AVRational& timebase)
{
    timebase = _time_base;
}

static AVPixelFormat get_hw_format(AVCodecContext* ctx, const AVPixelFormat* pix_fmts)
{
    const enum AVPixelFormat* p;

    for (p = pix_fmts; *p != -1; p++) {
        if (*p == _hw_pix_fmt)
            return *p;
    }

    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

void FFmpegCore::set_timestamp(s64 pts)
{
    av_seek_frame(_format_ctx, _stream_index, _start_time + pts, AVSEEK_FLAG_BACKWARD);
}

void FFmpegCore::scale(AVFrame* frame)
{
    if (frame->format == _scale_dest_format)
    {
        return;
    }

    int ret = 0;

    if (_sws_ctx == nullptr)
    {
        _sws_ctx = sws_getContext(frame->width, frame->height, (AVPixelFormat)frame->format,
            frame->width, frame->height, _scale_dest_format,
            SWS_BICUBIC, nullptr, nullptr, nullptr
        );
        _scale_alloc_size = av_image_alloc(_scale_frame->data, _scale_frame->linesize, frame->width, frame->height, (AVPixelFormat)_scale_dest_format, 1);
    }

    ret = sws_scale(_sws_ctx, frame->data, frame->linesize, 0, frame->height, _scale_frame->data, _scale_frame->linesize);

    av_buffer_unref(frame->buf);
    *frame->data = nullptr;

    av_image_alloc(frame->data, frame->linesize, frame->width, frame->height, (AVPixelFormat)_scale_dest_format, 1);
    av_image_copy(frame->data, frame->linesize, _scale_frame->data, _scale_frame->linesize, (AVPixelFormat)_scale_dest_format, frame->width, frame->height);

    AVBufferRef* buf = av_buffer_create(*frame->data, _scale_alloc_size, nullptr, nullptr, 0);

    frame->buf[0] = buf;

    frame->format = _scale_dest_format;
}

#pragma region circular queue

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

error_type FFmpegCore::input_packet(AVPacket* packet)
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

error_type FFmpegCore::output_packet(AVPacket*& packet)
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

error_type FFmpegCore::input_frame(AVFrame* frame)
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

error_type FFmpegCore::output_frame(AVFrame*& frame)
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

error_type FFmpegCore::output_frame(AVFrame*& frame, s32& index)
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

s32 FFmpegCore::get_packet_queue_size()
{
    if (is_empty_packet_queue())
    {
        return 0;
    }
    else if (_output_packet_index <= _input_packet_index)
    {
        return _input_packet_index - _output_packet_index + 1;
    }
    else
    {
        return _packet_queue_size - _output_packet_index + _input_packet_index + 1;
    }
}

int FFmpegCore::get_frame_queue_size()
{
    if (is_empty_frame_queue())
    {
        return 0;
    }
    else if (_output_frame_index <= _input_frame_index)
    {
        return _input_frame_index - _output_frame_index + 1;
    }
    else
    {
        return _frame_queue_size - _output_frame_index + _input_frame_index + 1;
    }
}

void FFmpegCore::create_hw_codec()
{
    int result = -1;

    while (result != 0)
    {
        if (_hw_decode_adapter_index == -1)
        {
            result = av_hwdevice_ctx_create(&_hw_device_ctx, _hw_device_type, NULL, NULL, 0);
        }
        else
        {
            result = av_hwdevice_ctx_create(&_hw_device_ctx, _hw_device_type, std::to_string(_hw_decode_adapter_index).c_str(), NULL, 0);
        }
    }
}

#pragma endregion
