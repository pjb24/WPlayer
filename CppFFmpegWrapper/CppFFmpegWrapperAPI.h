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

/// <summary>
/// FFmpeg instance 생성.
/// delete와 pair
/// </summary>
/// <returns> 생성된 instance 포인터 </returns>
EXPORT void* cpp_ffmpeg_wrapper_create();

/// <summary>
/// FFmpeg instance 제거.
/// create와 pair
/// </summary>
/// <param name="instance"> 제거할 FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_delete(void* instance);

/// <summary>
/// FFmpeg instance initialize.
/// shutdown과 pair
/// </summary>
/// <param name="instance"> initialize할 FFmpeg instance 포인터 </param>
/// <param name="cb"> 콜백 함수 </param>
/// <returns> true: 성공, false: 실패 </returns>
EXPORT bool cpp_ffmpeg_wrapper_initialize(void* instance, CALLBACK_PTR cb);

/// <summary>
/// FFmpeg instance shutdown.
/// initialize와 pair
/// </summary>
/// <param name="instance"> shutdown할 FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_shutdown(void* instance);

/// <summary>
/// FFmpeg instance의 _scene_index 설정
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="scene_index"> 설정할 scene_index </param>
EXPORT void cpp_ffmpeg_wrapper_set_scene_index(void* instance, uint32_t scene_index);

/// <summary>
/// FFmpeg instance의 _rect 설정
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="rect"> 설정할 rect </param>
EXPORT void cpp_ffmpeg_wrapper_set_rect(void* instance, RECT rect);

/// <summary>
/// FFmpeg instance의 _sync_group_index 설정.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="sync_group_index"> 설정할 sync_group_index </param>
EXPORT void cpp_ffmpeg_wrapper_set_sync_group_index(void* instance, uint32_t sync_group_index);

/// <summary>
/// FFmpeg instance의 _sync_group_count 설정.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="sync_group_count"> 설정할 sync_group_count </param>
EXPORT void cpp_ffmpeg_wrapper_set_sync_group_count(void* instance, uint16_t sync_group_count);

/// <summary>
/// FFmpeg instance의 _time_started를 0.0으로 설정.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_set_sync_group_time_started(void* instance);

/// <summary>
/// 이전 frame을 사용했음을 알리는 함수.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_set_sync_group_frame_numbering(void* instance);

/// <summary>
/// FFmpeg instance에서 frame scaling을 사용할지 설정하는 함수.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="scale"> bool, true: scale 사용함, false: scale 사용 안함 </param>
EXPORT void cpp_ffmpeg_wrapper_set_scale(void* instance, bool scale);

/// <summary>
/// FFmpeg instance에서 연결할 스트림의 url을 설정하는 함수.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="url"> 연결할 스트림의 url </param>
EXPORT void cpp_ffmpeg_wrapper_set_file_path(void* instance, char* url);

/// <summary>
/// 스트림에 연결 시도
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <returns> 0: 성공, 1: url 위치에 스트림이 없음, 7: file path가 설정되지 않음 </returns>
EXPORT int cpp_ffmpeg_wrapper_open_file(void* instance);

/// <summary>
/// 재생 시작
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="connection"> 명령 패킷을 보낸 connection </param>
EXPORT void cpp_ffmpeg_wrapper_play_start(void* instance, void* connection);

/// <summary>
/// 재생 일시정지
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="connection"> 명령 패킷을 보낸 connection </param>
EXPORT void cpp_ffmpeg_wrapper_play_pause(void* instance, void* connection);

/// <summary>
/// 재생 중지
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="connection"> 명령 패킷을 보낸 connection </param>
EXPORT void cpp_ffmpeg_wrapper_play_stop(void* instance, void* connection);

/// <summary>
/// frame을 요청함
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="frame"> frame을 가져갈 ref </param>
/// <returns> 0 이상: 성공, -1: 큐가 비어있음, -2: EOS/EOF (End Of Stream/End Of File), -3: 로직상 반환되는 경우가 없음 </returns>
EXPORT int32_t cpp_ffmpeg_wrapper_get_frame(void* instance, AVFrame*& frame);

/// <summary>
/// index를 다음 frame으로 이동, frame의 대기시간 고려하여 이동함.
/// 사용하지 않음
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <returns> 0: 성공, 10: frame 큐가 비어있음, 13: 이전 frame을 사용해야함 </returns>
EXPORT int32_t cpp_ffmpeg_wrapper_frame_to_next(void* instance);

/// <summary>
/// index를 다음 frame으로 이동, frame의 대기시간을 고려하지 않고 바로 이동함.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <returns> 0: 성공, 10: frame 큐가 비어있음 </returns>
EXPORT int32_t cpp_ffmpeg_wrapper_frame_to_next_non_waiting(void* instance);

