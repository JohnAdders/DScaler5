////////////////////////////////////////////////////////////////////////////
// $Id: CDeintDMO.cpp,v 1.7 2003-07-25 16:00:54 adcockj Exp $
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
// Revision 1.6  2003/05/21 17:05:57  adcockj
// Added new filter
//
// Revision 1.5  2003/05/21 13:41:11  adcockj
// Added new deinterlace methods
//
// Revision 1.4  2003/05/20 16:50:57  adcockj
// Interim checkin, preparation for DMO processing path
//
// Revision 1.3  2003/05/19 07:02:47  adcockj
// Fixes suggested by Torbjorn to media types
//
// Revision 1.2  2003/05/17 11:29:35  adcockj
// Fixed crashing
//
// Revision 1.1  2003/05/16 16:19:12  adcockj
// Added new files into DMO framework
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "CDeintDMO.h"


/////////////////////////////////////////////////////////////////////////////
// CDeintDMO

///////////////////////
//
// CDeintDMO::CDeintDMO
//
//  Constructor for CDeintDMO.
//
CDeintDMO::CDeintDMO(long FieldsToBuffer, long FieldsDelay, long Complexity):
    m_ComplexityIndex(Complexity),
    CVideoDMO(FieldsToBuffer, FieldsDelay, IS_DEINT)

{
    ATLASSERT(m_NumFieldsToBuffer < MAX_FIELDS_IN_BUFFER - 1);
    ZeroMemory(m_IncomingFields, sizeof(m_IncomingFields));
    m_FieldsInBuffer = 0;
    m_StartFieldsDone = 0;
    m_InternalState = STATE_START;
}


///////////////////////
//
// CDeintDMO::~CDeintDMO
//
//  Destructor for CDeintDMO.
//
CDeintDMO::~CDeintDMO()
{
}

///////////////////////////////////////
//
//  IMediaObjectImpl::InternalProcessInput
//
//  *** Called by ProcessInput, description below ***
//
//  The ProcessInput method delivers a buffer to the specified input stream.
//
//  Parameters
//      dwInputStreamIndex
//          Zero-based index of an input stream on the DMO.
//
//      pBuffer
//          Pointer to the buffer's IMediaBuffer interface.
//
//      dwFlags
//          Bitwise combination of zero or more flags from the
//          DMO_INPUT_DATA_BUFFER_FLAGS enumeration.
//
//      rtTimestamp
//          Time stamp that specifies the start time of the data in the buffer.
//          If the buffer has a valid time stamp, set the
//          DMO_INPUT_DATA_BUFFERF_TIME flag in the dwFlags parameter.
//          Otherwise, the DMO ignores this value.
//
//      rtTimelength
//          Reference time specifying the duration of the data in the buffer.
//          If this value is valid, set the DMO_INPUT_DATA_BUFFERF_TIMELENGTH
//          flag in the dwFlags parameter. Otherwise, the DMO ignores this value.
//
//  Return Value
//      S_FALSE No output to process
//      S_OK Success
//      DMO_E_INVALIDSTREAMINDEX Invalid stream index
//      DMO_E_NOTACCEPTING Data cannot be accepted
//
//  If the DMO does not process all the data in the buffer, it keeps a
//  reference count on the buffer. It releases the buffer once it has
//  generated all the output, unless it needs to perform lookahead on the data.
//  (To determine whether a DMO performs lookahead, call the GetInputStreamInfo
//  method.)
//
//  If this method returns DMO_E_NOTACCEPTING, call the ProcessOutput method
//  until the input stream can accept more data. To determine whether the stream
//  can accept more data, call the GetInputStatus method.
//
//  If the method returns S_FALSE, no output was generated from this input and the
//  application does not need to call ProcessOutput. However, a DMO is not required
//  to return S_FALSE in this situation; it might return S_OK.
//
//  Note:
//
//  Before this method calls InternalProcessInput, it calls
//  AllocateStreamingResources and InternalAcceptingInput. Therefore, the
//  implementation of InternalProcessInput can assume the following:
//
//  * All resources have been allocated.
//  * The input stream can accept data.
//
STDMETHODIMP CDeintDMO::InternalProcessInput(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimestamp, REFERENCE_TIME rtTimelength)
{
    HRESULT hr = S_OK;

    if (!pBuffer)
    {
        return E_POINTER;
    }
    
    if(InternalAcceptingInput(dwInputStreamIndex) == S_FALSE)
    {
        return DMO_E_NOTACCEPTING;
    }

    if(m_FieldsInBuffer > 0)
    {
        memmove(&m_IncomingFields[2], &m_IncomingFields[0], sizeof(TField) * m_FieldsInBuffer);
    }

    m_FieldsInBuffer += 2;
    ZeroMemory(&m_IncomingFields[0], sizeof(TField) * 2);

    BYTE* pData;
    DWORD Length;
    hr = pBuffer->GetBufferAndLength(&pData, &Length);
    if(FAILED(hr))
    {
        return hr;
    }

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(InputType(0)->pbFormat);
    if(InputInfo->dwInterlaceFlags & AMINTERLACE_Field1First)
    {
        m_IncomingFields[0].IsTopLine = FALSE;
        m_IncomingFields[1].IsTopLine = TRUE;
    }
    else
    {
        m_IncomingFields[0].IsTopLine = TRUE;
        m_IncomingFields[1].IsTopLine = FALSE;
    }

    m_IncomingFields[0].m_Buffer = pBuffer;
    m_IncomingFields[1].m_Buffer = pBuffer;


    if (dwFlags & DMO_INPUT_DATA_BUFFERF_TIME && dwFlags & DMO_INPUT_DATA_BUFFERF_TIMELENGTH)
    {
        m_IncomingFields[0].StartTime = rtTimestamp + rtTimelength / 2;
        m_IncomingFields[0].Length = rtTimelength - rtTimelength / 2;
        m_IncomingFields[1].StartTime = rtTimestamp;
        m_IncomingFields[1].Length = rtTimelength / 2;
        m_bValidTime = true;
    }
    else
    {
        m_bValidTime = false;
    }

    return hr;
}


