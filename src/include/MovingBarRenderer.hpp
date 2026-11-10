#pragma once

#include "Renderer.hpp"
#include "utils.hpp"
#include <basetsd.h>
#include <d3d12.h>
#include <minwindef.h>
#include <wil/resource.h>
#include <windef.h>
#include <winrt/base.h>

class MovingBarRenderer final : public Renderer
{
    Color m_barColor{Color::WHITE()};

    void drawMovingBar(D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle);

  public:
    MovingBarRenderer(HWND handle);
    MovingBarRenderer(HWND handle, const Color &barColor);

    void setupCommands(D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetView) override;
    void setBarColor(const Color &color) noexcept;
    Color &getBarColor() noexcept;
};