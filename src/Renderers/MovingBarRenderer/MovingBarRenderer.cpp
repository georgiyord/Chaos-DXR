#include "MovingBarRenderer.hpp"
#include "utils.hpp"
#include <Windows.h>
#include <directx/d3dx12.h>
#include <winrt/base.h>

MovingBarRenderer::MovingBarRenderer(HWND handle) : Renderer(handle)
{
}
MovingBarRenderer::MovingBarRenderer(HWND handle, const Color &barColor) : Renderer(handle), m_barColor(barColor)
{
}

void MovingBarRenderer::setupCommands()
{
    const UINT barWidth = get_width() / 10;
    if (barWidth == 0)
    {
        return;
    }
    const UINT barPos = get_frameIdx() % (2 * (get_width() - barWidth));
    const UINT maxLeft = get_width() - barWidth;
    const UINT left = barPos > maxLeft ? maxLeft - (barPos - maxLeft) : barPos;
    const CD3DX12_RECT rect(static_cast<LONG>(left), 0, static_cast<LONG>(left + barWidth),
                            static_cast<LONG>(get_height()));
    get_graphicsCommandList()->ClearRenderTargetView(get_currentRenderTargetView(), m_barColor.data(), 1, &rect);
}

void MovingBarRenderer::setBarColor(const Color &color) noexcept
{
    m_barColor = color;
}
Color &MovingBarRenderer::getBarColor() noexcept
{
    return m_barColor;
}