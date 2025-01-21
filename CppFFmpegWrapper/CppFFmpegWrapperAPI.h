#pragma once

#define EXPORT extern "C" __declspec(dllexport)

#include <stdint.h>

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#include <Windows.h>

extern "C"
{
#include "libavutil/frame.h"
}

#pragma comment(lib, "avutil.lib")

// CallbackType
typedef void(*CALLBACK_PTR)(void*);

/// <summary>
/// FFmpeg instance ����.
/// delete�� pair
/// </summary>
/// <returns> ������ instance ������ </returns>
EXPORT void* cpp_ffmpeg_wrapper_create();

/// <summary>
/// FFmpeg instance ����.
/// create�� pair
/// </summary>
/// <param name="instance"> ������ FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_delete(void* instance);

/// <summary>
/// FFmpeg instance initialize.
/// shutdown�� pair
/// </summary>
/// <param name="instance"> initialize�� FFmpeg instance ������ </param>
/// <param name="cb"> �ݹ� �Լ� </param>
/// <returns> true: ����, false: ���� </returns>
EXPORT bool cpp_ffmpeg_wrapper_initialize(void* instance, CALLBACK_PTR cb);

/// <summary>
/// FFmpeg instance shutdown.
/// initialize�� pair
/// </summary>
/// <param name="instance"> shutdown�� FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_shutdown(void* instance);

/// <summary>
/// FFmpeg instance�� _scene_index ����
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="scene_index"> ������ scene_index </param>
EXPORT void cpp_ffmpeg_wrapper_set_scene_index(void* instance, uint32_t scene_index);

/// <summary>
/// FFmpeg instance�� _rect ����
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="rect"> ������ rect </param>
EXPORT void cpp_ffmpeg_wrapper_set_rect(void* instance, RECT rect);

/// <summary>
/// FFmpeg instance�� _sync_group_index ����.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="sync_group_index"> ������ sync_group_index </param>
EXPORT void cpp_ffmpeg_wrapper_set_sync_group_index(void* instance, uint32_t sync_group_index);

/// <summary>
/// FFmpeg instance�� _sync_group_count ����.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="sync_group_count"> ������ sync_group_count </param>
EXPORT void cpp_ffmpeg_wrapper_set_sync_group_count(void* instance, uint16_t sync_group_count);

/// <summary>
/// FFmpeg instance�� _time_started�� 0.0���� ����.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_set_sync_group_time_started(void* instance);

/// <summary>
/// ���� frame�� ��������� �˸��� �Լ�.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_set_sync_group_frame_numbering(void* instance);

/// <summary>
/// FFmpeg instance���� frame scaling�� ������� �����ϴ� �Լ�.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="scale"> bool, true: scale �����, false: scale ��� ���� </param>
EXPORT void cpp_ffmpeg_wrapper_set_scale(void* instance, bool scale);

/// <summary>
/// FFmpeg instance���� ������ ��Ʈ���� url�� �����ϴ� �Լ�.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="url"> ������ ��Ʈ���� url </param>
EXPORT void cpp_ffmpeg_wrapper_set_file_path(void* instance, char* url);

/// <summary>
/// ��Ʈ���� ���� �õ�
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <returns> 0: ����, 1: url ��ġ�� ��Ʈ���� ����, 7: file path�� �������� ���� </returns>
EXPORT int cpp_ffmpeg_wrapper_open_file(void* instance);

/// <summary>
/// ��� ����
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="connection"> ��� ��Ŷ�� ���� connection </param>
EXPORT void cpp_ffmpeg_wrapper_play_start(void* instance, void* connection);

/// <summary>
/// ��� �Ͻ�����
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="connection"> ��� ��Ŷ�� ���� connection </param>
EXPORT void cpp_ffmpeg_wrapper_play_pause(void* instance, void* connection);

/// <summary>
/// ��� ����
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="connection"> ��� ��Ŷ�� ���� connection </param>
EXPORT void cpp_ffmpeg_wrapper_play_stop(void* instance, void* connection);

/// <summary>
/// frame�� ��û��
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="frame"> frame�� ������ ref </param>
/// <returns> 0 �̻�: ����, -1: ť�� �������, -2: EOS/EOF (End Of Stream/End Of File), -3: ������ ��ȯ�Ǵ� ��찡 ���� </returns>
EXPORT int32_t cpp_ffmpeg_wrapper_get_frame(void* instance, AVFrame*& frame);

/// <summary>
/// index�� ���� frame���� �̵�, frame�� ���ð� ����Ͽ� �̵���.
/// ������� ����
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <returns> 0: ����, 10: frame ť�� �������, 13: ���� frame�� ����ؾ��� </returns>
EXPORT int32_t cpp_ffmpeg_wrapper_frame_to_next(void* instance);

/// <summary>
/// index�� ���� frame���� �̵�, frame�� ���ð��� ������� �ʰ� �ٷ� �̵���.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <returns> 0: ����, 10: frame ť�� ������� </returns>
EXPORT int32_t cpp_ffmpeg_wrapper_frame_to_next_non_waiting(void* instance);

