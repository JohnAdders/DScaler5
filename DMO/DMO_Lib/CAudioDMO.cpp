////////////////////////////////////////////////////////////////////////////
// $Id$
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
// Revision 1.2  2004/02/06 12:17:15  adcockj
// Major changes to the Libraries to remove ATL and replace with YACL
// First draft of Mpeg2 video decoder filter
// Broken DScalerFilter part converted to new library
//
// Revision 1.1  2003/05/16 16:19:12  adcockj
// Added new files into DMO framework
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "params.h"
#include "CAudioDMO.h"
#include "DMO_Lib.h"
#include <uuids.h>


/////////////////////////////////////////////////////////////////////////////
// CAudioDMO
CAudioDMO::CAudioDMO(LPCWSTR Name) :
	CDMO(Name)
{
};

CAudioDMO::~CAudioDMO()
{
};


/////////////////////////
//
//  IMediaObjectImpl::InternalFlush
//
//  *** Called by Flush, description below ***
//
//  The Flush method flushes all internally buffered data.
//
// Return Value:
// Returns S_OK if successful. Otherwise, returns an HRESULT value indicating
// the cause of the error.
//
//  The DMO performs the following actions when this method is called:
//  *  Releases any IMediaBuffer references it holds.
//
//  *  Discards any values that specify the time stamp or sample length for a
//     media buffer.
//
//  *  Reinitializes any internal states that depend on the contents of a
//     media sample.
//
//  Media types, maximum latency, and locked state do not change.
//
//  When the method returns, every input stream accepts data. Output streams
//  cannot produce any data until the application calls the ProcessInput method
//  on at least one input stream.
//
//  Note:
//
//  The template keeps a private flag that indicates the object's flushed
//  state. The Flush method sets the flag to TRUE, and the ProcessInput method
//  resets it to FALSE. If Flush is called when the flag is already TRUE, the
//  method returns S_OK without calling the InternalFlush method.
//
HRESULT CAudioDMO::InternalFlush(void)
{
    // Just clear out the buffers
	m_Buffer.Detach();
	return S_OK;
}

////////////////////////////////////
//
//  IMediaObjectImpl::InternalGetInputSizeInfo
//
//  *** Called by GetInputSizeInfo, description below ***
//
//  The GetInputSizeInfo method retrieves the buffer requirements for a
//  specified input stream.
//
//  Parameters
//
//  dwInputStreamIndex:     Zero-based index of an input stream on the DMO.
//
//  pcbSize:                [out] Pointer to a variable that receives
//      the minimum size of an input buffer for this stream, in bytes.
//
//  pulSizeMaxLookahead:        [out] Pointer to a variable that receives the
//      maximum amount of data that the DMO will hold for lookahead, in bytes.
//      If the DMO does not perform lookahead on the stream, the value is zero.
//
//  pulSizeAlignment            [out] Pointer to a variable that receives the
//      required buffer alignment, in bytes. If the input stream has no
//      alignment requirement, the value is 1.
//
//  Return Value
//      S_OK Success
//      DMO_E_INVALIDSTREAMINDEX Invalid stream index
//      DMO_E_TYPE_NOT_SET Media type was not set
//
//  The buffer requirements may depend on the media types of the various
//  streams. Before calling this method, set the media type of each stream
//  by calling the SetInputType and SetOutputType methods. If the media types
//  have not been set, this method might return an error.
//
//  If the DMO performs lookahead on the input stream, it returns the
//  DMO_INPUT_STREAMF_HOLDS_BUFFERS flag in the GetInputStreamInfo method.
//  During processing, the DMO holds up to the number of bytes indicated by the
//  pulSizeMaxLookahead parameter. The application must allocate enough buffers for
//  the DMO to hold this much data.
//
//  A buffer is aligned if the buffer's start address is a multiple of
//  *pulSizeAlignment. The alignment must be a power of two. Depending on the
//  microprocessor, reads and writes to an aligned buffer might be faster than
//  to an unaligned buffer. Also, some microprocessors do not support unaligned
//  reads and writes.
//
//  Note:
//
//  GetInputSizeInfo returns DMO_E_TYPE_NOT_SET unless all of the non-optional
//  streams have media types. Therefore, in the derived class, the internal
//  methods can assume that all of the non-optional streams have media types.
//
HRESULT CAudioDMO::InternalGetInputSizeInfo(DWORD dwInputStreamIndex, DWORD *pcbSize, DWORD *pulSizeMaxLookahead, DWORD *pulSizeAlignment)
{
	// We don't have to do any validation, because it is all done in the base class

	HRESULT hr = S_OK;
	const DMO_MEDIA_TYPE* pmt;
	pmt = InputType(0);
    
    if(pmt->majortype == MEDIATYPE_Audio)
    {
	    const WAVEFORMATEX* pwfx = reinterpret_cast<const WAVEFORMATEX*>(pmt->pbFormat);
	    *pcbSize = pwfx->nBlockAlign;
	    *pulSizeMaxLookahead = 0;	// no look ahead
	    *pulSizeAlignment = 1;		// no alignment requirement
    }
    else
    {
        // what's going on
        hr = E_FAIL;
    }

	return hr;
}


