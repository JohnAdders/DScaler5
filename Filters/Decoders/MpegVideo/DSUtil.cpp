///////////////////////////////////////////////////////////////////////////////
// $Id: DSUtil.cpp,v 1.1 2004-02-06 12:17:16 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
//
//	Copyright (C) 2003 Gabest
//	http://www.gabest.org
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
bool ExtractBIH(const AM_MEDIA_TYPE* pmt, BITMAPINFOHEADER* bih)
{
	if(pmt)
	{
		if(pmt->formattype == FORMAT_VideoInfo)
		{
			VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->pbFormat;
			memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
		}
		else if(pmt->formattype == FORMAT_VideoInfo2)
		{
			VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
			memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
		}
		if(pmt->formattype == FORMAT_MPEGVideo)
		{
			VIDEOINFOHEADER* vih = &((MPEG1VIDEOINFO*)pmt->pbFormat)->hdr;
			memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
		}
		else if(pmt->formattype == FORMAT_MPEG2_VIDEO)
		{
			VIDEOINFOHEADER2* vih = &((MPEG2VIDEOINFO*)pmt->pbFormat)->hdr;
			memcpy(bih, &vih->bmiHeader, sizeof(BITMAPINFOHEADER));
		}

		return(true);
	}
	
	return(false);
}

bool ExtractDim(const AM_MEDIA_TYPE* pmt, int& w, int& h, int& arx, int& ary)
{
	w = h = arx = ary = 0;

	if(pmt->formattype == FORMAT_VideoInfo || pmt->formattype == FORMAT_MPEGVideo)
	{
		VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)pmt->pbFormat;
		w = vih->bmiHeader.biWidth;
		h = abs(vih->bmiHeader.biHeight);
		arx = w * vih->bmiHeader.biYPelsPerMeter;
		ary = h * vih->bmiHeader.biXPelsPerMeter;
	}
	else if(pmt->formattype == FORMAT_VideoInfo2 || pmt->formattype == FORMAT_MPEG2_VIDEO)
	{
		VIDEOINFOHEADER2* vih = (VIDEOINFOHEADER2*)pmt->pbFormat;
		w = vih->bmiHeader.biWidth;
		h = abs(vih->bmiHeader.biHeight);
		arx = vih->dwPictAspectRatioX;
		ary = vih->dwPictAspectRatioY;
	}
	else
	{
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

	DWORD a = arx, b = ary;
    while(a) {int tmp = a; a = b % tmp; b = tmp;}
	if(b) arx /= b, ary /= b;

	return(true);
}

