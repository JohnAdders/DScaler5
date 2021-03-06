///////////////////////////////////////////////////////////////////////////////
// $Id$
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

// uncomment for descriptive help messages instead of raw HRESULTS
//#define HAVE_DXSDK

#include "stdafx.h"
#include "Utils.h"
#ifdef HAVE_DXSDK
#include <dxerr9.h>
#pragma comment(lib, "dxerr9.lib")
#endif

#ifdef _DEBUG
int CurrentDebugLevel = DBGLOG_FLOW;
//int CurrentDebugLevel = DBGLOG_ALL;
#else
int CurrentDebugLevel = DBGLOG_ERROR;
#endif


void InitMediaType(AM_MEDIA_TYPE* TypeToInit)
{
    ZeroMemory(TypeToInit, sizeof(AM_MEDIA_TYPE));
}

void ClearMediaType(AM_MEDIA_TYPE* TypeToClear)
{
    if(TypeToClear->pUnk != NULL && !IsBadWritePtr(TypeToClear->pUnk, 4))
    {
        TypeToClear->pUnk->Release();
    }
    if(TypeToClear->cbFormat > 0 && !IsBadWritePtr(TypeToClear->pbFormat, 4))
    {
        CoTaskMemFree(TypeToClear->pbFormat);
    }
    ZeroMemory(TypeToClear, sizeof(AM_MEDIA_TYPE));
}

bool AreMediaTypesIdentical(const AM_MEDIA_TYPE* Type1, const AM_MEDIA_TYPE* Type2)
{
    if(Type1->majortype != Type2->majortype)
        return false;
    if(Type1->subtype != Type2->subtype)
        return false;
    if(Type1->formattype != Type2->formattype)
        return false;
    if(Type1->lSampleSize != Type2->lSampleSize)
        return false;
    if(Type1->cbFormat != Type2->cbFormat)
        return false;
    if(memcmp(Type1->pbFormat,Type2->pbFormat, Type1->cbFormat) != 0)
        return false;
    return true;
}


void FreeMediaType(AM_MEDIA_TYPE* TypeToFree)
{
    if(TypeToFree->pUnk != NULL)
    {
        TypeToFree->pUnk->Release();
    }
    if(TypeToFree->cbFormat > 0)
    {
        CoTaskMemFree(TypeToFree->pbFormat);
    }
    CoTaskMemFree(TypeToFree);
}

