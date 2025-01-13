#pragma once

#include "MyCefClient.h"
#include "MyCefRenderHandler.h"

class CppCefWrapper
{
public:
    CppCefWrapper(HWND window_handle, std::string url);
    ~CppCefWrapper();

    void get_deque_size(int& size);

    void get_deque_data(void*& buffer, int& width, int& height);

private:

    CefRefPtr<MyCefClient> cef_client;
    MyRenderHandler* render_handler;

    CefWindowInfo window_info;
    CefBrowserSettings browser_settings;
};
