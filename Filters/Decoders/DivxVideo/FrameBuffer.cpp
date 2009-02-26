///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// DivxVideo.dll - DirectShow filter for deinterlacing and video processing
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

#include "stdafx.h"
#include "DivxDecoder.h"

CDivxDecoder::CFrameBuffer::CFrameBuffer()
{
    Clear();
}

CDivxDecoder::CFrameBuffer::~CFrameBuffer()
{
}


void CDivxDecoder::CFrameBuffer::Clear()
{
    m_rtStartCoded = 0;
    m_rtStartDisplay = 0;
    m_UseCount = 0;
}
