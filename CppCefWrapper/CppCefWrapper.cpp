#include "CppCefWrapper.h"

#include <string>

CppCefWrapper::CppCefWrapper(HWND window_handle, std::string url, RECT rect)
{
    render_handler = new MyRenderHandler(rect);

    window_info.SetAsWindowless(window_handle);

    cef_client = new MyCefClient(render_handler);

    if (url.empty())
    {
        url.assign("www.google.com");
    }
    CefBrowserHost::CreateBrowser(window_info, cef_client, url.c_str(), browser_settings, nullptr, nullptr);
}

CppCefWrapper::~CppCefWrapper()
{
    int size = 0;

    do
    {
        render_handler->get_deque_size(size);
        if (size != 0)
        {
            void* buffer;
            int width;
            int height;

            render_handler->get_deque_data(buffer, width, height);
            render_handler->delete_buffer(buffer);
        }
    } while (size != 0);

    render_handler = nullptr;
    cef_client = nullptr;
}

void CppCefWrapper::get_deque_size(int& size)
{
    render_handler->get_deque_size(size);
}

void CppCefWrapper::get_deque_data(void*& buffer, int& width, int& height)
{
    render_handler->get_deque_data(buffer, width, height);
}

void CppCefWrapper::delete_buffer(void* buffer)
{
    render_handler->delete_buffer(buffer);
}

void CppCefWrapper::close_browser()
{
    cef_client.get()->GetBrowser()->GetHost()->CloseBrowser(false);
}
