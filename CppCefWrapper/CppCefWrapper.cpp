#include "CppCefWrapper.h"

#include <string>

CppCefWrapper::CppCefWrapper(HWND window_handle, std::string url)
{
    render_handler = new MyRenderHandler();

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
    cef_client.get()->GetBrowser()->GetHost()->CloseBrowser(false);

    delete render_handler;
}

void CppCefWrapper::get_deque_size(int& size)
{
    render_handler->get_deque_size(size);
}

void CppCefWrapper::get_deque_data(void*& buffer, int& width, int& height)
{
    render_handler->get_deque_data(buffer, width, height);
}
