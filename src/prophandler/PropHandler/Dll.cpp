//  Vista Metadata Project
//  Copyright (C) 2007-2008  Eric Schultz

//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; specifically version 2 of the License.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License along
//  with this program; if not, write to the Free Software Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#include <shlwapi.h>
#include <shlobj.h>  // SHChangeNotify
#include "atlbase.h"
#include "searchapi.h"
#include <msi.h>


//
// CLSID of the avi property handler object
//
// {5A07F0A8-60E4-474b-89CF-8705E4260D22}
const CLSID CLSID_AviPropertyStore = 
{ 0x5a07f0a8, 0x60e4, 0x474b, { 0x89, 0xcf, 0x87, 0x5, 0xe4, 0x26, 0xd, 0x22 } };


#define SZ_PROPERTYHANDLERSPATH         L"Software\\Microsoft\\Windows\\CurrentVersion\\PropertySystem\\PropertyHandlers"
#define SZ_APPROVEDSHELLEXTENSIONS      L"Software\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Approved"
#define SZ_AVIPROPERTYHANDLER			L"AVI Property Handler"
#define SZ_CLSID_AVIPROPERTYHANDLER		L"{5A07F0A8-60E4-474b-89CF-8705E4260D22}"
#define SZ_FILE_EXTENSION				L".avi"
#define SZ_SYSTEM_FILE_ASSOCIATIONS		L"SystemFileAssociations"


#define DEFAULT_PREVIEW_DETAILS L"prop:*System.Media.Duration;*System.Size;*System.Video.FrameWidth;*System.Video.FrameHeight;*System.OfflineAvailability;*System.OfflineStatus;*System.FileOwner;*System.DateModified;*System.DateCreated;*System.SharedWith;*System.Video.FrameRate;*System.Video.EncodingBitrate;*System.Video.TotalBitrate"
#define DEFAULT_INFO_TIP L"prop:System.ItemType;System.Size;System.Media.Duration;System.OfflineAvailability"
#define DEFAULT_FULL_DETAILS L"prop:System.PropGroup.Video;System.Media.Duration;System.Video.FrameWidth;System.Video.FrameHeight;System.Video.EncodingBitrate;System.Video.TotalBitrate;System.Video.FrameRate;System.PropGroup.Audio;System.Audio.EncodingBitrate;System.PropGroup.FileSystem;System.ItemNameDisplay;System.ItemType;System.ItemFolderPathDisplay;System.Size;System.DateCreated;System.DateModified;System.FileAttributes;System.OfflineAvailability;System.OfflineStatus;System.SharedWith;System.FileOwner;System.ComputerName"
#define DEFAULT_EXTENDED_TILE_INFO L"prop:System.ItemType;System.Size;System.Media.Duration;System.OfflineAvailability"

#define DEFAULT_PROPERTYHANDLER_CLSID L"{40C3D757-D6E4-4b49-BB41-0E5BBEA28817}"
#define DEFAULT_PERSISTENTHANDLER_CLSID L"{098f2470-bae0-11cd-b579-08002b30bfeb}"

