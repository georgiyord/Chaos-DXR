#pragma once

#include "Renderer.hpp"
class TriangleRenderer final : public Renderer
{
    Color m_triangleColor{1.F, 0.F, 0.F, 1.F};
    Vertex *m_vertices = nullptr;
    UINT m_verticesCount = 0;
    winrt::com_ptr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
    winrt::com_ptr<ID3D12RootSignature> m_rootSignature;
    bool m_triangleTranslation = false;
    bool m_triangleRotation = false;
    bool m_triangleCheckeredPattern = false;

    void setupCommands() override;
    winrt::com_ptr<ID3D12PipelineState> setupPipelineState();
    void init();
    void resetVertexBuffer() noexcept;
    void createVertexBufferNonCheckered();
    void createVertexBufferCheckered();

  public:
    TriangleRenderer(HWND handle);
    TriangleRenderer(HWND handle, const Color &triangleColor);

    void setTriangleColor(const Color &color) noexcept;
    Color &getTriangleColor() noexcept;
    void setTriangleTranslation(bool value) noexcept;
    void setTriangleRotation(bool value) noexcept;
    void setTriangleCheckeredPattern(bool value);
    void update() override;
};