#include "RechtRenderer.hpp"
#include "Renderer.hpp"
#include "utils.hpp"
#include <d3d12.h>
#include <wrl/client.h>
#include <stdexcept>

constexpr UINT64 WIDTH = 16;
constexpr UINT64 HEIGHT = 16;

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
    .SampleDesc = DXGI_SAMPLE_DESC{
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

RechtRenderer::RechtRenderer(ID3D12Device *d3d12device, ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue) : 
  m_d3d12device(d3d12device),
  m_commandAllocator(commandAllocator),
  m_commandQueue(commandQueue)
{
  THROW_IF_FAILED(m_d3d12device->CreateCommittedResource(&textureHeapProperties, D3D12_HEAP_FLAG_NONE, &textureResourceDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr, IID_PPV_ARGS(m_textureResource.ReleaseAndGetAddressOf())));
  THROW_IF_FAILED(m_d3d12device->CreateDescriptorHeap(&rtvDescriber, IID_PPV_ARGS(m_descriptorHeap.ReleaseAndGetAddressOf())));
  m_rtvHandle = m_descriptorHeap->GetCPUDescriptorHandleForHeapStart();
  m_d3d12device->CreateRenderTargetView(m_textureResource.Get(), nullptr, m_rtvHandle);
  
  UINT64 bufferSize;
  m_d3d12device->GetCopyableFootprints(&textureResourceDesc, 0, 1, 0, &srcFootprint, nullptr, nullptr, &bufferSize);

  const D3D12_RESOURCE_DESC readbackBufferDesc = {
    .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
    .Alignment = 0, // Automatic alignment
    .Width = bufferSize,
    .Height = 1,
    .DepthOrArraySize = 1, // 1 needed for buffers
    .MipLevels = 1,        // 1 needed for buffers
    .Format = DXGI_FORMAT_UNKNOWN,
    .SampleDesc = DXGI_SAMPLE_DESC{
      .Count = 1,
      .Quality = 0,
    },
    .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
    .Flags = D3D12_RESOURCE_FLAG_NONE,
  };
  
  THROW_IF_FAILED(m_d3d12device->CreateCommittedResource(&readbackBufferProperties, D3D12_HEAP_FLAG_NONE, &readbackBufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(m_readbackBuffer.ReleaseAndGetAddressOf())));
  THROW_IF_FAILED(m_d3d12device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.ReleaseAndGetAddressOf())));

  m_barrierToRender = {
    .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
    .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
    .Transition = {
      .pResource = m_textureResource.Get(),
      .Subresource = 0,
      .StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE,
      .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
    },
  };
  m_barrierToCopy = {
    .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
    .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
    .Transition = {
      .pResource = m_textureResource.Get(),
      .Subresource = 0,
      .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
      .StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE,
    },
  };

  m_copyLocationSrc = {
    .pResource = m_textureResource.Get(),
    .Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX,
    .SubresourceIndex = 0,
  };
  
  m_copyLocationDst = {
    .pResource = m_readbackBuffer.Get(),
    .Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT,
    .PlacedFootprint = srcFootprint,
  };

  m_synchEvent.create();
}

RechtData RechtRenderer::RenderSquare(const FLOAT color[4]) const
{
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> graphicsCommandList;
  THROW_IF_FAILED(m_d3d12device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(graphicsCommandList.ReleaseAndGetAddressOf())));

  graphicsCommandList->OMSetRenderTargets(1, &m_rtvHandle, FALSE, nullptr);
  graphicsCommandList->ClearRenderTargetView(m_rtvHandle, color, 0, NULL);
  graphicsCommandList->ResourceBarrier(1, &m_barrierToCopy);
  graphicsCommandList->CopyTextureRegion(&m_copyLocationDst, 0, 0, 0, &m_copyLocationSrc, nullptr);
  graphicsCommandList->ResourceBarrier(1, &m_barrierToRender);
  THROW_IF_FAILED(graphicsCommandList->Close());

  ID3D12CommandList *ppCommandLists[] = {graphicsCommandList.Get()};
  
  m_fence->Signal(0);
  m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

  THROW_IF_FAILED(m_commandQueue->Signal(m_fence.Get(), 1));

  if(m_fence->GetCompletedValue() < 1){
    THROW_IF_FAILED(m_fence->SetEventOnCompletion(1, m_synchEvent.get()));
    const DWORD result = WaitForSingleObject(m_synchEvent.get(), 10000);
    if (result == WAIT_TIMEOUT){
      throw std::runtime_error("Command queue fence timeout!");
    }
  }

  RechtData recht;
  THROW_IF_FAILED(m_readbackBuffer->Map(0, nullptr, &recht.data));
  recht.width = srcFootprint.Footprint.Width;
  recht.height = srcFootprint.Footprint.Height;
  recht.rowPitch = srcFootprint.Footprint.RowPitch;
  return recht;
}