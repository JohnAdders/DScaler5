/*
 *    Copyright (C) 2003 Gabest
 *    http://www.gabest.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#pragma once

extern IBaseFilter* GetFilterFromPin(IPin* pPin);
extern CLSID GetCLSID(IBaseFilter* pBF);
extern CLSID GetCLSID(IPin* pPin);
BITMAPINFOHEADER* ExtractBIH(const AM_MEDIA_TYPE* pmt);
extern bool ExtractDim(const AM_MEDIA_TYPE* pmt, int& w, int& h, long& arx, long& ary, int& pitch);

extern void Simplify(long& u, long& v);
extern void Simplify(unsigned long& u, unsigned long& v);

