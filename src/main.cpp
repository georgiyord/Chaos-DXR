#define D3DCOMPILE_DEBUG 1

#include "RechtRenderer.hpp"
#include "Renderer.hpp"
#include <array>
#include <random>
#include <wil/resource.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")

[[nodiscard]] std::array<FLOAT, 4> randomColor()
{
    static std::random_device randomDevice;
    static std::mt19937 gen(randomDevice());
    static std::uniform_real_distribution<float> realDist(0.F, 1.F);
    // Converts HSV to RGB space, where Saturation and Value are 100%
    // Color circle/donut is split into sixths for every pair of RGB channels,
    // where one is at max value
    static std::uniform_int_distribution<UINT> intDist(0, 5);

    UINT8 variation = static_cast<UINT8>(intDist(gen));
    float offset = realDist(gen);

    switch (variation)
    {
    case 0:
        return {1.F, offset, 0.F, 1.F};
    case 1:
        return {offset, 1.F, 0.F, 1.F};
    case 2:
        return {0.F, 1.F, offset, 1.F};
    case 3:
        return {0.F, offset, 1.F, 1.F};
    case 4:
        return {offset, 0.F, 1.F, 1.F};
    default:
        return {1.F, 0, offset, 1.F};
    }
}

void writeImage(RechtData recht, LPCWSTR name)
{
    wil::unique_hfile file{CreateFileW(name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)};
    // good practive would be to check the amount of written bytes, but knowing
    // this will not be used later, i can't be bothered
    WriteFile(file.get(), "P3\n", 3, nullptr, nullptr);
    char buf[40];
    _ui64toa_s(recht.width(), buf, sizeof(buf), 10);
    WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, sizeof(buf))), nullptr, nullptr);
    WriteFile(file.get(), "\n", 1, nullptr, nullptr);
    _ui64toa_s(recht.height(), buf, sizeof(buf), 10);
    WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, sizeof(buf))), nullptr, nullptr);
    WriteFile(file.get(), "\n", 1, nullptr, nullptr);
    WriteFile(file.get(), "255", 3, nullptr, nullptr);
    WriteFile(file.get(), "\n", 1, nullptr, nullptr);
    for (SIZE_T i = 0; i < recht.height(); ++i)
    {
        for (SIZE_T j = 0; j < recht.width(); ++j)
        {
            for (SIZE_T k = 0; k < 3; ++k)
            {
                _itoa_s(reinterpret_cast<const UINT8 *>(recht.data())[i * recht.rowPitch() + j * 4 + k], buf,
                        sizeof(buf), 10);
                WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, sizeof(buf))), nullptr, nullptr);
                WriteFile(file.get(), "\n", 1, nullptr, nullptr);
            }
        }
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(pCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    Renderer renderer;
    RechtRenderer *rechtRenderer = renderer.AsRechtRenderer();

    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out0.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out1.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out2.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out3.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out4.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out5.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out6.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out7.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out8.ppm");
    writeImage(rechtRenderer->RenderSquare(randomColor()), L"out9.ppm");

    return 0;
}