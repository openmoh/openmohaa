/***************************************************************************
* Copyright (C) 2012, Chain Studios.
* 
* This file is part of QeffectsGL source code.
* 
* QeffectsGL source code is free software; you can redistribute it 
* and/or modify it under the terms of the GNU General Public License 
* as published by the Free Software Foundation; either version 2 of 
* the License, or (at your option) any later version.
* 
* QeffectsGL source code is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied 
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
***************************************************************************/
#ifndef QFX_LIBRARY_H
#define QFX_LIBRARY_H

#include <cassert>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <cmath>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define QFX_LIBRARY_TITLE				"QeffectsGL"
#define QFX_LIBRARY_VERSION				"1.2"
#define QFX_LIBRARY_VENDOR				"Chain Studios (" QFX_LIBRARY_TITLE " " QFX_LIBRARY_VERSION ")"

#define QFX_SINGLETON

// make sure these identifiers do not conflict with anything
#define QFX_TEXTURE_IDENTIFIER_BASE		0xFFAA

#endif //QFX_LIBRARY_H