#define NEW_PREVIEW_DETAILS L"prop:*System.Title;*System.Media.Duration;*System.Size;*System.Video.FrameWidth;*System.Video.FrameHeight;System.Rating;System.Keywords;*System.Comment;System.Music.Artist;*System.Music.Genre;*System.ParentalRating;*System.OfflineAvailability;*System.OfflineStatus;*System.DateModified;*System.DateCreated;*System.SharedWith;*System.Media.SubTitle;*System.Media.Year;*System.Video.FrameRate;*System.Video.EncodingBitrate;*System.Video.TotalBitrate"
#define NEW_INFO_TIP L"prop:System.ItemType;System.Size;System.Media.Duration;System.OfflineAvailability"
#define NEW_FULL_DETAILS L"prop:System.PropGroup.Description;System.Title;System.Media.SubTitle;System.Rating;System.Keywords;System.Comment;System.PropGroup.Video;System.Media.Duration;System.Video.FrameWidth;System.Video.FrameHeight;System.Video.EncodingBitrate;System.Video.TotalBitrate;System.Video.FrameRate;System.PropGroup.Audio;System.Audio.EncodingBitrate;System.Audio.ChannelCount;System.Audio.SampleRate;System.PropGroup.Media;System.Music.Artist;System.Media.Year;System.Music.Genre;System.PropGroup.Origin;System.Video.Director;System.Media.Producer;System.Media.Writer;System.Media.Publisher;System.Media.ContentDistributor;System.Media.DateEncoded;System.Media.EncodedBy;System.Media.AuthorUrl;System.Media.PromotionUrl;System.Copyright;System.PropGroup.Content;System.ParentalRating;System.ParentalRatingReason;System.Music.Composer;System.Music.Conductor;System.Music.Period;System.Music.Mood;System.Music.PartOfSet;System.Music.InitialKey;System.Music.BeatsPerMinute;System.DRM.IsProtected;System.PropGroup.FileSystem;System.ItemNameDisplay;System.ItemType;System.ItemFolderPathDisplay;System.Size;System.DateCreated;System.DateModified;System.FileAttributes;System.OfflineAvailability;System.OfflineStatus;System.SharedWith;System.FileOwner;System.ComputerName"
#define NEW_EXTENDED_TILE_INFO L"prop:System.ItemType;System.Size;System.Media.Duration;System.OfflineAvailability"

#define NEW_PERSISTENTHANDLER_CLSID L""
//
// Reference count for the DLL
//
LONG g_cLocks = 0;


//
// Handle the the DLL's module
//



//
// Standard DLL functions
//
/*
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    g_hmodThis = (HMODULE)hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		XMPFiles::
	}
    return TRUE;
}
*/
STDAPI DllCanUnloadNow()
{
    // Only allow the DLL to be unloaded after all outstanding references have been released
    return (g_cLocks == 0) ? S_OK : S_FALSE;
}

void DllAddRef()
{
    // Increment the reference count on the DLL
    InterlockedIncrement(&g_cLocks);
}

void DllRelease()
{
    // Decrement the reference count on the DLL
    InterlockedDecrement(&g_cLocks);
}

//
// A struct to hold the information required for a registry entry
// hkeyRoot is the HKEY
//	pszKeyName is the actual key
// pszValueName is the value you want to set (NULL if its the default)
// dwType is the type of value you're entering (REG_SZ if its a string, REG_DWORD if its a dword)
// pszData is the string value if dwType is REG_SZ, otherwise just put in blank string
// dwData is the dword value if dwType is REG_DWORD, otherwise just put in 0
//
/*
struct REGISTRY_ENTRY
{
    HKEY    hkeyRoot;
    LPCWSTR pszKeyName;
    LPCWSTR pszValueName;
    DWORD   dwType;
    LPCWSTR pszData;
    DWORD   dwData;
};
*/

