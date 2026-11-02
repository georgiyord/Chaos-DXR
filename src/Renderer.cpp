#include "Renderer.hpp"
#include <wil/result.h>

RechtRenderer *Renderer::AsRechtRenderer() const
{
    if (!m_rechtRenderer)
    {
        m_rechtRenderer =
            std::make_unique<RechtRenderer>(m_d3d12device.get(), m_commandAllocator.get(), m_commandQueue.get());
    }
    return m_rechtRenderer.get();
}
const D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
                                                   D3D12_COMMAND_QUEUE_FLAG_NONE, 0};

Renderer::Renderer() : m_rechtRenderer(nullptr)
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
}