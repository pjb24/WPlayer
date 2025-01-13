#include "MyCefRenderHandler.h"

MyRenderHandler::MyRenderHandler()
{
    _deque_tuple = new std::deque<std::tuple<void*, int, int>*>();
    _mutex_deque_tuple = new std::mutex();
}

MyRenderHandler::~MyRenderHandler()
{
    _mutex_deque_tuple->lock();
    while (_deque_tuple->size() != 0)
    {
        std::tuple<void*, int, int>* data = _deque_tuple->front();
        void* buffer = std::get<0>(*data);
        delete[] buffer;
        buffer = nullptr;
        delete data;
        data = nullptr;
        _deque_tuple->pop_front();
    }
    delete _deque_tuple;
    _deque_tuple = nullptr;
    _mutex_deque_tuple->unlock();

    if (_mutex_deque_tuple)
    {
        delete _mutex_deque_tuple;
        _mutex_deque_tuple = nullptr;
    }
}

void MyRenderHandler::GetViewRect(CefRefPtr<CefBrowser> browser, CefRect& rect)
{
    rect = CefRect(0, 0, 1280, 720);
}

void MyRenderHandler::OnPaint(CefRefPtr<CefBrowser> browser, PaintElementType type, 
    const RectList& dirtyRects, const void* buffer, int width, int height)
{
    void* buffer_input = new uint8_t[width * height * 4];
    memcpy(buffer_input, buffer, width * height * 4);

    _mutex_deque_tuple->lock();
    _deque_tuple->push_back(new std::tuple<void*, int, int>(buffer_input, width, height));
    _mutex_deque_tuple->unlock();
}

void MyRenderHandler::get_deque_size(int& size)
{
    std::lock_guard<std::mutex> lk(*_mutex_deque_tuple);
    size = _deque_tuple->size();
}

void MyRenderHandler::get_deque_data(void*& buffer, int& width, int& height)
{
    std::lock_guard<std::mutex> lk(*_mutex_deque_tuple);

    int size = _deque_tuple->size();
    if (size == 0)
    {
        return;
    }

    std::tuple<void*, int, int>* data = _deque_tuple->front();
    _deque_tuple->pop_front();

    buffer = std::get<0>(*data);
    width = std::get<1>(*data);
    height = std::get<2>(*data);

    delete data;
}
