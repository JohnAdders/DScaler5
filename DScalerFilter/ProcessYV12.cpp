///////////////////////////////////////////////////////////////////////////////
// $Id: ProcessYV12.cpp,v 1.3 2003-07-25 16:00:55 adcockj Exp $
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
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.2  2003/05/09 07:03:26  adcockj
// Bug fixes for new format code
//
// Revision 1.1  2003/05/08 15:58:38  adcockj
// Better error handling, threading and format support
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"

///////////////////////////////////////////////////////////////////////////////
// Process the YV12 samples
// 4:2:0 samples go:
//  Y0 Y1 Y2 Y3
//  ...
//  V0 V2
//  ...
//  U0 U2
//  ...
///////////////////////////////////////////////////////////////////////////////
void ProcessYV12(int Lines, BITMAPINFOHEADER* InputBMI, BITMAPINFOHEADER* OutputBMI, BYTE* pInBuffer, BYTE* pOutBuffer)
{
    int i;

    // process Y samples
    for(i = 0; i < Lines; ++i)
    {
        memcpy(pOutBuffer, pInBuffer, InputBMI->biWidth);

        pOutBuffer += OutputBMI->biWidth;
        pInBuffer += InputBMI->biWidth;
    }
    // do V samples
    for(i = 0; i < Lines/2; ++i)
    {
        memcpy(pOutBuffer, pInBuffer, InputBMI->biWidth/2);

        pOutBuffer += OutputBMI->biWidth / 2;
        pInBuffer += InputBMI->biWidth / 2;
    }
    
    // do U samples
    for(i = 0; i < Lines/2; ++i)
    {
        memcpy(pOutBuffer, pInBuffer, InputBMI->biWidth/2);

        pOutBuffer += OutputBMI->biWidth / 2;
        pInBuffer += InputBMI->biWidth / 2;
    }
}

