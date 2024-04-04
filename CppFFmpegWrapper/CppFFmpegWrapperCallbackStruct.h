#pragma once

#include"TypesDefine.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#include <Windows.h>

struct ffmpeg_wrapper_callback_data
{
    u32 scene_index = u32_invalid_id;
    u16 command = u16_invalid_id;
    void* connection = nullptr;
    u16 result = u16_invalid_id;

    RECT rect = { 0, 0, 0, 0 };
    char url[260];
    u16 url_size = u16_invalid_id;

    u32 sync_group_index = u32_invalid_id;
    u16 sync_group_count = u16_invalid_id;
};
