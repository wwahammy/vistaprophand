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

#include <atlbase.h>
#include <vector>
#include <shobjidl.h>    // IInitializeWithStream, IDestinationStreamFactory
#include <propsys.h>     // Property System APIs and interfaces
#include <propkey.h>     // System PROPERTYKEY definitions
#include <propvarutil.h> // PROPVARIANT and VARIANT helper APIs
#include <wincrypt.h>    // CryptBinaryToString, CryptStringToBinary

#include <comutil.h>


#include <string>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <errno.h>
#include <algorithm>

HMODULE g_hmodThis;

#define TXMP_STRING_TYPE std::string
#define XMP_INCLUDE_XMPFILES 1
#include "XMP.incl_cpp"

#include "ErrorHandling.h"
#ifdef DEBUG
// Use Visual C++'s memory checking functionality

#define CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif // 


//Custom Definitions
#define kVideoFrameSize "videoFrameSize"
#define kVideoDimensionHeight "h"
#define kVideoDimensionWidth "w"
#define kVideoDimensionUnit "unit"
#define kVideoDuration "duration"
#define kVideoTimeValue "value"
#define kVideoTimeScale "scale"

BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    g_hmodThis = (HMODULE)hModule;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			#ifdef DEBUG
			//_crtBreakAlloc = 1828;
			_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF ); 
			#endif 
			SXMPMeta::Initialize();
			SXMPFiles::Initialize(kXMPFiles_NoQuickTimeInit);
			break;
		case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
            break;

        case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
            break;

        case DLL_PROCESS_DETACH:
			SXMPFiles::Terminate();
			SXMPMeta::Terminate();
            break;

	}
    return TRUE;
}

#if WIN_ENV
	#pragma warning ( disable : 4127 )	// conditional expression is constant
	#pragma warning ( disable : 4996 )	// '...' was declared deprecated
#endif
// Creative Commons definitions - begin

// URLS
#define by_nc_nd_url "http://creativecommons.org/licenses/by-nc-nd/3.0/"
#define by_nc_sa_url "http://creativecommons.org/licenses/by_nc_sa/3.0/"
#define by_nc_url "http://creativecommons.org/licenses/by_nc/3.0/"
#define by_nd_url "http://creativecommons.org/licenses/by_nd/3.0/"
#define by_sa_url "http://creativecommons.org/licenses/by_sa/3.0/"
#define by_url "http://creativecommons.org/licenses/by/3.0/"
#define devnations_url "http://creativecommons.org/licenses/devnations/2.0/"
#define gpl_url "http://creativecommons.org/licenses/GPL/2.0/"
#define lgpl_url "http://creativecommons.org/licenses/LGPL/2.1/"

//Names
#define by_nc_nd_name "Creative Commons Attribution Non_commercial No Derivatives"
#define by_nc_sa_name "Creative Commons Attribution Non_commercial Share Alike"
#define by_nc_name "Creative Commons Attribution Non_commercial"
#define by_nd_name "Creative Commons Attribution No Derivatives"
#define by_sa_name "Creative Commons Attribution Share Alike"
#define by_name "Creative Commons Attribution"
#define devnations_name "Developing Nations"
#define gpl_name "GNU General Public License"
#define lgpl_name "GNU Lesser General Public License"

//The CC schema URI
#define kXMP_CC "http://creativecommons.org/ns#"

// Creative Commons definitions - end


using namespace std;
/* {40C3D757-D6E4-4b49-BB41-0E5BBEA28817} */
static const CLSID CLSID_OrigAviHandler =
{ 0x40c3d757, 0xd6e4, 0x4b49, { 0xbb, 0xc41, 0xe, 0x5b, 0xbe, 0xa2, 0x88, 0x17 } };

//
// Releases the specified pointer if not NULL
//
#define SAFE_RELEASE(p) \
if (p)                  \
{                       \
    (p)->Release();     \
    (p) = NULL;         \
}

//
// Helper functions to opaquely serialize and deserialize PROPVARIANT values to and from string form
//
HRESULT SerializePropVariantAsString(REFPROPVARIANT propvar, PSTR *pszOut);
HRESULT DeserializePropVariantFromString(PCWSTR pszIn, PROPVARIANT *ppropvar);


//
// DLL lifetime management functions
//
void DllAddRef();
void DllRelease();


