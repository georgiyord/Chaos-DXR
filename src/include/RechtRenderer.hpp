#pragma once

#include "Renderer.hpp"
#include <basetsd.h>
#include <d3d12.h>
#include <minwindef.h>
#include <span>
#include <wil/resource.h>
#include <windef.h>
#include <winrt/base.h>

class RechtData
{
    void *m_data{};
    UINT64 m_width{};
    UINT64 m_height{};
    UINT64 m_rowPitch{};
    winrt::com_ptr<ID3D12Resource> m_readbackBuffer;
    UINT m_subresource{};

  public:
    // RechtData();

    RechtData(const winrt::com_ptr<ID3D12Resource> &readbackBuffer, UINT subresource,
              const D3D12_PLACED_SUBRESOURCE_FOOTPRINT &footprint);
    ~RechtData()
    {
        if (m_data != nullptr)
        {
            m_readbackBuffer->Unmap(m_subresource, nullptr);
        }
    }

    RechtData(const RechtData &) = delete;
    RechtData &operator=(const RechtData &) = delete;
    RechtData(RechtData &&rhs) noexcept;
    RechtData &operator=(RechtData &&rhs) noexcept;

    [[nodiscard]] const void *data() const
    {
        return m_data;
    }
    [[nodiscard]] UINT64 width() const
    {
        return m_width;
    }
    [[nodiscard]] UINT64 height() const
    {
        return m_height;
    }
    [[nodiscard]] UINT64 rowPitch() const
    {
        return m_rowPitch;
    }
};

class RechtRenderer : public Renderer
{
  private:
    winrt::com_ptr<ID3D12DescriptorHeap> m_descriptorHeap;
    winrt::com_ptr<ID3D12Fence> m_fence;
    D3D12_CPU_DESCRIPTOR_HANDLE m_rtvDescHandle{};
    UINT m_rtvDescriptorSize{};
    D3D12_RESOURCE_BARRIER m_barrierToPresent{};
    D3D12_RESOURCE_BARRIER m_barrierToRender{};
    winrt::com_ptr<ID3D12GraphicsCommandList> m_graphicsCommandList;
    wil::unique_event m_synchEvent;

    UINT m_textureWidth = 256;
    UINT m_textureHeight = 256;

    void bindSwapChainBufsTosDescriptors();

    template <auto DrawCallable> void renderVariation(std::span<const FLOAT, 4> color, UINT width, UINT height);
    template <auto DrawCallable> void renderVariation(std::span<const FLOAT, 4> color);

    void drawRecht(std::span<const FLOAT, 4> color, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle);
    void drawMovingBar(std::span<const FLOAT, 4> color, D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle);

  public:
    RechtRenderer(HWND handle);

    void renderRecht(std::span<const FLOAT, 4> color);
    void renderRecht(std::span<const FLOAT, 4> color, UINT width, UINT height);
    void renderMovingBar(std::span<const FLOAT, 4> color);
    void renderMovingBar(std::span<const FLOAT, 4> color, UINT width, UINT height);
};