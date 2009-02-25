///////////////////////////////////////////////////////////////////////////////
// $Id$
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
// Revision 1.1  2004/10/28 16:02:53  adcockj
// added new files
//
// Revision 1.3  2004/07/07 14:07:07  adcockj
// Added ATSC subtitle support
// Removed tabs
// Fixed film flag handling of progressive frames
//
// Revision 1.2  2004/02/08 19:09:17  adcockj
// Fixed VIH2 bug
//
// Revision 1.1  2004/02/06 12:17:16  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <initguid.h>
#include "MediaTypes.h"

#pragma pack(1)
struct VIH
{
    VIDEOINFOHEADER vih;
    UINT mask[3];
    int size;
    const GUID* subtype;
};
struct VIH2
{
    VIDEOINFOHEADER2 vih;
    UINT mask[3];
    int size;
    const GUID* subtype;
};
#pragma pack()

#define VIH_NORMAL (sizeof(VIDEOINFOHEADER))
#define VIH_BITFIELDS (sizeof(VIDEOINFOHEADER)+3*sizeof(RGBQUAD))
#define VIH2_NORMAL (sizeof(VIDEOINFOHEADER2))
#define VIH2_BITFIELDS (sizeof(VIDEOINFOHEADER2)+3*sizeof(RGBQUAD))
#define BIH_SIZE (sizeof(BITMAPINFOHEADER))

VIH vihs[] =
{
    // YUY2
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('Y','U','Y','2'), 0, 0, 0, 0, 0}     // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_YUY2                                              // subtype
    },
    // YV12
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('Y','V','1','2'), 0, 0, 0, 0, 0}     // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_YV12                                              // subtype
    },
    // NV12
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('N','V','1','2'), 0, 0, 0, 0, 0}     // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_NV12                                              // subtype
    },
    // IYUV
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('I','Y','U','V'), 0, 0, 0, 0, 0}     // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_IYUV                                              // subtype
    },
    // 8888 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_RGB32                                             // subtype
    },
    // 8888 bitf 
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_RGB32                                             // subtype
    },
    // A888 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_ARGB32                                            // subtype
    },
    // A888 bitf (I'm not sure if this exist...)
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_ARGB32                                            // subtype
    },
    // 888 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 24, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_RGB24                                             // subtype
    },
    // 888 bitf 
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 24, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_RGB24                                             // subtype
    },
    // 565 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_RGB565                                            // subtype
    },
    // 565 bitf
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0xF800, 0x07E0, 0x001F},                                       // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_RGB565                                            // subtype
    },
    // 555 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH_NORMAL,                                                     // size
        &MEDIASUBTYPE_RGB555                                            // subtype
    },
    // 555 bitf
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0x7C00, 0x03E0, 0x001F},                                       // mask[3]
        VIH_BITFIELDS,                                                  // size
        &MEDIASUBTYPE_RGB555                                            // subtype
    },
};

VIH2 vih2s[] =
{
    // YUY2
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, mmioFOURCC('Y','U','Y','2'), 0, 0, 0, 0, 0}     // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_YUY2                                              // subtype
    },
    // YV12
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('Y','V','1','2'), 0, 0, 0, 0, 0}     // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_YV12                                              // subtype
    },
    // NV12
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('N','V','1','2'), 0, 0, 0, 0, 0}     // bmiHeader

        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_NV12                                              // subtype
    },
    // IYUV
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 12, mmioFOURCC('I','Y','U','V'), 0, 0, 0, 0, 0}     // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_IYUV                                              // subtype
    },
    // 8888 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_RGB32                                             // subtype
    },
    // 8888 bitf 
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_RGB32                                             // subtype
    },
    // A888 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_ARGB32                                            // subtype
    },
    // A888 bitf (I'm not sure if this exist...)
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 32, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_ARGB32                                            // subtype
    },
    // 888 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 24, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_RGB24                                             // subtype
    },
    // 888 bitf 
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 24, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0xFF0000, 0x00FF00, 0x0000FF},                                 // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_RGB24                                             // subtype
    },
    // 565 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_RGB565                                            // subtype
    },
    // 565 bitf
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0xF800, 0x07E0, 0x001F},                                       // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_RGB565                                            // subtype
    },
    // 555 normal
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_RGB, 0, 0, 0, 0, 0}          // bmiHeader
        }, 
        {0, 0, 0},                                                      // mask[3]
        VIH2_NORMAL,                                                    // size
        &MEDIASUBTYPE_RGB555                                            // subtype
    },
    // 555 bitf
    {
        {                   
            {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            {BIH_SIZE, 0, 0, 1, 16, BI_BITFIELDS, 0, 0, 0, 0, 0}    // bmiHeader
        }, 
        {0x7C00, 0x03E0, 0x001F},                                       // mask[3]
        VIH2_BITFIELDS,                                                 // size
        &MEDIASUBTYPE_RGB555                                            // subtype
    },
};

int VIHSIZE = (sizeof(vihs) / sizeof(vihs[0]));

void CorrectMediaType(AM_MEDIA_TYPE* pmt)
{
    if(!pmt) return;

    if(pmt->formattype == FORMAT_VideoInfo)
    {

        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->pbFormat;

        for(int i = 0; i < VIHSIZE; i++)
        {
            if(pmt->subtype == *vihs[i].subtype
            && vih->bmiHeader.biCompression == vihs[i].vih.bmiHeader.biCompression)
            {
                BYTE* NewFormat = (BYTE*)CoTaskMemAlloc(vihs[i].size);
                memcpy(NewFormat, &vihs[i], vihs[i].size);
                memcpy(NewFormat, pmt->pbFormat, sizeof(VIDEOINFOHEADER));
                CoTaskMemFree(pmt->pbFormat);
                pmt->pbFormat = NewFormat;
                pmt->cbFormat = vihs[i].size;
                break;
            }   
        }
    }
    else if(pmt->formattype == FORMAT_VideoInfo2)
    {
        VIDEOINFOHEADER2* vih2 = (VIDEOINFOHEADER2*)pmt->pbFormat;

        for(int i = 0; i < VIHSIZE; i++)
        {
            if(pmt->subtype == *vih2s[i].subtype
            && vih2->bmiHeader.biCompression == vih2s[i].vih.bmiHeader.biCompression)
            {
                BYTE* NewFormat = (BYTE*)CoTaskMemAlloc(vih2s[i].size);
                memcpy(NewFormat, &vih2s[i], vih2s[i].size);
                memcpy(NewFormat, pmt->pbFormat, sizeof(VIDEOINFOHEADER2));
                CoTaskMemFree(pmt->pbFormat);
                pmt->pbFormat = NewFormat;
                pmt->cbFormat = vih2s[i].size;
                break;
            }
        }
    }
}
