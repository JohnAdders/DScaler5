///////////////////////////////////////////////////////////////////////////////
// $Id: FrameBuffer.cpp,v 1.2 2004-02-29 19:05:44 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// MpegVideo.dll - DirectShow filter for deinterlacing and video processing
// Copyright (c) 2003 John Adcock
///////////////////////////////////////////////////////////////////////////////
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
///////////////////////////////////////////////////////////////////////////////
// CVS Log
//
// $Log: not supported by cvs2svn $
// Revision 1.1  2004/02/25 17:17:25  adcockj
// New class to support buffer management
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MpegDecoder.h"

CMpegDecoder::CFrameBuffer::CFrameBuffer()
{
    m_Buf[0] = NULL;
    m_Buf[1] = NULL;
    m_Buf[2] = NULL;
    m_ActualBuf = NULL;
    m_AllocatedSize = 0;
    Clear();
}

CMpegDecoder::CFrameBuffer::~CFrameBuffer()
{
    FreeMem();
}

HRESULT CMpegDecoder::CFrameBuffer::AllocMem(int YSize, int UVSize)
{
    if(m_AllocatedSize >= YSize + 2 * UVSize)
    {
        m_CurrentSize = YSize + 2 * UVSize;
    }
    else
    {
        FreeMem();
        m_ActualBuf = malloc(YSize + 2 * UVSize + 15);
        if(m_ActualBuf == NULL)
        {
            m_AllocatedSize = 0;
            return E_OUTOFMEMORY;
        }
        m_CurrentSize = YSize + 2 * UVSize;
        m_AllocatedSize = YSize + 2 * UVSize + 15;
    }

    // force alignment to 16 byte boundaries
    m_Buf[0] = (BYTE*)(((DWORD)(m_ActualBuf) + 15) & ~15);
    m_Buf[1] = (BYTE*)(((DWORD)(m_ActualBuf) + YSize + 15) & ~15);
    m_Buf[2] = (BYTE*)(((DWORD)(m_ActualBuf) + YSize + UVSize + 15) & ~15);

	Clear();

    return S_OK;
}

void CMpegDecoder::CFrameBuffer::FreeMem()
{
    m_Buf[0] = NULL;
    m_Buf[1] = NULL;
    m_Buf[2] = NULL;
    free(m_ActualBuf);
    m_ActualBuf = NULL;
    m_CurrentSize = 0;
    m_AllocatedSize = 0;
}

void CMpegDecoder::CFrameBuffer::Clear()
{
    m_rtStart = 0;
    m_rtStop = 0;
    m_Flags = 0;
    m_NumFields = 0;
    m_UseCount = 0;
}


CMpegDecoder::CFrameBuffer& CMpegDecoder::CFrameBuffer::operator=(CFrameBuffer& RHS)
{
    ASSERT(RHS.m_CurrentSize <= m_AllocatedSize);

    memcpy(m_Buf[0], RHS.m_Buf[0], RHS.m_CurrentSize);
    m_rtStart = RHS.m_rtStart;
    m_rtStop = RHS.m_rtStop;
    m_Flags = RHS.m_Flags;
    m_NumFields = RHS.m_NumFields;
    m_CurrentSize = RHS.m_CurrentSize;
    m_UseCount = RHS.m_UseCount;
    return *this;
}