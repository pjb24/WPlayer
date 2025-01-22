#include "MyCefClient.h"

#include "include/wrapper/cef_helpers.h"
#include "include/cef_app.h"

MyCefClient::MyCefClient(CefRefPtr<MyRenderHandler> render_handler)
{
    _render_handler = render_handler;
}

CefRefPtr<CefRenderHandler> MyCefClient::GetRenderHandler()
{
    return _render_handler;
}

CefRefPtr<CefLifeSpanHandler> MyCefClient::GetLifeSpanHandler()
{
    return this;
}

void MyCefClient::OnAfterCreated(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    if (!_browser)
    {
        _browser = browser;
        _browser_id = browser->GetIdentifier();
    }

    _browser_count++;
}

bool MyCefClient::DoClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    if (_browser_id == browser->GetIdentifier())
    {
        _is_closing = true;
    }

    return false;
}

void MyCefClient::OnBeforeClose(CefRefPtr<CefBrowser> browser)
{
    CEF_REQUIRE_UI_THREAD();

    if (_browser_id == browser->GetIdentifier())
    {
        _browser = nullptr;
        _render_handler = nullptr;
    }

    if (--_browser_count == 0)
    {
        PostQuitMessage(0);
    }
}
