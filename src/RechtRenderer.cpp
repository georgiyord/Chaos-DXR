#include "RechtRenderer.hpp"
#include "utils.hpp"
#include <array>
#include <combaseapi.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgicommon.h>
#include <dxgiformat.h>
#include <iterator>
#include <stdexcept>
#include <strsafe.h>
#include <synchapi.h>
#include <utility>
#include <wil/result_macros.h>
#include <winerror.h>

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
    .NumDescriptors = 1,
    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    .NodeMask = 0,
};

constexpr D3D12_HEAP_PROPERTIES textureHeapProperties = {
    .Type = D3D12_HEAP_TYPE_DEFAULT,
    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask = 0,
    .VisibleNodeMask = 0,
};

constexpr D3D12_HEAP_PROPERTIES readbackBufferProperties = {
    .Type = D3D12_HEAP_TYPE_READBACK,
    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask = 0,
    .VisibleNodeMask = 0,
};

inline void RechtRenderer::allocateTextureMemory()
{
    m_textureResourceDesc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
        .Alignment = 0, // Automatic alignment
        .Width = m_textureWidth,
        .Height = m_textureHeight,
        .DepthOrArraySize = 1, // Number of 2D textures stored in the object
        .MipLevels = 1,        // Mipmap levels, 0 is automatic, 1 is base-case
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc =
            DXGI_SAMPLE_DESC{
                .Count = 1,
                .Quality = 0,
            },
        .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
    };

    THROW_IF_FAILED(m_d3d12device->CreateCommittedResource(&textureHeapProperties, D3D12_HEAP_FLAG_NONE,
                                                           &m_textureResourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                           nullptr, IID_PPV_ARGS(m_textureResource.put())));

    m_d3d12device->CreateRenderTargetView(m_textureResource.get(), nullptr, m_rtvHandle);

    UINT64 bufferSize{};
    m_d3d12device->GetCopyableFootprints(&m_textureResourceDesc, 0, 1, 0, &m_srcFootprint, nullptr, nullptr,
                                         &bufferSize);

    m_readbackBufferDesc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Alignment = 0, // Automatic alignment
        .Width = bufferSize,
        .Height = 1,
        .DepthOrArraySize = 1, // 1 needed for buffers
        .MipLevels = 1,        // 1 needed for buffers
        .Format = DXGI_FORMAT_UNKNOWN,
        .SampleDesc =
            DXGI_SAMPLE_DESC{
                .Count = 1,
                .Quality = 0,
            },
        .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags = D3D12_RESOURCE_FLAG_NONE,
    };

    THROW_IF_FAILED(m_d3d12device->CreateCommittedResource(&readbackBufferProperties, D3D12_HEAP_FLAG_NONE,
                                                           &m_readbackBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST,
                                                           nullptr, IID_PPV_ARGS(m_readbackBuffer.put())));
    m_barrierToRender = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition =
            {
                .pResource = m_textureResource.get(),
                .Subresource = 0,
                .StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE,
                .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
            },
    };
    m_barrierToCopy = {
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition =
            {
                .pResource = m_textureResource.get(),
                .Subresource = 0,
                .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
                .StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE,
            },
    };

    m_copyLocationSrc = {
        .pResource = m_textureResource.get(),
        .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
        .SubresourceIndex = 0,
    };

    m_copyLocationDst = {
        .pResource = m_readbackBuffer.get(),
        .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
        .PlacedFootprint = m_srcFootprint,
    };
}

RechtRenderer::RechtRenderer(ID3D12Device *d3d12device, ID3D12CommandAllocator *commandAllocator,
                             ID3D12CommandQueue *commandQueue)
    : m_d3d12device(d3d12device), m_commandAllocator(commandAllocator), m_commandQueue(commandQueue)
{
    THROW_IF_FAILED(m_d3d12device->CreateDescriptorHeap(&rtvDescriber, IID_PPV_ARGS(m_descriptorHeap.put())));
    m_rtvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    allocateTextureMemory();

    THROW_IF_FAILED(m_d3d12device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.put())));

    THROW_IF_FAILED(m_d3d12device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(),
                                                     nullptr, IID_PPV_ARGS(m_graphicsCommandList.put())));
    m_synchEvent.create();
}

RechtData RechtRenderer::RenderSquare(std::span<const FLOAT, 4> color, UINT width, UINT height)
{
    if (width != m_textureWidth || height != m_textureHeight)
    {
        m_textureWidth = width;
        m_textureHeight = height;
        allocateTextureMemory();
    }
    return RenderSquare(color);
}

RechtData RechtRenderer::RenderSquare(std::span<const FLOAT, 4> color)
{
    m_graphicsCommandList->OMSetRenderTargets(1, &m_rtvHandle, FALSE, nullptr);
    m_graphicsCommandList->ClearRenderTargetView(m_rtvHandle, color.data(), 0, nullptr);
    m_graphicsCommandList->ResourceBarrier(1, &m_barrierToCopy);
    m_graphicsCommandList->CopyTextureRegion(&m_copyLocationDst, 0, 0, 0, &m_copyLocationSrc, nullptr);
    m_graphicsCommandList->ResourceBarrier(1, &m_barrierToRender);
    THROW_IF_FAILED(m_graphicsCommandList->Close());

    m_fence->Signal(0);
    ID3D12GraphicsCommandList* tmp_commandLists = m_graphicsCommandList.get();
    m_commandQueue->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&tmp_commandLists));

    THROW_IF_FAILED(m_commandQueue->Signal(m_fence.get(), 1));

    if (m_fence->GetCompletedValue() < 1)
    {
        THROW_IF_FAILED(m_fence->SetEventOnCompletion(1, m_synchEvent.get()));
        const DWORD result = WaitForSingleObject(m_synchEvent.get(), 10000);
        if (result == WAIT_TIMEOUT)
        {
            throw std::runtime_error("Command queue fence timeout!");
        }
    }

    m_commandAllocator->Reset();
    m_graphicsCommandList->Reset(m_commandAllocator.Get(), nullptr);

    RechtData recht(m_readbackBuffer, 0, m_srcFootprint);
    return recht;
}
