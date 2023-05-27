/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#define	MAX_ENTITIES	1023		// can't be increased without changing drawsurf bit packing
#define  MAX_POINTS     32
#define  MAX_SPRITES    1024

#define RDF_FULLBRIGHT		2		// fullbright intensity

typedef struct
{
    float pos[2];
    float size[2];
} letterloc_t;

typedef struct fontheader_s
{
    int               indirection[256];
    letterloc_t       locations[256];
    char              name[64];
    float             height;
    float             aspectRatio;
    void* shader;
    int               trhandle; // the last renderer handle this font used
} fontheader_t;

typedef enum {
    lensflare = (1 << 0),
    viewlensflare = (1 << 1),
    additive = (1 << 2)
} dlighttype_t;
