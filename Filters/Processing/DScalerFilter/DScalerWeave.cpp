////////////////////////////////////////////////////////////////////////////
// $Id: DScalerWeave.cpp,v 1.5 2004-12-06 18:05:01 adcockj Exp $
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
// Revision 1.4  2004/11/01 14:09:55  adcockj
// More DScaler filter insipred changes
//
// Revision 1.3  2004/03/05 17:21:33  adcockj
// Better handling of dynamic format changes
//
// Revision 1.2  2004/03/05 15:56:30  adcockj
// Interim check in of DScalerFilter (compiles again)
//
// Revision 1.1  2004/02/06 12:17:17  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
// Revision 1.1  2003/10/31 17:19:37  adcockj
// Added support for manual pulldown selection (works with Elecard Filters)
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DScaler.h"
#include "DSVideoOutPin.h"

HRESULT CDScaler::Weave(IInterlacedBufferStack* Stack, IMediaBuffer* pOutputBuffer)
{
#ifdef _DEBUG
    DWORD NumFields(0);
    Stack->get_NumFields(&NumFields);
    ASSERT(NumFields > 1);
#endif

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(m_InternalMTInput.pbFormat);
    VIDEOINFOHEADER2* OutputInfo = (VIDEOINFOHEADER2*)(m_VideoOutPin->m_ConnectedMediaType.pbFormat);

    SI(IInterlacedField) pNewerBuffer;
    SI(IInterlacedField) pOlderBuffer;

    HRESULT hr = Stack->GetField(0, pNewerBuffer.GetReleasedInterfaceReference()); 
    if(FAILED(hr)) return hr;

    hr = Stack->GetField(1, pOlderBuffer.GetReleasedInterfaceReference()); 
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
            WeavePlanarChroma(pInputDataNewer, pInputDataOlder, pOutputData, InputInfo, OutputInfo);
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
            WeavePlanarChroma(pInputDataOlder, pInputDataNewer, pOutputData, InputInfo, OutputInfo);
        }
    }
    return S_OK;
}

void CDScaler::WeavePlanarChroma(BYTE* pUpperChroma, BYTE* pLowerChroma, BYTE* pOutputData, VIDEOINFOHEADER2* InputInfo, VIDEOINFOHEADER2* OutputInfo)
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
    pLowerChroma += InputInfo->bmiHeader.biWidth / 2;
    for(int i(0); i < InputInfo->bmiHeader.biHeight / 2; ++i)
    {
        memcpy(pOutputData, pUpperChroma, LineLength);
        pOutputData += OutputInfo->bmiHeader.biWidth / 2;
        memcpy(pOutputData, pLowerChroma, LineLength);
        pOutputData += OutputInfo->bmiHeader.biWidth / 2;
        pLowerChroma += InputInfo->bmiHeader.biWidth / 2;
        pUpperChroma += InputInfo->bmiHeader.biWidth / 2;
    }
}

