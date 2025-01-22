#include "CppCefWrapperAPI.h"

#include "include/cef_app.h"
#include "include/cef_client.h"
#include "include/cef_render_handler.h"
#include "include/cef_sandbox_win.h"

#include "CppCefWrapper.h"

int cpp_cef_wrapper_execute_process(HINSTANCE hInst)
{
    CefMainArgs args(hInst);

    void* sandbox_info = nullptr;

#if _DEBUG
#else
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif // _DEBUG

    return CefExecuteProcess(args, nullptr, sandbox_info);
}

int cpp_cef_wrapper_initialize(HINSTANCE hInst, const char* cef_client_path, int cef_client_path_size)
{
    CefMainArgs args(hInst);

    void* sandbox_info = nullptr;

#if _DEBUG
#else
    CefScopedSandboxInfo scoped_sandbox;
    sandbox_info = scoped_sandbox.sandbox_info();
#endif // _DEBUG

    std::string str;
    str.assign(cef_client_path, cef_client_path_size);

    CefSettings settings;
    if (!str.empty())
    {
        CefString(&settings.browser_subprocess_path).FromString(str);
    }

    settings.multi_threaded_message_loop = true;

#if _DEBUG
    settings.no_sandbox = true;
#endif // _DEBUG

    bool result = CefInitialize(args, settings, nullptr, sandbox_info);
    if (!result)
    {
        return -1;
    }

    return 0;
}

void cpp_cef_wrapper_shutdown()
{
    CefShutdown();
}

void* cpp_cef_wrapper_create(HWND window_handle, const char* url, int url_size, RECT rect)
{
    std::string str;
    str.assign(url, url_size);

    return new CppCefWrapper(window_handle, str, rect);
}

void cpp_cef_wrapper_delete(void* instance)
{
    CppCefWrapper* data = (CppCefWrapper*)instance;
    delete data;
}

void cpp_cef_wrapper_get_deque_size(void* instance, int& size)
{
    CppCefWrapper* data = (CppCefWrapper*)instance;
    data->get_deque_size(size);
}

void cpp_cef_wrapper_get_deque_data(void* instance, void*& buffer, int& width, int& height)
{
    CppCefWrapper* data = (CppCefWrapper*)instance;
    data->get_deque_data(buffer, width, height);
}

void cpp_cef_wrapper_delete_buffer(void* instance, void* buffer)
{
    CppCefWrapper* data = (CppCefWrapper*)instance;
    data->delete_buffer(buffer);
}

void cpp_cef_wrapper_close_browser(void* instance)
{
    CppCefWrapper* data = (CppCefWrapper*)instance;
    data->close_browser();
}
