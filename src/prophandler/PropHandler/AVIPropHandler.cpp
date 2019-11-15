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
#include <comutil.h>     // _bstr_t
#include <wincrypt.h>    // CryptBinaryToString, CryptStringToBinary
#include <strsafe.h>     // StringCchPrintf
#include <comutil.h>
#include "version.h"

#include <string>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdexcept>
#include <errno.h>

#include "ErrorHandling.h"
#define TXMP_STRING_TYPE std::string
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


#define TXMP_STRING_TYPE std::string
#define XMP_INCLUDE_XMPFILES 1
#include "XMP.hpp"

using namespace std;
/* {40C3D757-D6E4-4b49-BB41-0E5BBEA28817} */
const CLSID CLSID_OrigAviHandler =
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
    public IInitializeWithStream
{
public:
    static HRESULT CreateInstance(REFIID riid, void **ppv);

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        static const QITAB qit[] = {
            QITABENT(AviPropertyStore, IPropertyStore),
            QITABENT(AviPropertyStore, IPropertyStoreCapabilities),
            QITABENT(AviPropertyStore, IInitializeWithStream),
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

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);


protected:
    AviPropertyStore() :
        _cRef(1),
        _pStream(NULL),
        _grfMode(0),
        _pCache(NULL)
		
    {
	//InitializeCriticalSectionAndSpinCount(&OneXMPCritSection, 
      //  0x80000400) ;
        
        
		DllAddRef();
	
    }

    ~AviPropertyStore()
    {
        //DeleteCriticalSection(&OneXMPCritSection);
		SAFE_RELEASE(_pStream);
        
        SAFE_RELEASE(_pCache);
		

		
	
    }

    IStream*             _pStream; // data stream passed in to Initialize, and saved to on Commit
    DWORD                _grfMode; // STGM mode passed to Initialize

    IPropertyStoreCache* _pCache;  // internal value cache to abstract IPropertyStore operations from the DOM back-end
	
	

	//CRITICAL_SECTION OneXMPCritSection;
	HRESULT _LoadCacheFromSXMPMeta();
	HRESULT _LoadProperties();

	HRESULT _SaveCacheToSXMPMeta();
	HRESULT _SaveProperties();
	HRESULT _SaveProperty(PROPERTYKEY key, REFPROPVARIANT propvar, SXMPMeta * _xmpMeta);

	//	helper to convert from float seconds to 100ns
	UINT64 _secToHundNS(float seconds);

	//	helper to split strings
	void Split(const std::string& str, const std::string& delim, std::vector<std::string>& output);

private:
    long _cRef;
};


//
// Instantiates a recipe property store object
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
	
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {
        hr = _pCache->GetCount(pcProps);
    }

    return hr;
}

HRESULT AviPropertyStore::GetAt(DWORD iProp, __out PROPERTYKEY *pkey)
{
	
    HRESULT hr = E_UNEXPECTED;
    if (_pCache)
    {
        hr = _pCache->GetAt(iProp, pkey);
    }

    return hr;
}

