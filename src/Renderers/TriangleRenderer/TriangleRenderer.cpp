#include "TriangleRenderer.hpp"
#include "ConstColorPS.h"
#include "ConstColorVS.h"
#include "utils.hpp"
#include <Windows.h>
#include <array>
#include <basetsd.h>
#include <cmath>
#include <directx/d3d12.h>
#include <directx/d3dcommon.h>
#include <directx/d3dx12.h>
#include <directx/dxgiformat.h>
#include <minwindef.h>
#include <numbers>
#include <wil/result_macros.h>
#include <winrt/base.h>

// Triangle
constexpr Vertex tr_v0 = {0.0F, std::numbers::sqrt3_v<FLOAT> / 3.F};
constexpr Vertex tr_v1 = {0.5F, -std::numbers::sqrt3_v<FLOAT> / 6.F};
constexpr Vertex tr_v2 = {-0.5F, -std::numbers::sqrt3_v<FLOAT> / 6.F};
constexpr UINT nonCheckeredVertexCount = 3;

// Checker Pattern
constexpr UINT rows = 4;
constexpr UINT cols = 4;

constexpr UINT checkeredVertexCount = (rows * cols / 2) * 3 * 2;

void TriangleRenderer::resetVertexBuffer() noexcept
{
    if (m_vertexBuffer)
    {
        m_vertexBuffer->Unmap(0, nullptr);
    }
    m_vertexBuffer = nullptr;
    m_vertices = nullptr;
    m_verticesCount = 0;
    m_vertexBufferView = {};
}

void TriangleRenderer::createVertexBufferNonCheckered()
{
    m_verticesCount = nonCheckeredVertexCount;
    const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * m_verticesCount);

    THROW_IF_FAILED(get_d3d12device()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                                               D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                               IID_PPV_ARGS(m_vertexBuffer.put())));
    THROW_IF_FAILED(m_vertexBuffer->Map(0, nullptr, reinterpret_cast<void **>(&m_vertices)));

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(Vertex) * m_verticesCount;
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);

    m_vertices[0] = tr_v0;
    m_vertices[1] = tr_v1;
    m_vertices[2] = tr_v2;
}

void TriangleRenderer::createVertexBufferCheckered()
{
    m_verticesCount = checkeredVertexCount;
    const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_UPLOAD);
    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * m_verticesCount);

    THROW_IF_FAILED(get_d3d12device()->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
                                                               D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                                               IID_PPV_ARGS(m_vertexBuffer.put())));
    THROW_IF_FAILED(m_vertexBuffer->Map(0, nullptr, reinterpret_cast<void **>(&m_vertices)));

    m_vertexBufferView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
    m_vertexBufferView.SizeInBytes = sizeof(Vertex) * m_verticesCount;
    m_vertexBufferView.StrideInBytes = sizeof(Vertex);

    for (UINT i = 0; i < rows; ++i)
    {
        for (UINT j = 0; j < cols; ++j)
        {
            if ((i + j) % 2 == 0)
            {
                continue;
            }
            const FLOAT row0 = -1.0F + 2 * static_cast<FLOAT>(i) / static_cast<FLOAT>(rows);
            const FLOAT row1 = -1.0F + 2 * static_cast<FLOAT>(i + 1) / static_cast<FLOAT>(rows);
            const FLOAT col0 = -1.0F + 2 * static_cast<FLOAT>(j) / static_cast<FLOAT>(cols);
            const FLOAT col1 = -1.0F + 2 * static_cast<FLOAT>(j + 1) / static_cast<FLOAT>(cols);

            m_vertices[0 + (i * cols + j) * 3] = Vertex{row0, col0};
            m_vertices[1 + (i * cols + j) * 3] = Vertex{row1, col1};
            m_vertices[2 + (i * cols + j) * 3] = Vertex{row1, col0};
            m_vertices[3 + (i * cols + j) * 3] = Vertex{row0, col0};
            m_vertices[4 + (i * cols + j) * 3] = Vertex{row0, col1};
            m_vertices[5 + (i * cols + j) * 3] = Vertex{row1, col1};
        }
    }
}

void TriangleRenderer::init()
{
    if (m_triangleCheckeredPattern)
    {
        createVertexBufferCheckered();
    }
    else
    {
        createVertexBufferNonCheckered();
    }

    const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc(D3D12_ROOT_SIGNATURE_DESC1{
        .NumParameters = 0,
        .pParameters = nullptr,
        .NumStaticSamplers = 0,
        .pStaticSamplers = nullptr,
        .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
    });
    winrt::com_ptr<ID3DBlob> blob;
    winrt::com_ptr<ID3DBlob> errorBlob;

    THROW_IF_FAILED(D3D12SerializeVersionedRootSignature(&rootSignatureDesc, blob.put(), errorBlob.put()));

    THROW_IF_FAILED(get_d3d12device()->CreateRootSignature(0, blob->GetBufferPointer(), blob->GetBufferSize(),
                                                           IID_PPV_ARGS(m_rootSignature.put())));

    set_initialPipelineState(setupPipelineState());
}

TriangleRenderer::TriangleRenderer(HWND handle) : Renderer(handle)
{
    init();
}
TriangleRenderer::TriangleRenderer(HWND handle, const Color &triangleColor)
    : Renderer(handle), m_triangleColor(triangleColor)
{
    init();
}