HRESULT CopyMediaType(AM_MEDIA_TYPE* Dest, const AM_MEDIA_TYPE* Source)
{
    // protect against copying to ourselves
    if(Source == Dest)
    {
        return S_OK;
    }
    ClearMediaType(Dest);
    Dest->majortype = Source->majortype;
    Dest->subtype = Source->subtype;
    Dest->bFixedSizeSamples = Source->bFixedSizeSamples;
    Dest->bTemporalCompression = Source->bTemporalCompression;
    Dest->lSampleSize = Source->lSampleSize;
    Dest->formattype = Source->formattype;
    // this pUnk is pretty always NULL
    // but sometimes it isn't and doing the correct
    // thing causes crashes
    if(Dest->pUnk != NULL)
    {
        //Dest->pUnk->Release();
    }
    Dest->pUnk = Source->pUnk;
    if(Dest->pUnk != NULL)
    {
        //Dest->pUnk->AddRef();
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

/**
 * Function to get a string describing the guid.
 * @return Name of the guid. The return value is only valid until the next call to this function
 */
const char* GetGUIDName(const GUID &guid)
{
    static char fourcc_buffer[20];
    struct TGUID2NAME
    {
        char *szName;
        GUID guid;
    };
    #define OUR_GUID_ENTRY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    { #name, { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } } },
    TGUID2NAME names[] =
    {
        #include <uuids.h>
    };

    if(guid==GUID_NULL)
    {
        return "GUID_NULL";
    }
    for(int i=0;i<countof(names);i++)
    {
        if(names[i].guid==guid)
        {
            return names[i].szName;
        }
    }
    //if we get here, the return value is only valid until the next call to this function
    if(guid.Data2==0 && guid.Data3==0x10 && ((DWORD *)guid.Data4)[0]==0xaa000080 && ((DWORD *)guid.Data4)[1]==0x719b3800)
    {
        char tmp[sizeof(DWORD)+1];
        memset(tmp,0,sizeof(DWORD)+1);
        memcpy(tmp,&guid.Data1,sizeof(DWORD));
        _snprintf(fourcc_buffer,20,"FOURCC '%s'",tmp);
        return fourcc_buffer;
    }
    BYTE *Uuid=NULL;
    static char uuidbuffer[50];
    UuidToStringA(const_cast<UUID*>(&guid), &Uuid);
    sprintf(uuidbuffer,"{%s}",Uuid);
    RpcStringFreeA(&Uuid);
    return uuidbuffer;
}

#ifndef NOLOGGING

void _LOG(LPCSTR szFormat, ...)
{
    char szMessage[2048];
    va_list Args;

    va_start(Args, szFormat);
    int result=_vsnprintf(szMessage,2048, szFormat, Args);
    va_end(Args);
    if(result==-1)
    {
        OutputDebugStringA("DebugString too long, truncated!!\n");
    }
    OutputDebugStringA(szMessage);
}

void LogSample(IMediaSample* Sample, LPCSTR Desc)
{
    AM_SAMPLE2_PROPERTIES SampleProperties;
    SI(IMediaSample2) Sample2 = Sample;
    if(Sample2)
    {
        Sample2->GetProperties(sizeof(AM_SAMPLE2_PROPERTIES), (BYTE*)&SampleProperties);
        LOG(DBGLOG_ALL, ("%s - IMediaSample2 Dump\n", Desc));
        LOG(DBGLOG_ALL, (" cbData %d\n", SampleProperties.cbData));
        LOG(DBGLOG_ALL, (" dwTypeSpecificFlags %08x\n", SampleProperties.dwTypeSpecificFlags));
        LOG(DBGLOG_ALL, (" dwSampleFlags %08x\n", SampleProperties.dwSampleFlags));
        LOG(DBGLOG_ALL, (" lActual %d\n", SampleProperties.lActual));
        LOG(DBGLOG_ALL, (" Start %d\n", (long)SampleProperties.tStart));
        LOG(DBGLOG_ALL, (" Stop %d\n", (long)SampleProperties.tStop));
        LOG(DBGLOG_ALL, (" length %d\n", (long)(SampleProperties.tStop - SampleProperties.tStart)));
        LOG(DBGLOG_ALL, (" dwStreamId %08x\n", SampleProperties.dwStreamId));
        LOG(DBGLOG_ALL, (" cbBuffer %d\n", SampleProperties.cbBuffer));
    }
    else
    {
    }
}

void LogMediaType(const AM_MEDIA_TYPE* MediaType, LPCSTR Desc, int LogLevel)
{
    struct TFlag2String
    {
        DWORD dwFlag;
        char *szName;
    };
    //BYTE* Uuid;

    LOG(LogLevel, ("%s - AM_MEDIA_TYPE Dump\n", Desc));
    LOG(LogLevel, (" Major Type %s\n", GetGUIDName(MediaType->majortype)));
    LOG(LogLevel, (" Sub Type %s\n",GetGUIDName(MediaType->subtype)));
    LOG(LogLevel, (" Format size %d\n", MediaType->cbFormat));
    LOG(LogLevel, (" Format Type %s\n", GetGUIDName(MediaType->formattype)));

    if(MediaType->formattype==FORMAT_VideoInfo || MediaType->formattype==FORMAT_VideoInfo2 || MediaType->formattype==FORMAT_MPEG2_VIDEO || MediaType->formattype==FORMAT_MPEGVideo)
    {
        BITMAPINFOHEADER *pBMI=NULL;
        VIDEOINFOHEADER* pHeader = (VIDEOINFOHEADER*)MediaType->pbFormat;

        //parts of the struct that is common to both VIDEOINFOHEADER and VIDEOINFOHEADER2
        LOG(LogLevel, (" Source RECT: (L: %ld T: %ld R: %ld B: %ld )\n",pHeader->rcSource.left,pHeader->rcSource.top,pHeader->rcSource.right,pHeader->rcSource.bottom));
        LOG(LogLevel, (" Target RECT: (L: %ld T: %ld R: %ld B: %ld )\n",pHeader->rcTarget.left,pHeader->rcTarget.top,pHeader->rcTarget.right,pHeader->rcTarget.bottom));
        LOG(LogLevel, (" BitRate: %lu ErrorRate: %lu\n",pHeader->dwBitRate,pHeader->dwBitErrorRate));
        LOG(LogLevel, (" AvgTimePerFrame: %g fps\n",1/(pHeader->AvgTimePerFrame/(double)10000000)));

        if(MediaType->formattype == FORMAT_VideoInfo || MediaType->formattype==FORMAT_MPEGVideo)
        {
            pBMI=&pHeader->bmiHeader;
        }
        else
        {
            VIDEOINFOHEADER2* pHeader2 = (VIDEOINFOHEADER2*)MediaType->pbFormat;
            pBMI=&pHeader2->bmiHeader;

            char buffer[1024];
            ZeroMemory(buffer,1024);
            TFlag2String flags[]=
            {
                {AMINTERLACE_IsInterlaced,"AMINTERLACE_IsInterlaced"},
                {AMINTERLACE_1FieldPerSample,"AMINTERLACE_1FieldPerSample"},
                {AMINTERLACE_Field1First,"AMINTERLACE_Field1First"},
                {AMINTERLACE_FieldPatField1Only,"AMINTERLACE_FieldPatField1Only"},
                {AMINTERLACE_FieldPatField2Only,"AMINTERLACE_FieldPatField2Only"},
                {AMINTERLACE_FieldPatBothRegular,"AMINTERLACE_FieldPatBothRegular"},
                {AMINTERLACE_DisplayModeBobOnly,"AMINTERLACE_DisplayModeBobOnly"},
                {AMINTERLACE_DisplayModeWeaveOnly,"AMINTERLACE_DisplayModeWeaveOnly"},
                {AMINTERLACE_DisplayModeBobOrWeave,"AMINTERLACE_DisplayModeBobOrWeave"}
            };

            for(int i=0;i<countof(flags);i++)
            {
                if(flags[i].dwFlag&pHeader2->dwInterlaceFlags)
                {
                    if(i!=0)
                    {
                        strcat(buffer,"|");
                    }
                    strcat(buffer,flags[i].szName);
                }
            }
            LOG(LogLevel, (" InterlaceFlags: %lu (%s)\n",pHeader2->dwInterlaceFlags,buffer));
            LOG(LogLevel, (" AspectRatio: %lux%lu (%g)\n",pHeader2->dwPictAspectRatioX,pHeader2->dwPictAspectRatioY,pHeader2->dwPictAspectRatioX/(double)pHeader2->dwPictAspectRatioY));
            if(pHeader2->dwControlFlags&AMCONTROL_USED)
            {
                if(pHeader2->dwControlFlags&AMCONTROL_PAD_TO_4x3)
                {
                    LOG(LogLevel, (" ControllFlags: AMCONTROL_PAD_TO_4x3"));
                }
                else if(pHeader2->dwControlFlags&AMCONTROL_PAD_TO_16x9)
                {
                    LOG(LogLevel, (" ControllFlags: AMCONTROL_PAD_TO_16x9"));
                }
                else
                {
                    LOG(LogLevel, (" ControllFlags: Unknown (%lu)",pHeader2->dwControlFlags));
                }
            }
            if(MediaType->formattype == FORMAT_MPEG2_VIDEO)
            {
                MPEG2VIDEOINFO* pHeaderMpeg = (MPEG2VIDEOINFO*)MediaType->pbFormat;
                LOG(LogLevel, (" Mpeg Flags: %lu\n", pHeaderMpeg->dwFlags));


            }
        }
        //BITMAPINFOHEADER
        LOG(LogLevel, (" biSize %d\n", pBMI->biSize));
        LOG(LogLevel, (" biWidth %d\n", pBMI->biWidth));
        LOG(LogLevel, (" biHeight %d\n", pBMI->biHeight));
        LOG(LogLevel, (" biPlanes %d\n", pBMI->biPlanes));
        LOG(LogLevel, (" biBitCount %d\n", pBMI->biBitCount));
        LOG(LogLevel, (" biCompression %d\n", pBMI->biCompression));
        LOG(LogLevel, (" biSizeImage %d\n", pBMI->biSizeImage));
        LOG(LogLevel, (" biXPelsPerMeter %d\n", pBMI->biXPelsPerMeter));
        LOG(LogLevel, (" biYPelsPerMeter %d\n", pBMI->biYPelsPerMeter));
    }
    else if(MediaType->formattype==FORMAT_WaveFormatEx)
    {
        WAVEFORMATEX* wfe = (WAVEFORMATEX*)MediaType->pbFormat;
        LOG(LogLevel, (" wFormatTag %d\n", wfe->wFormatTag));
        LOG(LogLevel, (" nChannels %d\n", wfe->nChannels));
        LOG(LogLevel, (" nSamplesPerSec %d\n", wfe->nSamplesPerSec));
        LOG(LogLevel, (" nAvgBytesPerSec %d\n", wfe->nAvgBytesPerSec));
        LOG(LogLevel, (" nBlockAlign %d\n", wfe->nBlockAlign));
        LOG(LogLevel, (" wBitsPerSample %d\n", wfe->wBitsPerSample));
        LOG(LogLevel, (" cbSize %d\n", wfe->cbSize));
    }
}

void LogBadHRESULT(HRESULT hr, LPCSTR File, DWORD Line)
{
    //DXGetErrorString9 seems to be a bit better at returning a string for a
    //hresult but instead it only returns strings like
    //VFW_E_INVALID_MEDIA_TYPE but that shoud be enough
#ifdef HAVE_DXSDK
    const TCHAR *ErrorMsg=DXGetErrorString9(hr);
#else
    const TCHAR *ErrorMsg=NULL;
#endif
    if(ErrorMsg!=NULL)
    {
        String816 ErrorText(ErrorMsg);
        _LOG("%s(%d) : Bad HRESULT 0x%08x  - %s\n", File, Line, hr, (const char *)ErrorText);
    }
    else
    {
        _LOG("%s(%d) : Bad HRESULT 0x%08x\n", File, Line, hr);
    }
}

#endif // NOLOGGING

HRESULT RegisterFilter(const CLSID& clsidFilter, LPCWSTR wszName, const REGFILTER2* pRegFilter)
{
    SI(IFilterMapper2) FilterMapper;

    HRESULT hr = FilterMapper.CreateInstance(CLSID_FilterMapper, CLSCTX_INPROC_SERVER);
    CHECK(hr);
    hr = FilterMapper->RegisterFilter(clsidFilter, wszName, NULL, NULL, NULL, pRegFilter);
    CHECK(hr);
    return hr;
}

HRESULT UnregisterFilter(const CLSID& clsidFilter)
{
    SI(IFilterMapper2) FilterMapper;
    HRESULT hr = FilterMapper.CreateInstance(CLSID_FilterMapper, CLSCTX_INPROC_SERVER);
    CHECK(hr);
    hr = FilterMapper->UnregisterFilter(NULL, NULL, clsidFilter);
    CHECK(hr);
    return hr;
}

bool IsRunningInGraphEdit()
{
    char szCmdLine[MAX_PATH*2 + 1];
    strncpy(szCmdLine, GetCommandLineA(), MAX_PATH*2);
    _strlwr(szCmdLine);
    return (strstr(szCmdLine, "graphedt.exe") != NULL);
}