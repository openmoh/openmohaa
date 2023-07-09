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

#include "tr_local.h"
#include "../qcommon/str.h"

static byte cntColor[4];
static float cntSt[ 2 ];

static int Numbers[12][8] = {
    { 1, 3, 4, 5, 6, 7, 0, 0 },
    { 4, 5, 0, 0, 0, 0, 0, 0 },
    { 1, 4, 2, 7, 3, 0, 0, 0 },
    { 1, 4, 2, 5, 3, 0, 0, 0 },
    { 6, 4, 2, 5, 0, 0, 0, 0 },
    { 1, 6, 2, 5, 3, 0, 0, 0 },
    { 1, 6, 2, 5, 7, 3, 0, 0 },
    { 1, 8, 0, 0, 0, 0, 0, 0 },
    { 1, 2, 3, 4, 5, 6, 7, 0 },
    { 1, 6, 4, 2, 5, 3, 0, 0 },
    { 9, 10, 11, 12, 0, 0, 0, 0 },
    { 2, 0, 0, 0, 0, 0, 0, 0 }
};

static float Lines[13][4] = {
    { 0.0, 0.0, 0.0, 0.0 },
    { -4.0, 8.0, 4.0, 8.0 },
    { -4.0, 4.0, 4.0, 4.0 },
    { -4.0, 0.0, 4.0, 0.0 },
    { 4.0, 8.0, 4.0, 4.0 },
    { 4.0, 4.0, 4.0, 0.0 },
    { -4.0, 8.0, -4.0, 4.0 },
    { -4.0, 4.0, -4.0, 0.0 },
    { 4.0, 8.0, -4.0, 0.0 },
    { -1.0, 2.0, 1.0, 2.0 },
    { 1.0, 2.0, 1.0, 0.0 },
    { -1.0, 0.0, 1.0, 0.0 },
    { -1.0, 0.0, -1.0, 2.0 }
};

/*
===============
RB_StreamBegin
===============
*/
void RB_StreamBegin( shader_t *shader ) {
	RB_BeginSurface( shader );
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
static void addTriangle( void ) {
	tess.texCoords[ tess.numVertexes ][ 0 ][ 0 ] = cntSt[ 0 ];
	tess.texCoords[ tess.numVertexes ][ 0 ][ 1 ] = cntSt[ 1 ];
	tess.vertexColors[ tess.numVertexes ][ 0 ] = cntColor[ 0 ];
	tess.vertexColors[ tess.numVertexes ][ 1 ] = cntColor[ 1 ];
	tess.vertexColors[ tess.numVertexes ][ 2 ] = cntColor[ 2 ];
	tess.vertexColors[ tess.numVertexes ][ 3 ] = cntColor[ 3 ];
	tess.vertexColorValid = qtrue;
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

/*
===============
R_DrawDebugNumber
===============
*/
void R_DrawDebugNumber( const vec3_t org, float number, float scale, float r, float g, float b, int precision ) {
	vec3_t up, right;
	vec3_t pos1, pos2;
	vec3_t start;
	str text;
	char format[20];
	int i;
	int j;
	int l;
	int num;
	
	VectorCopy(tr.viewParms.ori.axis[2], up);
	VectorCopy(tr.viewParms.ori.axis[1], right);
	VectorNegate(right, right);

	VectorNormalize(up);
	VectorNormalize(right);

	VectorScale(up, scale, up);
	VectorScale(right, scale, right);

    if (precision > 0) {
		sprintf(format, "%%.%df", precision);
        text = va(format, precision);
	} else {
		text = va("%d", number);
	}

	l = 5 - 5 * text.length();
	start[0] = org[0] + l * right[0];
	start[1] = org[1] + l * right[1];
	start[2] = org[2] + l * right[2];

	for (i = 0; i < l; i++) {
		if (text[i] == '.') {
			num = 10;
		}
		else if (text[i] == '-') {
			num = 11;
		}
		else {
			num = text[i] - '0';
		}

		l = 32 * num;
		num = Numbers[num][0];

		for (j = 0; j < 8 && num; j++) {
            pos1[0] = start[0] + Lines[num][0] * right[0] + Lines[num][1] * up[0];
            pos1[1] = start[1] + Lines[num][0] * right[1] + Lines[num][1] * up[1];
            pos1[2] = start[2] + Lines[num][0] * right[2] + Lines[num][1] * up[2];

			pos2[0] = start[0] + Lines[num][2] * right[0] + Lines[num][3] * up[0];
			pos2[1] = start[1] + Lines[num][2] * right[1] + Lines[num][3] * up[1];
			pos2[2] = start[2] + Lines[num][2] * right[2] + Lines[num][3] * up[2];

			R_DebugLine(pos1, pos2, r, g, b, 1.0);

			l += 4;
			num = Numbers[l / 8][l % 8];
        }

        start[0] += right[0] * 10.0;
        start[1] += right[1] * 10.0;
        start[2] += right[2] * 10.0;
	}
}

/*
===============
R_DebugRotatedBBox
===============
*/
void R_DebugRotatedBBox( const vec3_t org, const vec3_t ang, const vec3_t mins, const vec3_t maxs, float r, float g, float b, float alpha ) {
	int i;
	vec3_t tmp;
	vec3_t points[8];
	vec3_t axis[3];

	AnglesToAxis(ang, axis);

	for (i = 0; i < 8; i++) {
		if (i & 1) {
			tmp[0] = mins[0];
		} else {
			tmp[0] = maxs[0];
        }

        if (i & 2) {
            tmp[1] = mins[1];
        }
        else {
            tmp[1] = maxs[1];
        }

		if (i & 4) {
			tmp[2] = mins[2];
		} else {
			tmp[2] = maxs[2];
		}

		points[i][0] = tmp[0] * axis[0][0];
		points[i][1] = tmp[0] * axis[0][1];
        points[i][2] = tmp[0] * axis[0][2];

        points[i][0] += tmp[1] * axis[1][0];
        points[i][1] += tmp[1] * axis[1][1];
        points[i][2] += tmp[1] * axis[1][2];

        points[i][0] += tmp[2] * axis[2][0];
        points[i][1] += tmp[2] * axis[2][1];
        points[i][2] += tmp[2] * axis[2][2];

		points[i][0] += org[0];
		points[i][1] += org[1];
		points[i][2] += org[2];
	}

    R_DebugLine(points[0], points[1], r, g, b, alpha);
    R_DebugLine(points[1], points[3], r, g, b, alpha);
    R_DebugLine(points[3], points[2], r, g, b, alpha);
    R_DebugLine(points[2], points[0], r, g, b, alpha);
    R_DebugLine(points[4], points[5], r, g, b, alpha);
    R_DebugLine(points[5], points[7], r, g, b, alpha);
    R_DebugLine(points[7], points[6], r, g, b, alpha);
    R_DebugLine(points[6], points[4], r, g, b, alpha);

	for (i = 0; i < 4; i++) {
		R_DebugLine(points[i], points[i + 4], r, g, b, alpha);
	}
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

	return shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadWidth;
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

	return shader->unfoggedStages[ 0 ]->bundle[ 0 ].image[ 0 ]->uploadHeight;
}
