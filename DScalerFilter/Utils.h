///////////////////////////////////////////////////////////////////////////////
// $Id: Utils.h,v 1.2 2003-05-02 07:03:14 adcockj Exp $
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

void InitMediaType(AM_MEDIA_TYPE* TypeToInit);
void ClearMediaType(AM_MEDIA_TYPE* TypeToClear);
HRESULT CopyMediaType(AM_MEDIA_TYPE* Dest, const AM_MEDIA_TYPE* Source);

#define DBGLOG_SILENT 0
#define DBGLOG_ERROR  1
#define DBGLOG_FLOW   2
#define DBGLOG_ALL    3

#ifndef NOLOGGING
void LOG(int DebugLevel, LPCSTR sFormat, ...);
void LogSample(IMediaSample* Sample, LPCSTR Desc);
void LogMediaType(const AM_MEDIA_TYPE* MediaType, LPCSTR Desc);
#else
#define LOG
#define LogSample
#define LogMediaType
#endif
