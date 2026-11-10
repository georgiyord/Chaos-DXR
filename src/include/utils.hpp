#pragma once

#include <array>
#include <basetsd.h>
#include <cmath>
#include <comdef.h>
#include <d3d12.h>
#include <minwindef.h>
#include <random>
#include <span>
#include <utility>
#include <wil/result_macros.h>
#include <windef.h>
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

class Color_R8G8B8A8
{
    std::array<FLOAT, 4> m_color;

  public:
    static constexpr Color_R8G8B8A8 WHITE() noexcept
    {
        return {1.F, 1.F, 1.F, 1.F};
    };
    static constexpr Color_R8G8B8A8 RED() noexcept
    {
        return {1.F, 0.F, 0.F, 1.F};
    };

    constexpr Color_R8G8B8A8(std::span<FLOAT, 4> span) noexcept : m_color({span[0], span[1], span[2], span[3]})
    {
    }

    constexpr Color_R8G8B8A8(FLOAT r, FLOAT g, FLOAT b, FLOAT a) noexcept : m_color({r, g, b, a})
    {
    }

    constexpr void set(std::span<FLOAT, 4> span) noexcept
    {
        m_color = {span[0], span[1], span[2], span[3]};
    }

    constexpr void set(FLOAT r, FLOAT g, FLOAT b, FLOAT a) noexcept
    {
        m_color = {r, g, b, a};
    }

    constexpr void set_Red(FLOAT r) noexcept
    {
        m_color[0] = r;
    }

    constexpr void set_Green(FLOAT g) noexcept
    {
        m_color[1] = g;
    }

    constexpr void set_Blue(FLOAT b) noexcept
    {
        m_color[2] = b;
    }

    constexpr void set_Alpha(FLOAT a) noexcept
    {
        m_color[3] = a;
    }

    [[nodiscard]] constexpr FLOAT get_Red() const noexcept
    {
        return m_color[0];
    }

    [[nodiscard]] constexpr FLOAT get_Green() const noexcept
    {
        return m_color[1];
    }

    [[nodiscard]] constexpr FLOAT get_Blue() const noexcept
    {
        return m_color[2];
    }

    [[nodiscard]] constexpr FLOAT get_Alpha() const noexcept
    {
        return m_color[3];
    }

    [[nodiscard]] constexpr FLOAT *data() noexcept
    {
        return m_color.data();
    }

    [[nodiscard]] constexpr operator std::span<FLOAT, 4>()
    {
        return m_color;
    }
};

using Color = Color_R8G8B8A8;

class HSVGradient
{
    std::array<FLOAT, 4> m_color{1.F, 0.F, 0.F, 1.F};
    UINT8 m_sextant = 0;
    FLOAT m_delta;

    void updateGradient()
    {
        assert(m_sextant >= 0 && m_sextant <= 5);
        const FLOAT floor = std::floor(m_delta);
        m_delta -= floor;
        m_sextant += static_cast<UINT8>(floor);
        m_sextant %= 6;
        switch (m_sextant)
        {
        case 0:
            m_color[1] += m_delta;
            if (m_color[1] > 1.F)
            {
                m_color[0] -= m_color[1] - 1.F;
                m_color[1] = 1.F;
                ++m_sextant;
            }
            return;
        case 1:
            m_color[0] -= m_delta;
            if (m_color[0] < 0.F)
            {
                m_color[2] -= m_color[0];
                m_color[0] = 0.F;
                ++m_sextant;
            }
            return;
        case 2:
            m_color[2] += m_delta;
            if (m_color[2] > 1.F)
            {
                m_color[1] -= m_color[2] - 1.F;
                m_color[2] = 1.F;
                ++m_sextant;
            }
            return;
        case 3:
            m_color[1] -= m_delta;
            if (m_color[1] < 0.F)
            {
                m_color[0] -= m_color[1];
                m_color[1] = 0.F;
                ++m_sextant;
            }
            return;
        case 4:
            m_color[0] += m_delta;
            if (m_color[0] > 1.F)
            {
                m_color[2] -= m_color[0] - 1.F;
                m_color[0] = 1.F;
                ++m_sextant;
            }
            return;
        case 5:
            m_color[2] -= m_delta;
            if (m_color[2] < 0.F)
            {
                m_color[1] -= m_color[2];
                m_color[2] = 0.F;
                m_sextant = 0;
            }
            return;
        default:
            std::unreachable();
        }
    }

  public:
    HSVGradient(FLOAT m_delta = 0.F) : m_delta(m_delta) {};
    [[nodiscard]] constexpr std::span<FLOAT, 4> getColorView()
    {
        updateGradient();
        return m_color;
    }
    constexpr void updateDelta(FLOAT delta)
    {
        m_delta = delta;
    }
};

class RedGreenGradient
{
    enum struct State
    {
        RedToYellow = 0,
        YellowToGreen = 1,
        GreenToYellow = 2,
        YellowToRed = 3,
    };
    State state = State::RedToYellow;
    Color m_color = Color::RED();
    FLOAT m_delta;

    constexpr FLOAT getProgress()
    {
        switch (state)
        {
        case State::RedToYellow:
            return m_color.get_Green();
        case State::YellowToGreen:
            return 1.F - m_color.get_Red();
        case State::GreenToYellow:
            return m_color.get_Red();
        case State::YellowToRed:
            return 1.F - m_color.get_Green();
        }
        std::unreachable();
    }

    constexpr void resetProgress()
    {
        switch (state)
        {
        case State::RedToYellow:
            m_color = {1.F, 0.F, 0.F, 1.F};
            return;
        case State::YellowToGreen:
            m_color = {1.F, 1.F, 0.F, 1.F};
            return;
        case State::GreenToYellow:
            m_color = {0.F, 1.F, 0.F, 1.F};
            return;
        case State::YellowToRed:
            m_color = {1.F, 1.F, 0.F, 1.F};
            return;
        }
    }

    constexpr void setProgress(FLOAT x)
    {
        assert(x <= 1.F && x >= 0.F);
        switch (state)
        {
        case State::RedToYellow:
            m_color.set_Green(x);
            return;
        case State::YellowToGreen:
            m_color.set_Red(1.F - x);
            return;
        case State::GreenToYellow:
            m_color.set_Red(x);
            return;
        case State::YellowToRed:
            m_color.set_Green(1.F - x);
            return;
        }
    }

    constexpr void updateProgressState(UINT step)
    {
        step %= 4;
        state = static_cast<State>((static_cast<UINT>(state) + step) % 4);
    }

    void updateGradient()
    {
        FLOAT norm_delta = std::fmod(m_delta, 4.F);
        UINT floor = static_cast<UINT>(std::floor(norm_delta));
        FLOAT progress = getProgress();
        progress += norm_delta - static_cast<FLOAT>(floor);
        if (progress >= 1.F)
        {
            floor += 1;
            floor %= 4;
            progress -= 1.F;
        }
        updateProgressState(floor);
        resetProgress();
        setProgress(progress);
    }

  public:
    RedGreenGradient(FLOAT m_delta = 0.F) : m_delta(m_delta) {};
    [[nodiscard]] constexpr const Color &getColorView()
    {
        updateGradient();
        return m_color;
    }
    constexpr void updateDelta(FLOAT delta)
    {
        m_delta = delta;
    }
};

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