////////////////////////////////////////////////////////////////////////////
// $Id: StdAfx.h,v 1.4 2006-10-06 13:46:18 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
// This software was based on sample code generated by the 
// DMO project wizard.  That code is (c) Microsoft Corporation
/////////////////////////////////////////////////////////////////////////////
//
// This file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.3  2004/03/05 15:56:14  adcockj
// Interim check in of DScalerFilter (compiles again)
//
// Revision 1.2  2004/02/06 12:17:16  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#define _CRT_SECURE_NO_DEPRECATE

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <windowsx.h>

#ifndef _DEBUG
#define _ASSERTE(__x__) 
#endif

#pragma warning(disable:4786)       
#include <deque>
#pragma warning(default:4786)       

#define FIX_LOCK_NAME
#include <dmo.h>
#include <dmoimpl.h>
#include <mmreg.h>
#include <uuids.h>
#include <dvdmedia.h>
#include <amvideo.h>
#include <medparam.h>

#define DLLSVC
#define _INLINEISEQUALGUID_DEFINED
#include "yacl\include\combook.h"