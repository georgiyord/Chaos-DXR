#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <wrl/client.h>
#include <D3d12.h>
#include <dxgi1_6.h>
#include <vector>
#include <iostream>
#include <string_view>

int main()
{
    Microsoft::WRL::ComPtr<IDXGIFactory7> pFactory;
    HRESULT factoryResult = CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory));
    if (FAILED(factoryResult)){
        return 1;
    }

    std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter4>> vAdapters; 
    for ( UINT i = 0; ; ++i){
        Microsoft::WRL::ComPtr<IDXGIAdapter4> pAdapter; 
        HRESULT hr = pFactory->EnumAdapters1(i, (IDXGIAdapter1**)pAdapter.GetAddressOf());
        if(hr == DXGI_ERROR_NOT_FOUND || FAILED(hr)){
            break;
        }
        vAdapters.push_back(std::move(pAdapter));
    }

    std::cout << "Adapters: " << vAdapters.size() << '\n';

    for (UINT i = 0; i < vAdapters.size(); ++i){
        DXGI_ADAPTER_DESC3 adapterDescription;
        vAdapters[i]->GetDesc3(&adapterDescription);
        std::wcout << 
                    "Adapter " <<
                    i <<
                    "\n\n\tDescription: " <<
                    std::wstring_view(adapterDescription.Description, wcsnlen(adapterDescription.Description, 128)) <<
                    "\n\tVendor ID: " <<
                    adapterDescription.VendorId <<
                    "\n\tDevice ID: " <<
                    adapterDescription.DeviceId <<
                    "\n\tSubsystem ID: " <<
                    adapterDescription.SubSysId <<
                    "\n\tRevision: " <<
                    adapterDescription.Revision <<
                    "\n\tDedicated Video Memory: " <<
                    adapterDescription.DedicatedVideoMemory <<
                    "\n\tDedicated System Memory: " <<
                    adapterDescription.DedicatedSystemMemory <<
                    "\n\tShared System Memory: " <<
                    adapterDescription.SharedSystemMemory <<
                    // "\n\tAdapter LUID: " <<
                    // adapterDescription.AdapterLuid <<
                    "\n\tFlags: " <<
                    adapterDescription.Flags <<
                    "\n\n";

        std::vector <Microsoft::WRL::ComPtr<IDXGIOutput6>> vOutputs; 
        for (UINT j = 0; ; ++j){
            Microsoft::WRL::ComPtr<IDXGIOutput6> pOutput;
            HRESULT hr = vAdapters[i]->EnumOutputs(j, (IDXGIOutput**)pOutput.GetAddressOf());
            if(hr == DXGI_ERROR_NOT_FOUND || FAILED(hr)){
                break;
            }
            vOutputs.push_back(std::move(pOutput));
        }

        for (UINT j = 0; j < vOutputs.size(); ++j){
            DXGI_OUTPUT_DESC1 outputDescription;
            vOutputs[j]->GetDesc1(&outputDescription);
            std::wcout << 
                        "\tOutput " <<
                        j <<
                        "\n\n\t\tDevice Name: " <<
                        std::wstring_view(outputDescription.DeviceName, wcsnlen(outputDescription.DeviceName, 32)) <<
                        // "\n\tDesktop Coordinates: " <<
                        // outputDescription.DesktopCoordinates <<
                        "\n\t\tAttached to desktop?: " <<
                        (outputDescription.AttachedToDesktop ? "True" : "False") <<
                        // "\n\t\tRotation: " <<
                        // outputDescription.Rotation <<
                        // "\n\t\tMonitor: " <<
                        // outputDescription.Monitor <<
                        "\n\n";
        }        
    }
    return 0;
}