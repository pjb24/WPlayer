#pragma once

#include "include/cef_client.h"

#include "MyCefRenderHandler.h"

class MyCefClient : public CefClient, public CefLifeSpanHandler
{
public:
    MyCefClient(MyRenderHandler* render_handler);

    virtual CefRefPtr<CefRenderHandler> GetRenderHandler();

    virtual CefRefPtr<CefLifeSpanHandler> GetLifeSpanHandler() override;

    void OnAfterCreated(CefRefPtr<CefBrowser> browser) override;
    bool DoClose(CefRefPtr<CefBrowser> browser) override;
    void OnBeforeClose(CefRefPtr<CefBrowser> browser) override;

    CefRefPtr<CefBrowser> GetBrowser() const { return _browser; }
    bool IsClosing() const { return _is_closing; }

private:
    CefRefPtr<CefRenderHandler> _render_handler;
    CefRefPtr<CefBrowser> _browser;
    int _browser_id = -1;
    int _browser_count = 0;
    bool _is_closing = false;

public:
    IMPLEMENT_REFCOUNTING(MyCefClient);
};
