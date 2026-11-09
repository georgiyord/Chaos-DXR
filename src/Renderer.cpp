#include "Renderer.hpp"
#include <basetsd.h>
#include <combaseapi.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <dxgiformat.h>
#include <minwindef.h>
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
const winrt::com_ptr<IDXGISwapChain3> &Renderer::get_swapChain() const
{
    return m_swapChain;
}
HWND Renderer::get_HWND() const
{
    return m_windowHandle;
}
SIZE_T Renderer::get_frameIdx() const
{
    return m_frameIdx;
}

void Renderer::incrementFrameIdx()
{
    ++m_frameIdx;
}

Renderer::Renderer(HWND handle) : m_windowHandle(handle) /* , m_rechtRenderer(nullptr) */
{
    UINT factoryDebugFlag = 0;
#if defined(_DEBUG) && WIN_HOST
    {
        winrt::com_ptr<ID3D12Debug6> debugInterface;
        THROW_IF_FAILED(D3D12GetDebugInterface(IID_PPV_ARGS(debugInterface.put())));
        debugInterface->EnableDebugLayer();
        factoryDebugFlag = DXGI_CREATE_FACTORY_DEBUG;
    }
#elif defined(_DEBUG)
    std::cerr << "Could not enable D3D12 debug layer: Not supported on wine/vkd3d/proton\n";
#endif
    winrt::com_ptr<IDXGIFactory7> pFactory;
    THROW_IF_FAILED(CreateDXGIFactory2(factoryDebugFlag, IID_PPV_ARGS(pFactory.put())));

    winrt::com_ptr<IDXGIAdapter1> pAdapter;
    THROW_IF_FAILED(pFactory->EnumAdapters1(0, pAdapter.put()));

    THROW_IF_FAILED(D3D12CreateDevice(pAdapter.get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_d3d12device.put())));
    THROW_IF_FAILED(m_d3d12device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_commandQueue.put())));
    THROW_IF_FAILED(
        m_d3d12device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.put())));

    BOOL allowTearing = FALSE;
    THROW_IF_FAILED(
        pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)));

    constexpr UINT SWAP_CHAIN_WIDTH = 128;
    constexpr UINT SWAP_CHAIN_HEIGHT = 128;

    constexpr DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {
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
        .Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING,
    };

    winrt::com_ptr<IDXGISwapChain1> swapChain;

    THROW_IF_FAILED(pFactory->CreateSwapChainForHwnd(m_commandQueue.get(), m_windowHandle, &swapChainDesc, nullptr,
                                                     nullptr, swapChain.put()));

    THROW_IF_FAILED(pFactory->MakeWindowAssociation(m_windowHandle, DXGI_MWA_NO_ALT_ENTER));

    m_swapChain = swapChain.as<IDXGISwapChain3>();
}