//
// Recipe property handler class definition
//
class AviPropertyStore :
    public IPropertyStore,
    public IPropertyStoreCapabilities,
    public IInitializeWithFile
{
public:
    static HRESULT CreateInstance(REFIID riid, void **ppv);

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(AviPropertyStore, IPropertyStore),
            QITABENT(AviPropertyStore, IPropertyStoreCapabilities),
            QITABENT(AviPropertyStore, IInitializeWithFile),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        DllAddRef();
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        DllRelease();
        ULONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IPropertyStore
    IFACEMETHODIMP GetCount(__out DWORD *pcProps);
    IFACEMETHODIMP GetAt(DWORD iProp, __out PROPERTYKEY *pkey);
    IFACEMETHODIMP GetValue(REFPROPERTYKEY key, __out PROPVARIANT *pPropVar);
    IFACEMETHODIMP SetValue(REFPROPERTYKEY key, REFPROPVARIANT propVar);
    IFACEMETHODIMP Commit();

    // IPropertyStoreCapabilities
    IFACEMETHODIMP IsPropertyWritable(REFPROPERTYKEY key);

    // IInitializeWithFile
    IFACEMETHODIMP Initialize(LPCWSTR pStream, DWORD grfMode);


protected:
    AviPropertyStore() :
        _cRef(1),
        _grfMode(0),
        _pCache(NULL),
		_pFile(NULL),
		_xmpMeta(0),
		_xmpFiles(0),
		_packet(0)
		
    {
		DllAddRef();
    }

    ~AviPropertyStore()
    {        
        //SAFE_RELEASE(_pCache);
		//SAFE_RELEASE(_pFile);
		delete _packet;
		delete _xmpMeta;
		delete _xmpFiles;
    }

    
    DWORD                _grfMode; // STGM mode passed to Initialize

    IPropertyStoreCache* _pCache;  // internal value cache to abstract IPropertyStore operations from the DOM back-end
	SXMPMeta * _xmpMeta;
	SXMPFiles * _xmpFiles;
	XMP_PacketInfo * _packet;
	LPCWSTR  _pFile;
	
	
	static bool _initialized;

	
	HRESULT _LoadCacheFromSXMPMeta();
	HRESULT _LoadProperties();

	HRESULT _SaveCacheToSXMPMeta();
	HRESULT _SaveProperties();
	HRESULT _SaveProperty(PROPERTYKEY key, REFPROPVARIANT propvar, SXMPMeta * _xmpMeta);

	//	helper to convert from float seconds to 100ns
	UINT64 _secToHundNS(float seconds);

	//	helper to split strings
	void Split(const std::string &str, const std::string &delim, std::vector<std::string> &output);

private:
    long _cRef;
};


//
// Instantiates a AVI property store object
//
HRESULT AviPropertyStore::CreateInstance(REFIID riid, void **ppv)
{
	
    HRESULT hr = E_OUTOFMEMORY;

    AviPropertyStore *pNew = new AviPropertyStore;

    if (pNew)
    {
        hr = pNew->QueryInterface(riid, ppv);
        SAFE_RELEASE(pNew);
    }

    return hr;
}

HRESULT AviPropertyStore_CreateInstance(REFIID riid, void **ppv)
{
    return AviPropertyStore::CreateInstance(riid, ppv);
}


//
// Accessor methods forward directly to internal value cache
//
HRESULT AviPropertyStore::GetCount(__out DWORD *pcProps)
{
	//BREAKPADSETUP("AviPropertyStore");
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {
        hr = _pCache->GetCount(pcProps);
    }

    return hr;
}

HRESULT AviPropertyStore::GetAt(DWORD iProp, __out PROPERTYKEY *pkey)
{
	//BREAKPADSETUP("AviPropertyStore");
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {
        hr = _pCache->GetAt(iProp, pkey);
    }

    return hr;
}

HRESULT AviPropertyStore::GetValue(REFPROPERTYKEY key, __out PROPVARIANT *pPropVar)
{
	//BREAKPADSETUP("AviPropertyStore");
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {	
        hr = _pCache->GetValue(key, pPropVar);
    }

    return hr;
}


//
// SetValue just updates the internal value cache
//
HRESULT AviPropertyStore::SetValue(REFPROPERTYKEY key, REFPROPVARIANT propVar)
{
	//BREAKPADSETUP("AviPropertyStore");
	HRESULT hr = E_UNEXPECTED;
	
    if (_pCache)
    {
        // check grfMode to ensure writes are allowed
        hr = STG_E_ACCESSDENIED;
        if (_grfMode & STGM_READWRITE 
            )  // this property is read-only
        {
			
            hr = _pCache->SetValueAndState(key, &propVar, PSC_DIRTY);
        }
    }
	
	
	
	
    return hr;
}


//
// Commit writes the internal value cache back out to the stream passed to Initialize
//
HRESULT AviPropertyStore::Commit()
{
	//BREAKPADSETUP("AviPropHandler");
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {
        // check grfMode to ensure writes are allowed
        hr = STG_E_ACCESSDENIED;
        if (_grfMode & STGM_READWRITE)
        {
			std::string str;
			_xmpFiles->GetFileInfo( &str, NULL, NULL, NULL);
            // reopen _xmpFiles for update
			_xmpFiles->CloseFile();
			
			

			
			if (_xmpFiles->OpenFile(str.c_str(), kXMP_AVIFile, kXMPFiles_OpenForUpdate))
				hr = _SaveCacheToSXMPMeta();
			
            if (SUCCEEDED(hr))
            {
				try
				{
					
					_xmpFiles->CloseFile();
					
				}
				catch (...)
				{
					hr = E_UNEXPECTED;
				}
				
            }

			
        }
    }
    return hr;
}