HRESULT AviPropertyStore::GetValue(REFPROPERTYKEY key, __out PROPVARIANT *pPropVar)
{
	
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
            // save the internal value cache to XML DOM object
            hr = _SaveCacheToSXMPMeta();
            if (SUCCEEDED(hr))
            {
				
				_pStream->Commit(STGC_DEFAULT);
                
				
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
	//BREAKPADSETUP("AviPropHandler");
	if (IsEqualPropertyKey(key, PKEY_Media_Duration) | 
		IsEqualPropertyKey(key, PKEY_Audio_Compression) |
		IsEqualPropertyKey(key, PKEY_Audio_EncodingBitrate) |
		IsEqualPropertyKey(key, PKEY_Audio_Format) | 
		IsEqualPropertyKey(key, PKEY_Audio_SampleRate) | 
		IsEqualPropertyKey(key, PKEY_Video_FrameHeight) |
		IsEqualPropertyKey(key, PKEY_Video_FrameWidth) |
		IsEqualPropertyKey(key, PKEY_Video_FrameRate) | 
		IsEqualPropertyKey(key, PKEY_Video_TotalBitrate) | 
		IsEqualPropertyKey(key, PKEY_Video_EncodingBitrate) |
		IsEqualPropertyKey(key, PKEY_Audio_ChannelCount)){
			return S_FALSE;
	}
	else {
		return S_OK;
	}
}


//
// Initialize populates the internal value cache with data from the specified stream
//
HRESULT AviPropertyStore::Initialize(IStream *pStream, DWORD grfMode)
{

    HRESULT hr = E_UNEXPECTED;
    if (!_pStream)
    {
        hr = E_POINTER;
        if (pStream)
        {
            

          /*  if (SUCCEEDED(hr))
            {*/
                // load the DOM object's contents from the stream
            
					
						// load the internal value cache from the DOM object
						
					
							
							// save a reference to the stream as well as the grfMode
							hr = pStream->QueryInterface(IID_PPV_ARGS(&_pStream));
							if (SUCCEEDED(hr))
							{
								_grfMode = grfMode;
								
								 hr = _LoadCacheFromSXMPMeta();
								 if (SUCCEEDED(hr)) {
									 } 
								else {
									hr = E_FAIL;
								}
							}
					
						
					
            //}
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

HRESULT AviPropertyStore::_LoadCacheFromSXMPMeta(void) {
	HRESULT hr = S_OK;
	

	if (!_pCache){
		hr = PSCreateMemoryPropertyStore(IID_PPV_ARGS(&_pCache));
		if (SUCCEEDED(hr))
		{
			_LoadProperties(); 
		}
	}
	return hr;
}

HRESULT AviPropertyStore::_LoadProperties(void) {
	
	//EnterCriticalSection(&OneXMPCritSection); 
		XMP_PacketInfo		_xmpPacket;
		const char *blank = "";
		bool meta = SXMPMeta::Initialize();
		bool files = SXMPFiles::Initialize();
	
	HRESULT hr = S_OK;
	{	

		
		
			
	SXMPFiles			 _xmpFile;
	SXMPMeta			 _xmpMeta;
	BOOL ok;   
	
			ok = _xmpFile.OpenFile(_pStream, kXMP_UnknownFile, kXMPFiles_OpenForUpdate);
                if (ok)
                {
					ok = _xmpFile.GetXMP(&_xmpMeta,0, &_xmpPacket);
					
                }
                else
                {
                    hr = E_FAIL;
                }

                if (FAILED(hr))
                {
					return hr;
                }
	
	float vidTime;
		PROPVARIANT ppropvarValues = {0};
	PropVariantInit(&ppropvarValues);
	/* begin title - just gets x-default for now*/
	
	{
		

		std::string s (blank, 129);
		
		PCWSTR pcwstrValues;
		
			
		if(_xmpMeta.GetLocalizedText(kXMP_NS_DC, "title", 0, "x-default", 0, &s, 0)){
			
			wchar_t *wCharTemp = new wchar_t[s.length()];
			errno_t error = mbstowcs_s(0, wCharTemp, s.length() + 1, s.data(), s.length() + 1);
		
		
			pcwstrValues = wCharTemp;
			
		
			

		
		hr = InitPropVariantFromString(pcwstrValues, &ppropvarValues);

		

		if (hr == S_OK){
			hr = PSCoerceToCanonicalValue(PKEY_Title,&ppropvarValues);
				if (SUCCEEDED(hr)) {
					hr = _pCache->SetValueAndState(PKEY_Title, &ppropvarValues, PSC_NORMAL);
				}
		}
		
		}
		
		PropVariantClear(&ppropvarValues);
		
	}
	
	if (hr != S_OK){
		return hr;
	}
    
	/* end title */


	/* Get tags */
	{
	XMP_Int32 index = _xmpMeta.CountArrayItems(kXMP_NS_DC, "subject");

	PCWSTR * pcwstrValues = new PCWSTR[index];

	
	
	
	for (int i = 1 ; i< index + 1; i++){
		
		

		std::string s (blank, 127);
		int count = i - 1;
		if(_xmpMeta.GetArrayItem(kXMP_NS_DC, "subject", i, &s, 0)){
			wchar_t *wCharTemp = new wchar_t[s.length()];
			mbstowcs_s(0, wCharTemp, s.length() + 1, s.data(), s.length() + 1);
			
			
			pcwstrValues[count] = wCharTemp;
			
			
		}
		else {
			hr = E_FAIL;
		}
	}
	if (hr == S_OK){
		hr = InitPropVariantFromStringVector(pcwstrValues, index, &ppropvarValues);
	}
	for (long iValue = 0; iValue < index; iValue++)
                {
                    
					
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
	

	
	std::string propVal (blank, 258);
	
	if(_xmpMeta.GetProperty(kXMP_NS_DM, "artist", &propVal, 0)){
		std::string delim(";");
		std::vector<std::string> outputVector;
		Split(propVal, delim, outputVector);

		int vectorSize = outputVector.size();
		PCWSTR * pcwstrValues = new PCWSTR[vectorSize];
		
		for(int i = 0; i< vectorSize; i++){
			wchar_t *wCharTemp = new wchar_t[outputVector[i].length()];
			mbstowcs_s(0, wCharTemp, outputVector[i].length() + 1, outputVector[i].data(),outputVector[i].length() + 1);
			
			pcwstrValues[i] = wCharTemp;
			
		}
	
		hr = InitPropVariantFromStringVector(pcwstrValues, vectorSize, &ppropvarValues);
		for (long iValue = 0; iValue < vectorSize; iValue++)
                {
                   // SysFreeString(pbstrValues[iValue]);
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
	
		

		std::string s (blank, 129);
		
		PCWSTR pcwstrValues;
	
		if (_xmpMeta.GetLocalizedText(kXMP_NS_EXIF, "UserComment", NULL, "x-default", NULL, &s, 0)){
		
			wchar_t *wCharTemp = new wchar_t[s.length()];
			errno_t error = mbstowcs_s(0, wCharTemp, s.length() + 1, s.data(), s.length() + 1);
		
		
			pcwstrValues = wCharTemp;
		
			

		
		hr = InitPropVariantFromString(pcwstrValues, &ppropvarValues);
		
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
	
	if (_xmpMeta.GetProperty_Int(kXMP_NS_XMP , "rating", &rating, 0)){
		
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
		
		ok = _xmpMeta.GetStructField(kXMP_NS_DM, kVideoDuration, kXMP_NS_DM, kVideoTimeValue, &timeStr, 0);
		ok = _xmpMeta.GetStructField(kXMP_NS_DM, kVideoDuration, kXMP_NS_DM, kVideoTimeScale, &scaleStr, 0);
		if (!ok){
			hr = E_FAIL;
		}

		if (hr == S_OK){
			float  frames;
			long scale1, scale2;
			std::string deliminator("/");
			std::vector<std::string> tempVector;
			Split(scaleStr, deliminator, tempVector);
			
			frames = atof(timeStr.c_str());
			scale1 = atol(tempVector[0].c_str());
			scale2 = atol(tempVector[1].c_str());
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
	ok = _xmpMeta.GetStructField( kXMP_NS_DM, kVideoFrameSize,kXMP_NS_XMP_Dimensions, kVideoDimensionHeight,  &heightStr, 0);
	if (!ok){
			hr = E_FAIL;
		}

	if (hr == S_OK){
	height = atol(heightStr.c_str());
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
	ok = _xmpMeta.GetStructField( kXMP_NS_DM, kVideoFrameSize,kXMP_NS_XMP_Dimensions, kVideoDimensionWidth,  &widthStr, 0);
	if (!ok){
			hr = E_FAIL;
		}

	if (hr == S_OK){
	width = atol(widthStr.c_str());
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
	ok = _xmpMeta.GetProperty(kXMP_NS_DM, "videoFrameRate", &framerateStr, 0);
	if (!ok){
			hr = E_FAIL;
		}
	if (hr == S_OK) {
		framerate = atol(framerateStr.c_str()) * 1000;
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
	}
	/*end encoding bitrate */
	
	/*begin total bitrate */ /*
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

	/* end all that use AVIOGM */
	}

	
	SXMPFiles::Terminate();
	SXMPMeta::Terminate();
	//LeaveCriticalSection(&OneXMPCritSection); 
	
	return hr;
}

HRESULT AviPropertyStore::_SaveCacheToSXMPMeta(){
	//EnterCriticalSection(&OneXMPCritSection); 
	XMP_PacketInfo		_xmpPacket;

	DWORD cProps;
	SXMPMeta::Initialize();
	SXMPFiles::Initialize();
    HRESULT hr = _pCache->GetCount(&cProps);
    if (SUCCEEDED(hr))
    {
		SXMPFiles			 _xmpFile;
		SXMPMeta			 _xmpMeta;
		
		bool ok = _xmpFile.OpenFile(_pStream, kXMP_UnknownFile, kXMPFiles_OpenForUpdate);
		if (ok)
                {
					ok = _xmpFile.GetXMP(&_xmpMeta,0, &_xmpPacket);
					
                }
                else
                {
                    hr = E_FAIL;
                }

                if (FAILED(hr))
                {
					return hr;
                }
		
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
                        
                        
                       
                        hr = _SaveProperty(key, propvar, &_xmpMeta);
                                


        

                        
                    }
					PropVariantClear(&propvar);
                }
            }
        }
		_xmpFile.PutXMP(_xmpMeta);
		_xmpFile.CloseFile();
		
    }
	SXMPFiles::Terminate();
	SXMPMeta::Terminate();
	//LeaveCriticalSection(&OneXMPCritSection); 
	return hr;
}

HRESULT AviPropertyStore::_SaveProperty(PROPERTYKEY key, REFPROPVARIANT propvars, SXMPMeta * _xmpMeta){
	HRESULT hr = S_OK;
	if (key.fmtid == PKEY_Keywords.fmtid){
			
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
					PWSTR pszValue = (PWSTR)malloc( 255 );
					hr = PropVariantToString(propvarValue, pszValue, 255);
					size_t   i;
					///buffersize is 100
					
					char      *realStr = (char *)malloc( 255 );

					wcstombs_s(&i,realStr, (size_t)255, pszValue,(size_t)255);
					if (SUCCEEDED(hr)){
						
						_xmpMeta->AppendArrayItem(kXMP_NS_DC, "subject", kXMP_PropValueIsArray, realStr , 0);
						hr = S_OK;
					}
				}
				PropVariantClear(&propvarValue);
			}
			long newIndex = _xmpMeta->CountArrayItems(kXMP_NS_DC, "subject");
	}
	
	else if (key.fmtid == PKEY_Music_Artist.fmtid){
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
					PWSTR pszValue = (PWSTR)malloc( 255 );
					hr = PropVariantToString(propvarValue, pszValue, 255);
					size_t   i;
					
					char *tempStr = (char *)malloc(wcsnlen(pszValue, 255) +1);
					
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
					free(tempStr);
				}
				PropVariantClear(&propvarValue);
			}
		_xmpMeta->SetProperty(kXMP_NS_DM, "artist",realStr.data(), 0);

	}
	
	else if (key.fmtid == PKEY_Rating.fmtid){
		_xmpMeta->DeleteProperty(kXMP_NS_XMP, "rating");
	
		ULONG rating;
		hr = PropVariantGetUInt32Elem(propvars, 0, &rating);
		if (SUCCEEDED(hr)){
			_xmpMeta->SetProperty_Int(kXMP_NS_XMP, "rating", rating, 0);
		}
		
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