//
// Creates a registry key (if needed) and sets the default value of the key
//
/*
HRESULT CreateRegKeyAndSetValue(REGISTRY_ENTRY *pRegistryEntry)
{
    HRESULT hr = E_INVALIDARG;

    if (pRegistryEntry != NULL)
    {
        // create the key, or obtain its handle if it already exists
        HKEY hKey;
        LONG lr = RegCreateKeyExW(pRegistryEntry->hkeyRoot,
                                  pRegistryEntry->pszKeyName, 
                                  0, 
                                  NULL, 
                                  REG_OPTION_NON_VOLATILE,
                                  KEY_ALL_ACCESS, 
                                  NULL, 
                                  &hKey, 
                                  NULL);
        hr = HRESULT_FROM_WIN32(lr);
        if (SUCCEEDED(hr))
        {
            // extract the data from the struct according to its type
            LPBYTE pData = NULL;
            DWORD cbData = 0;
            hr = S_OK;
            switch (pRegistryEntry->dwType)
            {
            case REG_SZ:
                pData = (LPBYTE) pRegistryEntry->pszData;
                cbData = ((DWORD) wcslen(pRegistryEntry->pszData) + 1) * sizeof(WCHAR);
                break;

            case REG_DWORD:
                pData = (LPBYTE) &pRegistryEntry->dwData;
                cbData = sizeof(pRegistryEntry->dwData);
                break;

            default:
                hr = E_INVALIDARG;
                break;
            }

            if (SUCCEEDED(hr))
            {
                // attempt to set the value
                lr = RegSetValueExW(hKey,
                                    pRegistryEntry->pszValueName,
                                    0,
                                    pRegistryEntry->dwType,
                                    pData,
                                    cbData);
                hr = HRESULT_FROM_WIN32(lr);
            }

            RegCloseKey(hKey);
        }
    }

    return hr;
}
*/
//
// Registers this COM server
//
/*
STDAPI DllRegisterServer()
{
    HRESULT hr = E_FAIL;
 
    WCHAR szModuleName[MAX_PATH];

    if (!GetModuleFileNameW(g_hmodThis, szModuleName, ARRAYSIZE(szModuleName)))
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
        //
        // List of property-handler specific registry entries we need to create
        //
        REGISTRY_ENTRY rgRegistryEntries[] =
            {
                //
                // COM information
                //
                {
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\CLSID\\" SZ_CLSID_AVIPROPERTYHANDLER,
                    NULL,
                    REG_SZ,
                    SZ_AVIPROPERTYHANDLER,
                    0
                },
				{
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\CLSID\\" SZ_CLSID_AVIPROPERTYHANDLER,
                    L"ManualSafeSave",
                    REG_DWORD,
                    NULL,
                    1
                },
                {
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\CLSID\\" SZ_CLSID_AVIPROPERTYHANDLER L"\\InProcServer32",
                    NULL,
                    REG_SZ,
                    szModuleName,
                    0
                },
                {
                    HKEY_LOCAL_MACHINE,
                    L"SOFTWARE\\Classes\\CLSID\\" SZ_CLSID_AVIPROPERTYHANDLER L"\\InProcServer32",
                    L"ThreadingModel",
                    REG_SZ,
                    L"Apartment",
                    0
                },

                //
                // Shell information
                //
                {
                    HKEY_LOCAL_MACHINE,
                    SZ_PROPERTYHANDLERSPATH L"\\" SZ_FILE_EXTENSION,
                    NULL,
                    REG_SZ,
                    SZ_CLSID_AVIPROPERTYHANDLER,
                    0
                },
			/*	{
                    HKEY_CLASSES_ROOT,
                    L".avi\\PersistentHandler",
                    NULL,
                    REG_SZ,
                    NEW_PERSISTENTHANDLER_CLSID,
                    0
                },/
                {
                    HKEY_CLASSES_ROOT,
                    SZ_SYSTEM_FILE_ASSOCIATIONS L"\\" SZ_FILE_EXTENSION,
                    L"PreviewDetails",
                    REG_SZ,
                    NEW_PREVIEW_DETAILS,
                    0
                },
				{
                    HKEY_LOCAL_MACHINE,
                    SZ_APPROVEDSHELLEXTENSIONS,
                    SZ_CLSID_AVIPROPERTYHANDLER,
                    REG_SZ,
                    SZ_AVIPROPERTYHANDLER,
                    0
                }
            };

        hr = S_OK;
        for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
        {
            hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i]);
        }
		//	SHDeleteKeyW(HKEY_CLASSES_ROOT, L".avi\\PersistentHandler");
        if (SUCCEEDED(hr))
        {
            // inform Explorer, et al of the new handler
            SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
        }
    }
 
    return hr;
}
*/
 
