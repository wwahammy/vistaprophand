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

#include <windows.h>
#include <msi.h>
#include <shlobj.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#define SZ_CLSID_AVITHUMBNAILPROVIDER	L"{99A45086-08F6-4F69-BDB3-7D1952D3236C}"
#define SZ_CLSID_AVIPROPERTYHANDLER		L"{5A07F0A8-60E4-474B-89CF-8705E4260D22}"

extern "C" UINT __stdcall ShellReload(MSIHANDLE hInstall)
{
	#ifdef _DEBUG
		// Display messagebox, so debugger can attach to process
	//	::MessageBox(0,L"<ShellReload>: Attach the debugger...",L"MSI Debug",MB_ICONINFORMATION | MB_OK); 
	#endif

	HKEY hKey;

	/*
	* This whole section is used to clear out the shell extension cache.
	* If we don't, there could be problems.
	*/
	if (RegOpenKeyEx( HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Cached"),
        0,
        KEY_READ,
        &hKey) == ERROR_SUCCESS)
	{
		RegCloseKey (hKey);
		
		if (RegOpenKeyEx( HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Shell Extensions\\Cached"),
        0,
        KEY_READ | KEY_SET_VALUE,
        &hKey) == ERROR_SUCCESS)
		{ 
			TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
			DWORD    cchClassName = MAX_PATH;  // size of class string 
			DWORD    cSubKeys=0;               // number of subkeys 
			DWORD    cbMaxSubKey;              // longest subkey size 
			DWORD    cchMaxClass;              // longest class string 
			DWORD    cValues;              // number of values for key 
			DWORD    cchMaxValue;          // longest value name 
			DWORD    cbMaxValueData;       // longest value data 
			DWORD    cbSecurityDescriptor; // size of security descriptor 
			FILETIME ftLastWriteTime;      // last write time 
		 
			DWORD i, retCode; 
		 
			TCHAR achValue[MAX_VALUE_NAME]; 
			DWORD cchValue = MAX_VALUE_NAME; 

			retCode = RegQueryInfoKey(
				hKey,                // key handle 
				achClass,                // buffer for class name 
				&cchClassName,           // size of class string 
				NULL,                    // reserved 
				&cSubKeys,               // number of subkeys 
				&cbMaxSubKey,            // longest subkey size 
				&cchMaxClass,            // longest class string 
				&cValues,                // number of values for this key 
				&cchMaxValue,            // longest value name 
				&cbMaxValueData,         // longest value data 
				&cbSecurityDescriptor,   // security descriptor 
				&ftLastWriteTime);       // last write time 
			
			if (cValues)
			{
				for (i=0, retCode=ERROR_SUCCESS; i<cValues; i++) 
				{ 
					cchValue = MAX_VALUE_NAME; 
					achValue[0] = '\0'; 
					retCode = RegEnumValue(hKey, i, 
						achValue, 
						&cchValue, 
						NULL, 
						NULL,
						NULL,
						NULL);
		 
					if (retCode == ERROR_SUCCESS ) 
					{ 
						wchar_t * upperCaseValue = _wcsupr(achValue);
						LONG returnVal = 0;
						wchar_t * test1 = new wchar_t[MAX_VALUE_NAME];
						test1[0] = '\0';
						wchar_t * test2 = new wchar_t[MAX_VALUE_NAME];
						test2[0] = '\0';
						test1 = wcsstr(achValue, SZ_CLSID_AVITHUMBNAILPROVIDER);
						test2 = wcsstr(achValue, SZ_CLSID_AVIPROPERTYHANDLER);
						if(test1 != NULL || test2 != NULL )
						{
							returnVal = RegDeleteValue(hKey, achValue);
						}

						delete [] test1;
						delete [] test2;
						//delete [] upperCaseValue;
					} 
				}

			}
		}


	}


	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, 0, 0);
	return ERROR_SUCCESS;
}