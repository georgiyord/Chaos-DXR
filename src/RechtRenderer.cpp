#include "RechtRenderer.hpp"
#include "utils.hpp"
#include <array>
#include <cstdint>
#include <wrl/client.h>

constexpr UINT64 WIDTH = 16;
constexpr UINT64 HEIGHT = 16;

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

const D3D12_DESCRIPTOR_HEAP_DESC rtvDescriber = {
    .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
    .NumDescriptors = 1,
    .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    .NodeMask = 0,
};

const D3D12_HEAP_PROPERTIES textureHeapProperties = {
    .Type = D3D12_HEAP_TYPE_DEFAULT,
    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask = 0,
    .VisibleNodeMask = 0,
};

const D3D12_RESOURCE_DESC textureResourceDesc = {
    .Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D,
    .Alignment = 0, // Automatic alignment
    .Width = WIDTH,
    .Height = HEIGHT,
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
const D3D12_HEAP_PROPERTIES readbackBufferProperties = {
    .Type = D3D12_HEAP_TYPE_READBACK,
    .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
    .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
    .CreationNodeMask = 0,
    .VisibleNodeMask = 0,
};

RechtRenderer::RechtRenderer(ID3D12Device *d3d12device, ID3D12CommandAllocator *commandAllocator,
                             ID3D12CommandQueue *commandQueue)
    : m_d3d12device(d3d12device), m_commandAllocator(commandAllocator), m_commandQueue(commandQueue)
{
    THROW_IF_FAILED(m_d3d12device->CreateCommittedResource(&textureHeapProperties, D3D12_HEAP_FLAG_NONE,
                                                           &textureResourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
                                                           nullptr, IID_PPV_ARGS(m_textureResource.put())));
    THROW_IF_FAILED(m_d3d12device->CreateDescriptorHeap(&rtvDescriber, IID_PPV_ARGS(m_descriptorHeap.put())));
    m_rtvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
    m_d3d12device->CreateRenderTargetView(m_textureResource.get(), nullptr, m_rtvHandle);

    UINT64 bufferSize{};
    m_d3d12device->GetCopyableFootprints(&textureResourceDesc, 0, 1, 0, &srcFootprint, nullptr, nullptr, &bufferSize);

    const D3D12_RESOURCE_DESC readbackBufferDesc = {
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
                                                           &readbackBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr,
                                                           IID_PPV_ARGS(m_readbackBuffer.put())));
    THROW_IF_FAILED(m_d3d12device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.put())));

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
        .PlacedFootprint = srcFootprint,
    };

    m_synchEvent.create();
}

RechtData RechtRenderer::RenderSquare(std::span<const FLOAT, 4> color) const
{
    std::array<ID3D12CommandList *, 1> ppCommandLists{};
    winrt::com_ptr<ID3D12GraphicsCommandList> graphicsCommandList;
    THROW_IF_FAILED(m_d3d12device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(),
                                                     nullptr, IID_PPV_ARGS(graphicsCommandList.put())));

    graphicsCommandList->OMSetRenderTargets(1, &m_rtvHandle, FALSE, nullptr);
    graphicsCommandList->ClearRenderTargetView(m_rtvHandle, color.data(), 0, nullptr);
    graphicsCommandList->ResourceBarrier(1, &m_barrierToCopy);
    graphicsCommandList->CopyTextureRegion(&m_copyLocationDst, 0, 0, 0, &m_copyLocationSrc, nullptr);
    graphicsCommandList->ResourceBarrier(1, &m_barrierToRender);
    THROW_IF_FAILED(graphicsCommandList->Close());
    ppCommandLists[0] = graphicsCommandList.get();

    m_fence->Signal(0);
    m_commandQueue->ExecuteCommandLists(std::size(ppCommandLists), ppCommandLists.data());

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

    RechtData recht(m_readbackBuffer, 0, srcFootprint);
    return recht;
}
