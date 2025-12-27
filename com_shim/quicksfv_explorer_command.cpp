/*
 *   QuickerSFV - A fast checksum verifier
 *   Copyright (C) 2025  Andreas Weis (quickersfv@andreas-weis.net)
 *
 *   This file is part of QuickerSFV.
 *
 *   QuickerSFV is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   QuickerSFV is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*
 * Minimal COM Shim for Windows Explorer Integration
 * 
 * This file implements a minimal COM object that integrates with Windows Explorer
 * by implementing the IExplorerCommand interface. It calls into the Rust core
 * library for CRC32 computation.
 * 
 * Build with: cl /LD /EHsc quicksfv_explorer_command.cpp /link quicksfv_core.lib
 */

#ifndef _WIN32
#error "This file is Windows-specific and requires Windows headers"
#endif

#include <windows.h>
#include <shobjidl.h>
#include <shlwapi.h>
#include <atlbase.h>
#include <atlcom.h>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

// Import Rust library functions
extern "C" {
    uint32_t quicksfv_crc32(const uint8_t* ptr, size_t len);
    uint32_t quicksfv_crc32_file(const uint16_t* path_ptr);
    const uint8_t* quicksfv_version();
}

// {A5B8C3D2-1F2E-4D5A-9B3C-7E8F9A1B2C3D}
static const CLSID CLSID_QuickerSFVExplorerCommand = 
{ 0xa5b8c3d2, 0x1f2e, 0x4d5a, { 0x9b, 0x3c, 0x7e, 0x8f, 0x9a, 0x1b, 0x2c, 0x3d } };

// Module reference count for DLL unloading
static LONG g_moduleRefCount = 0;

// Helper function to increment module reference count
void ModuleAddRef() {
    InterlockedIncrement(&g_moduleRefCount);
}

// Helper function to decrement module reference count
void ModuleRelease() {
    InterlockedDecrement(&g_moduleRefCount);
}

// Get current module reference count
LONG ModuleGetRefCount() {
    return g_moduleRefCount;
}

/*
 * QuickerSFVExplorerCommand
 * 
 * Implements IExplorerCommand to provide context menu integration in Windows Explorer.
 * When invoked, it computes the CRC32 checksum of selected files using the Rust library.
 */
class QuickerSFVExplorerCommand : 
    public IExplorerCommand,
    public IObjectWithSite
{
private:
    LONG m_refCount;
    IUnknown* m_site;

public:
    QuickerSFVExplorerCommand() : m_refCount(1), m_site(nullptr) {
        ModuleAddRef();
    }

    virtual ~QuickerSFVExplorerCommand() {
        if (m_site) {
            m_site->Release();
        }
        ModuleRelease();
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (!ppv) return E_POINTER;

        *ppv = nullptr;

        if (riid == IID_IUnknown) {
            *ppv = static_cast<IUnknown*>(static_cast<IExplorerCommand*>(this));
        } else if (riid == IID_IExplorerCommand) {
            *ppv = static_cast<IExplorerCommand*>(this);
        } else if (riid == IID_IObjectWithSite) {
            *ppv = static_cast<IObjectWithSite*>(this);
        } else {
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release() {
        LONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) {
            delete this;
        }
        return count;
    }

    // IObjectWithSite
    STDMETHODIMP SetSite(IUnknown* pUnkSite) {
        if (m_site) {
            m_site->Release();
            m_site = nullptr;
        }
        if (pUnkSite) {
            m_site = pUnkSite;
            m_site->AddRef();
        }
        return S_OK;
    }

    STDMETHODIMP GetSite(REFIID riid, void** ppv) {
        if (!ppv) return E_POINTER;
        if (!m_site) return E_FAIL;
        return m_site->QueryInterface(riid, ppv);
    }

    // IExplorerCommand
    STDMETHODIMP GetTitle(IShellItemArray* psiItemArray, LPWSTR* ppszName) {
        if (!ppszName) return E_POINTER;
        
        // Allocate title string
        LPCWSTR title = L"Compute Hash (MTSFV)";
        size_t titleLen = wcslen(title) + 1;
        *ppszName = (LPWSTR)CoTaskMemAlloc(titleLen * sizeof(WCHAR));
        if (!*ppszName) return E_OUTOFMEMORY;
        
        wcscpy_s(*ppszName, titleLen, title);
        return S_OK;
    }

    STDMETHODIMP GetIcon(IShellItemArray* psiItemArray, LPWSTR* ppszIcon) {
        if (!ppszIcon) return E_POINTER;
        *ppszIcon = nullptr;
        return E_NOTIMPL; // Use default icon
    }

    STDMETHODIMP GetToolTip(IShellItemArray* psiItemArray, LPWSTR* ppszInfotip) {
        if (!ppszInfotip) return E_POINTER;
        
        LPCWSTR tooltip = L"Compute checksums using MTSFV";
        size_t tooltipLen = wcslen(tooltip) + 1;
        *ppszInfotip = (LPWSTR)CoTaskMemAlloc(tooltipLen * sizeof(WCHAR));
        if (!*ppszInfotip) return E_OUTOFMEMORY;
        
        wcscpy_s(*ppszInfotip, tooltipLen, tooltip);
        return S_OK;
    }

    STDMETHODIMP GetCanonicalName(GUID* pguidCommandName) {
        if (!pguidCommandName) return E_POINTER;
        *pguidCommandName = CLSID_QuickerSFVExplorerCommand;
        return S_OK;
    }

    STDMETHODIMP GetState(IShellItemArray* psiItemArray, BOOL fOkToBeSlow, EXPCMDSTATE* pCmdState) {
        if (!pCmdState) return E_POINTER;
        
        // Enable command for file selections
        *pCmdState = ECS_ENABLED;
        return S_OK;
    }

    STDMETHODIMP Invoke(IShellItemArray* psiItemArray, IBindCtx* pbc) {
        if (!psiItemArray) return E_INVALIDARG;

        DWORD count = 0;
        HRESULT hr = psiItemArray->GetCount(&count);
        if (FAILED(hr) || count == 0) {
            return E_INVALIDARG;
        }

        std::wstringstream results;
        results << L"MTSFV - Hash Computation Results\n";
        results << L"Using Rust core library v" 
                << reinterpret_cast<const char*>(quicksfv_version()) 
                << L"\n\n";

        // Process each selected file
        for (DWORD i = 0; i < count; i++) {
            IShellItem* pItem = nullptr;
            hr = psiItemArray->GetItemAt(i, &pItem);
            if (FAILED(hr)) continue;

            LPWSTR filePath = nullptr;
            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &filePath);
            if (SUCCEEDED(hr) && filePath) {
                // Call Rust library to compute CRC32
                uint32_t crc = quicksfv_crc32_file(reinterpret_cast<const uint16_t*>(filePath));
                
                // Format result
                results << PathFindFileNameW(filePath) << L": ";
                if (crc != 0) {
                    results << std::hex << std::uppercase << std::setw(8) << std::setfill(L'0') << crc;
                } else {
                    results << L"ERROR (file not found or read error)";
                }
                results << L"\n";

                CoTaskMemFree(filePath);
            }
            pItem->Release();
        }

        // Display results in message box
        MessageBoxW(nullptr, results.str().c_str(), L"MTSFV - Hash Results", 
                    MB_OK | MB_ICONINFORMATION);

        return S_OK;
    }

    STDMETHODIMP GetFlags(EXPCMDFLAGS* pFlags) {
        if (!pFlags) return E_POINTER;
        *pFlags = ECF_DEFAULT;
        return S_OK;
    }

    STDMETHODIMP EnumSubCommands(IEnumExplorerCommand** ppEnum) {
        if (!ppEnum) return E_POINTER;
        *ppEnum = nullptr;
        return E_NOTIMPL; // No subcommands
    }
};

