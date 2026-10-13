#ifndef UNICODE
#define UNICODE
#include <cmath>
#endif

#include <d3d12.h>
#include <dxgi1_6.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <wrl/client.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")

namespace DX {
// Helper class for COM exceptions
class com_exception : public std::exception {
public:
  com_exception(HRESULT hr) : result(hr) {}

  const char *what() const noexcept override {
    static char s_str[64] = {};
    sprintf_s(s_str, "Failure with HRESULT of %08X",
              static_cast<unsigned int>(result));
    return s_str;
  }

private:
  HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(HRESULT hr) {
  if (FAILED(hr)) {
    throw com_exception(hr);
  }
}
} // namespace DX

using AdapterOutputsPair =
    std::pair<Microsoft::WRL::ComPtr<IDXGIAdapter4>,
              std::vector<Microsoft::WRL::ComPtr<IDXGIOutput6>>>;

// void ErrorExit()
// {
//     // Retrieve the system error message for the last-error code

//     LPVOID lpMsgBuf;
//     DWORD dw = GetLastError();

//     if (FormatMessage(
//         FORMAT_MESSAGE_ALLOCATE_BUFFER |
//         FORMAT_MESSAGE_FROM_SYSTEM |
//         FORMAT_MESSAGE_IGNORE_INSERTS,
//         NULL,
//         dw,
//         MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
//         (LPTSTR) &lpMsgBuf,
//         0, NULL) == 0) {
//         MessageBox(NULL, TEXT("FormatMessage failed"), TEXT("Error"), MB_OK);
//         ExitProcess(dw);
//     }

//     MessageBox(NULL, (LPCTSTR)lpMsgBuf, TEXT("Error"), MB_OK);

//     LocalFree(lpMsgBuf);
//     ExitProcess(dw);
// }

int main() {
  Microsoft::WRL::ComPtr<IDXGIFactory7> pFactory;
  DX::ThrowIfFailed(
      CreateDXGIFactory2(0, IID_PPV_ARGS(pFactory.ReleaseAndGetAddressOf())));

  std::vector<AdapterOutputsPair> adapterOutputsPairs;

  for (UINT i = 0;; ++i) {
    Microsoft::WRL::ComPtr<IDXGIAdapter4> pAdapter;
    HRESULT hr = pFactory->EnumAdapters1(
        i, (IDXGIAdapter1 **)pAdapter.ReleaseAndGetAddressOf());
    if (hr == DXGI_ERROR_NOT_FOUND || FAILED(hr)) {
      break;
    }
    adapterOutputsPairs.push_back({std::move(pAdapter), {}});
  }

  for (UINT i = 0; i < adapterOutputsPairs.size(); ++i) {
    std::vector<Microsoft::WRL::ComPtr<IDXGIOutput6>> vOutputs;
    for (UINT j = 0;; ++j) {
      Microsoft::WRL::ComPtr<IDXGIOutput6> pOutput;
      HRESULT hr = adapterOutputsPairs[i].first->EnumOutputs(
          j, (IDXGIOutput **)pOutput.ReleaseAndGetAddressOf());
      if (hr == DXGI_ERROR_NOT_FOUND || FAILED(hr)) {
        break;
      }
      adapterOutputsPairs[i].second.push_back(std::move(pOutput));
    }
  }

  Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter = adapterOutputsPairs[0].first;
  //   Microsoft::WRL::ComPtr<IDXGIOutput6> output =
  //       adapterOutputsPairs[0].second[0];

  const D3D12_COMMAND_QUEUE_DESC commandQueueDesc = {
      D3D12_COMMAND_LIST_TYPE_DIRECT, D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
      D3D12_COMMAND_QUEUE_FLAG_NONE, 0};

  Microsoft::WRL::ComPtr<ID3D12Device> d3d12device;
  Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
  Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
  Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> graphicsCommandList;

  DX::ThrowIfFailed(
      D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0,
                        IID_PPV_ARGS(d3d12device.ReleaseAndGetAddressOf())));
  DX::ThrowIfFailed(d3d12device->CreateCommandQueue(
      &commandQueueDesc, IID_PPV_ARGS(commandQueue.ReleaseAndGetAddressOf())));
  DX::ThrowIfFailed(d3d12device->CreateCommandAllocator(
      D3D12_COMMAND_LIST_TYPE_DIRECT,
      IID_PPV_ARGS(commandAllocator.ReleaseAndGetAddressOf())));
  DX::ThrowIfFailed(d3d12device->CreateCommandList(
      0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr,
      IID_PPV_ARGS(graphicsCommandList.ReleaseAndGetAddressOf())));

  return 0;
}