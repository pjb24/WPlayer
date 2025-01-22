#pragma once

#define EXPORT extern "C" __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <string>

/// <summary>
/// 
/// </summary>
/// <param name="hInst"></param>
/// <returns> 0보다 크거나 같으면 오류임. CEF와 관련된 함수를 사용하지 않아야 함. </returns>
EXPORT int cpp_cef_wrapper_execute_process(HINSTANCE hInst);

/// <summary>
/// 
/// </summary>
/// <param name="hInst"></param>
/// <param name="cef_client_path"> cefclient 파일의 path </param>
/// <returns> 0: 성공, -1: 실패 </returns>
EXPORT int cpp_cef_wrapper_initialize(HINSTANCE hInst, const char* cef_client_path, int cef_client_path_size);

EXPORT void cpp_cef_wrapper_shutdown();

EXPORT void* cpp_cef_wrapper_create(HWND window_handle, const char* url, int url_size, RECT rect);

EXPORT void cpp_cef_wrapper_delete(void* instance);

EXPORT void cpp_cef_wrapper_get_deque_size(void* instance, int& size);

EXPORT void cpp_cef_wrapper_get_deque_data(void* instance, void*& buffer, int& width, int& height);

EXPORT void cpp_cef_wrapper_delete_buffer(void* instance, void* buffer);

EXPORT void cpp_cef_wrapper_close_browser(void* instance);
