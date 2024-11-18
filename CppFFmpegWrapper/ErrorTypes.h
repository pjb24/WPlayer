#pragma once

enum error_type : int32_t
{
    ok = 0,
    file_not_exist = 1,
    read_eof = 2,
    open_codec_fail = 3,
    decode_eof = 4,
    input_null_dictionary = 5,
    input_unallocated_packet = 6,
    file_path_unsetted = 7,
    insufficient_input_packets = 8,
    queue_is_full = 9,
    queue_is_empty = 10,
    read_audio_packet = 11,
    codec_not_opened = 12,
    use_previous_frame = 13,
    read_timeout = 14,
    read_fail_connect = 15,

};
