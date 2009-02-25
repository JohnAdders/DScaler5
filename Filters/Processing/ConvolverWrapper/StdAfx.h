///////////////////////////////////////////////////////////////////////////////
// $Id$
///////////////////////////////////////////////////////////////////////////////
// ConvolverWrapper.dll - DirectShow filter for detecting audio type in PCM streams
// Copyright (c) 2004 John Adcock
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

#define _CRT_SECURE_NO_DEPRECATE

#define STRICT
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <windows.h>
#include <windowsx.h>

#include <strmif.h>
#include <vfwmsgs.h>
#include <uuids.h>
#include <d3d9.h>
#include <vmr9.h>
#include <amvideo.h>
#include <amstream.h>
#include <dvdmedia.h>
#include <medparam.h>
#include <Dmo.h>

#include "Utils.h"
#include "GenDMOProp.h"

#include <stdio.h>
#include <stdarg.h>

#include <list>
#include <vector>
#include <string>

#define DLLSVC
#define _INLINEISEQUALGUID_DEFINED
#include "yacl\include\combook.h"