///////////////////////////////////
//
//  IMediaObjectImpl::InternalProcessOutput
//
//  *** Called by ProcessOutput, description below ***
//
//  The ProcessOutput method generates output from the current input data.
//
//  Parameters
//
//      dwFlags
//          Bitwise combination of zero or more flags from the
//          DMO_PROCESS_OUTPUT_FLAGS enumeration.
//
//      cOutputBufferCount
//          Number of output buffers.
//
//      pOutputBuffers
//          [in, out] Pointer to an array of DMO_OUTPUT_DATA_BUFFER structures
//          containing the output buffers. Specify the size of the array in the
//          cOutputBufferCount parameter.
//
//      pdwStatus
//          [out] Pointer to a variable that receives a reserved value (zero).
//          The application should ignore this value.
//
//  Return Value
//      S_FALSE No output was generated
//      S_OK Success
//      E_FAIL Failure
//      E_INVALIDARG Invalid argument
//      E_POINTER NULL pointer argument
//
//  The pOutputBuffers parameter points to an array of DMO_OUTPUT_DATA_BUFFER
//  structures. The application must allocate one structure for each output
//  stream. To determine the number of output streams, call the GetStreamCount
//  method. Set the cOutputBufferCount parameter to this number.
//
//  Each DMO_OUTPUT_DATA_BUFFER structure contains a pointer to a buffer's
//  IMediaBuffer interface. The application allocates these buffers. The other
//  members of the structure are status fields. The DMO sets these fields if
//  the method succeeds. If the method fails, their values are undefined.
//
//  When the application calls ProcessOutput, the DMO processes as much input
//  data as possible. It writes the output data to the output buffers, starting
//  from the end of the data in each buffer. (To find the end of the data, call
//  the IMediaBuffer::GetBufferAndLength method.) The DMO never holds a
//  reference count on an output buffer.
//
//  If the DMO fills an entire output buffer and still has input data to
//  process, the DMO returns the DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE flag in the
//  DMO_OUTPUT_DATA_BUFFER structure. The application should check for this
//  flag by testing the dwStatus member of each structure.
//
//  If the method returns S_FALSE, no output was generated. However, a DMO is
//  not required to return S_FALSE in this situation; it might return S_OK.
//
//  Discarding data:
//
//  You can discard data from a stream by setting the
//  DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER flag in the dwFlags parameter.
//  For each stream that you want to discard, set the pBuffer member of the
//  DMO_OUTPUT_DATA_BUFFER structure to NULL.
//
//  For each stream in which pBuffer is NULL:
//
//  If the DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER flag is set, and the
//  stream is discardable or optional, the DMO discards the data.
//
//  If the flag is set but the stream is neither discardable nor optional, the
//  DMO discards the data if possible. It is not guaranteed to discard the
//  data.
//
//  If the flag is not set, the DMO does not produce output data for that
//  stream, but does not discard the data.
//
//  To check whether a stream is discardable or optional, call the
//  GetOutputStreamInfo method.
//
//  Note:
//
//  Before this method calls InternalProcessOutput, it calls
//  AllocateStreamingResources. Therefore, the implementation of
//  InternalProcessOutput can assume that all resources have been allocated.
//
STDMETHODIMP CDeintDMO::InternalProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, DMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus)
{
    HRESULT hr = S_OK;
    BOOL IsMoreToDo(FALSE);

    IMediaBuffer* pOutputBuffer = pOutputBuffers[0].pBuffer;

    // Overall the process we need to follow depends on where we are in the
    // stream
    // 1) If we are at the beginning and we haven't yet got enough buffers
    //    to send to our real method use the single field mode (normally Bob)
    //    But we don't remove the ones we've done at the end as they will
    //    be used as history
    // 2) In the middle we process as many of the fields that we can with our 
    //    Full potentially multi-field method and clear a field for each frame we
    //    produce
    // 3) At the end we need to clear down the remaining field by using the single 
    //    field method

    switch(m_InternalState)
    {
    case STATE_START:
        if(m_StartFieldsDone < m_FieldsInBuffer)
        {
            if(pOutputBuffer != NULL)
            {
                ProcessSingleFrame(pOutputBuffers);
            }
            ++m_StartFieldsDone;
        }
        // have we finished the beginning stuff
        if(m_StartFieldsDone == m_NumFieldsToBuffer - 1)
        {
            m_InternalState = STATE_RUNNING;
            IsMoreToDo = (m_FieldsInBuffer >= m_NumFieldsToBuffer);
        }
        else
        {
            IsMoreToDo = (m_StartFieldsDone < m_FieldsInBuffer);
        }
        break;
    case STATE_RUNNING:
        if(pOutputBuffer != NULL)
        {
            DoDeinterlacingMethod(pOutputBuffers);
            RemoveOneFieldFromBuffer();
        }
        else
        {
            // only discard data if we're asked to
            if(dwFlags & DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER)
            {
                RemoveOneFieldFromBuffer();
            }
        }
        IsMoreToDo = (m_FieldsInBuffer >= m_NumFieldsToBuffer);
        break;
    case STATE_FINISHING:
        if(m_FieldsInBuffer > 0)
        {
            if(pOutputBuffer != NULL)
            {
                ProcessSingleFrame(pOutputBuffers);
                RemoveOneFieldFromBuffer();
            }
            else
            {
                // only discard data if we're asked to
                if(dwFlags & DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER)
                {
                    RemoveOneFieldFromBuffer();
                }
            }
        }
        IsMoreToDo = (m_FieldsInBuffer > 0);
        break;
    }

    if(IsMoreToDo)
    {
        pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;
    }
    else
    {
        pOutputBuffers[0].dwStatus &= ~DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;
    }

    return hr;
}



