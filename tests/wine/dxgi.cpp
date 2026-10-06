#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <D3d12.h>
#include <dxgi.h>
#include <vector>
#include <iostream>
#include <string_view>

int main()
{
    IDXGIFactory1 *pFactory;
    HRESULT factoryResult = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**) &pFactory);
    if (factoryResult != S_OK){
        return 1;
    }

    UINT i = 0; 
    IDXGIAdapter1 * pAdapter; 
    std::vector <IDXGIAdapter1*> vAdapters; 
    while(pFactory->EnumAdapters1(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) 
    { 
        vAdapters.push_back(pAdapter); 
        ++i; 
    }

    std::cout << "Adapters: " << vAdapters.size() << '\n';

    for (i = 0; i < vAdapters.size(); ++i){
        pAdapter = vAdapters[i];
        DXGI_ADAPTER_DESC1 adapterDescription;
        pAdapter->GetDesc1(&adapterDescription);
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

        UINT j = 0; 
        IDXGIOutput * pOutput; 
        std::vector <IDXGIOutput*> vOutputs; 
        while(pAdapter->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND) 
        { 
            vOutputs.push_back(pOutput); 
            ++j; 
        }

        for (j = 0; j < vOutputs.size(); ++j){
            pOutput = vOutputs[j];
            DXGI_OUTPUT_DESC outputDescription;
            pOutput->GetDesc(&outputDescription);
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
            
            pOutput->Release();
        }        
        pAdapter->Release();
    }
    pFactory->Release();


    return 0;
}