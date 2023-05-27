/*
===========================================================================
Copyright (C) 2010 su44

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
// tr_sprite.c - MoHAA sprite system

#include "tr_local.h"

/*
=====================
SPR_RegisterSprite
=====================
*/
sprite_t *SPR_RegisterSprite(const char *name)
{
	image_t *spriteImage;
	shader_t *shader;
	sprite_t *spr;
	char shadername[256];
	COM_StripExtension(name,shadername,sizeof(shadername));
	shader = R_FindShader(shadername,-1,qfalse,qfalse,qfalse,qfalse);
	if(shader) {
		spriteImage = 0;
		if(shader->unfoggedStages[0])
			spriteImage = shader->unfoggedStages[0]->bundle[0].image[0];
		if ( !spriteImage ) {
			ri.Printf(1, "Could not find image for sprite in shader %s\n", name);
			return 0;
		}
		spr = ri.Hunk_Alloc(sizeof(sprite_t), h_dontcare);
		spr->height = spriteImage->height;
		spr->width = spriteImage->width;
		spr->origin_x = spr->width * 0.5;
		spr->origin_y = spr->height * 0.5;
		spr->shader = shader;
		spr->scale = spr->shader->sprite.scale;
		return spr;
	}
	return 0;
}

/*
=====================
RB_DrawSprite
=====================
*/
void RB_DrawSprite( const refSprite_t *spr ) {
	// FIXME: stub
}
