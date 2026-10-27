#pragma once

// #include <comdef.h>
// #include <wrl/client.h>
#include <winrt/base.h>

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
