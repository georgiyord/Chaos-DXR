#include "Renderer.hpp"
#include <basetsd.h>
#include <combaseapi.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_3.h>
#include <dxgi1_4.h>
#include <dxgi1_5.h>
#include <dxgi1_6.h>
#include <dxgiformat.h>
#include <minwindef.h>
#include <sstream>
#include <wil/result_macros.h>
#include <winrt/base.h>

const D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {
    .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
    .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
    .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
    .NodeMask = 0,
};

const winrt::com_ptr<ID3D12Device> &Renderer::get_d3d12device() const noexcept
{
    return m_d3d12device;
}
const winrt::com_ptr<ID3D12CommandAllocator> &Renderer::get_commandAllocator() const noexcept
{
    return m_commandAllocator;
}
const winrt::com_ptr<ID3D12CommandQueue> &Renderer::get_commandQueue() const noexcept
{
    return m_commandQueue;
}
const winrt::com_ptr<ID3D12GraphicsCommandList> &Renderer::get_graphicsCommandList() const noexcept
{
    return m_graphicsCommandList;
}
const winrt::com_ptr<IDXGISwapChain3> &Renderer::get_swapChain() const noexcept
{
    return m_swapChain;
}
HWND Renderer::get_HWND() const noexcept
{
    return m_windowHandle;
}
UINT Renderer::get_width() const noexcept
{
    return m_width;
}
UINT Renderer::get_height() const noexcept
{
    return m_height;
}

SIZE_T Renderer::get_frameIdx() const noexcept
{
    return m_frameIdx;
}

void Renderer::incrementFrameIdx() noexcept
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

    THROW_IF_FAILED(m_d3d12device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, get_commandAllocator().get(),
                                                     nullptr, IID_PPV_ARGS(m_graphicsCommandList.put())));

    // BOOL allowTearing = FALSE;
    // THROW_IF_FAILED(
    //     pFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing)));

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
        .BufferCount = swapChainBuffersN,
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

    constexpr D3D12_DESCRIPTOR_HEAP_DESC rtvDescriber = {
        .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        .NumDescriptors = swapChainBuffersN,
        .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
        .NodeMask = 0,
    };

    THROW_IF_FAILED(m_d3d12device->CreateDescriptorHeap(&rtvDescriber, IID_PPV_ARGS(m_descriptorHeap.put())));
    m_rtvDescHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();

    THROW_IF_FAILED(m_d3d12device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.put())));

    m_rtvDescriptorSize = m_d3d12device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void Renderer::resize(UINT width, UINT height)
{
    m_width = width;
    m_height = height;

    const std::array<UINT, 2> creationNodeMask{0, 0};
    const std::array<IUnknown *, 2> pPresentQueue{get_commandQueue().get(), get_commandQueue().get()};

    waitForGPU();
    THROW_IF_FAILED(get_swapChain()->ResizeBuffers1(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                                    DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING, creationNodeMask.data(),
                                                    pPresentQueue.data()));
    bindSwapChainBufsTosDescriptors();
}

void Renderer::bindSwapChainBufsTosDescriptors()
{
    winrt::com_ptr<ID3D12Resource> swapChainBuffer;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle{m_rtvDescHandle};
    std::wstring baseName = L"Swap Chain buffer #";

    for (UINT i = 0; i < swapChainBuffersN; ++i)
    {
        std::wstringstream finalName{baseName};
        finalName << i + 1;
        finalName << '\0'; // make sure the buffer is null terminated
        THROW_IF_FAILED(get_swapChain()->GetBuffer(i, IID_PPV_ARGS(swapChainBuffer.put())));
        THROW_IF_FAILED(swapChainBuffer->SetName(finalName.view().data()));
        m_d3d12device->CreateRenderTargetView(swapChainBuffer.get(), nullptr, rtvDescHandle);
        rtvDescHandle.ptr += m_rtvDescriptorSize;
    }
}

void Renderer::waitForGPU()
{
    const UINT64 fenceNextValue = m_fence->GetCompletedValue() + 1;
    THROW_IF_FAILED(m_commandQueue->Signal(m_fence.get(), fenceNextValue));
    if (m_fence->GetCompletedValue() < fenceNextValue)
    {
        THROW_IF_FAILED(m_fence->SetEventOnCompletion(fenceNextValue, m_synchEvent.get()));
        const DWORD result = WaitForSingleObject(m_synchEvent.get(), 10000);
        if (result == WAIT_TIMEOUT)
        {
            throw std::runtime_error("Command queue fence timeout!");
        }
    }
}

void Renderer::render()
{
    auto renderTargetView = m_rtvDescHandle;
    frameBegin(renderTargetView);
    setupCommands(renderTargetView);
    frameEnd();
}

void Renderer::frameBegin(D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetView)
{
    winrt::com_ptr<ID3D12Resource> currentBuffer;
    const UINT bufferIdx = get_swapChain()->GetCurrentBackBufferIndex();
    renderTargetView.ptr += static_cast<SIZE_T>(m_rtvDescriptorSize) * bufferIdx;

    get_swapChain()->GetBuffer(bufferIdx, IID_PPV_ARGS(currentBuffer.put()));
    m_barrierToRender.Transition.pResource = currentBuffer.get();
    m_barrierToPresent.Transition.pResource = currentBuffer.get();

    m_graphicsCommandList->ResourceBarrier(1, &m_barrierToRender);
    m_graphicsCommandList->OMSetRenderTargets(1, &renderTargetView, FALSE, nullptr);
}

void Renderer::frameEnd()
{
    m_graphicsCommandList->ResourceBarrier(1, &m_barrierToPresent);
    THROW_IF_FAILED(m_graphicsCommandList->Close());

    THROW_IF_FAILED(m_fence->Signal(m_fence->GetCompletedValue() + 1));
    ID3D12GraphicsCommandList *tmp_commandLists = m_graphicsCommandList.get();
    get_commandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList **>(&tmp_commandLists));

    waitForGPU();

    constexpr DXGI_PRESENT_PARAMETERS presentParams = {
        .DirtyRectsCount = 0,
        .pDirtyRects = nullptr,
        .pScrollRect = nullptr,
        .pScrollOffset = nullptr,
    };

    THROW_IF_FAILED(get_swapChain()->Present1(0, DXGI_PRESENT_ALLOW_TEARING, &presentParams));

    THROW_IF_FAILED(get_commandAllocator()->Reset());
    THROW_IF_FAILED(m_graphicsCommandList->Reset(get_commandAllocator().get(), nullptr));

    incrementFrameIdx();
}