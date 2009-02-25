////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
/////////////////////////////////////////////////////////////////////////////
//
// This file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Library General Public License as published by
// the Free Software Foundation; either version 2 of the License, or (at your
// option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Library General Public License for more details
//
/////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InputPin.h"
#include "DScaler.h"
#include "OutputPin.h"
#include "EnumMediaTypes.h"
#include "MediaBufferWrapper.h"
#include "Process.h"

STDMETHODIMP CInputPin::Weave(IInterlacedBufferStack* Stack, IMediaBuffer* pOutputBuffer)
{
#ifdef _DEBUG
    DWORD NumFields(0);
    Stack->get_NumFields(&NumFields);
    ATLASSERT(NumFields > 1);
#endif

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMediaType.pbFormat);
    VIDEOINFOHEADER2* OutputInfo = (VIDEOINFOHEADER2*)(m_OutputPin->m_CurrentMediaType.pbFormat);

    CComPtr<IInterlacedField> pNewerBuffer;
    CComPtr<IInterlacedField> pOlderBuffer;

    HRESULT hr = Stack->GetField(0, &pNewerBuffer); 
    if(FAILED(hr)) return hr;

    hr = Stack->GetField(1, &pOlderBuffer); 
    if(FAILED(hr)) return hr;

    BOOLEAN IsTopLine = FALSE;

    hr = pNewerBuffer->get_TopFieldFirst(&IsTopLine);
    if(FAILED(hr)) return hr;

    BYTE* pInputDataNewer;
    DWORD InputLengthNewer;
    BYTE* pInputDataOlder;
    DWORD InputLengthOlder;
    BYTE* pOutputData;
    DWORD OutputLength;
    
    hr = pNewerBuffer->GetBufferAndLength(&pInputDataNewer, &InputLengthNewer);
    if(FAILED(hr)) return hr;

    hr = pOlderBuffer->GetBufferAndLength(&pInputDataOlder, &InputLengthOlder);
    if(FAILED(hr)) return hr;

    hr = pOutputBuffer->GetBufferAndLength(&pOutputData, &OutputLength);
    if(FAILED(hr)) return hr;

    if(OutputLength < OutputInfo->bmiHeader.biSizeImage)
    {
        hr = pOutputBuffer->SetLength(OutputInfo->bmiHeader.biSizeImage);
        if(FAILED(hr)) return hr;
    }
    // do 4:2:2 format up here and we need to 
    // worry about deinterlacing both luma and chroma
    if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
    {
		DWORD LineLength;
		if(InputInfo->rcSource.right > 0)
		{
			LineLength = InputInfo->rcSource.right * 2;
		}
		else
		{
			LineLength = InputInfo->bmiHeader.biWidth * 2;
		}

		if(IsTopLine == TRUE)
        {
			pInputDataOlder += InputInfo->bmiHeader.biWidth * 2;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputDataNewer, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                memcpy(pOutputData, pInputDataOlder, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                pInputDataNewer += InputInfo->bmiHeader.biWidth * 4;
                pInputDataOlder += InputInfo->bmiHeader.biWidth * 4;
            }
        }
        else
        {
            pInputDataNewer += InputInfo->bmiHeader.biWidth * 2;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputDataOlder, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                memcpy(pOutputData, pInputDataNewer, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                pInputDataNewer += InputInfo->bmiHeader.biWidth * 4;
                pInputDataOlder += InputInfo->bmiHeader.biWidth * 4;
            }
        }
    }
    // otherwise it's a 4:2:0 plannar format and we just 
    // worry about deinterlacing the luma and just copy the chroma
    else
    {
		DWORD LineLength;
		if(InputInfo->rcSource.right > 0)
		{
			LineLength = InputInfo->rcSource.right;
		}
		else
		{
			LineLength = InputInfo->bmiHeader.biWidth;
		}

		// process luma
        if(IsTopLine == TRUE)
        {
            pInputDataOlder += InputInfo->bmiHeader.biWidth;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputDataNewer, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                memcpy(pOutputData, pInputDataOlder, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                pInputDataNewer += InputInfo->bmiHeader.biWidth * 2;
                pInputDataOlder += InputInfo->bmiHeader.biWidth * 2;
            }
            pInputDataOlder -= InputInfo->bmiHeader.biWidth;
        }
        else
        {
            pInputDataNewer += InputInfo->bmiHeader.biWidth;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputDataOlder, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                memcpy(pOutputData, pInputDataNewer, LineLength);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                pInputDataNewer += InputInfo->bmiHeader.biWidth * 2;
                pInputDataOlder += InputInfo->bmiHeader.biWidth * 2;
            }
            pInputDataNewer -= InputInfo->bmiHeader.biWidth;
        }
        ProcessPlanarChroma(pInputDataNewer, pOutputData, InputInfo, OutputInfo);
    }
    return S_OK;
}

void CInputPin::ProcessPlanarChroma(BYTE* pInputData, BYTE* pOutputData, VIDEOINFOHEADER2* InputInfo, VIDEOINFOHEADER2* OutputInfo)
{
    // copy chroma over
    // the formats for YV12 and NV12 are different
    // but the both 
    if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','V','1','2'))
    {
		DWORD LineLength;
		if(InputInfo->rcSource.right > 0)
		{
			LineLength = InputInfo->rcSource.right / 2;
		}
		else
		{
			LineLength = InputInfo->bmiHeader.biWidth / 2;
		}
        // copy V then U
        // there are biWidth / 2 x biHeight/2 of V 
        // followed by biWidth / 2 x biHeight/2 of U
        for(int i(0); i < InputInfo->bmiHeader.biHeight; ++i)
        {
            memcpy(pOutputData, pInputData, LineLength);
            pOutputData += OutputInfo->bmiHeader.biWidth / 2;
            pInputData += InputInfo->bmiHeader.biWidth / 2;
        }
    }
    else if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('N','V','1','2'))
    {
		DWORD LineLength;
		if(InputInfo->rcSource.right > 0)
		{
			LineLength = InputInfo->rcSource.right;
		}
		else
		{
			LineLength = InputInfo->bmiHeader.biWidth;
		}

		// copy U & V - there are biWidth / 2 x biHeight/2 of UV samples
        for(int i(0); i < InputInfo->bmiHeader.biHeight / 2; ++i)
        {
            memcpy(pOutputData, pInputData, LineLength);
            pOutputData += OutputInfo->bmiHeader.biWidth;
            pInputData += InputInfo->bmiHeader.biWidth;
        }
    }
    else
    {
        ; // very unexpected
    }
}

