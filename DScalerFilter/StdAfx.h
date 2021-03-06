///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// DScalerFilter.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_STDAFX_H__0D718700_7563_11D7_B84A_0002A5623377__INCLUDED_)
#define AFX_STDAFX_H__0D718700_7563_11D7_B84A_0002A5623377__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif
#define _ATL_APARTMENT_THREADED

//#define _ATL_DEBUG_INTERFACES
//#define ATL_TRACE_LEVEL 2
//#define _ATL_DEBUG_QI

#include <atlbase.h>
//You may derive a class from CComModule and use it if you want to override
//something, but do not change the name of _Module
extern CComModule _Module;
#include <atlcom.h>

#include <strmif.h>
#include <vfwmsgs.h>
#include <uuids.h>
#include <d3d9.h>
#include <vmr9.h>
#include <amvideo.h>
#include <amstream.h>
#include <dvdmedia.h>
#include <medparam.h>
#include <Dmo.h>

#include "Utils.h"
#include "..\GenDMOProp\GenDMOProp.h"

#include <stdio.h>
#include <stdarg.h>

#include <list>
#include <vector>
#include <string>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0D718700_7563_11D7_B84A_0002A5623377__INCLUDED)
