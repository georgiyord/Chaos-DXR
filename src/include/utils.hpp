#pragma once

#include <comdef.h>
#include <random>
#include <windows.h>
#include <winrt/base.h>
#include <wrl/client.h>

template <typename T> class access_ptr
{
  private:
    T *m_ptr;

  public:
    access_ptr() noexcept : m_ptr(nullptr)
    {
    }
    access_ptr(T *ptr) noexcept : m_ptr(ptr)
    {
    }
    access_ptr(winrt::com_ptr<T> ptr) noexcept : m_ptr(ptr.Get())
    {
    }

    access_ptr(const access_ptr &ptr) noexcept = default;
    access_ptr(access_ptr &&ptr) noexcept = default;
    access_ptr &operator=(const access_ptr &ptr) noexcept = default;
    access_ptr &operator=(access_ptr &&ptr) noexcept = default;
    ~access_ptr() = default;

    T *operator->() const noexcept
    {
        return m_ptr;
    }
    T *Get() const noexcept
    {
        return m_ptr;
    }
};

[[nodiscard]] inline std::array<FLOAT, 4> randomColor()
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