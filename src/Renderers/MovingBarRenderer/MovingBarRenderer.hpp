#pragma once

#include "Renderer.hpp"
#include "utils.hpp"
#include <Windows.h>
#include <directx/d3dx12.h>

class MovingBarRenderer final : public Renderer
{
    Color m_barColor{Color::WHITE()};

    void drawMovingBar(CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle);
    void setupCommands() override;

  public:
    MovingBarRenderer(HWND handle);
    MovingBarRenderer(HWND handle, const Color &barColor);

    void setBarColor(const Color &color) noexcept;
    Color &getBarColor() noexcept;
};