/// <summary>
/// 스트림에서 이동할 수 있으면 입력된 pts 위치로 읽기 위치를 이동함.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="pts"> 이동할 위치의 pts 값 </param>
EXPORT void cpp_ffmpeg_wrapper_seek_pts(void * instance, int64_t pts);

/// <summary>
/// 재생 위치를 forward로 이동함.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="connection"> 명령 패킷을 보낸 connection </param>
EXPORT void cpp_ffmpeg_wrapper_jump_forward(void * instance, void* connection);

/// <summary>
/// 재생 위치를 backwards로 이동함.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="connection"> 명령 패킷을 보낸 connection </param>
EXPORT void cpp_ffmpeg_wrapper_jump_backwards(void * instance, void* connection);

/// <summary>
/// _frame_numbering 카운트 증가
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_frame_numbering(void* instance);

/// <summary>
/// FFmpeg instance에서 하드웨어 디코딩 사용을 활성화함
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_set_hw_decode(void* instance);

/// <summary>
/// FFmpeg instance에서 하드웨어 디코딩에 사용할 디코딩 타입을 설정함.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="hw_device_type"> 하드웨어 디코딩에 사용할 디코딩 타입.
/// 2: AV_HWDEVICE_TYPE_CUDA
/// 4: AV_HWDEVICE_TYPE_DXVA2
/// 7: AV_HWDEVICE_TYPE_D3D11VA
/// 12: AV_HWDEVICE_TYPE_D3D12VA
/// </param>
EXPORT void cpp_ffmpeg_wrapper_set_hw_device_type(void* instance, int hw_device_type);

/// <summary>
/// FFmpeg instance에서 하드웨어 디코딩에 사용할 Device를 설정함.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="hw_decode_adapter_index"> 사용할 Device의 index </param>
EXPORT void cpp_ffmpeg_wrapper_set_hw_decode_adapter_index(void* instance, int hw_decode_adapter_index);

/// <summary>
/// _repeat_flag를 true로 설정함.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_set_repeat_flag(void* instance);

/// <summary>
/// 재생 위치를 0으로 설정하고 다시 재생함.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_repeat_sync_group(void* instance);

/// <summary>
/// _repeat_flag를 false로 설정함.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_unset_repeat_flag(void* instance);

/// <summary>
/// sync_group들이 다음 frame으로 이동할 수 있는지 확인하는 함수.
/// 사용하지 않음.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <returns> 0: 성공, 10: frame 큐가 비어있음, 13: 이전 frame을 사용해야함. </returns>
EXPORT int32_t cpp_ffmpeg_wrapper_check_frame_to_next_sync_group(void* instance);

/// <summary>
/// 반복 재생 스레드 시작
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_start_thread_repeat(void* instance);

/// <summary>
/// FFmpeg instance의 timebase 요청
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="timebase"> timebase를 가져갈 ref </param>
EXPORT void cpp_ffmpeg_wrapper_get_timebase(void* instance, AVRational& timebase);

/// <summary>
/// 연결 스트림이 실시간 스트림인지 확인하는 플래그 값 요청
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="is_realtime"> 연결 스트림이 실시간 스트림인지 확인하는 플래그의 ref </param>
EXPORT void cpp_ffmpeg_wrapper_get_is_realtime(void* instance, bool& is_realtime);

/// <summary>
/// 스트림 연결이 성공했는지 확인하는 플래그 값 요청
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="flag_succeed_open_input"> 스트림 연결이 성공했는지 확인하는 플래그의 ref </param>
EXPORT void cpp_ffmpeg_wrapper_get_flag_succeed_open_input(void* instance, bool& flag_succeed_open_input);

/// <summary>
/// 스트림 재생이 시작되었는지 확인하는 플래그 값 요청
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="flag_play_started"> 스트림 재생이 시작되었는지 확인하는 플래그의 ref </param>
EXPORT void cpp_ffmpeg_wrapper_get_flag_play_started(void* instance, bool& flag_play_started);

/// <summary>
/// 연결 스트림이 실시간 스트림일 때, EOS가 발생하면 스트림 재연결을 위해 사용하는 함수.
/// cpp_ffmpeg_wrapper_shutdown_small()을 먼저 사용해야함.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
/// <param name="cb"> 콜백 함수 </param>
EXPORT void cpp_ffmpeg_wrapper_initialize_small(void* instance, CALLBACK_PTR cb);

/// <summary>
/// 연결 스트림이 실시간 스트림일 때, EOS가 발생하면 스트림 재연결을 위해 사용하는 함수.
/// cpp_ffmpeg_wrapper_initialize_small()보다 먼저 사용되어야 함.
/// </summary>
/// <param name="instance"> FFmpeg instance 포인터 </param>
EXPORT void cpp_ffmpeg_wrapper_shutdown_small(void* instance);
