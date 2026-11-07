#pragma once

#include <d3d12.h>
#include <dxgi1_2.h>
#include <winrt/base.h>

class Renderer
{
  private:
    winrt::com_ptr<ID3D12Device> m_d3d12device;
    winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
    winrt::com_ptr<ID3D12CommandQueue> m_commandQueue;
    winrt::com_ptr<IDXGISwapChain1> m_swapChain;
    HWND m_windowHandle;

  protected:
    [[nodiscard]] const winrt::com_ptr<ID3D12Device> &get_d3d12device() const;
    [[nodiscard]] const winrt::com_ptr<ID3D12CommandAllocator> &get_commandAllocator() const;
    [[nodiscard]] const winrt::com_ptr<ID3D12CommandQueue> &get_commandQueue() const;
    [[nodiscard]] const winrt::com_ptr<IDXGISwapChain1> &get_swapChain() const;
    [[nodiscard]] HWND get_HWND() const;

  public:
    Renderer(HWND handle);
};
