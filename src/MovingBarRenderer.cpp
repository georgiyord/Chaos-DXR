#include "MovingBarRenderer.hpp"
#include "Renderer.hpp"
#include <array>
#include <combaseapi.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_2.h>
#include <dxgi1_4.h>
#include <dxgiformat.h>
#include <minwindef.h>
#include <strsafe.h>
#include <synchapi.h>
#include <wil/result_macros.h>
#include <winerror.h>
#include <winrt/base.h>

MovingBarRenderer::MovingBarRenderer(HWND handle) : Renderer(handle)
{
}
MovingBarRenderer::MovingBarRenderer(HWND handle, const Color &barColor) : Renderer(handle), m_barColor(barColor)
{
}

void MovingBarRenderer::setupCommands(D3D12_CPU_DESCRIPTOR_HANDLE &renderTargetView)
{
    constexpr std::array<FLOAT, 4> black{0.F, 0.F, 0.F, 1.F};
    get_graphicsCommandList()->ClearRenderTargetView(renderTargetView, black.data(), 0, nullptr);

    const UINT barWidth = get_width() / 10;
    if (barWidth == 0)
    {
        return;
    }
    const UINT barPos = get_frameIdx() % (2 * (get_width() - barWidth));
    const UINT maxLeft = get_width() - barWidth;
    const UINT left = barPos > maxLeft ? maxLeft - (barPos - maxLeft) : barPos;
    const D3D12_RECT rect{
        .left = static_cast<LONG>(left),
        .top = 0,
        .right = static_cast<LONG>(left + barWidth),
        .bottom = static_cast<LONG>(get_height()),
    };
    get_graphicsCommandList()->ClearRenderTargetView(renderTargetView, m_barColor.data(), 1, &rect);
}

void MovingBarRenderer::setBarColor(const Color &color) noexcept
{
    m_barColor = color;
}
Color &MovingBarRenderer::getBarColor() noexcept
{
    return m_barColor;
}