///////////////////////////////////////////////////////////////////////////////
// $Id: Utils.h,v 1.2 2004-02-10 13:24:13 adcockj Exp $
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

#pragma once

#include "ProtectCode.h"

void InitMediaType(AM_MEDIA_TYPE* TypeToInit);
void ClearMediaType(AM_MEDIA_TYPE* TypeToClear);
void FreeMediaType(AM_MEDIA_TYPE* TypeToClear);
HRESULT CopyMediaType(AM_MEDIA_TYPE* Dest, const AM_MEDIA_TYPE* Source);

HRESULT RegisterFilter(const CLSID& clsidFilter, LPCWSTR wszName, const REGFILTER2* pRegFilter);
HRESULT UnregisterFilter(const CLSID& clsidFilter);


#define DBGLOG_SILENT 0
#define DBGLOG_ERROR  1
#define DBGLOG_FLOW   2
#define DBGLOG_ALL    3

#ifndef NOLOGGING
extern int CurrentDebugLevel;
void _LOG(LPCSTR sFormat, ...);
#define LOG(x,y) {if(x <= CurrentDebugLevel){_LOG("%s(%d) : ", __FILE__, (DWORD)__LINE__); _LOG##y;}}
#define TRACE(x,y) { if(x <= TRACE_LEVEL) {DbgPrint(FunctionName); DbgPrint(" - "); DbgPrint##y; DbgPrint("\n");}}
void LogSample(IMediaSample* Sample, LPCSTR Desc);
void LogMediaType(const AM_MEDIA_TYPE* MediaType, LPCSTR Desc);
void LogBadHRESULT(HRESULT hr, LPCSTR File, DWORD Line);
#define CHECK(hr) if(FAILED(hr)) { LogBadHRESULT(hr, __FILE__, (DWORD)__LINE__); DebugBreak(); return hr;}
#else
#define LOG
#define LogSample
#define LogMediaType
#define CHECK(hr) if(FAILED(hr)) { return hr;}
#define LogBadHRESULT(hr, File, Line)
#endif
