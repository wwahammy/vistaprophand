/***************************************************************************
 *   Copyright (C) 2005 - 2007 by                                          *
 *      Last.fm Ltd <client@last.fm>  (with modifications by Eric Schultz) *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.          *
 ***************************************************************************/

#include "BreakpadDllExportMacro.h"
#include <string>
#include <shlobj.h>

#ifndef NBREAKPAD

#ifdef __APPLE__
#   include "breakpad/client/mac/handler/exception_handler.h"
#elif defined WIN32
#   include "breakpad/client/windows/handler/exception_handler.h"
#elif defined __linux__
#   include "breakpad/client/linux/handler/exception_handler.h"
#else
#   define NBREAKPAD
#endif
std::wstring breakpadDumpPath;

void BreakpadDumpPath()
{
	
	PWSTR path;
	HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0,
		NULL, &path);
	breakpadDumpPath.assign(path);
	breakpadDumpPath.append(L"\\VMP\\CrashReports");
	
}


class BREAKPAD_DLLEXPORT BreakPad : public google_breakpad::ExceptionHandler
{
    const char* m_product_name; // yes! It MUST be const char[]
	const char* m_version_num;

public:
	BreakPad( const std::wstring &dump_write_dirpath );
    ~BreakPad();

    void setProductName( const char* s ) { m_product_name = s; };
    const char* productName() const { return m_product_name; }
	
	void setVersionNum(const char* s) {m_version_num = s;};
	const char* versionNum() const {return m_version_num;};
};
#endif


#undef char
