///////////////////////////////////////////////////////////////////////////////
// $Id: Process.h,v 1.1 2003-05-08 15:58:38 adcockj Exp $
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

// These are just simple copy functions at the moment
// \todo need to think about how this is going to work
// but the main aim is to seperate out the Filter stuff from the 
// actual processing code so that as much as possible can be made 
// cross-platform
void ProcessYUY2(int Lines, BITMAPINFOHEADER* InputBMI, BITMAPINFOHEADER* OutputBMI, BYTE* pInBuffer, BYTE* pOutBuffer);
void ProcessYV12(int Lines, BITMAPINFOHEADER* InputBMI, BITMAPINFOHEADER* OutputBMI, BYTE* pInBuffer, BYTE* pOutBuffer);
void ProcessNV12(int Lines, BITMAPINFOHEADER* InputBMI, BITMAPINFOHEADER* OutputBMI, BYTE* pInBuffer, BYTE* pOutBuffer);

