///////////////////////////////////////////////////////////////////////////////
// $Id: Utils.cpp,v 1.2 2003-05-01 16:21:53 adcockj Exp $
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
// Revision 1.1.1.1  2003/04/30 13:01:22  adcockj
// Initial Import
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utils.h"

#ifdef _DEBUG
static int CurrentDebugLevel = DBGLOG_ALL;
#else
static int CurrentDebugLevel = DBGLOG_ERROR;
#endif


void InitMediaType(AM_MEDIA_TYPE* TypeToInit)
{
    ZeroMemory(TypeToInit, sizeof(AM_MEDIA_TYPE));
}

void ClearMediaType(AM_MEDIA_TYPE* TypeToClear)
{
    if(TypeToClear->pUnk != NULL)
    {
        TypeToClear->pUnk->AddRef();
    }
    if(TypeToClear->cbFormat > 0)
    {
        CoTaskMemFree(TypeToClear->pbFormat);
    }
    ZeroMemory(TypeToClear, sizeof(AM_MEDIA_TYPE));
}

HRESULT CopyMediaType(AM_MEDIA_TYPE* Dest, const AM_MEDIA_TYPE* Source)
{
    ClearMediaType(Dest);
    Dest->majortype = Source->majortype;
    Dest->subtype = Source->subtype;
    Dest->bFixedSizeSamples = Source->bFixedSizeSamples;
    Dest->bTemporalCompression = Source->bTemporalCompression;
    Dest->lSampleSize = Source->lSampleSize;
    Dest->formattype = Source->formattype;
    // this pUnk is pretty always NULL
    // but MS might bung something in here
    // so we had better cope properly with it
    // just in case
    if(Dest->pUnk != NULL)
    {
        Dest->pUnk->Release();
    }
    Dest->pUnk = Source->pUnk;
    if(Dest->pUnk != NULL)
    {
        Dest->pUnk->AddRef();
    }
    Dest->cbFormat = Source->cbFormat;
    if(Source->cbFormat > 0)
    {
        Dest->pbFormat = (BYTE*)CoTaskMemAlloc(Source->cbFormat);
        if(Dest->pbFormat != NULL)
        {
            memcpy(Dest->pbFormat, Source->pbFormat, Source->cbFormat);
        }
        else
        {
            ZeroMemory(Dest, sizeof(AM_MEDIA_TYPE));
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        Dest->pbFormat = NULL;
    }
    
    return S_OK;
}

#ifndef NOLOGGING
void LOG(int DebugLevel, LPCSTR szFormat, ...)
{
    if(DebugLevel <= CurrentDebugLevel)
    {
        char szMessage[2048];
        va_list Args;

        va_start(Args, szFormat);
        int result=_vsnprintf(szMessage,2048, szFormat, Args);
        va_end(Args);
        if(result==-1)
	    {
		    OutputDebugString("DebugString too long, truncated!!\n");
	    }
	    OutputDebugString(szMessage);
    }
}

void LogSample(IMediaSample* Sample, LPCSTR Desc)
{
    AM_SAMPLE2_PROPERTIES SampleProperties;
    CComQIPtr<IMediaSample2> Sample2 = Sample;
    if(Sample2 != NULL)
    {
        Sample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&SampleProperties);
        LOG(DBGLOG_FLOW, "%s - IMediaSample2 Dump\n", Desc);
        LOG(DBGLOG_ALL, " cbData %d\n", SampleProperties.cbData);
        LOG(DBGLOG_ALL, " dwTypeSpecificFlags %08x\n", SampleProperties.dwTypeSpecificFlags);
        LOG(DBGLOG_ALL, " dwSampleFlags %08x\n", SampleProperties.dwSampleFlags);
        LOG(DBGLOG_ALL, " lActual %d\n", SampleProperties.lActual);
        LOG(DBGLOG_ALL, " length %d\n", (long)(SampleProperties.tStop - SampleProperties.tStart));
        LOG(DBGLOG_ALL, " dwStreamId %08x\n", SampleProperties.dwStreamId);
        LOG(DBGLOG_ALL, " cbBuffer %d\n", SampleProperties.cbBuffer);
    }
    else
    {
    }

}

