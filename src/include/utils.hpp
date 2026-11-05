#pragma once

#include <array>
#include <basetsd.h>
#include <comdef.h>
#include <minwindef.h>
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

// void writeImage(RechtData recht, LPCWSTR name)
// {
//     wil::unique_hfile file{CreateFileW(name, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)};
//     // good practive would be to check the amount of written bytes, but knowing
//     // this will not be used later, i can't be bothered
//     WriteFile(file.get(), "P3\n", 3, nullptr, nullptr);
//     char buf[40];
//     _ui64toa_s(recht.width(), buf, sizeof(buf), 10);
//     WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, sizeof(buf))), nullptr, nullptr);
//     WriteFile(file.get(), "\n", 1, nullptr, nullptr);
//     _ui64toa_s(recht.height(), buf, sizeof(buf), 10);
//     WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, sizeof(buf))), nullptr, nullptr);
//     WriteFile(file.get(), "\n", 1, nullptr, nullptr);
//     WriteFile(file.get(), "255", 3, nullptr, nullptr);
//     WriteFile(file.get(), "\n", 1, nullptr, nullptr);
//     for (SIZE_T i = 0; i < recht.height(); ++i)
//     {
//         for (SIZE_T j = 0; j < recht.width(); ++j)
//         {
//             for (SIZE_T k = 0; k < 3; ++k)
//             {
//                 _itoa_s(reinterpret_cast<const UINT8 *>(recht.data())[i * recht.rowPitch() + j * 4 + k], buf,
//                         sizeof(buf), 10);
//                 WriteFile(file.get(), buf, static_cast<DWORD>(strnlen_s(buf, sizeof(buf))), nullptr, nullptr);
//                 WriteFile(file.get(), "\n", 1, nullptr, nullptr);
//             }
//         }
//     }
// }

template <UINT64 Capacity> class UINTRingBuffer
{
    std::array<UINT64, Capacity> m_buf{};
    UINT64 m_size = 0;
    UINT64 last = 0;

  public:
    void push(UINT64 x)
    {
        if (m_size >= Capacity) [[likely]]
        {
            ++last;
            last = last >= Capacity ? 0 : last;
            m_buf[last] = x;
        }
        else
        {
            ++last;
            m_buf[m_size++] = x;
        }
    }

    // vulnerable to overflow. not an issue for use case
    UINT64 average()
    {
        UINT64 result = 0;
        for (UINT64 i = 0; i < m_size; ++i)
        {
            result += m_buf[i];
        }
        return result / m_size;
    }
};