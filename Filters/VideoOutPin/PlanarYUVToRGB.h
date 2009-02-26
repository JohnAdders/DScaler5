//    VirtualDub - Video processing and capture application
//    Copyright (C) 1998-2001 Avery Lee
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  Notes: 
//  - BitBltFromI420ToRGB is from VirtualDub
//    - The core assembly function of CCpuID is from DVD2AVI
//    (- vd.cpp/h should be renamed to something more sensible already :)

#pragma once

extern bool BitBltFromI420ToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch);
extern bool BitBltFromI422ToRGB(int w, int h, BYTE* dst, int dstpitch, int dbpp, BYTE* srcy, BYTE* srcu, BYTE* srcv, int srcpitch);

