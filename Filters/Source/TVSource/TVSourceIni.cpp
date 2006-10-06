///////////////////////////////////////////////////////////////////////////////
// $Id: TVSourceIni.cpp,v 1.1 2006-10-06 17:00:35 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
// 
// You should have received a copy of the GNU General Public
// License along with this package; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TVSource.h"
#include "MoreUuids.h"

long CTVSource::getInteger(LPCWSTR settingName, long defaultValue)
{
    return GetPrivateProfileIntW(L"TV", settingName, defaultValue, m_FileName.c_str());
}

std::wstring CTVSource::getString(LPCWSTR settingName, LPCWSTR defaultValue)
{
    wchar_t result[256];
    
    GetPrivateProfileStringW(L"TV", settingName, defaultValue, result, 255, m_FileName.c_str());

    return result;
}
