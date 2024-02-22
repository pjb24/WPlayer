//#include "D3D12Data.h"
//
//u32 enum_adapters()
//{
//    if (!_factory)
//    {
//        return u32();
//    }
//
//    HRESULT hr = S_OK;
//
//    IDXGIAdapter1* adapter = nullptr;
//    DXGI_ADAPTER_DESC1 adapter_desc{};
//
//    for (u32 i = 0; ; i++)
//    {
//        hr = _factory->EnumAdapters1(i, &adapter);
//        if (hr != S_OK)
//        {
//            break;
//        }
//
//        adapter->GetDesc1(&adapter_desc);
//
//        if (adapter_desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
//        {
//            adapter->Release();
//            continue;
//        }
//
//        hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nullptr);
//        if (hr == S_OK)
//        {
//            _adapters.push_back(adapter);
//        }
//    }
//
//    return u32();
//}
//
//u32 enum_outputs()
//{
//    if (_adapters.empty())
//    {
//        return u32();
//    }
//
//    HRESULT hr = S_OK;
//    IDXGIAdapter1* adapter = nullptr;
//
//    for (IDXGIAdapter1* adapter : _adapters)
//    {
//        for (u32 i = 0; ; i++)
//        {
//            IDXGIOutput* output = nullptr;
//            DXGI_OUTPUT_DESC output_desc{};
//
//            hr = adapter->EnumOutputs(i, &output);
//            if (hr != S_OK)
//            {
//                break;
//            }
//
//            output->GetDesc(&output_desc);
//
//            _output_descs.push_back(output_desc);
//
//            _outputs.push_back(output);
//        }
//    }
//
//    return u32();
//}
//
//u32 create_factory()
//{
//    HRESULT hr = S_OK;
//
//    u32 dxgi_factory_flags = 0;
//
//#if defined(_DEBUG)
//    // Enable the debug layer (requires the Graphics Tools "optional feature").
//    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
//    {
//        ComPtr<ID3D12Debug1> debug_controller;
//        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller))))
//        {
//            debug_controller->EnableDebugLayer();
//#if 1
//            debug_controller->SetEnableGPUBasedValidation(true);
//#endif // 1
//
//            // Enable additional debug layers.
//            dxgi_factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
//        }
//    }
//#endif
//
//    hr = CreateDXGIFactory2(dxgi_factory_flags, IID_PPV_ARGS(&_factory));
//
//    return u32();
//}
//
//u32 create_device()
//{
//    if (_adapters.empty())
//    {
//        return u32();
//    }
//
//    HRESULT hr = S_OK;
//
//    for (auto adapter : _adapters)
//    {
//        ID3D12Device* device = nullptr;
//
//        hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));
//
//        if (hr == S_OK)
//        {
//            _devices.push_back(device);
//        }
//    }
//
//    return u32();
//}
//
//u32 create_command_queue()
//{
//    if (_devices.empty())
//    {
//        return u32();
//    }
//
//    HRESULT hr = S_OK;
//
//    for (auto device : _devices)
//    {
//        ID3D12CommandQueue* cmd_queue = nullptr;
//
//        D3D12_COMMAND_QUEUE_DESC queue_desc{};
//        queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
//        queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
//
//        hr = device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&cmd_queue));
//
//        if (hr == S_OK)
//        {
//            _cmd_queues.push_back(cmd_queue);
//        }
//    }
//
//    return u32();
//}
