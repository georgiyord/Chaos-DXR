#include "Renderer.hpp"
#include <wil/result.h>

RechtRenderer *Renderer::AsRechtRenderer() const
{
    if (!m_rechtRenderer)
    {
        m_rechtRenderer = std::make_unique<RechtRenderer>(m_d3d12device.Get(), m_commandAllocator.Get(), m_commandQueue.Get());
    }
    return m_rechtRenderer.get();
}
const D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL, D3D12_COMMAND_QUEUE_FLAG_NONE, 0};

Renderer::Renderer() : m_rechtRenderer(nullptr)
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> pFactory;
    THROW_IF_FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(pFactory.ReleaseAndGetAddressOf())));

    Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;
    THROW_IF_FAILED(pFactory->EnumAdapters1(0, pAdapter.ReleaseAndGetAddressOf()));

    Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> graphicsCommandList;

    THROW_IF_FAILED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(m_d3d12device.ReleaseAndGetAddressOf())));
    THROW_IF_FAILED(m_d3d12device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(m_commandQueue.ReleaseAndGetAddressOf())));
    THROW_IF_FAILED(m_d3d12device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_commandAllocator.ReleaseAndGetAddressOf())));
}