/*
 * Class Factory for COM object creation
 */
class QuickerSFVClassFactory : public IClassFactory {
private:
    LONG m_refCount;

public:
    QuickerSFVClassFactory() : m_refCount(1) {
        ModuleAddRef();
    }

    virtual ~QuickerSFVClassFactory() {
        ModuleRelease();
    }

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) {
        if (!ppv) return E_POINTER;

        *ppv = nullptr;

        if (riid == IID_IUnknown || riid == IID_IClassFactory) {
            *ppv = static_cast<IClassFactory*>(this);
        } else {
            return E_NOINTERFACE;
        }

        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() {
        return InterlockedIncrement(&m_refCount);
    }

    STDMETHODIMP_(ULONG) Release() {
        LONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) {
            delete this;
        }
        return count;
    }

    // IClassFactory
    STDMETHODIMP CreateInstance(IUnknown* pUnkOuter, REFIID riid, void** ppv) {
        if (!ppv) return E_POINTER;

        *ppv = nullptr;

        if (pUnkOuter) {
            return CLASS_E_NOAGGREGATION;
        }

        QuickerSFVExplorerCommand* pCommand = new (std::nothrow) QuickerSFVExplorerCommand();
        if (!pCommand) {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = pCommand->QueryInterface(riid, ppv);
        pCommand->Release();

        return hr;
    }

    STDMETHODIMP LockServer(BOOL fLock) {
        if (fLock) {
            ModuleAddRef();
        } else {
            ModuleRelease();
        }
        return S_OK;
    }
};

/*
 * DLL Entry Point
 */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

/*
 * DLL Exports for COM registration
 */

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv) {
    if (!ppv) return E_POINTER;

    *ppv = nullptr;

    if (rclsid != CLSID_QuickerSFVExplorerCommand) {
        return CLASS_E_CLASSNOTAVAILABLE;
    }

    QuickerSFVClassFactory* pFactory = new (std::nothrow) QuickerSFVClassFactory();
    if (!pFactory) {
        return E_OUTOFMEMORY;
    }

    HRESULT hr = pFactory->QueryInterface(riid, ppv);
    pFactory->Release();

    return hr;
}

STDAPI DllCanUnloadNow() {
    return (ModuleGetRefCount() == 0) ? S_OK : S_FALSE;
}

/*
 * Registration helper functions
 */

STDAPI DllRegisterServer() {
    // In production, this would register the COM class and shell extension
    // For now, return success to indicate the DLL can be registered
    // Actual registration should be done via .reg file or installer
    return S_OK;
}

STDAPI DllUnregisterServer() {
    // In production, this would unregister the COM class
    return S_OK;
}
