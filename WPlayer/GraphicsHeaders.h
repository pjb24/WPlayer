#pragma once

#include <dxgi1_6.h>
#include <d3d12.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "d3dx12.h"
#pragma comment(lib, "dxguid.lib")

#include <wrl.h>
using namespace Microsoft::WRL;

#include <DirectXMath.h>

#ifdef _DEBUG
// Sets the name of the COM object and outputs a debug string in Visual Studio's output panel.
#define NAME_D3D12_OBJECT(obj, name) obj->SetName(name); OutputDebugString(L"::D3D12 Object Created: "); OutputDebugString(name); OutputDebugString(L"\n");
// The indexed variant will include the index in the name of the object
#define NAME_D3D12_OBJECT_INDEXED(obj, n, name)             \
{                                                           \
wchar_t full_name[128];                                     \
if (swprintf_s(full_name, L"%s[%llu]", name, (u64)n) > 0)   \
{                                                           \
	obj->SetName(full_name);                                \
	OutputDebugString(L"::D3D12 Object Created: ");         \
	OutputDebugString(full_name);                           \
	OutputDebugString(L"\n");                               \
}}
// The indexed variant will include the index in the name of the object
#define NAME_D3D12_OBJECT_INDEXED_2(obj, n1, n2, name)             \
{                                                           \
wchar_t full_name[128];                                     \
if (swprintf_s(full_name, L"%s[%llu_%llu]", name, (u64)n1, (u64)n2) > 0)   \
{                                                           \
	obj->SetName(full_name);                                \
	OutputDebugString(L"::D3D12 Object Created: ");         \
	OutputDebugString(full_name);                           \
	OutputDebugString(L"\n");                               \
}}
// The indexed variant will include the index in the name of the object
#define NAME_D3D12_OBJECT_INDEXED_3(obj, n1, n2, n3, name)             \
{                                                           \
wchar_t full_name[128];                                     \
if (swprintf_s(full_name, L"%s[%llu_%llu_%llu]", name, (u64)n1, (u64)n2, (u64)n3) > 0)   \
{                                                           \
	obj->SetName(full_name);                                \
	OutputDebugString(L"::D3D12 Object Created: ");         \
	OutputDebugString(full_name);                           \
	OutputDebugString(L"\n");                               \
}}
#else
#define NAME_D3D12_OBJECT(x, name)
#define NAME_D3D12_OBJECT_INDEXED(x, n, name)
#define NAME_D3D12_OBJECT_INDEXED_2(x, n1, n2, name)
#define NAME_D3D12_OBJECT_INDEXED_3(obj, n1, n2, n3, name)
#endif // _DEBUG