/// <summary>
/// ��Ʈ������ �̵��� �� ������ �Էµ� pts ��ġ�� �б� ��ġ�� �̵���.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="pts"> �̵��� ��ġ�� pts �� </param>
EXPORT void cpp_ffmpeg_wrapper_seek_pts(void * instance, int64_t pts);

/// <summary>
/// ��� ��ġ�� forward�� �̵���.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="connection"> ��� ��Ŷ�� ���� connection </param>
EXPORT void cpp_ffmpeg_wrapper_jump_forward(void * instance, void* connection);

/// <summary>
/// ��� ��ġ�� backwards�� �̵���.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="connection"> ��� ��Ŷ�� ���� connection </param>
EXPORT void cpp_ffmpeg_wrapper_jump_backwards(void * instance, void* connection);

/// <summary>
/// _frame_numbering ī��Ʈ ����
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_frame_numbering(void* instance);

/// <summary>
/// FFmpeg instance���� �ϵ���� ���ڵ� ����� Ȱ��ȭ��
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_set_hw_decode(void* instance);

/// <summary>
/// FFmpeg instance���� �ϵ���� ���ڵ��� ����� ���ڵ� Ÿ���� ������.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="hw_device_type"> �ϵ���� ���ڵ��� ����� ���ڵ� Ÿ��.
/// 2: AV_HWDEVICE_TYPE_CUDA
/// 4: AV_HWDEVICE_TYPE_DXVA2
/// 7: AV_HWDEVICE_TYPE_D3D11VA
/// 12: AV_HWDEVICE_TYPE_D3D12VA
/// </param>
EXPORT void cpp_ffmpeg_wrapper_set_hw_device_type(void* instance, int hw_device_type);

/// <summary>
/// FFmpeg instance���� �ϵ���� ���ڵ��� ����� Device�� ������.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="hw_decode_adapter_index"> ����� Device�� index </param>
EXPORT void cpp_ffmpeg_wrapper_set_hw_decode_adapter_index(void* instance, int hw_decode_adapter_index);

/// <summary>
/// _repeat_flag�� true�� ������.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_set_repeat_flag(void* instance);

/// <summary>
/// ��� ��ġ�� 0���� �����ϰ� �ٽ� �����.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_repeat_sync_group(void* instance);

/// <summary>
/// _repeat_flag�� false�� ������.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_unset_repeat_flag(void* instance);

/// <summary>
/// sync_group���� ���� frame���� �̵��� �� �ִ��� Ȯ���ϴ� �Լ�.
/// ������� ����.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <returns> 0: ����, 10: frame ť�� �������, 13: ���� frame�� ����ؾ���. </returns>
EXPORT int32_t cpp_ffmpeg_wrapper_check_frame_to_next_sync_group(void* instance);

/// <summary>
/// �ݺ� ��� ������ ����
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_start_thread_repeat(void* instance);

/// <summary>
/// FFmpeg instance�� timebase ��û
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="timebase"> timebase�� ������ ref </param>
EXPORT void cpp_ffmpeg_wrapper_get_timebase(void* instance, AVRational& timebase);

/// <summary>
/// ���� ��Ʈ���� �ǽð� ��Ʈ������ Ȯ���ϴ� �÷��� �� ��û
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="is_realtime"> ���� ��Ʈ���� �ǽð� ��Ʈ������ Ȯ���ϴ� �÷����� ref </param>
EXPORT void cpp_ffmpeg_wrapper_get_is_realtime(void* instance, bool& is_realtime);

/// <summary>
/// ��Ʈ�� ������ �����ߴ��� Ȯ���ϴ� �÷��� �� ��û
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="flag_succeed_open_input"> ��Ʈ�� ������ �����ߴ��� Ȯ���ϴ� �÷����� ref </param>
EXPORT void cpp_ffmpeg_wrapper_get_flag_succeed_open_input(void* instance, bool& flag_succeed_open_input);

/// <summary>
/// ��Ʈ�� ����� ���۵Ǿ����� Ȯ���ϴ� �÷��� �� ��û
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="flag_play_started"> ��Ʈ�� ����� ���۵Ǿ����� Ȯ���ϴ� �÷����� ref </param>
EXPORT void cpp_ffmpeg_wrapper_get_flag_play_started(void* instance, bool& flag_play_started);

/// <summary>
/// ���� ��Ʈ���� �ǽð� ��Ʈ���� ��, EOS�� �߻��ϸ� ��Ʈ�� �翬���� ���� ����ϴ� �Լ�.
/// cpp_ffmpeg_wrapper_shutdown_small()�� ���� ����ؾ���.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
/// <param name="cb"> �ݹ� �Լ� </param>
EXPORT void cpp_ffmpeg_wrapper_initialize_small(void* instance, CALLBACK_PTR cb);

/// <summary>
/// ���� ��Ʈ���� �ǽð� ��Ʈ���� ��, EOS�� �߻��ϸ� ��Ʈ�� �翬���� ���� ����ϴ� �Լ�.
/// cpp_ffmpeg_wrapper_initialize_small()���� ���� ���Ǿ�� ��.
/// </summary>
/// <param name="instance"> FFmpeg instance ������ </param>
EXPORT void cpp_ffmpeg_wrapper_shutdown_small(void* instance);
