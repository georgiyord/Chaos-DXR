#pragma once

#include "utils.hpp"
#include <d3d12.h>
#include <wil/resource.h>

struct RechtData
{
    void *data; // rgba u8
    UINT64 width;
    UINT64 height;
    UINT64 rowPitch;
};

class RechtRenderer
{
private:
    access_ptr<ID3D12Device> m_d3d12device;
    access_ptr<ID3D12CommandAllocator> m_commandAllocator;
    access_ptr<ID3D12CommandQueue> m_commandQueue;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_textureResource;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_readbackBuffer;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT srcFootprint;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle;
    D3D12_RESOURCE_BARRIER m_barrierToCopy;
    D3D12_RESOURCE_BARRIER m_barrierToRender;
    D3D12_TEXTURE_COPY_LOCATION m_copyLocationSrc;
    D3D12_TEXTURE_COPY_LOCATION m_copyLocationDst;
    wil::unique_event m_synchEvent;

public:
    RechtRenderer(ID3D12Device *d3d12device, ID3D12CommandAllocator *commandAllocator, ID3D12CommandQueue *commandQueue);
    RechtData RenderSquare(const FLOAT color[4]) const;
};