STDMETHODIMP CDeintDMO::get_ComplexityIndex(long* pComplexity)
{
    if(pComplexity == NULL)
    {
        return E_POINTER;
    }
    *pComplexity = m_ComplexityIndex;
    return S_OK;
}

HRESULT CDeintDMO::InternalCheckInputType(DWORD dwInputStreamIndex, const DMO_MEDIA_TYPE *pmt)
{
	HRESULT hr = S_OK;

	// Check that we're being given
    // video in a videoinfoheader2 
	if((NULL == pmt) ||
		(MEDIATYPE_Video != pmt->majortype) ||
		(FORMAT_VIDEOINFO2 != pmt->formattype))
	{
		return DMO_E_INVALIDTYPE;
	}

    if (OutputTypeSet(0))
    {
        if(pmt->subtype != OutputType(0)->subtype)
        {
            return DMO_E_INVALIDTYPE;
        }
    }
    else
    {
        // check that it's one of the subtypes we understand
        if(pmt->subtype != MEDIASUBTYPE_YUY2 && 
           pmt->subtype != MEDIASUBTYPE_YV12 &&
           pmt->subtype != MEDIASUBTYPE_NV12)
        {
            return DMO_E_INVALIDTYPE;
        }
    }
    
    VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
    if((Format->dwInterlaceFlags & AMINTERLACE_IsInterlaced) && 
        (Format->dwInterlaceFlags & AMINTERLACE_FieldPatBothRegular))
    {
        return S_OK;
    }
    else
    {
        return DMO_E_INVALIDTYPE;
    }

	return hr;
}


