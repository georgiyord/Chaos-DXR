#pragma once

#include <wrl/client.h>
#include <dxgi1_6.h>
#include <d3d12.h>
#include "RechtRenderer.hpp"
#include <memory>

class Renderer
{
private:
    Microsoft::WRL::ComPtr<ID3D12Device>            m_d3d12device;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>  m_commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>      m_commandQueue;

    mutable std::unique_ptr<RechtRenderer> m_rechtRenderer;

    friend RechtRenderer;

public:
    Renderer();
    RechtRenderer* AsRechtRenderer() const;
};