//
// Indicates whether the users should be able to edit values for the given property key
//
HRESULT AviPropertyStore::IsPropertyWritable(REFPROPERTYKEY key)
{
	
	if (IsEqualPropertyKey(key, PKEY_Keywords) | 
		IsEqualPropertyKey(key, PKEY_Title) |
		IsEqualPropertyKey(key, PKEY_Music_Artist) |
		IsEqualPropertyKey(key, PKEY_Comment)|
		IsEqualPropertyKey(key, PKEY_Rating)){
			return S_OK;
	}
	else {
		return S_FALSE;
	}
}


//
// Initialize populates the internal value cache with data from the specified stream
//
HRESULT AviPropertyStore::Initialize(LPCWSTR pStream, DWORD grfMode)
{
    HRESULT hr = E_UNEXPECTED;
    if (!_pFile)
    {
        hr = E_POINTER;
        if (pStream)
        {
			//_pFile = (LPCWSTR)CoTaskMemAlloc(char_traits<wchar_t>::length(pStream) +1);
			_pFile = new CONST WCHAR [ char_traits<wchar_t>::length(pStream) +2];
			_pFile = pStream;
			
			
				_grfMode = grfMode;
				
				 hr = _LoadCacheFromSXMPMeta();
				 if (SUCCEEDED(hr)) {
					 } 
				else {
					hr = E_FAIL;
				
			}
					
						
					

        }
    }
    return hr;
}


//
// Serializes a PROPVARIANT value to string form
//
HRESULT SerializePropVariantAsString(REFPROPVARIANT propvar, PSTR *pszOut)
{
    SERIALIZEDPROPERTYVALUE *pBlob;
    ULONG cbBlob;

    // serialize PROPVARIANT to binary form
    HRESULT hr = StgSerializePropVariant(&propvar, &pBlob, &cbBlob);
    if (SUCCEEDED(hr))
    {
        // determine the required buffer size
        hr = E_FAIL;
        DWORD cchString;
        if (CryptBinaryToStringA((BYTE *)pBlob, cbBlob, CRYPT_STRING_BASE64, NULL, &cchString))
        {
            // allocate a sufficient buffer
            hr = E_OUTOFMEMORY;
            *pszOut = (PSTR)CoTaskMemAlloc(sizeof(CHAR) * cchString);
            if (*pszOut)
            {
                // convert the serialized binary blob to a string representation
                hr = E_FAIL;
                if (CryptBinaryToStringA((BYTE *)pBlob, cbBlob, CRYPT_STRING_BASE64, *pszOut, &cchString))
                {
                    hr = S_OK;
                }
                else
                {
                    CoTaskMemFree(*pszOut);
                }
            }
        }

        CoTaskMemFree(pBlob);
    }

    return S_OK;
}


//
// Deserializes a string value back into PROPVARIANT form
//
HRESULT DeserializePropVariantFromString(PCWSTR pszIn, PROPVARIANT *ppropvar)
{
    HRESULT hr = E_FAIL;
    DWORD dwFormatUsed, dwSkip, cbBlob;

    // compute and validate the required buffer size
    if (CryptStringToBinaryW(pszIn, 0, CRYPT_STRING_BASE64, NULL, &cbBlob, &dwSkip, &dwFormatUsed) &&
        dwSkip == 0 &&
        dwFormatUsed == CRYPT_STRING_BASE64)
    {
        // allocate a buffer to hold the serialized binary blob
        hr = E_OUTOFMEMORY;
        BYTE *pbSerialized = (BYTE *)CoTaskMemAlloc(cbBlob);
        if (pbSerialized)
        {
            // convert the string to a serialized binary blob
            hr = E_FAIL;
            if (CryptStringToBinaryW(pszIn, 0, CRYPT_STRING_BASE64, pbSerialized, &cbBlob, &dwSkip, &dwFormatUsed))
            {
                // deserialized the blob back into a PROPVARIANT value
                hr = StgDeserializePropVariant((SERIALIZEDPROPERTYVALUE *)pbSerialized, cbBlob, ppropvar);
            }

            CoTaskMemFree(pbSerialized);
        }
    }

    return hr;
}

HRESULT AviPropertyStore::_LoadCacheFromSXMPMeta() {
	HRESULT hr = S_OK;
	

	if (!_pCache){
		hr = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&_pCache));
		if (SUCCEEDED(hr))
		{
			hr = _LoadProperties(); 
		}
	}
	return hr;
}