HRESULT CDeintDMO::InternalCheckOutputType(DWORD dwOutputStreamIndex,const DMO_MEDIA_TYPE *pmt)
{
	HRESULT hr = S_OK;

	// Check that we're being given
    // video in a videoinfoheader2 
	if((NULL == pmt) ||
		(MEDIATYPE_Video != pmt->majortype) ||
		(FORMAT_VIDEOINFO2 != pmt->formattype))
	{
		return DMO_E_INVALIDTYPE;
	}

    if (InputTypeSet(0))
    {
        if(pmt->subtype != InputType(0)->subtype)
        {
            return DMO_E_INVALIDTYPE;
        }
    }
    else
    {
        // check that it's one of the subtypes we understand
        if(pmt->subtype != MEDIASUBTYPE_YUY2 && 
           pmt->subtype != MEDIASUBTYPE_YV12 &&
           pmt->subtype != MEDIASUBTYPE_NV12)
        {
            return DMO_E_INVALIDTYPE;
        }
    }

    // check that we are being asked for a suitable output format    
    // which is either non interlaced or 
    // anything but AMINTERLACE_DisplayModeBobOnly and regular
    VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
    if(Format->dwInterlaceFlags == 0 || 
        ((Format->dwInterlaceFlags & AMINTERLACE_IsInterlaced) && 
          (Format->dwInterlaceFlags & AMINTERLACE_FieldPatBothRegular) && 
          (Format->dwInterlaceFlags & AMINTERLACE_DisplayModeMask)))
    {
        return S_OK;
    }
    else
    {
        return DMO_E_INVALIDTYPE;
    }

	return hr;
}

