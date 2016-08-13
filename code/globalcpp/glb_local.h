/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

This file is part of OpenMoHAA source code.

OpenMoHAA source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMoHAA source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMoHAA source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef __GLB_LOCAL_H__
#define __GLB_LOCAL_H__

#if defined( GAME_DLL )

#define glbs gi

#include "g_local.h"
#include <script.h>
#include "qcommon.h"

#else

#if defined( CGAME_DLL )

#define glbs cgi

#else

#define glbs bi

#endif

#include "../qcommon/q_shared.h"

#include <linklist.h>
#include <mem_blockalloc.h>
#include <vector.h>
#include <str.h>
#include <container.h>
#include <const_str.h>
#include <short3.h>
#include <con_set.h>
#include <con_arrayset.h>
#include <scriptexception.h>
#include <class.h>
#include <containerclass.h>
#include <stack.h>
#include <listener.h>
#include <script.h>

#include "../qcommon/qcommon.h"

#if defined ( CGAME_DLL )

#if defined ( CGAME_HOOK )

#include "../cgame_hook/cgamex86.h"

#endif

#else

#define glbs bi

#include "baseimp.h"

#endif

#endif

#endif /* glb_local.h */
