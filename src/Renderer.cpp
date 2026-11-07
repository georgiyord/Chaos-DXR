#include "Renderer.hpp"
#include <__msvc_ostream.hpp>
#include <combaseapi.h>
#include <d3dcommon.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <wil/result_macros.h>
#include <winrt/base.h>

const D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {
    .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
    .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
    .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
    .NodeMask = 0,
};

const winrt::com_ptr<ID3D12Device> &Renderer::get_d3d12device() const
{
    return m_d3d12device;
}
const winrt::com_ptr<ID3D12CommandAllocator> &Renderer::get_commandAllocator() const
{
    return m_commandAllocator;
}
const winrt::com_ptr<ID3D12CommandQueue> &Renderer::get_commandQueue() const
{
    return m_commandQueue;
}
const winrt::com_ptr<IDXGISwapChain1> &Renderer::get_swapChain() const
{
    return m_swapChain;
}
HWND Renderer::get_HWND() const
{
    return m_windowHandle;
}

Renderer::Renderer(HWND handle) : m_windowHandle(handle) /* , m_rechtRenderer(nullptr) */
{
#if defined(_DEBUG) && WIN_HOST
    {
        winrt::com_ptr<ID3D12Debug6> debugInterface;
        THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(debugInterface.put())));
        debugInterface->EnableDebugLayer();
    }
#elif defined(_DEBUG)
    std::cerr << "Could not enable D3D12 debug layer: Not supported on wine/vkd3d/proton\n";
#endif
    winrt::com_ptr<IDXGIFactory4> pFactory;
    THROW_IF_FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(pFactory.put())));

    winrt::com_ptr<IDXGIAdapter1> pAdapter;
    THROW_IF_FAILED(pFactory->EnumAdapters1(0, pAdapter.put()));

    THROW_IF_FAILED(D3D12CreateDevice(pAdapter.get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_d3d12device.put())));
    THROW_IF_FAILED(m_d3d12device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_commandQueue.put())));
    THROW_IF_FAILED(
        m_d3d12device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.put())));

    constexpr UINT SWAP_CHAIN_WIDTH = 128;
    constexpr UINT SWAP_CHAIN_HEIGHT = 128;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
        .Width = SWAP_CHAIN_WIDTH,
        .Height = SWAP_CHAIN_HEIGHT,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .Stereo = FALSE,
        .SampleDesc = {.Count = 1, .Quality = 0},
        .BufferUsage =
            DXGI_USAGE_RENDER_TARGET_OUTPUT, // https://learn.microsoft.com/en-us/windows/win32/direct3ddxgi/dxgi-usage
        .BufferCount = 2,
        .Scaling = DXGI_SCALING_NONE,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
        .Flags = {},
    };

    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFullscreenDesc = {
        .RefreshRate = {.Numerator = 0, .Denominator = 1},
        .ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED,
        .Scaling = DXGI_MODE_SCALING_UNSPECIFIED,
        .Windowed = TRUE,
    };

    pFactory->CreateSwapChainForHwnd(m_d3d12device.get(), m_windowHandle, &swapChainDesc, &swapChainFullscreenDesc,
                                     nullptr, m_swapChain.put());
}