HRESULT CDeintDMO::InternalGetInputType(DWORD dwInputStreamIndex, DWORD dwTypeIndex, DMO_MEDIA_TYPE *pmt)
{
	HRESULT hr = S_OK;

    // if output is connected then we prefer that one
    // with the change in interlaced flags
    if (OutputTypeSet(0))
    {
        if(dwTypeIndex == 0)
        {
            hr = MoCopyMediaType(pmt, OutputType(0));
            if(FAILED(hr)) 
            {
                return hr;
            }
            
            VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
            Format->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular; 
            
            return hr;
        }
        else
        {
    		return DMO_E_NO_MORE_ITEMS;
        }
    }


	if (dwTypeIndex >= 0 && dwTypeIndex <= 2)
	{
        // If pmt is NULL, and the type index is in range, we return S_OK
        if (pmt == NULL)
        {
            return S_OK;
        }

	    hr = MoInitMediaType(pmt, sizeof(VIDEOINFOHEADER2));

	    if (SUCCEEDED(hr))
	    {
		    pmt->majortype = MEDIATYPE_Video;
		    pmt->formattype = FORMAT_VIDEOINFO2;

            VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
            ZeroMemory(Format, sizeof(VIDEOINFOHEADER2));
            Format->dwInterlaceFlags = AMINTERLACE_IsInterlaced | AMINTERLACE_FieldPatBothRegular; 
            Format->dwPictAspectRatioX = 4;
            Format->dwPictAspectRatioY = 3;
            Format->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            Format->bmiHeader.biHeight = 576;
            Format->bmiHeader.biWidth = 768;

            switch(dwTypeIndex)
            {
            case 0:
    		    pmt->subtype = MEDIASUBTYPE_YUY2;
                Format->bmiHeader.biBitCount = 16;
                Format->bmiHeader.biCompression = MAKEFOURCC('Y', 'U', 'Y', '2');
                Format->bmiHeader.biSizeImage = 576*768*2;
                break;
            case 1:
    		    pmt->subtype = MEDIASUBTYPE_YV12;
                Format->bmiHeader.biBitCount = 12;
                Format->bmiHeader.biCompression = MAKEFOURCC('Y', 'V', '1', '2');
                Format->bmiHeader.biSizeImage = 576*768*3/2;
                break;
            case 2:
    		    pmt->subtype = MEDIASUBTYPE_NV12;
                Format->bmiHeader.biBitCount = 12;
                Format->bmiHeader.biCompression = MAKEFOURCC('N', 'V', '1', '2');
                Format->bmiHeader.biSizeImage = 576*768*3/2;
                break;
            default:
                break;
            }
	    }
    }
    else
    {
		return DMO_E_NO_MORE_ITEMS;
	}

	return hr;
}

HRESULT CDeintDMO::InternalGetOutputType(DWORD dwOutputStreamIndex, DWORD dwTypeIndex, DMO_MEDIA_TYPE *pmt)
{
	HRESULT hr = S_OK;

    // if output is connected then we prefer that one
    // with the change in interlaced flags
    if (InputTypeSet(0))
    {
        if(dwTypeIndex == 0)
        {
            hr = MoCopyMediaType(pmt, InputType(0));
            if(FAILED(hr)) 
            {
                return hr;
            }
            
            VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
            Format->dwInterlaceFlags = 0; 
            
            return hr;
        }
        else
        {
    		return DMO_E_NO_MORE_ITEMS;
        }
    }

	if (dwTypeIndex >= 0 && dwTypeIndex <= 2)
	{
        // If pmt is NULL, and the type index is in range, we return S_OK
        if (pmt == NULL)
        {
            return S_OK;
        }

	    hr = MoInitMediaType(pmt, sizeof(VIDEOINFOHEADER2));

	    if (SUCCEEDED(hr))
	    {
		    pmt->majortype = MEDIATYPE_Video;
		    pmt->formattype = FORMAT_VIDEOINFO2;
            VIDEOINFOHEADER2* Format = (VIDEOINFOHEADER2*)pmt->pbFormat;
            ZeroMemory(Format, sizeof(VIDEOINFOHEADER2));
            Format->dwPictAspectRatioX = 4;
            Format->dwPictAspectRatioY = 3;
            Format->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            Format->bmiHeader.biHeight = 576;
            Format->bmiHeader.biWidth = 768;

            switch(dwTypeIndex)
            {
            case 0:
    		    pmt->subtype = MEDIASUBTYPE_YUY2;
                Format->bmiHeader.biBitCount = 16;
                Format->bmiHeader.biCompression = MAKEFOURCC('Y', 'U', 'Y', '2');
                Format->bmiHeader.biSizeImage = 576*768*2;
                break;
            case 1:
    		    pmt->subtype = MEDIASUBTYPE_YV12;
                Format->bmiHeader.biBitCount = 12;
                Format->bmiHeader.biCompression = MAKEFOURCC('Y', 'V', '1', '2');
                Format->bmiHeader.biSizeImage = 576*768*3/2;
                break;
            case 2:
    		    pmt->subtype = MEDIASUBTYPE_NV12;
                Format->bmiHeader.biBitCount = 12;
                Format->bmiHeader.biCompression = MAKEFOURCC('N', 'V', '1', '2');
                Format->bmiHeader.biSizeImage = 576*768*3/2;
                break;
            default:
                break;
            }
	    }
    }
    else
    {
		return DMO_E_NO_MORE_ITEMS;
	}

	return hr;
}

