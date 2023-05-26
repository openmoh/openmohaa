/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
/*
** QGL.H
*/

#pragma once

//===========================================================================

/*
** multitexture extension definitions
*/
#define GL_ACTIVE_TEXTURE_ARB               0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB        0x84E1
#define GL_MAX_ACTIVE_TEXTURES_ARB          0x84E2

#define GL_TEXTURE0_ARB                     0x84C0
#define GL_TEXTURE1_ARB                     0x84C1
#define GL_TEXTURE2_ARB                     0x84C2
#define GL_TEXTURE3_ARB                     0x84C3

#define GL_COMBINE                          0x8570
#define GL_COMBINE4_NV                      0x8503
#define GL_COMBINE_RGB                      0x8571
#define GL_COMBINE_ALPHA                    0x8572

#define GL_SOURCE0_RGB                      0x8580
#define GL_SOURCE1_RGB                      0x8581
#define GL_SOURCE2_RGB                      0x8582
#define GL_SOURCE3_RGB                      0x8583
#define GL_SOURCE0_ALPHA                    0x8588
#define GL_SOURCE1_ALPHA                    0x8589
#define GL_SOURCE2_ALPHA                    0x858A
#define GL_SOURCE3_ALPHA                    0x858B
#define GL_OPERAND0_RGB                     0x8590
#define GL_OPERAND1_RGB                     0x8591
#define GL_OPERAND2_RGB                     0x8592
#define GL_OPERAND3_RGB                     0x8593
#define GL_OPERAND0_ALPHA                   0x8598
#define GL_OPERAND1_ALPHA                   0x8599
#define GL_OPERAND2_ALPHA                   0x859A
#define GL_OPERAND3_ALPHA                   0x859B
