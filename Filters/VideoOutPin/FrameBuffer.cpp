///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 John Adcock
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
// Revision 1.1  2004/11/05 17:45:53  adcockj
// Added new decoder
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FrameBuffer.h"

CFrameBuffer::CFrameBuffer()
{
    m_Buf[0] = NULL;
    m_Buf[1] = NULL;
    m_Buf[2] = NULL;
    m_ActualBuf = NULL;
    m_AllocatedSize = 0;
    Clear();
}

CFrameBuffer::~CFrameBuffer()
{
    FreeMem();
}

HRESULT CFrameBuffer::AllocMem(int YSize, int UVSize)
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

void CFrameBuffer::FreeMem()
{
    m_Buf[0] = NULL;
    m_Buf[1] = NULL;
    m_Buf[2] = NULL;
    free(m_ActualBuf);
    m_ActualBuf = NULL;
    m_CurrentSize = 0;
    m_AllocatedSize = 0;
}

void CFrameBuffer::Clear()
{
    m_rtStart = 0;
    m_rtStop = 0;
    m_Flags = 0;
    m_NumFields = 0;
    m_UseCount = 0;
}


CFrameBuffer& CFrameBuffer::operator=(CFrameBuffer& RHS)
{
    memcpy(m_Buf[0], RHS.m_Buf[0], RHS.m_CurrentSize);
    m_rtStart = RHS.m_rtStart;
    m_rtStop = RHS.m_rtStop;
    m_Flags = RHS.m_Flags;
    m_NumFields = RHS.m_NumFields;
    m_CurrentSize = RHS.m_CurrentSize;
    m_UseCount = RHS.m_UseCount;
    return *this;
}