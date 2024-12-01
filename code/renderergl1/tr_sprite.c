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

static int CullSprite(const vec3_t* points) {
	int i, j;

	for (i = 0; i < backEnd.viewParms.fog.extrafrustums + 4; i++) {
		for (j = 0; j < 4; j++) {
			if (DotProduct(points[j], backEnd.viewParms.frustum[i].normal) - backEnd.viewParms.frustum[i].dist > 0.0f) {
				break;
			}
		}

		if (j == 4) {
			return CULL_CLIP;
		}
	}

	return CULL_IN;
}

/*
=====================
RB_DrawSprite
=====================
*/
void RB_DrawSprite( const refSprite_t *spr ) {
	model_t* model;
	vec3_t norm;
	vec3_t up, right;
	vec3_t points[4];
	float org_x, org_y;
	float scale;

	if (!spr->hModel)
	{
        ri.Printf(PRINT_WARNING, "No model found for Sprite\n");
        return;
	}

    model = tr.models;
	if (spr->hModel > 0 && spr->hModel < tr.numModels) {
		model = &tr.models[spr->hModel];
	}

	scale = spr->scale * model->d.sprite->scale;

	switch (model->d.sprite->shader->sprite.type)
    {
    case SPRITE_PARALLEL_ORIENTED:
		{
			float invmag;
			float cr, sr;
			int i;

			invmag = 1.0 / sqrt(spr->axis[1][2] * spr->axis[1][2] + spr->axis[1][1] * spr->axis[1][1]);

			cr = invmag * spr->axis[1][1];
			sr = invmag * spr->axis[1][2];

			for (i = 0; i < 3; i++)
			{
				up[i] = backEnd.viewParms.ori.axis[2][i] * cr - backEnd.viewParms.ori.axis[1][i] * sr;
				right[i] = backEnd.viewParms.ori.axis[1][i] * cr + backEnd.viewParms.ori.axis[2][i] * sr;
			}
		}
        break;
    case SPRITE_PARALLEL:
		VectorCopy(backEnd.viewParms.ori.axis[2], up);
		if (!backEnd.viewParms.isMirror) {
			VectorNegate(backEnd.viewParms.ori.axis[1], right);
		} else {
			VectorCopy(backEnd.viewParms.ori.axis[1], right);
		}
        break;
    case SPRITE_ORIENTED:
		VectorCopy(spr->axis[1], right);
		VectorCopy(spr->axis[2], up);
        break;
    case SPRITE_PARALLEL_UPRIGHT:
		VectorCopy(backEnd.viewParms.ori.axis[0], norm);
		VectorCopy(backEnd.viewParms.ori.axis[1], right);
		VectorCopy(backEnd.viewParms.ori.axis[2], up);
		if (backEnd.viewParms.ori.axis[0][2] > 0.999) {
			return;
		} else if (backEnd.viewParms.ori.axis[0][2] < -0.999) {
			return;
		}

		VectorSet(up, 0.0f, 0.0f, 1.0f);
		VectorSet(right, backEnd.viewParms.ori.axis[0][1], -backEnd.viewParms.ori.axis[0][0], 0.0f);
		VectorNormalize(right);
		VectorSet(norm, -right[1], right[0], 0.0f);
		break;
	}

	org_x = model->d.sprite->origin_x * scale;
	org_y = model->d.sprite->origin_y * scale;

    VectorScale(up, org_y, up);
    VectorScale(right, org_x, right);

	points[0][0] = spr->origin[0] + up[0] - right[0];
	points[1][0] = spr->origin[0] + up[0] + right[0];
	points[2][0] = spr->origin[0] - up[0] - right[0];
	points[3][0] = spr->origin[0] - up[0] + right[0];

    points[0][1] = spr->origin[1] + up[1] - right[1];
    points[1][1] = spr->origin[1] + up[1] + right[1];
    points[2][1] = spr->origin[1] - up[1] - right[1];
    points[3][1] = spr->origin[1] - up[1] + right[1];

    points[0][2] = spr->origin[2] + up[2] - right[2];
    points[1][2] = spr->origin[2] + up[2] + right[2];
    points[2][2] = spr->origin[2] - up[2] - right[2];
    points[3][2] = spr->origin[2] - up[2] + right[2];

	if (CullSprite(points) == CULL_OUT) {
		return;
	}

	RB_CHECKOVERFLOW(4, 6);

    memcpy(tess.vertexColors[tess.numVertexes], spr->shaderRGBA, sizeof(byte) * 4);
    memcpy(tess.vertexColors[tess.numVertexes + 1], spr->shaderRGBA, sizeof(byte) * 4);
    memcpy(tess.vertexColors[tess.numVertexes + 2], spr->shaderRGBA, sizeof(byte) * 4);
    memcpy(tess.vertexColors[tess.numVertexes + 3], spr->shaderRGBA, sizeof(byte) * 4);

    tess.vertexColorValid = qtrue;
    tess.texCoords[tess.numVertexes][0][0] = 0.0f;
    tess.texCoords[tess.numVertexes][0][1] = 0.0f;
    tess.texCoords[tess.numVertexes + 1][0][0] = 1.0f;
    tess.texCoords[tess.numVertexes + 1][0][1] = 0.0f;
    tess.texCoords[tess.numVertexes + 2][0][0] = 0.0f;
    tess.texCoords[tess.numVertexes + 2][0][1] = 1.0f;
    tess.texCoords[tess.numVertexes + 3][0][0] = 1.0f;
    tess.texCoords[tess.numVertexes + 3][0][1] = 1.0f;

    tess.xyz[tess.numVertexes][0] = points[0][0];
    tess.xyz[tess.numVertexes][1] = points[0][1];
    tess.xyz[tess.numVertexes][2] = points[0][2];
    tess.xyz[tess.numVertexes + 1][0] = points[1][0];
    tess.xyz[tess.numVertexes + 1][1] = points[1][1];
    tess.xyz[tess.numVertexes + 1][2] = points[1][2];
    tess.xyz[tess.numVertexes + 2][0] = points[2][0];
    tess.xyz[tess.numVertexes + 2][1] = points[2][1];
    tess.xyz[tess.numVertexes + 2][2] = points[2][2];
    tess.xyz[tess.numVertexes + 3][0] = points[3][0];
    tess.xyz[tess.numVertexes + 3][1] = points[3][1];
    tess.xyz[tess.numVertexes + 3][2] = points[3][2];

    tess.indexes[tess.numIndexes] = tess.numVertexes + 2;
    tess.indexes[tess.numIndexes + 1] = tess.numVertexes + 1;
    tess.indexes[tess.numIndexes + 2] = tess.numVertexes;
    tess.indexes[tess.numIndexes + 3] = tess.numVertexes + 2;
    tess.indexes[tess.numIndexes + 4] = tess.numVertexes + 3;
    tess.indexes[tess.numIndexes + 5] = tess.numVertexes + 1;

    tess.numVertexes += 4;
    tess.numIndexes += 6;
}