//
// Unregisters this COM server
//
/*
STDAPI DllUnregisterServer()
{
    HRESULT hr = S_OK;

	// VERY IMPORTANT!!!!!! This needs to put the original extensions data back into the registry... VE
    // attempt to delete everything, even if some operations fail
    DWORD dwCLSID =           SHDeleteKeyW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Classes\\CLSID\\" SZ_CLSID_AVIPROPERTYHANDLER);
    //DWORD dwPropertyHandler = SHDeleteKeyW(HKEY_LOCAL_MACHINE, SZ_PROPERTYHANDLERSPATH L"\\" SZ_FILE_EXTENSION);
    DWORD dwShellExtension =  SHDeleteValueW(HKEY_LOCAL_MACHINE, SZ_APPROVEDSHELLEXTENSIONS, SZ_CLSID_AVIPROPERTYHANDLER);

    // return first error encountered as HRESULT
    if (dwCLSID != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwCLSID);
    }
    else if (dwShellExtension != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(dwShellExtension);
    }
///the original registry data to be restoreed

	REGISTRY_ENTRY rgRegistryEntries[] = {
		{			HKEY_LOCAL_MACHINE,
                    SZ_PROPERTYHANDLERSPATH L"\\" SZ_FILE_EXTENSION,
                    NULL,
                    REG_SZ,
                    DEFAULT_PROPERTYHANDLER_CLSID,
                    0
		},
	/*	{
					HKEY_CLASSES_ROOT,
					L".avi\\PersistentHandler",
					NULL,
					REG_SZ,
					DEFAULT_PERSISTENTHANDLER_CLSID,
					0
        },/
		{
                    HKEY_CLASSES_ROOT,
                    SZ_SYSTEM_FILE_ASSOCIATIONS L"\\" SZ_FILE_EXTENSION,
                    L"PreviewDetails",
                    REG_SZ,
                    DEFAULT_PREVIEW_DETAILS,
                    0
        }
	};

    for (int i = 0; i < ARRAYSIZE(rgRegistryEntries) && SUCCEEDED(hr); i++)
    {
		hr = CreateRegKeyAndSetValue(&rgRegistryEntries[i]);
    }

    if (SUCCEEDED(hr))
    {
        // inform Explorer, et al that the handler has been unregistered
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
    }

    return hr;
}
*/

//
// Constructor for AviPropertyStore
//
HRESULT AviPropertyStore_CreateInstance(REFIID riid, void **ppv);


//
// Class factory for CRecipePropertyHandler
//
class AviPropertyHandlerClassFactory  : public IClassFactory
{
public:
    //
    // IUnknown methods
    //
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(AviPropertyHandlerClassFactory, IClassFactory),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()  
    {
        DllAddRef();
        return 2; // Object's lifetime is tied to the DLL, so no need to track its refcount
    }

    IFACEMETHODIMP_(ULONG) Release() 
    {
        DllRelease();
        return 1; // Object's lifetime is tied to the DLL, so no need to track its refcount or manually delete it
    }

    //
    // IClassFactory methods
    //
    IFACEMETHODIMP CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
    {
        // Aggregation is not supported
        return pUnkOuter ? CLASS_E_NOAGGREGATION : AviPropertyStore_CreateInstance(riid, ppv);
    }

    IFACEMETHODIMP LockServer(BOOL fLock)
    {
        if (fLock)
        {
            DllAddRef();
        }
        else
        {
            DllRelease();
        }

        return S_OK;
    }
};

AviPropertyHandlerClassFactory g_cfAviPropertyHandler;

//
// Export called by CoCreateInstance to obtain a class factory for the specified CLSID
//
STDAPI DllGetClassObject(REFCLSID clsid, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    // we only support CLSID_AviPropertyStore
    if (clsid == CLSID_AviPropertyStore)
    {
        hr = g_cfAviPropertyHandler.QueryInterface(riid, ppv);
    }

    return hr;
}
/*
STDAPI PostSetupRun()
{
	CComPtr<ISearchCatalogManager> searchcat;
	HRESULT hr = E_FAIL;
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);

	hr = CoCreateInstance(CLSID_CSearchCatalogManager, 0, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&searchcat));
	if (SUCCEEDED(hr))
	{
		hr = searchcat->ReindexMatchingURLs(L"*.avi");
		
	}
	
	return ERROR_SUCCESS;
}*/