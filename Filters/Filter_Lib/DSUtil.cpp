///////////////////////////////////////////////////////////////////////////////
// $Id: DSUtil.cpp,v 1.2 2004-11-04 16:01:13 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2003 Gabest
//  http://www.gabest.org
//
///////////////////////////////////////////////////////////////////////////////
//
//  This Program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2, or (at your option)
//  any later version.
//   
//  This Program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//   
//  You should have received a copy of the GNU General Public License
//  along with GNU Make; see the file COPYING.  If not, write to
//  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
//  http://www.gnu.org/copyleft/gpl.html
//
///////////////////////////////////////////////////////////////////////////////
//
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2004/10/28 15:52:24  adcockj
// Moved video output pin code into new class
//
// Revision 1.6  2004/08/06 08:38:53  adcockj
// Added optional YV12 output type
//
// Revision 1.5  2004/07/29 13:44:59  adcockj
// More fixes for Laurent's issues
//
// Revision 1.4  2004/07/07 14:07:07  adcockj
// Added ATSC subtitle support
// Removed tabs
// Fixed film flag handling of progressive frames
//
// Revision 1.3  2004/05/06 06:38:06  adcockj
// Interim fixes for connection and PES streams
//
// Revision 1.2  2004/04/14 16:31:34  adcockj
// Subpicture fixes, AFD started and minor fixes
//
// Revision 1.1  2004/02/06 12:17:16  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "DSUtil.h"

IBaseFilter* GetFilterFromPin(IPin* pPin)
{
    if(!pPin) return NULL;
    IBaseFilter* pBF = NULL;
    PIN_INFO pi;
    if(pPin && SUCCEEDED(pPin->QueryPinInfo(&pi)))
    {
        pBF = pi.pFilter;
        pBF->Release();
    }
    return(pBF);
}


CLSID GetCLSID(IBaseFilter* pBF)
{
    CLSID clsid = GUID_NULL;
    pBF->GetClassID(&clsid);
    return(clsid);
}

CLSID GetCLSID(IPin* pPin)
{
    return(GetCLSID(GetFilterFromPin(pPin)));
}
const BITMAPINFOHEADER* ExtractBIH(const AM_MEDIA_TYPE* pmt)
{
    if(pmt)
    {
        if(pmt->formattype == FORMAT_VideoInfo)
        {
            VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->pbFormat;
            return &vih->bmiHeader;
        }
        else if(pmt->formattype == FORMAT_VideoInfo2)
        {
            VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
            return &vih->bmiHeader;
        }
        if(pmt->formattype == FORMAT_MPEGVideo)
        {
            VIDEOINFOHEADER* vih = &((MPEG1VIDEOINFO*)pmt->pbFormat)->hdr;
            return &vih->bmiHeader;
        }
        else if(pmt->formattype == FORMAT_MPEG2_VIDEO)
        {
            VIDEOINFOHEADER2* vih = &((MPEG2VIDEOINFO*)pmt->pbFormat)->hdr;
            return &vih->bmiHeader;
        }
    }
    
    return NULL;
}



bool ExtractDim(const AM_MEDIA_TYPE* pmt, int& w, int& h, long& arx, long& ary)
{
    w = h = arx = ary = 0;

    if(pmt->formattype == FORMAT_VideoInfo || pmt->formattype == FORMAT_MPEGVideo)
    {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->pbFormat;
        w = vih->bmiHeader.biWidth;
        h = vih->bmiHeader.biHeight;
        arx = w * vih->bmiHeader.biYPelsPerMeter;
        ary = h * vih->bmiHeader.biXPelsPerMeter;
    }
    else if(pmt->formattype == FORMAT_VideoInfo2 || pmt->formattype == FORMAT_MPEG2_VIDEO)
    {
        VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
        w = vih->bmiHeader.biWidth;
        h = vih->bmiHeader.biHeight;
        arx = vih->dwPictAspectRatioX;
        ary = vih->dwPictAspectRatioY;
    }
    else
    {
        w = 720;
        h = 480;
        arx = 4;
        ary = 3;
        return(false);
    }

    BYTE* ptr = NULL;
    DWORD len = 0;

    if(pmt->formattype == FORMAT_MPEGVideo)
    {
        ptr = ((MPEG1VIDEOINFO*)pmt->pbFormat)->bSequenceHeader;
        len = ((MPEG1VIDEOINFO*)pmt->pbFormat)->cbSequenceHeader;

        if(ptr && len >= 8)
        {
            w = (ptr[4]<<4)|(ptr[5]>>4);
            h = ((ptr[5]&0xf)<<8)|ptr[6];
            float ar[] = 
            {
                1.0000f,1.0000f,0.6735f,0.7031f,
                0.7615f,0.8055f,0.8437f,0.8935f,
                0.9157f,0.9815f,1.0255f,1.0695f,
                1.0950f,1.1575f,1.2015f,1.0000f,
            };
            arx = (int)((float)w / ar[ptr[7]>>4] + 0.5);
            ary = h;
        }
    }
    else if(pmt->formattype == FORMAT_MPEG2_VIDEO)
    {
        ptr = (BYTE*)((MPEG2VIDEOINFO*)pmt->pbFormat)->dwSequenceHeader; 
        len = ((MPEG2VIDEOINFO*)pmt->pbFormat)->cbSequenceHeader;

        if(ptr && len >= 8)
        {
            w = (ptr[4]<<4)|(ptr[5]>>4);
            h = ((ptr[5]&0xf)<<8)|ptr[6];
            struct {int x, y;} ar[] = {{w,h},{4,3},{16,9},{221,100},{w,h}};
            int i = min(max(ptr[7]>>4, 1), 5)-1;
            arx = ar[i].x;
            ary = ar[i].y;
        }
    }

    if(ptr && len >= 8)
    {

    }
    
    // protect against zero values
    if(arx == 0 || ary == 0)
    {
        arx = w;
        ary = h;
    }


    DWORD a = arx, b = ary;
    while(a) {int tmp = a; a = b % tmp; b = tmp;}
    if(b) arx /= b, ary /= b;

    return(true);
}

// function taken from 
// header.c
// Copyright (C) 2000-2003 Michel Lespinasse <walken@zoy.org>
// Copyright (C) 2003      Regis Duchesne <hpreg@zoy.org>
// Copyright (C) 1999-2000 Aaron Holtzman <aholtzma@ess.engr.uvic.ca>
//
// This file is part of mpeg2dec, a free MPEG-2 video stream decoder.
// See http://libmpeg2.sourceforge.net/ for updates.
void Simplify(long& u, long& v)
{
    long a, b, tmp;

    a = u;  
    b = v;
    
    while (a) 
    {   
        /* find greatest common divisor */
        tmp = a;    
        a = b % tmp;    
        b = tmp;
    }
    u /= b;
    v /= b;
}

void Simplify(unsigned long& u, unsigned long& v)
{
    unsigned long a, b, tmp;

    a = u;  
    b = v;
    
    while (a) 
    {   
        /* find greatest common divisor */
        tmp = a;    
        a = b % tmp;    
        b = tmp;
    }
    u /= b;
    v /= b;
}