winrt::com_ptr<ID3D12PipelineState> TriangleRenderer::setupPipelineState()
{
    D3D12_INPUT_ELEMENT_DESC inputElementDescs = {
        .SemanticName = "POSITION",
        .SemanticIndex = 0,
        .Format = DXGI_FORMAT_R32G32_FLOAT,
        .InputSlot = 0,
        .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
        .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
        .InstanceDataStepRate = 0,
    };

    winrt::com_ptr<ID3D12PipelineState> pipelineState;
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pipelineStateDesc{};
    pipelineStateDesc.pRootSignature = m_rootSignature.get();
    pipelineStateDesc.VS = CD3DX12_SHADER_BYTECODE(ConstColorVS, sizeof(ConstColorVS));
    pipelineStateDesc.PS = CD3DX12_SHADER_BYTECODE(ConstColorPS, sizeof(ConstColorPS));
    pipelineStateDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pipelineStateDesc.SampleMask = UINT_MAX;
    pipelineStateDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pipelineStateDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
    pipelineStateDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    pipelineStateDesc.InputLayout = {.pInputElementDescs = &inputElementDescs, .NumElements = 1};
    pipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineStateDesc.NumRenderTargets = 1;
    pipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pipelineStateDesc.SampleDesc = {.Count = 1, .Quality = 0};
    pipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    THROW_IF_FAILED(
        get_d3d12device()->CreateGraphicsPipelineState(&pipelineStateDesc, IID_PPV_ARGS(pipelineState.put())));
    return pipelineState;
}

void TriangleRenderer::setupCommands()
{
    const CD3DX12_VIEWPORT viewport(0.F, 0.F, static_cast<FLOAT>(get_width()), static_cast<FLOAT>(get_height()), 0.F,
                                    0.F);
    const CD3DX12_RECT rect(0, 0, static_cast<LONG>(get_width()), static_cast<LONG>(get_height()));
    get_graphicsCommandList()->SetGraphicsRootSignature(m_rootSignature.get());
    get_graphicsCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    get_graphicsCommandList()->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    get_graphicsCommandList()->RSSetViewports(1, &viewport);
    get_graphicsCommandList()->RSSetScissorRects(1, &rect);
    get_graphicsCommandList()->DrawInstanced(m_verticesCount, 1, 0, 0);
}

void TriangleRenderer::update()
{
    Renderer::update();
    constexpr UINT cycle = 1000;
    static FLOAT offset = 0;
    FLOAT s = 0.F;
    FLOAT c = 1.F;
    constexpr FLOAT translationModifier = 0.001F;
    constexpr FLOAT rotationModifier = 0.001F;
    if (m_triangleTranslation)
    {
        if ((get_frameIdx() + cycle / 2) % (cycle * 2) < cycle)
        {
            offset += translationModifier;
        }
        else
        {
            offset -= translationModifier;
        }
    }
    if (m_triangleRotation)
    {
        s = std::sinf(get_frameIdx() * rotationModifier);
        c = std::cosf(get_frameIdx() * rotationModifier);
    }
    constexpr auto transformVertex = [](const Vertex &v, FLOAT s, FLOAT c, FLOAT offset) {
        return Vertex{v.x * c + v.y * s + offset, v.x * (-s) + v.y * c};
    };
    if (m_triangleCheckeredPattern)
    {
        UINT counter = 0;
        for (UINT i = 0; i < rows; ++i)
        {
            for (UINT j = 0; j < cols; ++j)
            {
                if ((i + j) % 2 == 0)
                {
                    continue;
                }
                const FLOAT row0 = -1.0F + 2 * static_cast<FLOAT>(i) / static_cast<FLOAT>(rows);
                const FLOAT row1 = -1.0F + 2 * static_cast<FLOAT>(i + 1) / static_cast<FLOAT>(rows);
                const FLOAT col0 = -1.0F + 2 * static_cast<FLOAT>(j) / static_cast<FLOAT>(cols);
                const FLOAT col1 = -1.0F + 2 * static_cast<FLOAT>(j + 1) / static_cast<FLOAT>(cols);

                m_vertices[counter + 0] = transformVertex(Vertex{row0, col0}, s, c, offset);
                m_vertices[counter + 1] = transformVertex(Vertex{row1, col1}, s, c, offset);
                m_vertices[counter + 2] = transformVertex(Vertex{row1, col0}, s, c, offset);
                m_vertices[counter + 3] = transformVertex(Vertex{row0, col0}, s, c, offset);
                m_vertices[counter + 4] = transformVertex(Vertex{row0, col1}, s, c, offset);
                m_vertices[counter + 5] = transformVertex(Vertex{row1, col1}, s, c, offset);
                counter += 6;
            }
        }
    }
    else
    {
        m_vertices[0] = transformVertex(tr_v0, s, c, offset);
        m_vertices[1] = transformVertex(tr_v1, s, c, offset);
        m_vertices[2] = transformVertex(tr_v2, s, c, offset);
    }
}

void TriangleRenderer::setTriangleColor(const Color &color) noexcept
{
    m_triangleColor = color;
}
Color &TriangleRenderer::getTriangleColor() noexcept
{
    return m_triangleColor;
}

void TriangleRenderer::setTriangleTranslation(bool value) noexcept
{
    m_triangleTranslation = value;
}
void TriangleRenderer::setTriangleRotation(bool value) noexcept
{
    m_triangleRotation = value;
}
void TriangleRenderer::setTriangleCheckeredPattern(bool value)
{
    if (m_triangleCheckeredPattern == value)
    {
        return;
    }
    m_triangleCheckeredPattern = value;
    resetVertexBuffer();
    if (value)
    {
        createVertexBufferCheckered();
    }
    else
    {
        createVertexBufferNonCheckered();
    }
}