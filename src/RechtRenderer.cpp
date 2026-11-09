#include "RechtRenderer.hpp"
#include "Renderer.hpp"
#include <array>
#include <combaseapi.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <minwindef.h>
#include <stdexcept>
#include <strsafe.h>
#include <synchapi.h>
#include <utility>
#include <wil/result_macros.h>
#include <winerror.h>
#include <winrt/base.h>

RechtData::RechtData(const winrt::com_ptr<ID3D12Resource> &readbackBuffer, const UINT subresource,
                     const D3D12_PLACED_SUBRESOURCE_FOOTPRINT &footprint)
    : m_width(footprint.Footprint.Width), m_height(footprint.Footprint.Height),
      m_rowPitch(footprint.Footprint.RowPitch), m_readbackBuffer(readbackBuffer), m_subresource(subresource)
{
    THROW_IF_FAILED(m_readbackBuffer->Map(subresource, nullptr, &m_data));
}

RechtData::RechtData(RechtData &&rhs) noexcept
    : m_data(rhs.m_data), m_width(rhs.m_width), m_height(rhs.m_height), m_rowPitch(rhs.m_rowPitch),
      m_readbackBuffer(std::move(rhs.m_readbackBuffer)), m_subresource(rhs.m_subresource)
{
    rhs.m_data = nullptr;
}

RechtData &RechtData::operator=(RechtData &&rhs) noexcept
{
    if (this != &rhs)
    {
        if (m_data != nullptr)
        {
            m_readbackBuffer->Unmap(m_subresource, nullptr);
        }
        m_data = rhs.m_data;
        rhs.m_data = nullptr;
        m_width = rhs.m_width;
        m_height = rhs.m_height;
        m_rowPitch = rhs.m_rowPitch;
        m_readbackBuffer = std::move(rhs.m_readbackBuffer);
        m_subresource = rhs.m_subresource;
    }
    return *this;
}

enum class FENCE_STATUS : uint_fast8_t
{
    WAITING = 0,
    FINISHED = 1,
};

constexpr D3D12_DESCRIPTOR_HEAP_DESC rtvDescriber = {
    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    .NumDescriptors = 2,
    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    .NodeMask = 0,
};

inline void RechtRenderer::bindSwapChainBufsTosDescriptors()
{
    winrt::com_ptr<ID3D12Resource> swapChainBuffer;
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle{m_rtvDescHandle};

    THROW_IF_FAILED(get_swapChain()->GetBuffer(0, IID_PPV_ARGS(swapChainBuffer.put())));
    THROW_IF_FAILED(swapChainBuffer->SetName(L"Swap Chain buffer #1"));
    get_d3d12device()->CreateRenderTargetView(swapChainBuffer.get(), nullptr, rtvDescHandle);

    rtvDescHandle.ptr += m_rtvDescriptorSize;

    THROW_IF_FAILED(get_swapChain()->GetBuffer(1, IID_PPV_ARGS(swapChainBuffer.put())));
    THROW_IF_FAILED(swapChainBuffer->SetName(L"Swap Chain buffer #2"));
    get_d3d12device()->CreateRenderTargetView(swapChainBuffer.get(), nullptr, rtvDescHandle);
}

RechtRenderer::RechtRenderer(HWND handle)
    : Renderer(handle),
      m_rtvDescriptorSize(get_d3d12device()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV))
{
    THROW_IF_FAILED(get_d3d12device()->CreateDescriptorHeap(&rtvDescriber, IID_PPV_ARGS(m_descriptorHeap.put())));
    m_rtvDescHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    bindSwapChainBufsTosDescriptors();

    THROW_IF_FAILED(get_d3d12device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.put())));

    THROW_IF_FAILED(get_d3d12device()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                         get_commandAllocator().get(), nullptr,
                                                         IID_PPV_ARGS(m_graphicsCommandList.put())));
    THROW_IF_FAILED(m_graphicsCommandList->SetName(L"Main Graphics Command List"));
    m_synchEvent.create();

    m_barrierToRender = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition =
            {
                .pResource = nullptr,
                .Subresource = 0,
                .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
                .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
            },
    };

    m_barrierToPresent = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition =
            {
                .pResource = nullptr,
                .Subresource = 0,
                .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
                .StateAfter = D3D12_RESOURCE_STATE_PRESENT,
            },
    };
}

