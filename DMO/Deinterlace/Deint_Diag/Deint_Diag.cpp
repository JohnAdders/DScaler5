////////////////////////////////////////////////////////////////////////////
// $Id: Deint_Diag.cpp,v 1.4 2003-07-25 16:02:56 adcockj Exp $
/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2003 John Adcock.  All rights reserved.
// This software was based on sample code generated by the 
// DMO project wizard.  That code is (c) Microsoft Corporation
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
// Revision 1.3  2003/07/25 16:00:54  adcockj
// Remove 704 stuff
//
// Revision 1.2  2003/07/18 09:26:34  adcockj
// Corrections to assembler files (does not compile)
//
// Revision 1.1  2003/05/21 13:41:12  adcockj
// Added new deinterlace methods
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Deint_Diag.h"

extern "C"
{
    void __cdecl Deint_Diag_Core_YUY2_MMX(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_YUY2_SSE(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_YUY2_3dNow(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_Luma_MMX(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_Luma_SSE(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    void __cdecl Deint_Diag_Core_Luma_3dNow(BYTE* M1, BYTE* T2, BYTE* B2, BYTE* M3, BYTE* T4, BYTE* B4, BYTE* Dest, DWORD Count);
    extern unsigned __int64 MOVE;
}

static ParamInfo* g_params = NULL;

#define NUM_FIELDS 4

// Diag method requires :
//  4 fields to operate on
//  introduce one field delay
//  And has a medium high complexity
CDeint_Diag::CDeint_Diag() :
    CDeintDMO(NUM_FIELDS, 1, 7)
{
	// Initialize parameters in IMediaParam
	InitParams(1, &GUID_TIME_REFERENCE, 0, 0, 0, g_params);

}

CDeint_Diag::~CDeint_Diag()
{
}

void CDeint_Diag::DoDeinterlacingMethod(DMO_OUTPUT_DATA_BUFFER *pOutputBuffer)
{
    ATLASSERT(m_FieldsInBuffer >= NUM_FIELDS);

    HRESULT hr;

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(InputType(0)->pbFormat);
    VIDEOINFOHEADER2* OutputInfo = (VIDEOINFOHEADER2*)(OutputType(0)->pbFormat);

    BYTE* pInputData[NUM_FIELDS];
    DWORD InputLength[NUM_FIELDS];
    BYTE* pOutputData;
    DWORD OutputLength;
    DWORD LineLength;
    DWORD TwoLineLength;
    DWORD OutputPitch;

    for (int i(0); i < NUM_FIELDS; ++i)
    {
        hr = m_IncomingFields[m_FieldsInBuffer - i - 1].m_Buffer->GetBufferAndLength(&pInputData[i], &InputLength[i]);
        if(FAILED(hr)) return;
    }

    hr = pOutputBuffer->pBuffer->GetBufferAndLength(&pOutputData, &OutputLength);
    if(FAILED(hr)) return;

    if(OutputLength < OutputInfo->bmiHeader.biSizeImage)
    {
        hr = pOutputBuffer->pBuffer->SetLength(OutputInfo->bmiHeader.biSizeImage);
        if(FAILED(hr)) return;
    }
    // do 4:2:2 format up here and we need to 
    // worry about deinterlacing both luma and chroma
    if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','U','Y','2'))
    {
        LineLength = InputInfo->bmiHeader.biWidth * 2;
        TwoLineLength = InputInfo->bmiHeader.biWidth * 4;
        OutputPitch = OutputInfo->bmiHeader.biWidth * 2;

        if(m_IncomingFields[m_FieldsInBuffer - 1].IsTopLine == TRUE)
        {
            pInputData[1] += LineLength;
            pInputData[3] += LineLength;

            memcpy(pOutputData, pInputData[0], LineLength);
            pOutputData += LineLength;

            for(int i(0); i < InputInfo->bmiHeader.biHeight/2 - 1; ++i)
            {
                memcpy(pOutputData, pInputData[1], LineLength);
                pOutputData += OutputPitch;

                pInputData[0] += TwoLineLength;
                pInputData[2] += TwoLineLength;

                Deint_Diag_Core_YUY2_MMX(
                                            pInputData[0],
                                            pInputData[1],
                                            pInputData[1] + TwoLineLength,
                                            pInputData[2],
                                            pInputData[3],
                                            pInputData[3] + TwoLineLength,
                                            pOutputData,
                                            LineLength
                                        );
                pOutputData += OutputPitch;

                pInputData[1] += TwoLineLength;
                pInputData[3] += TwoLineLength;
            }

            memcpy(pOutputData, pInputData[1], LineLength);
        }
        else
        {
            pInputData[0] += LineLength;
            pInputData[2] += LineLength;

            for(int i(0); i < InputInfo->bmiHeader.biHeight/2 - 1; ++i)
            {
                memcpy(pOutputData, pInputData[1], LineLength);
                pOutputData += LineLength;

                Deint_Diag_Core_YUY2_MMX(
                                            pInputData[0],
                                            pInputData[1],
                                            pInputData[1] + TwoLineLength,
                                            pInputData[2],
                                            pInputData[3],
                                            pInputData[3] + TwoLineLength,
                                            pOutputData,
                                            LineLength
                                        );
                pOutputData += OutputPitch;

                for(int j(0); j < NUM_FIELDS; ++j)
                {
                    pInputData[j] += TwoLineLength;
                }
            }

            memcpy(pOutputData, pInputData[1], LineLength);
            pOutputData += LineLength;
            memcpy(pOutputData, pInputData[0], LineLength);
            pOutputData += LineLength;
        }
    }
    // otherwise it's a 4:2:0 plannar format and we just 
    // worry about deinterlacing the luma and just copy the chroma
    else
    {
        LineLength = InputInfo->bmiHeader.biWidth;
        TwoLineLength = InputInfo->bmiHeader.biWidth * 2;
        OutputPitch = OutputInfo->bmiHeader.biWidth;


        // process luma
        if(m_IncomingFields[m_FieldsInBuffer - 1].IsTopLine == TRUE)
        {
            pInputData[1] += LineLength;
            pInputData[3] += LineLength;

            memcpy(pOutputData, pInputData[0], LineLength);
            pOutputData += LineLength;

            for(int i(0); i < InputInfo->bmiHeader.biHeight/2 - 1; ++i)
            {
                memcpy(pOutputData, pInputData[1], LineLength);
                pOutputData += OutputPitch;

                pInputData[0] += TwoLineLength;
                pInputData[2] += TwoLineLength;

                Deint_Diag_Core_Luma_MMX(
                                            pInputData[0],
                                            pInputData[1],
                                            pInputData[1] + TwoLineLength,
                                            pInputData[2],
                                            pInputData[3],
                                            pInputData[3] + TwoLineLength,
                                            pOutputData,
                                            LineLength
                                        );
                pOutputData += OutputPitch;

                pInputData[1] += TwoLineLength;
                pInputData[3] += TwoLineLength;
            }

            memcpy(pOutputData, pInputData[1], LineLength);
        }
        else
        {
            pInputData[0] += LineLength;
            pInputData[2] += LineLength;

            for(int i(0); i < InputInfo->bmiHeader.biHeight/2 - 1; ++i)
            {
                memcpy(pOutputData, pInputData[1], LineLength);
                pOutputData += LineLength;

                Deint_Diag_Core_Luma_MMX(
                                            pInputData[0],
                                            pInputData[1],
                                            pInputData[1] + TwoLineLength,
                                            pInputData[2],
                                            pInputData[3],
                                            pInputData[3] + TwoLineLength,
                                            pOutputData,
                                            LineLength
                                        );
                pOutputData += OutputPitch;

                for(int j(0); j < NUM_FIELDS; ++j)
                {
                    pInputData[j] += TwoLineLength;
                }
            }

            memcpy(pOutputData, pInputData[1], LineLength);
            pOutputData += LineLength;
            memcpy(pOutputData, pInputData[0], LineLength);
            pOutputData += LineLength;
        }
        ProcessPlanarChroma(pInputData[1], pOutputData, InputInfo, OutputInfo);
    }
}


HRESULT CDeint_Diag::SetParamInternal(DWORD dwParamIndex, MP_DATA value, bool fSkipPasssingToParamManager)
{
    return S_OK;
}

HRESULT CDeint_Diag::GetClassID(CLSID *pClsid)
{
	// Check for valid pointer
	if( NULL == pClsid )
	{
		return E_POINTER;
	}

	*pClsid = __uuidof(CDeint_Diag);
	return S_OK;

} // GetClassID

STDMETHODIMP CDeint_Diag::get_Name(BSTR* Name)
{
    if(Name == NULL)
    {
        return E_POINTER;
    }
    CComBSTR Result;
    if(Result.LoadString(IDS_NAME))
    {
        return Result.CopyTo(Name);
    }
    else
    {
        return E_UNEXPECTED;
    }
}

STDMETHODIMP CDeint_Diag::get_License(eFreeLicense* License)
{
    if(License == NULL)
    {
        return E_POINTER;
    }
    *License = LGPL;
    return S_OK;
}

STDMETHODIMP CDeint_Diag::get_Authors(BSTR* Authors)
{
    if(Authors == NULL)
    {
        return E_POINTER;
    }
    CComBSTR Result;
    if(Result.LoadString(IDS_AUTHORS))
    {
        return Result.CopyTo(Authors);
    }
    else
    {
        return E_UNEXPECTED;
    }
}


///////////////////////
//
// Required ATL COM stuff
//
CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(__uuidof(CDeint_Diag), CDeint_Diag)
END_OBJECT_MAP()

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
	if (DLL_PROCESS_ATTACH == dwReason)
	{
		_Module.Init(ObjectMap, hInstance);
		DisableThreadLibraryCalls(hInstance);
	}
	else if (DLL_PROCESS_DETACH == dwReason)
    {
		_Module.Term();
    }
	return TRUE;    // ok
}

STDAPI DllRegisterServer()
{
    return DMODllRegisterDeintDMO(L"Diag", __uuidof(CDeint_Diag));
}

STDAPI DllUnregisterServer()
{
    return DMODllUnregisterDeintDMO(__uuidof(CDeint_Diag));
}
