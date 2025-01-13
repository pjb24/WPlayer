#pragma once

#include "include/cef_render_handler.h"

#include <deque>
#include <tuple>
#include <mutex>

class MyRenderHandler : public CefRenderHandler
{
public:
    MyRenderHandler();
    ~MyRenderHandler();

    virtual void GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect);

    virtual void OnPaint(CefRefPtr<CefBrowser> browser,
        PaintElementType type,
        const RectList& dirtyRects,
        const void* buffer,
        int width,
        int height);

    void get_deque_size(int& size);

    void get_deque_data(void*& buffer, int& width, int& height);

private:

    std::deque<std::tuple<void*, int, int>*>* _deque_tuple = nullptr;
    std::mutex* _mutex_deque_tuple = nullptr;

public:
    IMPLEMENT_REFCOUNTING(MyRenderHandler);
};
