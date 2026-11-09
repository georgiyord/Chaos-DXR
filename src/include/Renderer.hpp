#pragma once

#include <basetsd.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <windef.h>
#include <winrt/base.h>

class Renderer
{
  private:
    winrt::com_ptr<ID3D12Device> m_d3d12device;
    winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
    winrt::com_ptr<ID3D12CommandQueue> m_commandQueue;
    winrt::com_ptr<IDXGISwapChain3> m_swapChain;
    HWND m_windowHandle;
    SIZE_T m_frameIdx = 0;

  protected:
    [[nodiscard]] const winrt::com_ptr<ID3D12Device> &get_d3d12device() const;
    [[nodiscard]] const winrt::com_ptr<ID3D12CommandAllocator> &get_commandAllocator() const;
    [[nodiscard]] const winrt::com_ptr<ID3D12CommandQueue> &get_commandQueue() const;
    [[nodiscard]] const winrt::com_ptr<IDXGISwapChain3> &get_swapChain() const;
    [[nodiscard]] HWND get_HWND() const;
    void incrementFrameIdx();

  public:
    Renderer(HWND handle);
    [[nodiscard]] SIZE_T get_frameIdx() const;
};