void LogMediaType(AM_MEDIA_TYPE* MediaType, LPCSTR Desc)
{
    BYTE* Uuid;
    
    LOG(DBGLOG_FLOW, "%s - AM_MEDIA_TYPE Dump\n", Desc);

    if(MediaType->majortype == MEDIATYPE_Video)
    {
        LOG(DBGLOG_ALL, " MEDIATYPE_Video\n");
    }
    else
    {
        UuidToString(&MediaType->majortype, &Uuid);
        LOG(DBGLOG_ALL, " Major Type {%s}\n", Uuid);
        RpcStringFree(&Uuid);
    }
    if(MediaType->subtype == MEDIASUBTYPE_YUY2)
    {
        LOG(DBGLOG_ALL, " MEDIASUBTYPE_YUY2\n");
    }
    else if(MediaType->subtype == MEDIASUBTYPE_YV12)
    {
        LOG(DBGLOG_ALL, " MEDIASUBTYPE_YV12\n");
    }
    else if(MediaType->subtype == MEDIASUBTYPE_RGB32)
    {
        LOG(DBGLOG_ALL, " MEDIASUBTYPE_RGB32\n");
    }
    else
    {
        UuidToString(&MediaType->subtype, &Uuid);
        LOG(DBGLOG_ALL, " Sub Type {%s}\n", Uuid);
        RpcStringFree(&Uuid);
    }
    if(MediaType->formattype == FORMAT_VideoInfo)
    {
        LOG(DBGLOG_ALL, " FORMAT_VideoInfo\n");
        VIDEOINFOHEADER* Format = (VIDEOINFOHEADER*)MediaType->pbFormat;
        LOG(DBGLOG_ALL, " cbData %d\n", Format->bmiHeader);
        LOG(DBGLOG_ALL, " biSize %d\n", Format->bmiHeader.biSize);
        LOG(DBGLOG_ALL, " biWidth %d\n", Format->bmiHeader.biWidth);
        LOG(DBGLOG_ALL, " biHeight %d\n", Format->bmiHeader.biHeight);
        LOG(DBGLOG_ALL, " biPlanes %d\n", Format->bmiHeader.biPlanes);
        LOG(DBGLOG_ALL, " biBitCount %d\n", Format->bmiHeader.biBitCount);
        LOG(DBGLOG_ALL, " biCompression %d\n", Format->bmiHeader.biCompression);
        LOG(DBGLOG_ALL, " biSizeImage %d\n", Format->bmiHeader.biSizeImage);
    }
    else if(MediaType->formattype == FORMAT_VIDEOINFO2)
    {
        LOG(DBGLOG_ALL, " FORMAT_VIDEOINFO2\n");
        VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)MediaType->pbFormat;
        LOG(DBGLOG_ALL, " cbData %d\n", Format->bmiHeader);
        LOG(DBGLOG_ALL, " biSize %d\n", Format->bmiHeader.biSize);
        LOG(DBGLOG_ALL, " biWidth %d\n", Format->bmiHeader.biWidth);
        LOG(DBGLOG_ALL, " biHeight %d\n", Format->bmiHeader.biHeight);
        LOG(DBGLOG_ALL, " biPlanes %d\n", Format->bmiHeader.biPlanes);
        LOG(DBGLOG_ALL, " biBitCount %d\n", Format->bmiHeader.biBitCount);
        LOG(DBGLOG_ALL, " biCompression %d\n", Format->bmiHeader.biCompression);
        LOG(DBGLOG_ALL, " biSizeImage %d\n", Format->bmiHeader.biSizeImage);
    }
    else
    {
        UuidToString(&MediaType->formattype, &Uuid);
        LOG(DBGLOG_ALL, " Format Type {%s}\n", Uuid);
        RpcStringFree(&Uuid);
    }
}
#endif // NOLOGGING

void GetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    CComQIPtr<IMediaSample2> Sample2 = Sample;
    if(Sample2 != NULL)
    {
        Sample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)SampleProperties);
    }
    else
    {
        ZeroMemory(SampleProperties, sizeof(AM_SAMPLE2_PROPERTIES));
        SampleProperties->cbData =  sizeof(AM_SAMPLE2_PROPERTIES);
        SampleProperties->lActual =  Sample->GetSize();
        if(Sample->GetTime(&SampleProperties->tStart, &SampleProperties->tStop) == S_OK)
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_TIMEVALID;
        }
        Sample->GetMediaType(&SampleProperties->pMediaType);
        Sample->GetPointer(&SampleProperties->pbBuffer);
        SampleProperties->cbBuffer = Sample->GetActualDataLength();
        if(Sample->IsDiscontinuity())
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_TIMEDISCONTINUITY;
        }
        if(Sample->IsPreroll())
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_PREROLL;
        }
        if(Sample->IsSyncPoint())
        {
            SampleProperties->dwSampleFlags |= AM_SAMPLE_SPLICEPOINT;
        }

        SampleProperties->dwStreamId = AM_STREAM_MEDIA;
    }
}

HRESULT SetSampleProperties(IMediaSample* Sample, AM_SAMPLE2_PROPERTIES* SampleProperties)
{
    HRESULT hr = S_OK;
    CComQIPtr<IMediaSample2> Sample2 = Sample;
    if(Sample2 != NULL)
    {
        hr = Sample2->SetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&SampleProperties);
    }
    else
    {
        if(SampleProperties->dwSampleFlags & AM_SAMPLE_TIMEVALID)
        {
            hr = Sample->SetTime(&SampleProperties->tStart, &SampleProperties->tStop);
            if(FAILED(hr)) return hr;
        }

        hr = Sample->SetMediaType(SampleProperties->pMediaType);
        if(FAILED(hr)) return hr;
        
        hr = Sample->SetDiscontinuity((SampleProperties->dwSampleFlags & AM_SAMPLE_TIMEDISCONTINUITY) > 0);
        if(FAILED(hr)) return hr;

        hr = Sample->SetPreroll((SampleProperties->dwSampleFlags & AM_SAMPLE_PREROLL) > 0);
        if(FAILED(hr)) return hr;

        hr = Sample->SetSyncPoint((SampleProperties->dwSampleFlags & AM_SAMPLE_SPLICEPOINT) > 0);
        if(FAILED(hr)) return hr;
    }
    return hr;
}