HRESULT AviPropertyStore::_LoadProperties() {
	
	 
	HRESULT hr = S_OK;
	#define BLANK ""
	_xmpMeta = new SXMPMeta();
	_xmpFiles = new SXMPFiles();
	_packet = new XMP_PacketInfo();
	BOOL ok;   
	try{
		int testLength = char_traits<wchar_t>::length(_pFile) +1;
		char * filename = (char *) CoTaskMemAlloc(testLength);
		filename[testLength-1] = '\0';
		
		errno_t err = wcstombs_s(NULL, filename,testLength, _pFile, testLength);
		if (err == 0)
		{
			ok = _xmpFiles->OpenFile(filename, kXMP_AVIFile, kXMPFiles_OpenForRead);
		}
		else 
		{
			ok = FALSE;
		}
		CoTaskMemFree( (void*) filename);
	}
	catch( ... )
	{
		hr = E_FAIL;
		return hr;
	}
	
	if (ok)
    {
		ok = _xmpFiles->GetXMP(_xmpMeta,0, _packet);
		
    }
    else
    {
        hr = E_FAIL;
    }

    if (FAILED(hr))
    {
		return hr;
    }
	
		{	

		wchar_t *wCharTemp;
		float vidTime;
		PROPVARIANT ppropvarValues = {0};
		PropVariantInit(&ppropvarValues);
		/* begin title - just gets x-default for now*/
		
		
		{
			

			std::string * s = new std::string(BLANK, 127);
			
			PCWSTR pcwstrValues;
			
				
			if(_xmpMeta->GetLocalizedText(kXMP_NS_DC, "title", 0, "x-default", 0, s, 0)){
				
				
				wchar_t *wCharTemp = (wchar_t *)CoTaskMemAlloc(s->length() * 2 + 2);
				mbstowcs(wCharTemp, s->c_str(), s->length() + 1);
				
				pcwstrValues = wCharTemp;
				

			
				hr = InitPropVariantFromString(pcwstrValues, &ppropvarValues);
				CoTaskMemFree((void*)wCharTemp);
			

				if (hr == S_OK){
					hr = PSCoerceToCanonicalValue(PKEY_Title,&ppropvarValues);
						if (SUCCEEDED(hr)) {
							hr = _pCache->SetValueAndState(PKEY_Title, &ppropvarValues, PSC_NORMAL);
						}
				}
			
			}
			
			PropVariantClear(&ppropvarValues);
			delete s;
		}
		
		if (hr != S_OK){
			return hr;
		}
	    
		/* end title */
		

		/* Get tags */
		{
		XMP_Int32 index = _xmpMeta->CountArrayItems(kXMP_NS_DC, "subject");

		PCWSTR * pcwstrValues = new PCWSTR[index];

		
		
		
		for (int i = 1 ; i< index + 1; i++){
			
			

			std::string * s = new std::string(BLANK, 127);
			int count = i - 1;
			if(_xmpMeta->GetArrayItem(kXMP_NS_DC, "subject", i, s, 0)){
				
				
				
				wchar_t *wCharTemp = (wchar_t *)CoTaskMemAlloc(s->length() * 2 + 2);
				mbstowcs(wCharTemp, s->c_str(), s->length() + 1);
			
				
				
				
				
				pcwstrValues[count] = wCharTemp;
				//CoTaskMemFree((void*)wCharTemp);
				
			}
			else {
				hr = E_FAIL;
			}
			delete s;
		}
		if (hr == S_OK){
			hr = InitPropVariantFromStringVector(pcwstrValues, index, &ppropvarValues);
		}
		for (long iValue = 0; iValue < index; iValue++)
					{
	                 CoTaskMemFree((void*)pcwstrValues[iValue]);
						
					}
		delete[] pcwstrValues;
		
		if (hr == S_OK){
			hr = PSCoerceToCanonicalValue(PKEY_Keywords,&ppropvarValues);
				if (SUCCEEDED(hr)) {
					hr = _pCache->SetValueAndState(PKEY_Keywords, &ppropvarValues, PSC_NORMAL);
				}
		}
		
		PropVariantClear(&ppropvarValues);
		}
		if (hr != S_OK){
			return hr;
		}
		/* end tags */
		{
		/* begin actors (or artists)
		
			XMP Dynamic Media schema does not use "bag" for artist like they should. 
			Instead it is simply a Text field. Since there can be more than one artist/actor
			artists are simply separated with semicolons (which is a standard I made up).  I 
			don't like this; Adobe should change this.
		
		*/
		

		
		std::string propVal(BLANK, 256);
		
		if(_xmpMeta->GetProperty(kXMP_NS_DM, "artist", &propVal, 0)){
			std::string delim(";");
			std::vector<std::string> outputVector;
			Split(propVal, delim, outputVector);
			
			
			int vectorSize = outputVector.size();
			PCWSTR * pcwstrValues = new PCWSTR[vectorSize];
			
			for(int i = 0; i< vectorSize; i++){
				
				wchar_t *wCharTemp = (wchar_t *)CoTaskMemAlloc(outputVector[i].length() * 2 + 2);
				mbstowcs(wCharTemp, outputVector[i].c_str(), outputVector[i].length() *2 + 1);
			
				pcwstrValues[i] = wCharTemp;
				//CoTaskMemFree((void*)wCharTemp);
				
			}
		
			hr = InitPropVariantFromStringVector(pcwstrValues, vectorSize, &ppropvarValues);
			for (long iValue = 0; iValue < vectorSize; iValue++)
					{
					  CoTaskMemFree((void*)pcwstrValues[iValue]);
					}
			delete[] pcwstrValues;
			

		
		if (hr == S_OK){
			hr = PSCoerceToCanonicalValue(PKEY_Music_Artist,&ppropvarValues);
				if (SUCCEEDED(hr)) {
					hr = _pCache->SetValueAndState(PKEY_Music_Artist, &ppropvarValues, PSC_NORMAL);
				}
		}

		PropVariantClear(&ppropvarValues);
		}}
		if (hr != S_OK){
			return hr;
		}

		/* end actors */
		
		/* begin user comment - just gets x-default for now*/
		
		{
		
			

			std::string s (BLANK, 129);
			
			PCWSTR pcwstrValues;
			//wchar_t *wCharTemp;
		
			if (_xmpMeta->GetLocalizedText(kXMP_NS_EXIF, "UserComment", NULL, "x-default", NULL, &s, 0)){
			
			
				//errno_t error = mbstowcs_s(0, wCharTemp, s.length() + 1, s.data(), s.length() + 1);
				wchar_t *wCharTemp = (wchar_t *)CoTaskMemAlloc(s.length() * 2 + 2);
				mbstowcs(wCharTemp, s.c_str(), s.length() + 1);
				
				pcwstrValues = wCharTemp;
				//CoTaskMemFree((void*)wCharTemp);
			
				

			
			hr = InitPropVariantFromString(pcwstrValues, &ppropvarValues);
			CoTaskMemFree((void*)pcwstrValues);
			if (hr == S_OK){
				hr = PSCoerceToCanonicalValue(PKEY_Comment,&ppropvarValues);
					if (SUCCEEDED(hr)) {
						hr = _pCache->SetValueAndState(PKEY_Comment, &ppropvarValues, PSC_NORMAL);
					}
			}}
			PropVariantClear(&ppropvarValues);
			
		}
		
		if (hr != S_OK){
			return hr;
		}
	    
		/* end user comment */



		/* begin rating */
		{
		
		LONG rating;
		
		if (_xmpMeta->GetProperty_Int(kXMP_NS_XMP , "rating", &rating, 0)){
			
			if (rating > 0){
			
				hr = InitPropVariantFromInt32(rating, &ppropvarValues);
				
				
				if (hr == S_OK){
					hr = PSCoerceToCanonicalValue(PKEY_Rating,&ppropvarValues);
						if (SUCCEEDED(hr)) {
							hr = _pCache->SetValueAndState(PKEY_Rating, &ppropvarValues, PSC_NORMAL);
						}
				}
				PropVariantClear(&ppropvarValues);
			}
		}

		}
		
		if (hr != S_OK){
			return hr;
		}
		/* end rating */

		/* begin thumbnail
		   This wonderful bit of engineering is needed because the shell thumbnail handler 
		   actually just calls the damn property handler. Why? Ask Microsoft. In addition,
		   the built-in property handler only implements IInitializeWithFile. Why?
		   Ask Microsoft.*/
	/*
		{
		
			 
			IPropertyStore *prop = NULL;
			
			hr = CoCreateInstance(CLSID_OrigAviHandler, NULL, CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&prop));
			IInitializeWithFile *interfaceInit = NULL;
			if (SUCCEEDED(hr)){
				hr = prop->QueryInterface(IID_IInitializeWithFile, (void **)&interfaceInit);
				STATSTG stats;
				hr = _pStream->Stat(&stats,STATFLAG_DEFAULT);
				//			hr = prop->Initialize(_pStream,STGM_READ);
				if (SUCCEEDED(hr)){
			//		hr= initializer->QueryInterface(IID_PPV_ARGS(&prop));
					if (SUCCEEDED(hr)){
						hr = prop->GetValue(PKEY_ItemNameDisplay, &ppropvarValues);
					}
				}
			}
			SAFE_RELEASE(prop);
			SAFE_RELEASE(interfaceInit);
			PropVariantClear(&ppropvarValues);
			
		}

		if (hr != S_OK){
			return hr;
		}
		/*end thumbnail */

		/* start duration */
		
		{
			std::string timeStr, scaleStr;
			
			ok = _xmpMeta->GetStructField(kXMP_NS_DM, kVideoDuration, kXMP_NS_DM, kVideoTimeValue, &timeStr, 0);
			ok = _xmpMeta->GetStructField(kXMP_NS_DM, kVideoDuration, kXMP_NS_DM, kVideoTimeScale, &scaleStr, 0);
			if (!ok){
				hr = E_FAIL;
			}

			if (hr == S_OK){
				float  frames;
				long scale1, scale2;
				std::string deliminator("/");
				std::vector<std::string> tempVector;
				Split(scaleStr, deliminator, tempVector);
				
				frames = SXMPUtils::ConvertToFloat(timeStr);
				scale1 = SXMPUtils::ConvertToInt(tempVector[0]);
				scale2 = SXMPUtils::ConvertToInt(tempVector[1]);;
				vidTime = frames * scale1 / scale2;

			
			hr =  InitPropVariantFromInt64(_secToHundNS(vidTime), &ppropvarValues);
			if (hr == S_OK){
				hr = PSCoerceToCanonicalValue(PKEY_Media_Duration,&ppropvarValues);
					if (SUCCEEDED(hr)) {
						hr = _pCache->SetValueAndState(PKEY_Media_Duration, &ppropvarValues, PSC_NORMAL);
					}
			}
			PropVariantClear(&ppropvarValues);
			}}
		
		if (hr != S_OK){
			return hr;
		}
		/* end duration */

		/*begin height */
		
		{
		unsigned long height;
		std::string heightStr;
		ok = _xmpMeta->GetStructField( kXMP_NS_DM, kVideoFrameSize,kXMP_NS_XMP_Dimensions, kVideoDimensionHeight,  &heightStr, 0);
		if (!ok){
				hr = E_FAIL;
			}

		if (hr == S_OK){
			height = SXMPUtils::ConvertToInt(heightStr);
		hr =  InitPropVariantFromUInt32(height, &ppropvarValues);
		if (hr == S_OK){
			hr = PSCoerceToCanonicalValue(PKEY_Video_FrameHeight,&ppropvarValues);
				if (SUCCEEDED(hr)) {
					hr = _pCache->SetValueAndState(PKEY_Video_FrameHeight, &ppropvarValues, PSC_NORMAL);
				}
		}
		PropVariantClear(&ppropvarValues);
			}}
		if (hr != S_OK){
			return hr;
		}
		/*end height */

		/*begin width */

		{
		unsigned long width;
		std::string widthStr;
		ok = _xmpMeta->GetStructField( kXMP_NS_DM, kVideoFrameSize,kXMP_NS_XMP_Dimensions, kVideoDimensionWidth,  &widthStr, 0);
		if (!ok){
				hr = E_FAIL;
			}

		if (hr == S_OK){
			
		width = SXMPUtils::ConvertToInt(widthStr);
		hr =  InitPropVariantFromUInt32(width, &ppropvarValues);
		if (hr == S_OK){
			hr = PSCoerceToCanonicalValue(PKEY_Video_FrameWidth,&ppropvarValues);
				if (SUCCEEDED(hr)) {
					hr = _pCache->SetValueAndState(PKEY_Video_FrameWidth, &ppropvarValues, PSC_NORMAL);
				}
		}
		PropVariantClear(&ppropvarValues); }}
		if (hr != S_OK){
			return hr;
		}
		/*end width */
		/* begin framerate */

		{
		unsigned long framerate;
		std::string framerateStr;
		ok = _xmpMeta->GetProperty(kXMP_NS_DM, "videoFrameRate", &framerateStr, 0);
		if (!ok){
				hr = E_FAIL;
			}
		if (hr == S_OK) {
			framerate = (unsigned long)(SXMPUtils::ConvertToFloat(framerateStr) * 1000);
			hr =  InitPropVariantFromUInt32(framerate, &ppropvarValues);
			if (hr == S_OK){
			hr = PSCoerceToCanonicalValue(PKEY_Video_FrameRate,&ppropvarValues);
				if (SUCCEEDED(hr)) {
					hr = _pCache->SetValueAndState(PKEY_Video_FrameRate, &ppropvarValues, PSC_NORMAL);
				}
		}
		PropVariantClear(&ppropvarValues);
		}}
		if (hr != S_OK){
			return hr;
		}
		
		/* end framerate */


		/*begin encoding (just video) bitrate *//*
		unsigned long encBitrate;
		encBitrate = _info->GetVideoBitRate();
		hr = InitPropVariantFromUInt32(encBitrate, &ppropvarValues);
		if (hr == S_OK){
			hr = PSCoerceToCanonicalValue(PKEY_Video_EncodingBitrate,&ppropvarValues);
				if (SUCCEEDED(hr)) {
					hr = _pCache->SetValueAndState(PKEY_Video_EncodingBitrate, &ppropvarValues, PSC_NORMAL);
				}
		}
		PropVariantClear(&ppropvarValues);
		if (hr != S_OK){
			return hr;
		}*/
		/*end encoding bitrate */
		
		/*begin total bitrate */ 
		/*
		{
		unsigned long audBitrate = _info->GetAudioBitRate(0);
		hr = InitPropVariantFromUInt32(encBitrate + audBitrate, &ppropvarValues);
		if (hr == S_OK){
			hr = PSCoerceToCanonicalValue(PKEY_Video_TotalBitrate,&ppropvarValues);
				if (SUCCEEDED(hr)) {
					hr = _pCache->SetValueAndState(PKEY_Video_TotalBitrate, &ppropvarValues, PSC_NORMAL);
				}
		}
		PropVariantClear(&ppropvarValues);
		if (hr != S_OK){
			return hr;
		}

		/* end total bitrate */
		
		//this is a passthrough to the original Property Store
		{
			CComPtr<IPropertyStore> pIPropStore;
			CComPtr<IInitializeWithFile> pInitFile;
			hr = CoCreateInstance(CLSID_OrigAviHandler,  NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIPropStore));
			if (SUCCEEDED(hr))
			{
				 hr = pIPropStore->QueryInterface(IID_PPV_ARGS(&pInitFile));
			}

			if (SUCCEEDED(hr))
			{
				hr = pInitFile->Initialize(_pFile, STGM_READ);
			}

			if (SUCCEEDED(hr))
			{
				
				hr = pIPropStore->GetValue(PKEY_Thumbnail, &ppropvarValues);
				if (SUCCEEDED(hr))
					_pCache->SetValue(PKEY_Thumbnail, ppropvarValues);
				PropVariantClear(&ppropvarValues);
			}

			if (SUCCEEDED(hr))
			{
				
				hr = pIPropStore->GetValue(PKEY_ThumbnailStream, &ppropvarValues);
				if (SUCCEEDED(hr))
					_pCache->SetValue(PKEY_ThumbnailStream, ppropvarValues);
				PropVariantClear(&ppropvarValues);
			}

		}


		
		}
	
	
	
	 
	
	return hr;
}

