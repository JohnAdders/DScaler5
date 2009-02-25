///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
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

#pragma once 

/////////////////////////////////////////////////////////////////////////////
// CFrameBuffer
class CFrameBuffer
{
public:
    CFrameBuffer();
    ~CFrameBuffer();
    HRESULT AllocMem(int YSize, int UVSize);
    void FreeMem();
    void Clear();
    BYTE* m_Buf[3];
    REFERENCE_TIME m_rtStart;
    REFERENCE_TIME m_rtStop;
    DWORD m_Flags;
    unsigned int m_NumFields;
    int m_CurrentSize;
    CFrameBuffer& operator=(CFrameBuffer& RHS);
    void AddRef() {m_UseCount++;};
    void Release() {m_UseCount--;};
    bool NotInUse() {return (m_UseCount <= 0);};
private:
    void* m_ActualBuf;
    int m_AllocatedSize;
    int m_UseCount;
};
