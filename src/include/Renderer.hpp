#pragma once

#include "utils.hpp"
#include <basetsd.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wil/resource.h>
#include <windef.h>
#include <winrt/base.h>

class Renderer
{
  private:
    static constexpr UINT swapChainBuffersN = 2;

    winrt::com_ptr<ID3D12Device> m_d3d12device;
    winrt::com_ptr<ID3D12CommandAllocator> m_commandAllocator;
    winrt::com_ptr<ID3D12CommandQueue> m_commandQueue;
    winrt::com_ptr<ID3D12GraphicsCommandList> m_graphicsCommandList;
    winrt::com_ptr<IDXGISwapChain3> m_swapChain;
    winrt::com_ptr<ID3D12Fence> m_fence;
    winrt::com_ptr<ID3D12DescriptorHeap> m_descriptorHeap;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvDescHandle{};
    D3D12_RESOURCE_BARRIER m_barrierToPresent{
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition =
            {
                .pResource = nullptr,
                .Subresource = 0,
                .StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET,
                .StateAfter = D3D12_RESOURCE_STATE_PRESENT,
            },
    };
    D3D12_RESOURCE_BARRIER m_barrierToRender{
        .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
        .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
        .Transition =
            {
                .pResource = nullptr,
                .Subresource = 0,
                .StateBefore = D3D12_RESOURCE_STATE_PRESENT,
                .StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
            },
    };
    UINT m_rtvDescriptorSize{};
    HWND m_windowHandle;
    SIZE_T m_frameIdx = 0;
    UINT m_width;
    UINT m_height;
    wil::unique_event m_synchEvent;
    std::array<FLOAT, 4> bgColor{0.F, 0.F, 0.F, 1.F};

  protected:
    [[nodiscard]] const winrt::com_ptr<ID3D12Device> &get_d3d12device() const noexcept;
    [[nodiscard]] const winrt::com_ptr<ID3D12CommandAllocator> &get_commandAllocator() const noexcept;
    [[nodiscard]] const winrt::com_ptr<ID3D12CommandQueue> &get_commandQueue() const noexcept;
    [[nodiscard]] const winrt::com_ptr<ID3D12GraphicsCommandList> &get_graphicsCommandList() const noexcept;
    [[nodiscard]] const winrt::com_ptr<IDXGISwapChain3> &get_swapChain() const noexcept;
    [[nodiscard]] const winrt::com_ptr<ID3D12Fence> &get_fence() const noexcept;
    [[nodiscard]] const winrt::com_ptr<ID3D12DescriptorHeap> &get_descriptorHeap() const noexcept;
    [[nodiscard]] HWND get_HWND() const noexcept;
    [[nodiscard]] UINT get_width() const noexcept;
    [[nodiscard]] UINT get_height() const noexcept;

    void incrementFrameIdx() noexcept;
    void bindSwapChainBufsTosDescriptors();
    void frameBegin(D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetView);
    void frameEnd();
    virtual void setupCommands(D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetView) = 0;
    void waitForGPU();

  public:
    Renderer(HWND handle);
    [[nodiscard]] SIZE_T get_frameIdx() const noexcept;
    void resize(UINT width, UINT height);
    void render();
};