HRESULT AviPropertyStore::_SaveCacheToSXMPMeta(){
	
	

	DWORD cProps;

    HRESULT hr = _pCache->GetCount(&cProps);
    if (SUCCEEDED(hr))
    {
	
		
		
		
        for (UINT i = 0; SUCCEEDED(hr) && i < cProps; ++i)
        {
            PROPERTYKEY key;
            hr = _pCache->GetAt(i, &key);
            if (SUCCEEDED(hr))
            {
                // check the cache state; only save dirty properties
                PSC_STATE psc;
                hr = _pCache->GetState(key, &psc);
                if (SUCCEEDED(hr) && psc == PSC_DIRTY)
                {
                    // get the cached value
                    PROPVARIANT propvar = { 0 };
                    hr = _pCache->GetValue(key, &propvar);
                    if (SUCCEEDED(hr))
                    {
                        
                        hr = _SaveProperty(key, propvar, _xmpMeta);
                       
                        
                    }
					PropVariantClear(&propvar);
                }
            }
        }
		if (SUCCEEDED(hr))
			_xmpFiles->PutXMP(*_xmpMeta);
		
		
    }

	
	return hr;
}

HRESULT AviPropertyStore::_SaveProperty(PROPERTYKEY key, REFPROPVARIANT propvars, SXMPMeta * _xmpMeta){
	HRESULT hr = S_OK;
	if (IsEqualPropertyKey(key, PKEY_Keywords)){
			
			long index = _xmpMeta->CountArrayItems(kXMP_NS_DC, "subject");
			while (_xmpMeta->CountArrayItems(kXMP_NS_DC, "subject") > 0){
			
				_xmpMeta->DeleteArrayItem(kXMP_NS_DC, "subject", 1);
			
			}
			
			ULONG cValues = PropVariantGetElementCount(propvars);
			for (ULONG iValue = 0; SUCCEEDED(hr) && (iValue < cValues); iValue++)
			{
				PROPVARIANT propvarValue = {0};
				hr = PropVariantGetElem(propvars, iValue, &propvarValue);
				if (SUCCEEDED(hr))
				{
					PWSTR pszValue = (PWSTR)CoTaskMemAlloc( 255 );
					hr = PropVariantToString(propvarValue, pszValue, 255);
					size_t   i;
					
					
					char      *realStr = (char *)CoTaskMemAlloc( 255 );

					wcstombs_s(&i,realStr, (size_t)255, pszValue,(size_t)255);
					if (SUCCEEDED(hr)){
						
						_xmpMeta->AppendArrayItem(kXMP_NS_DC, "subject", kXMP_PropValueIsArray, realStr , 0);
						hr = S_OK;
					}
					else { hr = E_FAIL;}
					CoTaskMemFree((LPVOID)pszValue);
					CoTaskMemFree((LPVOID)realStr);
				}
				else { hr = E_FAIL;}
				PropVariantClear(&propvarValue);
			}
			long newIndex = _xmpMeta->CountArrayItems(kXMP_NS_DC, "subject");
			
	}
	
	else if (IsEqualPropertyKey(key,PKEY_Music_Artist)){
		//clean out the artist value
		_xmpMeta->DeleteProperty(kXMP_NS_DM, "artist");
		ULONG cValues = PropVariantGetElementCount(propvars);
		
		std::string realStr;
		for (ULONG iValue = 0; SUCCEEDED(hr) && (iValue < cValues); iValue++)
			{
				PROPVARIANT propvarValue = {0};
				
				hr = PropVariantGetElem(propvars, iValue, &propvarValue);
				if (SUCCEEDED(hr))
				{
					PWSTR pszValue = (PWSTR)CoTaskMemAlloc( 255 );
					hr = PropVariantToString(propvarValue, pszValue, 255);
					if (SUCCEEDED(hr)){
					size_t   i;
					
					char *tempStr = (char *)CoTaskMemAlloc(wcsnlen(pszValue, 255) +1);
					
					errno_t error_code = wcstombs_s(&i,tempStr, wcsnlen(pszValue, 255)+1, pszValue,wcsnlen(pszValue, 255)+1);
					if (error_code == 0){
							hr = S_OK;
						}
						else {
							hr = E_FAIL;
						}
					if (SUCCEEDED(hr)){
						
						realStr.append(tempStr);
						
						if (iValue < cValues - 1){
							realStr.append(";");
							
						}	
					}
					else { hr = E_FAIL;}
					CoTaskMemFree((LPVOID)pszValue);
					CoTaskMemFree((LPVOID)tempStr);
					
					}
					else { hr = E_FAIL;}
				}
				else { hr = E_FAIL;}
				PropVariantClear(&propvarValue);
			}
		_xmpMeta->SetProperty(kXMP_NS_DM, "artist",realStr.data(), 0);
		
	}
	
	else if (IsEqualPropertyKey(key, PKEY_Rating)){
		_xmpMeta->DeleteProperty(kXMP_NS_XMP, "rating");
	
		ULONG rating;
		hr = PropVariantGetUInt32Elem(propvars, 0, &rating);
		
		if (SUCCEEDED(hr)){
			_xmpMeta->SetProperty_Int(kXMP_NS_XMP, "rating", rating, 0);
		}
		else { hr = E_FAIL;}
		
	}
	else if (IsEqualPropertyKey(key, PKEY_Title))
	{
		PWSTR pszValue = (PWSTR)CoTaskMemAlloc( 255 );
		hr = PropVariantToString(propvars, pszValue, 255);
		if (SUCCEEDED(hr))
		{
			size_t   i;
					
			char *tempStr = (char *)CoTaskMemAlloc(wcsnlen(pszValue, 255) +1);
			
			errno_t error_code = wcstombs_s(&i,tempStr, wcsnlen(pszValue, 255)+1, pszValue,wcsnlen(pszValue, 255)+1);
			if (error_code == 0)
			{
					hr = S_OK;
				}
				else {
					hr = E_FAIL;
				}
			if (SUCCEEDED(hr))
			{
				std::string realStr;
				realStr.append(tempStr);
				_xmpMeta->SetLocalizedText(kXMP_NS_DC, "title",  0, "x-default", realStr.data(), 0); 
					
			}
			else { hr = E_FAIL;}
			CoTaskMemFree((LPVOID)tempStr);
			
		}
		else { hr = E_FAIL;}
		CoTaskMemFree((LPVOID)pszValue);
	}
	else {
		// don't know about this	
		hr = E_FAIL;
	}

	return hr;
}

UINT64 AviPropertyStore::_secToHundNS(float seconds){
	UINT64 returnUInt;
	returnUInt = seconds * 10000000;
	return returnUInt;
}

void AviPropertyStore::Split(const std::string& str, const std::string& delim, std::vector<std::string>& output)
{
    unsigned int offset = 0;
    unsigned int delimIndex = 0;
    
    delimIndex = str.find(delim, offset);

    while (delimIndex != string::npos)
    {
        output.push_back(str.substr(offset, delimIndex - offset));
        offset += delimIndex - offset + delim.length();
        delimIndex = str.find(delim, offset);
    }

    output.push_back(str.substr(offset));
}