#include "Renderer.hpp"
#include <array>
#include <random>
#include <wil/resource.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")

[[nodiscard]] std::array<FLOAT, 4> randomColor()
{
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<float> realDist(0.f, 1.f);
  // Converts HSV to RGB space, where Saturation and Value are 100%
  // Color circle/donut is split into sixths for every pair of RGB channels,
  // where one is at max value
  static std::uniform_int_distribution<UINT> intDist(0, 5);

  UINT8 variation = intDist(gen);
  float offset = realDist(gen);

  switch (variation)
  {
  case 0:
    return {1.f, offset, 0.f, 1.f};
  case 1:
    return {offset, 1.f, 0.f, 1.f};
  case 2:
    return {0.f, 1.f, offset, 1.f};
  case 3:
    return {0.f, offset, 1.f, 1.f};
  case 4:
    return {offset, 0.f, 1.f, 1.f};
  default:
    return {1.f, 0, offset, 1.f};
  }
}

void writeImage(RechtData recht, LPCWSTR name)
{
  wil::unique_hfile file{CreateFileW(name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)};
  // good practive would be to check the amount of written bytes, but knowing this will not be used later, i can't be bothered
  WriteFile(file.get(), "P3\n", 3, nullptr, nullptr);
  char buf[40];
  _ui64toa_s(recht.width, buf, 40, 10);
  WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, 40)), nullptr, nullptr);
  WriteFile(file.get(), "\n", 1, nullptr, nullptr);
  _ui64toa_s(recht.height, buf, 40, 10);
  WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, 40)), nullptr, nullptr);
  WriteFile(file.get(), "\n", 1, nullptr, nullptr);
  WriteFile(file.get(), "255", 3, nullptr, nullptr);
  WriteFile(file.get(), "\n", 1, nullptr, nullptr);
  for (UINT i = 0; i < recht.height; ++i)
  {
    for (UINT j = 0; j < recht.width; ++j)
    {
      for (UINT k = 0; k < 3; ++k)
      {
        _itoa_s(reinterpret_cast<UINT8 *>(recht.data)[i * recht.rowPitch + j * 4 + k], buf, 40, 10);
        WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, 40)), nullptr, nullptr);
        WriteFile(file.get(), "\n", 1, nullptr, nullptr);
      }
    }
  }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
  Renderer renderer;
  RechtRenderer *rechtRenderer = renderer.AsRechtRenderer();

  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out0.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out1.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out2.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out3.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out4.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out5.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out6.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out7.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out8.ppm");
  writeImage(rechtRenderer->RenderSquare(randomColor().data()), L"out9.ppm");

  return 0;
}