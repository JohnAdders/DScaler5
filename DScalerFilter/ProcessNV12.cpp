///////////////////////////////////////////////////////////////////////////////
// $Id: ProcessNV12.cpp,v 1.1 2003-05-08 15:58:38 adcockj Exp $
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
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Process.h"

///////////////////////////////////////////////////////////////////////////////
// Process the NV12 samples
// 4:2:0 samples go:
//  Y0 Y1 Y2 Y3
//  ...
//  U0 V0 U2 V2
//  ...
///////////////////////////////////////////////////////////////////////////////
void ProcessNV12(int Lines, BITMAPINFOHEADER* InputBMI, BITMAPINFOHEADER* OutputBMI, BYTE* pInBuffer, BYTE* pOutBuffer)
{
    int i;

    // process Y samples
    for(i = 0; i < Lines; ++i)
    {
        // black out first 8 pixels if we have a 
        // 704 width source that we are extending up to 720
        if(InputBMI->biWidth == 704)
        {
            for(int i(0); i < 8; ++i)
            {
                *(++pOutBuffer) = 0;
            }
        }

        memcpy(pOutBuffer, pInBuffer, InputBMI->biWidth);

        // black out last 8 pixels if we have a 
        // 704 width source that we are extending up to 720
        if(InputBMI->biWidth == 704)
        {
            pOutBuffer += 704;
            for(int i(0); i < 8; ++i)
            {
                *(++pOutBuffer) = 0;
            }
            pOutBuffer -= 720;
        }

        pOutBuffer += OutputBMI->biWidth;
        pInBuffer += InputBMI->biWidth;
    }
    
    // do UV samples
    for(i = 0; i < Lines/2; ++i)
    {
        // black out first 8 samples if we have a 
        // 704 width source that we are extending up to 720
        if(InputBMI->biWidth == 704)
        {
            for(int i(0); i < 8; ++i)
            {
                *(++pOutBuffer) = 128;
            }
        }

        memcpy(pOutBuffer, pInBuffer, InputBMI->biWidth);

        // black out last 8 samples if we have a 
        // 704 width source that we are extending up to 720
        if(InputBMI->biWidth == 704)
        {
            pOutBuffer += 704;
            for(int i(0); i < 8; ++i)
            {
                *(++pOutBuffer) = 0;
            }
            pOutBuffer -= 720;
        }

        pOutBuffer += OutputBMI->biWidth;
        pInBuffer += InputBMI->biWidth;
    }
    
}