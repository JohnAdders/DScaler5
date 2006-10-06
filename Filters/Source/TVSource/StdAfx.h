///////////////////////////////////////////////////////////////////////////////
// $Id: StdAfx.h,v 1.1 2006-10-06 17:00:35 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2004 John Adcock
///////////////////////////////////////////////////////////////////////////////
//  This file is subject to the terms of the GNU General Public License as
//  published by the Free Software Foundation.  A copy of this license is
//  included with this software distribution in the file COPYING.  If you
//  do not have a copy, you may obtain a copy by writing to the Free
//  Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details
// 
// You should have received a copy of the GNU General Public
// License along with this package; if not, write to the Free Software
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
#include "Mpeg2data.h"

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
