#pragma once

#include "RechtRenderer.hpp"
#include <d3d12.h>
#include <memory>
#include <winrt/base.h>

class Renderer
{
  private:
    winrt::com_ptr<ID3D12Device> m_d3d12device;
    winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
    winrt::com_ptr<ID3D12CommandQueue> m_commandQueue;

    mutable std::unique_ptr<RechtRenderer> m_rechtRenderer;

    friend RechtRenderer;

  public:
    Renderer();
    RechtRenderer *AsRechtRenderer() const;
};