void CDeintDMO::ProcessSingleFrame(DMO_OUTPUT_DATA_BUFFER *pOutputBuffer)
{
    ATLASSERT(m_FieldsInBuffer > 0);

    VIDEOINFOHEADER2* InputInfo = (VIDEOINFOHEADER2*)(InputType(0)->pbFormat);
    VIDEOINFOHEADER2* OutputInfo = (VIDEOINFOHEADER2*)(OutputType(0)->pbFormat);

    BYTE* pInputData;
    DWORD InputLength;
    BYTE* pOutputData;
    DWORD OutputLength;
    HRESULT hr = m_IncomingFields[m_FieldsInBuffer - 1].m_Buffer->GetBufferAndLength(&pInputData, &InputLength);
    if(FAILED(hr)) return;

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
        if(m_IncomingFields[m_FieldsInBuffer - 1].IsTopLine == TRUE)
        {
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth * 2);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth * 2);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                pInputData += InputInfo->bmiHeader.biWidth * 4;
            }

        }
        else
        {
            pInputData += InputInfo->bmiHeader.biWidth * 2;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth * 2);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth * 2);
                pOutputData += OutputInfo->bmiHeader.biWidth * 2;
                pInputData += InputInfo->bmiHeader.biWidth * 4;
            }
        }
    }
    // otherwise it's a 4:2:0 plannar format and we just 
    // worry about deinterlacing the luma and just copy the chroma
    else
    {
        // process luma
        if(m_IncomingFields[m_FieldsInBuffer - 1].IsTopLine == TRUE)
        {
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                pInputData += InputInfo->bmiHeader.biWidth * 2;
            }

        }
        else
        {
            pInputData += InputInfo->bmiHeader.biWidth;
            for(int i(0); i < InputInfo->bmiHeader.biHeight/2; ++i)
            {
                memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth);
                pOutputData += OutputInfo->bmiHeader.biWidth;
                pInputData += InputInfo->bmiHeader.biWidth * 2;
            }
            pInputData -= InputInfo->bmiHeader.biWidth;
        }
        ProcessPlanarChroma(pInputData, pOutputData, InputInfo, OutputInfo);
    }
}

void CDeintDMO::ProcessPlanarChroma(BYTE* pInputData, BYTE* pOutputData, VIDEOINFOHEADER2* InputInfo, VIDEOINFOHEADER2* OutputInfo)
{
    // copy chroma over
    // the formats for YV12 and NV12 are different
    // but the both 
    if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('Y','V','1','2'))
    {
        // copy V then U
        // there are biWidth / 2 x biHeight/2 of V 
        // followed by biWidth / 2 x biHeight/2 of U
        for(int i(0); i < InputInfo->bmiHeader.biHeight; ++i)
        {
            memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth / 2);
            pOutputData += OutputInfo->bmiHeader.biWidth / 2;
            pInputData += InputInfo->bmiHeader.biWidth / 2;
        }
    }
    else if(InputInfo->bmiHeader.biCompression == MAKEFOURCC('N','V','1','2'))
    {
        // copy U & V - there are biWidth / 2 x biHeight/2 of UV samples
        for(int i(0); i < InputInfo->bmiHeader.biHeight / 2; ++i)
        {
            memcpy(pOutputData, pInputData, InputInfo->bmiHeader.biWidth);
            pOutputData += OutputInfo->bmiHeader.biWidth;
            pInputData += InputInfo->bmiHeader.biWidth;
        }
    }
    else
    {
        ; // very unexpected
    }
}