//////////////////////////////////////
//
//  IMediaObjectImpl::InternalGetOutputSizeInfo
//
//  *** Called by GetOutputSizeInfo, description below ***
//
//  The GetOutputSizeInfo method retrieves the buffer requirements for a
//  specified output stream.
//
//  Parameters
//
//      dwOutputStreamIndex
//          Zero-based index of an output stream on the DMO.
//
//      pcbSize
//          [out] Pointer to a variable that receives the minimum size of an
//          output buffer for this stream, in bytes.
//
//      pulSizeAlignment
//          [out] Pointer to a variable that receives the required buffer
//          alignment, in bytes. If the output stream has no alignment
//          requirement, the value is 1.
//
//  Return Value
//      S_OK Success
//      DMO_E_INVALIDSTREAMINDEX Invalid stream index
//      DMO_E_TYPE_NOT_SET Media type was not set
//
//  The buffer requirements may depend on the media types set for each of the
//  streams.
//
//  Before calling this method, set the media type of each stream by calling
//  the SetInputType and SetOutputType methods. If the media types have not
//  been set, this method might return an error. However, if a stream is
//  optional, and the application will not use the stream, you do not have to
//  set the media type for the stream.
//
//  A buffer is aligned if the buffer's start address is a multiple of
//  *pulSizeAlignment. Depending on the architecture of the microprocessor, it is
//  faster to read and write to an aligned buffer than to an unaligned buffer.
//  On some microprocessors, reading and writing to an unaligned buffer is not
//  supported and can cause the program to crash. Zero is not a valid alignment.
//
//  Note:
//
//  GetOutputSizeInfo returns DMO_E_TYPE_NOT_SET unless all of the non-optional
//  streams have media types. Therefore, in the derived class, the internal
//  methods can assume that all of the non-optional streams have media types.
//
HRESULT CAudioDMO::InternalGetOutputSizeInfo(DWORD dwOutputStreamIndex, DWORD *pcbSize, DWORD *pulSizeAlignment)
{
    // We don't have to do any validation, because it is all done in the base class
	HRESULT hr = S_OK;
	const DMO_MEDIA_TYPE* pmt;
	pmt = OutputType(0);
    if(pmt->majortype == MEDIATYPE_Audio)
    {
	    const WAVEFORMATEX* pwfx = reinterpret_cast<const WAVEFORMATEX*>(pmt->pbFormat);
	    *pcbSize = pwfx->nBlockAlign;
	    *pulSizeAlignment = 1;
    }
    else
    {
        // what's going on
        hr = E_FAIL;
    }

    return hr;
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
HRESULT CAudioDMO::InternalProcessInput(DWORD dwInputStreamIndex, IMediaBuffer *pBuffer, DWORD dwFlags, REFERENCE_TIME rtTimestamp, REFERENCE_TIME rtTimelength)
{
    HRESULT hr = S_OK;

    if (!pBuffer)
    {
        return E_POINTER;
    }
        
    // for audio we process one buffer at a time
    // this can change if required
    if(m_Buffer == NULL)
    {
        // Get the size of the input buffer
        hr = pBuffer->GetBufferAndLength(&m_pbInputData, &m_cbInputLength);
        if (SUCCEEDED(hr))
        {
            m_Buffer = pBuffer;
        }
        else
        {
            return hr;
        }
    }
    else
    {
        return DMO_E_NOTACCEPTING;
    }

    if (dwFlags & DMO_INPUT_DATA_BUFFERF_TIME)
    {
        m_bValidTime = true;
        m_rtTimestamp = rtTimestamp;
        m_rtTimestamp = rtTimelength;
    }
    else
    {
        m_bValidTime = false;
    }

    if (dwFlags & DMO_INPUT_DATA_BUFFERF_TIMELENGTH)
    {
        m_bValidLength = true;
        m_rtTimelength = rtTimelength;
    }
    else
    {
        m_bValidLength = false;
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
HRESULT CAudioDMO::InternalProcessOutput(DWORD dwFlags, DWORD cOutputBufferCount, DMO_OUTPUT_DATA_BUFFER *pOutputBuffers, DWORD *pdwStatus)
{
    HRESULT         hr = S_OK;
    BYTE            *pbData = NULL;
    DWORD           cbData = 0;
    DWORD           cbOutputLength = 0;
    DWORD           cbBytesProcessed = 0;
    bool            bComplete = false;
    const DWORD     UNITS = 10000000;  // 1 sec = 100 * UNITS ns

    SI(IMediaBuffer) pOutputBuffer = pOutputBuffers[0].pBuffer;

    if (!m_Buffer || !pOutputBuffer)
    {
        return S_FALSE;  // Did not produce output
    }

    // Get the size of the output buffer
    hr = pOutputBuffer->GetBufferAndLength(&pbData, &cbData);

    if (SUCCEEDED(hr))
    {
        hr = pOutputBuffer->GetMaxLength(&cbOutputLength);
    }

    if (SUCCEEDED(hr))
    {
        // Skip past any valid data in the output buffer
        pbData += cbData;
        cbOutputLength -= cbData;

        // Calculate how many quanta we can process
        if (m_cbInputLength > cbOutputLength)
        {
            cbBytesProcessed = cbOutputLength;
        }
        else
        {
            cbBytesProcessed = m_cbInputLength;
            bComplete = true;
        }

        // Process the data
        hr = DoProcess(pbData, m_pbInputData, cbBytesProcessed / m_WaveFormat.Format.nBlockAlign);
    }

    if (SUCCEEDED(hr))
    {
        hr = pOutputBuffer->SetLength(cbBytesProcessed + cbData);
    }

    if (SUCCEEDED(hr))
    {
        if (m_bValidTime)
        {
            pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_TIME;
            pOutputBuffers[0].rtTimestamp = m_rtTimestamp;

            // Estimate how far along we are
            pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_TIMELENGTH;
            double dTime = (double)(cbBytesProcessed) / m_WaveFormat.Format.nAvgBytesPerSec;
            pOutputBuffers[0].rtTimelength = (REFERENCE_TIME)(dTime * UNITS);
        }

        if (bComplete)
        {
            m_Buffer.Detach();   // Release input buffer
	    }
        else
        {
            pOutputBuffers[0].dwStatus |= DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE;
            m_cbInputLength -= cbBytesProcessed;
            m_pbInputData += cbBytesProcessed;
            m_rtTimestamp += pOutputBuffers[0].rtTimelength;
        }
    }
    return hr;
}

////////////////////////////////////////
//
//  IMediaObjectImpl::InternalAcceptingInput
//
//  Queries whether an input stream can accept more input. The derived class
//  must declare and implement this method.
//
//  Parameters
//
//      dwInputStreamIndex
//          Index of an input stream.
//
//  Return Value
//
//      Returns S_OK if the input stream can accept input, or S_FALSE otherwise.
//
//  Note:
//
//  Called by IMediaObject::GetInputStatus
//
HRESULT CAudioDMO::InternalAcceptingInput(DWORD dwInputStreamIndex)
{
    // Do not accept input if there is already enough input data to process
    return (m_Buffer != NULL ? S_FALSE : S_OK);
}

HRESULT CAudioDMO::InternalFreeStreamingResources(void)
{
	return S_OK;
}
