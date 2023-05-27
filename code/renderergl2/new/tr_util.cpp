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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// tr_util.c -- renderer utility

#include "../tr_local.h"

static const size_t colorSize = (1 << (sizeof(uint16_t) * 8)) - 1;
static uint16_t cntColor[4];
static float cntSt[ 2 ];

/*
===============
RB_StreamBegin
===============
*/
void RB_StreamBegin( shader_t *shader ) {
	RB_BeginSurface( shader, 0, 0 );
}

/*
===============
RB_StreamEnd
===============
*/
void RB_StreamEnd( void ) {
	int i;

	if( tess.numVertexes <= 2 ) {
		RB_EndSurface();
		return;
	}

	tess.indexes[ 0 ] = 0;
	tess.indexes[ 1 ] = 1;
	tess.indexes[ 2 ] = 2;

	for( i = 0; i < tess.numVertexes - 2; i++ ) {
		tess.indexes[ i * 3 + 0 ] = ( i & 1 ) + i;
		tess.indexes[ i * 3 + 1 ] = i - ( ( i & 1 ) - 1 );
		tess.indexes[ i * 3 + 2 ] = i + 2;
		tess.numIndexes += 3;
	}

	RB_EndSurface();
}

/*
===============
RB_StreamBeginDrawSurf
===============
*/
void RB_StreamBeginDrawSurf( void ) {
	backEnd.dsStreamVert = tess.numVertexes;
}

/*
===============
RB_StreamEndDrawSurf
===============
*/
void RB_StreamEndDrawSurf( void ) {
	int		numverts;
	int		i;

	if( tess.numVertexes - backEnd.dsStreamVert <= 2 ) {
		tess.numVertexes = backEnd.dsStreamVert;
		return;
	}

	numverts = tess.numVertexes - backEnd.dsStreamVert - 2;
	for( i = 0; i < numverts; i++ ) {
		tess.indexes[ i + tess.numIndexes ] = ( i & 1 ) + i + backEnd.dsStreamVert;
		tess.indexes[ i + tess.numIndexes + 1 ] = i + backEnd.dsStreamVert - ( ( i & 1 ) - 1 );
		tess.indexes[ i + tess.numIndexes + 2 ] = i + backEnd.dsStreamVert + 2;
		tess.numIndexes += 3;
	}
}

/*
===============
addTriangle
===============
*/
static void addTriangle(void) {
    tess.texCoords[tess.numVertexes][0] = cntSt[0];
    tess.texCoords[tess.numVertexes][1] = cntSt[1];
    tess.color[tess.numVertexes][0] = cntColor[0];
    tess.color[tess.numVertexes][1] = cntColor[1];
    tess.color[tess.numVertexes][2] = cntColor[2];
    tess.color[tess.numVertexes][3] = cntColor[3];
	tess.numVertexes++;
}

/*
===============
RB_Vertex3fv
===============
*/
void RB_Vertex3fv( vec3_t v ) {
	VectorCopy( v, tess.xyz[ tess.numVertexes ] );
	addTriangle();
}

/*
===============
RB_Vertex3f
===============
*/
void RB_Vertex3f( vec_t x, vec_t y, vec_t z ) {
	tess.xyz[ tess.numVertexes ][ 0 ] = x;
	tess.xyz[ tess.numVertexes ][ 1 ] = y;
	tess.xyz[ tess.numVertexes ][ 2 ] = z;
	addTriangle();
}

/*
===============
RB_Vertex2f
===============
*/
void RB_Vertex2f( vec_t x, vec_t y ) {
	RB_Vertex3f( x, y, 0 );
}

/*
===============
RB_Color4f
===============
*/
void RB_Color4f( vec_t r, vec_t g, vec_t b, vec_t a ) {
	cntColor[ 0 ] = r * tr.identityLightByte;
	cntColor[ 1 ] = g * tr.identityLightByte;
	cntColor[ 2 ] = b * tr.identityLightByte;
	cntColor[ 3 ] = a * 255.0;
}

/*
===============
RB_Color3f
===============
*/
void RB_Color3f( vec_t r, vec_t g, vec_t b ) {
	RB_Color4f( r, g, b, 1.0 );
}

/*
===============
RB_Color3fv
===============
*/
void RB_Color3fv( vec3_t col ) {
	RB_Color3f( col[ 0 ], col[ 1 ], col[ 2 ] );
}

/*
===============
RB_Color4bv
===============
*/
void RB_Color4bv( unsigned char *colors ) {
	cntColor[ 0 ] = colors[ 0 ];
	cntColor[ 1 ] = colors[ 1 ];
	cntColor[ 2 ] = colors[ 2 ];
	cntColor[ 3 ] = colors[ 3 ];
}

/*
===============
RB_Texcoord2f
===============
*/
void RB_Texcoord2f( float s, float t ) {
	cntSt[ 0 ] = s;
	cntSt[ 1 ] = t;
}

/*
===============
RB_Texcoord2fv
===============
*/
void RB_Texcoord2fv( vec2_t st ) {
	cntSt[ 0 ] = st[ 0 ];
	cntSt[ 1 ] = st[ 1 ];
}

static int Numbers[ 12 ][ 8 ];
static float Lines[ 13 ][ 4 ];

/*
===============
R_DrawDebugNumber
===============
*/
void R_DrawDebugNumber( const vec3_t org, float number, float scale, float r, float g, float b, int precision ) {
	// FIXME: unimplemented
}

/*
===============
R_DebugRotatedBBox
===============
*/
void R_DebugRotatedBBox( const vec3_t org, vec3_t ang, vec3_t mins, vec3_t maxs, float r, float g, float b, float alpha ) {

}

/*
===============
RE_GetShaderWidth
===============
*/
int RE_GetShaderWidth( qhandle_t hShader ) {
	shader_t *shader;

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	} else {
		shader = tr.defaultShader;
	}

	return shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadWidth;
}

/*
===============
RE_GetShaderHeight
===============
*/
int RE_GetShaderHeight( qhandle_t hShader ) {
	shader_t *shader;

	if( hShader ) {
		shader = R_GetShaderByHandle( hShader );
	}
	else {
		shader = tr.defaultShader;
	}

	return shader->stages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadHeight;
}
