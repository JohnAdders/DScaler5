///////////////////////////////////////////////////////////////////////////////
// $Id: CSSauth.h,v 1.1 2005-02-08 15:32:34 adcockj Exp $
///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2005 Gabest
///////////////////////////////////////////////////////////////////////////////
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
///////////////////////////////////////////////////////////////////////////////
#pragma once

extern void CSSkey1(int varient, byte const *challenge, byte *key);
extern void CSSkey2(int varient, byte const *challenge, byte *key);
extern void CSSbuskey(int varient, byte const *challenge, byte *key);