template <auto DrawCallable>
void RechtRenderer::renderVariation(std::span<const FLOAT, 4> color, UINT width, UINT height)
{
    if (width != m_textureWidth || height != m_textureHeight)
    {
        m_textureWidth = width;
        m_textureHeight = height;

        std::array<UINT, 2> creationNodeMask{0, 0};
        std::array<IUnknown *, 2> pPresentQueue{get_commandQueue().get(), get_commandQueue().get()};

        const UINT64 fenceNextValue = m_fence->GetCompletedValue() + 1;
        THROW_IF_FAILED(get_commandQueue()->Signal(m_fence.get(), fenceNextValue));
        if (m_fence->GetCompletedValue() < fenceNextValue)
        {
            THROW_IF_FAILED(m_fence->SetEventOnCompletion(fenceNextValue, m_synchEvent.get()));
            const DWORD result = WaitForSingleObject(m_synchEvent.get(), 10000);
            if (result == WAIT_TIMEOUT)
            {
                throw std::runtime_error("Command queue fence timeout!");
            }
        }
        THROW_IF_FAILED(get_swapChain()->ResizeBuffers1(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM,
                                                        DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING, creationNodeMask.data(),
                                                        pPresentQueue.data()));
        bindSwapChainBufsTosDescriptors();
    }
    renderVariation<DrawCallable>(color);
}

template <auto DrawCallable> void RechtRenderer::renderVariation(std::span<const FLOAT, 4> color)
{
    auto rtvDescHandle = m_rtvDescHandle;
    winrt::com_ptr<ID3D12Resource> currentBuffer;
    const UINT bufferIdx = get_swapChain()->GetCurrentBackBufferIndex();
    rtvDescHandle.ptr += static_cast<SIZE_T>(m_rtvDescriptorSize) * bufferIdx;

    get_swapChain()->GetBuffer(bufferIdx, IID_PPV_ARGS(currentBuffer.put()));
    m_barrierToRender.Transition.pResource = currentBuffer.get();
    m_barrierToPresent.Transition.pResource = currentBuffer.get();

    m_graphicsCommandList->ResourceBarrier(1, &m_barrierToRender);
    m_graphicsCommandList->OMSetRenderTargets(1, &rtvDescHandle, FALSE, nullptr);

    (this->*DrawCallable)(color, rtvDescHandle);

    m_graphicsCommandList->ResourceBarrier(1, &m_barrierToPresent);
    THROW_IF_FAILED(m_graphicsCommandList->Close());

    THROW_IF_FAILED(m_fence->Signal(m_fence->GetCompletedValue() + 1));
    ID3D12GraphicsCommandList *tmp_commandLists = m_graphicsCommandList.get();
    get_commandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList **>(&tmp_commandLists));

    const UINT64 fenceNextValue = m_fence->GetCompletedValue() + 1;
    THROW_IF_FAILED(get_commandQueue()->Signal(m_fence.get(), fenceNextValue));

    if (m_fence->GetCompletedValue() < fenceNextValue)
    {
        THROW_IF_FAILED(m_fence->SetEventOnCompletion(fenceNextValue, m_synchEvent.get()));
        const DWORD result = WaitForSingleObject(m_synchEvent.get(), 10000);
        if (result == WAIT_TIMEOUT)
        {
            throw std::runtime_error("Command queue fence timeout!");
        }
    }

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

void RechtRenderer::drawRecht(std::span<const FLOAT, 4> color, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle)
{
    m_graphicsCommandList->ClearRenderTargetView(rtvDescHandle, color.data(), 0, nullptr);
}

void RechtRenderer::drawMovingBar(std::span<const FLOAT, 4> color, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle)
{
    constexpr std::array<FLOAT, 4> black{0.F, 0.F, 0.F, 1.F};
    m_graphicsCommandList->ClearRenderTargetView(rtvDescHandle, black.data(), 0, nullptr);

    const UINT barWidth = m_textureWidth / 10;
    if (barWidth == 0)
    {
        return;
    }
    const UINT barPos = get_frameIdx() % (2 * (m_textureWidth - barWidth));
    const UINT maxLeft = m_textureWidth - barWidth;
    const UINT left = barPos > maxLeft ? maxLeft - (barPos - maxLeft) : barPos;
    const D3D12_RECT rect{
        .left = static_cast<LONG>(left),
        .top = 0,
        .right = static_cast<LONG>(left + barWidth),
        .bottom = static_cast<LONG>(m_textureHeight),
    };
    m_graphicsCommandList->ClearRenderTargetView(rtvDescHandle, color.data(), 1, &rect);
}

void RechtRenderer::renderRecht(std::span<const FLOAT, 4> color, UINT width, UINT height)
{
    renderVariation<&RechtRenderer::drawRecht>(color, width, height);
}

void RechtRenderer::renderMovingBar(std::span<const FLOAT, 4> color, UINT width, UINT height)
{
    renderVariation<&RechtRenderer::drawMovingBar>(color, width, height);
}

void RechtRenderer::renderRecht(std::span<const FLOAT, 4> color)
{
    renderVariation<&RechtRenderer::drawRecht>(color);
}

void RechtRenderer::renderMovingBar(std::span<const FLOAT, 4> color)
{
    renderVariation<&RechtRenderer::drawMovingBar>(color);
}
