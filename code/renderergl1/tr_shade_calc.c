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
// tr_shade_calc.c

#include "tr_local.h"


#define	WAVEVALUE( table, base, amplitude, phase, freq )  ((base) + table[ myftol( ( ( (phase) + tess.shaderTime * (freq) ) * FUNCTABLE_SIZE ) ) & FUNCTABLE_MASK ] * (amplitude))

static float *TableForFunc( genFunc_t func ) 
{
	switch ( func )
	{
	case GF_SIN:
		return tr.sinTable;
	case GF_TRIANGLE:
		return tr.triangleTable;
	case GF_SQUARE:
		return tr.squareTable;
	case GF_SAWTOOTH:
		return tr.sawToothTable;
	case GF_INVERSE_SAWTOOTH:
		return tr.inverseSawToothTable;
	case GF_NONE:
	default:
		break;
	}

	ri.Error( ERR_DROP, "TableForFunc called with invalid function '%d' in shader '%s'\n", func, tess.shader->name );
	return NULL;
}

/*
** EvalWaveForm
**
** Evaluates a given waveForm_t, referencing backEnd.refdef.time directly
*/
static float EvalWaveForm( const waveForm_t *wf ) 
{
	float	*table;
	float	base;
	float	amplitude;
	float	phase;
	float	frequency;

	table = TableForFunc( wf->func );

    if (wf->base != 1234567.0f) {
        base = wf->base;
    } else if (backEnd.currentEntity) {
        base = (float)backEnd.currentEntity->e.surfaces[0] / 16.0 - 8.0;
    } else {
        base = r_static_shaderdata0->value;
    }

    if (wf->amplitude != 1234567.0f) {
		amplitude = wf->amplitude;
    } else if (backEnd.currentEntity) {
		amplitude = (float)backEnd.currentEntity->e.surfaces[1] / 16.0;
    } else {
		amplitude = r_static_shaderdata1->value;
    }

    if (wf->phase != 1234567.0f) {
		phase = wf->phase;
    } else if (backEnd.currentEntity) {
		phase = (float)backEnd.currentEntity->e.surfaces[2] / 16.0 - 8.0;
    } else {
		phase = r_static_shaderdata2->value;
    }

    if (wf->frequency != 1234567.0f) {
		frequency = wf->frequency;
    } else if (backEnd.currentEntity) {
		frequency = (float)backEnd.currentEntity->e.surfaces[3] / 16.0;
    } else {
		frequency = r_static_shaderdata1->value;
    }

	if (!backEnd.currentEntity)
	{
		base *= r_static_shadermultiplier0->value;
		amplitude *= r_static_shadermultiplier1->value;
		phase *= r_static_shadermultiplier2->value;
		frequency *= r_static_shadermultiplier3->value;
	}

	return WAVEVALUE( table, base, amplitude, phase, frequency );
}

static float EvalWaveFormClamped( const waveForm_t *wf )
{
	float glow  = EvalWaveForm( wf );

	if ( glow < 0 )
	{
		return 0;
	}

	if ( glow > 1 )
	{
		return 1;
	}

	return glow;
}

/*
** RB_CalcStretchTexCoords
*/
void RB_CalcStretchTexCoords( const waveForm_t *wf, float *st )
{
	float p;
	texModInfo_t tmi;

	p = 1.0f / EvalWaveForm( wf );

	tmi.matrix[0][0] = p;
	tmi.matrix[1][0] = 0;
	tmi.translate[0] = 0.5f - 0.5f * p;

	tmi.matrix[0][1] = 0;
	tmi.matrix[1][1] = p;
	tmi.translate[1] = 0.5f - 0.5f * p;

	RB_CalcTransformTexCoords( &tmi, st );
}

/*
========================
RB_CalcTransWaveTexCoords

========================
*/
void RB_CalcTransWaveTexCoords(const waveForm_t* wf, float* st)
{
	float p;
	int i;

    p = EvalWaveForm(wf);
    for (i = 0; i < tess.numVertexes; i++, st += 2) {
        st[0] += p;
    }
}

/*
========================
RB_CalcTransWaveTexCoordsT

========================
*/
void RB_CalcTransWaveTexCoordsT(const waveForm_t* wf, float* st)
{
    float p;
    int i;

    p = EvalWaveForm(wf);
    for (i = 0; i < tess.numVertexes; i++, st += 2) {
        st[1] += p;
    }
}

/*
========================
RB_CalcBulgeTexCoords

========================
*/
void RB_CalcBulgeTexCoords(const waveForm_t* wf, float* st)
{
	int i;
	int off;
	float offset;
	float now;
	float amplitude;
	float width;

    amplitude = wf->amplitude;
    width = wf->base;
    now = backEnd.refdef.time / 1000.0 * wf->frequency + wf->phase;

	for (i = 0; i < tess.numVertexes; i++, st += 2) {
		off = (now + st[0] * width) * FUNCTABLE_SIZE;
		offset = tr.sinTable[off & FUNCTABLE_MASK] * amplitude;
	}
}

/*
====================================================================

DEFORMATIONS

====================================================================
*/

/*
========================
RB_CalcDeformVertexes

========================
*/
void RB_CalcDeformVertexes( deformStage_t *ds )
{
	int i;
	vec3_t	offset;
	float	scale;
	float	*xyz = ( float * ) tess.xyz;
	float	*normal = ( float * ) tess.normal;
	float	*table;
	float	base;
	float	amplitude;
	float	phase;
	float	frequency;

    if (ds->deformationWave.base != 1234567.0f) {
        base = ds->deformationWave.base;
    } else if (backEnd.currentEntity) {
        base = (float)backEnd.currentEntity->e.surfaces[0] / 16.0 - 8.0;
    } else {
        base = r_static_shaderdata0->value;
    }

    if (ds->deformationWave.amplitude != 1234567.0f) {
		amplitude = ds->deformationWave.amplitude;
    } else if (backEnd.currentEntity) {
		amplitude = (float)backEnd.currentEntity->e.surfaces[1] / 16.0;
    } else {
		amplitude = r_static_shaderdata1->value;
    }

    if (ds->deformationWave.phase != 1234567.0f) {
		phase = ds->deformationWave.phase;
    } else if (backEnd.currentEntity) {
		phase = (float)backEnd.currentEntity->e.surfaces[2] / 16.0 - 8.0;
    } else {
		phase = r_static_shaderdata2->value;
    }

    if (ds->deformationWave.frequency != 1234567.0f) {
		frequency = ds->deformationWave.frequency;
    } else if (backEnd.currentEntity) {
		frequency = (float)backEnd.currentEntity->e.surfaces[3] / 16.0;
    } else {
		frequency = r_static_shaderdata1->value;
    }

	if (!backEnd.currentEntity)
	{
		base *= r_static_shadermultiplier0->value;
		amplitude *= r_static_shadermultiplier1->value;
		phase *= r_static_shadermultiplier2->value;
		frequency *= r_static_shadermultiplier3->value;
	}

	if ( frequency == 0 )
	{
		scale = EvalWaveForm( &ds->deformationWave );

		for ( i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4 )
		{
			VectorScale( normal, scale, offset );
			
			xyz[0] += offset[0];
			xyz[1] += offset[1];
			xyz[2] += offset[2];
		}
	}
	else
	{
		table = TableForFunc( ds->deformationWave.func );

		for ( i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4 )
		{
			float off = ( xyz[0] + xyz[1] + xyz[2] ) * ds->deformationSpread;

			scale = WAVEVALUE( table, base, amplitude, phase + off, frequency );

			VectorScale( normal, scale, offset );
			
			xyz[0] += offset[0];
			xyz[1] += offset[1];
			xyz[2] += offset[2];
		}
	}
}

void RB_CalcFlapVertexes(deformStage_t* ds, texDirection_t coordsToUse)
{
    int    i;
    vec3_t offset;
    float  scale;
    vec4_t* xyz, * normal;
	float* table;
    float  base;
    float  amplitude;
    float  phase;
    float  frequency;
    float  min, max;
    vec2_t (*st)[2];
    float  vertexScale;

    xyz = tess.xyz;
    normal = tess.normal;
    min = ds->bulgeWidth;
    max = ds->bulgeHeight;
    st = tess.texCoords;

    if (ds->deformationWave.base != 1234567.0f) {
        base = ds->deformationWave.base;
    } else if (backEnd.currentEntity) {
        base = (float)backEnd.currentEntity->e.surfaces[0] / 16.0 - 8.0;
    } else {
        base = r_static_shaderdata0->value;
    }

    if (ds->deformationWave.amplitude != 1234567.0f) {
        amplitude = ds->deformationWave.amplitude;
    } else if (backEnd.currentEntity) {
        amplitude = (float)backEnd.currentEntity->e.surfaces[1] / 16.0;
    } else {
        amplitude = r_static_shaderdata1->value;
    }

    if (ds->deformationWave.phase != 1234567.0f) {
        phase = ds->deformationWave.phase;
    } else if (backEnd.currentEntity) {
        phase = (float)backEnd.currentEntity->e.surfaces[2] / 16.0 - 8.0;
    } else {
        phase = r_static_shaderdata2->value;
    }

    if (ds->deformationWave.frequency != 1234567.0f) {
        frequency = ds->deformationWave.frequency;
    } else if (backEnd.currentEntity) {
        frequency = (float)backEnd.currentEntity->e.surfaces[3] / 16.0;
    } else {
        frequency = r_static_shaderdata3->value;
    }

    if (!backEnd.currentEntity) {
        base = base * r_static_shadermultiplier0->value;
        amplitude = amplitude * r_static_shadermultiplier1->value;
        phase = phase * r_static_shadermultiplier2->value;
        frequency = frequency * r_static_shadermultiplier3->value;
    }

	if (frequency) {
		table = TableForFunc(ds->deformationWave.func);
		for (i = 0; i < tess.numVertexes; i++, xyz++, st++, normal++) {
			float off;

			off = ((*xyz)[0] + (*xyz)[1] + (*xyz)[2]) * ds->deformationSpread;
			scale = WAVEVALUE(table, base, amplitude, phase, frequency);
            vertexScale = (max - min) * (*st)[0][coordsToUse] + min;
            offset[0] = scale * vertexScale * (*normal)[0];
            offset[1] = scale * vertexScale * (*normal)[1];
            offset[2] = scale * vertexScale * (*normal)[2];
			VectorAdd(*xyz, offset, *xyz);
		}
	} else {
        scale = EvalWaveForm(&ds->deformationWave);
        for (i = 0; i < tess.numVertexes; i++, xyz++, st++, normal++) {
            vertexScale = (max - min) * (*st)[0][coordsToUse] + min;
            offset[0] = vertexScale * scale * (*normal)[0];
            offset[1] = vertexScale * scale * (*normal)[1];
            offset[2] = vertexScale * scale * (*normal)[2];
            VectorAdd(*xyz, offset, *xyz);
		}
	}
}

/*
=========================
RB_CalcDeformNormals

Wiggle the normals for wavy environment mapping
=========================
*/
void RB_CalcDeformNormals( deformStage_t *ds ) {
	int i;
	float	scale;
	float	*xyz = ( float * ) tess.xyz;
	float	*normal = ( float * ) tess.normal;

	for ( i = 0; i < tess.numVertexes; i++, xyz += 4, normal += 4 ) {
		scale = 0.98f;
		scale = R_NoiseGet4f( xyz[0] * scale, xyz[1] * scale, xyz[2] * scale,
			tess.shaderTime * ds->deformationWave.frequency );
		normal[ 0 ] += ds->deformationWave.amplitude * scale;

		scale = 0.98f;
		scale = R_NoiseGet4f( 100 + xyz[0] * scale, xyz[1] * scale, xyz[2] * scale,
			tess.shaderTime * ds->deformationWave.frequency );
		normal[ 1 ] += ds->deformationWave.amplitude * scale;

		scale = 0.98f;
		scale = R_NoiseGet4f( 200 + xyz[0] * scale, xyz[1] * scale, xyz[2] * scale,
			tess.shaderTime * ds->deformationWave.frequency );
		normal[ 2 ] += ds->deformationWave.amplitude * scale;

		VectorNormalizeFast( normal );
	}
}

/*
========================
RB_CalcBulgeVertexes

========================
*/
void RB_CalcBulgeVertexes( deformStage_t *ds ) {
	int i;
	const float *st = ( const float * ) tess.texCoords[0];
	float		*xyz = ( float * ) tess.xyz;
	float		*normal = ( float * ) tess.normal;
	float		now;

	now = backEnd.refdef.time * ds->bulgeSpeed * 0.001f;

	for ( i = 0; i < tess.numVertexes; i++, xyz += 4, st += 4, normal += 4 ) {
		int		off;
		float scale;

		off = FUNCTABLE_SIZE * ( st[0] * ds->bulgeWidth + now );

		scale = tr.sinTable[ off & FUNCTABLE_MASK ] * ds->bulgeHeight;
			
		xyz[0] += normal[0] * scale;
		xyz[1] += normal[1] * scale;
		xyz[2] += normal[2] * scale;
	}
}


/*
======================
RB_CalcMoveVertexes

A deformation that can move an entire surface along a wave path
======================
*/
void RB_CalcMoveVertexes( deformStage_t *ds ) {
	int			i;
	float		*xyz;
	float		*table;
	float		scale;
	vec3_t		offset;

	table = TableForFunc( ds->deformationWave.func );

	scale = WAVEVALUE( table, ds->deformationWave.base, 
		ds->deformationWave.amplitude,
		ds->deformationWave.phase,
		ds->deformationWave.frequency );

	VectorScale( ds->moveVector, scale, offset );

	xyz = ( float * ) tess.xyz;
	for ( i = 0; i < tess.numVertexes; i++, xyz += 4 ) {
		VectorAdd( xyz, offset, xyz );
	}
}


/*
=============
DeformText

Change a polygon into a bunch of text polygons
=============
*/
void DeformText( const char *text ) {
	int		i;
	vec3_t	origin, width, height;
	size_t	len;
	int		ch;
	byte	color[4];
	float	bottom, top;
	vec3_t	mid;

	height[0] = 0;
	height[1] = 0;
	height[2] = -1;
	CrossProduct( tess.normal[0], height, width );

	// find the midpoint of the box
	VectorClear( mid );
	bottom = 999999;
	top = -999999;
	for ( i = 0 ; i < 4 ; i++ ) {
		VectorAdd( tess.xyz[i], mid, mid );
		if ( tess.xyz[i][2] < bottom ) {
			bottom = tess.xyz[i][2];
		}
		if ( tess.xyz[i][2] > top ) {
			top = tess.xyz[i][2];
		}
	}
	VectorScale( mid, 0.25f, origin );

	// determine the individual character size
	height[0] = 0;
	height[1] = 0;
	height[2] = ( top - bottom ) * 0.5f;

	VectorScale( width, height[2] * -0.75f, width );

	// determine the starting position
	len = strlen( text );
	VectorMA( origin, (len-1), width, origin );

	// clear the shader indexes
	tess.numIndexes = 0;
	tess.numVertexes = 0;

	color[0] = color[1] = color[2] = color[3] = 255;

	// draw each character
	for ( i = 0 ; i < len ; i++ ) {
		ch = text[i];
		ch &= 255;

		if ( ch != ' ' ) {
			int		row, col;
			float	frow, fcol, size;

			row = ch>>4;
			col = ch&15;

			frow = row*0.0625f;
			fcol = col*0.0625f;
			size = 0.0625f;

			RB_AddQuadStampExt( origin, width, height, color, fcol, frow, fcol + size, frow + size );
		}
		VectorMA( origin, -2, width, origin );
	}
}

/*
==================
GlobalVectorToLocal
==================
*/
static void GlobalVectorToLocal( const vec3_t in, vec3_t out ) {
	out[0] = DotProduct( in, backEnd.ori.axis[0] );
	out[1] = DotProduct( in, backEnd.ori.axis[1] );
	out[2] = DotProduct( in, backEnd.ori.axis[2] );
}

/*
=====================
AutospriteDeform

Assuming all the triangles for this shader are independant
quads, rebuild them as forward facing sprites
=====================
*/
static void AutospriteDeform(void) {
    int		i;
    int		oldVerts;
    float* xyz;
    vec3_t	mid, delta;
    float	radius;
    vec3_t	left, up;
    vec3_t	leftDir, upDir;

    if (tess.numVertexes & 3) {
        ri.Printf(PRINT_WARNING, "Autosprite shader %s had odd vertex count\n", tess.shader->name);
    }
    if (tess.numIndexes != (tess.numVertexes >> 2) * 6) {
        ri.Printf(PRINT_WARNING, "Autosprite shader %s had odd index count\n", tess.shader->name);
    }

    oldVerts = tess.numVertexes;
    tess.numVertexes = 0;
    tess.numIndexes = 0;

    if (backEnd.currentEntity != &tr.worldEntity) {
        GlobalVectorToLocal(backEnd.viewParms.ori.axis[1], leftDir);
        GlobalVectorToLocal(backEnd.viewParms.ori.axis[2], upDir);
    } else {
        VectorCopy(backEnd.viewParms.ori.axis[1], leftDir);
        VectorCopy(backEnd.viewParms.ori.axis[2], upDir);
    }

    for (i = 0; i < oldVerts; i += 4) {
        // find the midpoint
        xyz = tess.xyz[i];

        mid[0] = 0.25f * (xyz[0] + xyz[4] + xyz[8] + xyz[12]);
        mid[1] = 0.25f * (xyz[1] + xyz[5] + xyz[9] + xyz[13]);
        mid[2] = 0.25f * (xyz[2] + xyz[6] + xyz[10] + xyz[14]);

        VectorSubtract(xyz, mid, delta);
        radius = VectorLength(delta) * 0.707f;		// / sqrt(2)

        VectorScale(leftDir, radius, left);
        VectorScale(upDir, radius, up);

        if (backEnd.viewParms.isMirror) {
            VectorSubtract(vec3_origin, left, left);
        }

        // compensate for scale in the axes if necessary
        if (backEnd.currentStaticModel || backEnd.currentEntity->e.nonNormalizedAxes) {
            float axisLength;

			if (backEnd.currentStaticModel) {
				axisLength = VectorLength(backEnd.currentStaticModel->axis[0]);
			} else {
				axisLength = VectorLength(backEnd.currentEntity->e.axis[0]);
			}
            
            if (!axisLength) {
                axisLength = 0;
            }
            else {
                axisLength = 1.0f / axisLength;
            }
            VectorScale(left, axisLength, left);
            VectorScale(up, axisLength, up);
        }

        RB_AddQuadStamp(mid, left, up, tess.vertexColors[i]);
    }
}


/*
=====================
Autosprite2Deform

Autosprite2 will pivot a rectangular quad along the center of its long axis
=====================
*/
int edgeVerts[6][2] = {
	{ 0, 1 },
	{ 0, 3 },
	{ 0, 2 },
	{ 1, 3 },
	{ 1, 2 },
	{ 3, 2 }
};

static void Autosprite2Deform( void ) {
	int		i, j;
	int		indexes;
	float	*xyz;
	vec3_t	forward;

	if ( tess.numVertexes & 3 ) {
		ri.Printf( PRINT_WARNING, "Autosprite2 shader %s had odd vertex count\n", tess.shader->name );
	}
	if ( tess.numIndexes != ( tess.numVertexes >> 2 ) * 6 ) {
		ri.Printf( PRINT_WARNING, "Autosprite2 shader %s had odd index count\n", tess.shader->name );
	}

	if ( backEnd.currentStaticModel || backEnd.currentEntity != &tr.worldEntity ) {
		GlobalVectorToLocal( backEnd.viewParms.ori.axis[0], forward );
	} else {
		VectorCopy( backEnd.viewParms.ori.axis[0], forward );
	}

	// this is a lot of work for two triangles...
	// we could precalculate a lot of it is an issue, but it would mess up
	// the shader abstraction
	for ( i = 0, indexes = 0 ; i < tess.numVertexes ; i+=4, indexes+=6 ) {
		float shortLengths[2];
		int shortNums[2];
		float longLengths[2];
		int longNums[2];
		vec3_t	mid[2];
		vec3_t	major, minor;
		float	*v1, *v2, *v3, *v4;
		qboolean firstOnLeft, secondOnLeft;
		float edgeLength;

		// find the midpoint
		xyz = tess.xyz[i];

		// identify the two shortest edges
		shortNums[0] = shortNums[1] = 0;
		shortLengths[0] = shortLengths[1] = 1000000000;
		longNums[1] = longNums[0] = 0;
		longLengths[1] = longLengths[0] = 0;

		for ( j = 0 ; j < 6 ; j++ ) {
			float	l;
			vec3_t	temp;

			v1 = xyz + 4 * edgeVerts[j][0];
			v2 = xyz + 4 * edgeVerts[j][1];

			VectorSubtract( v1, v2, temp );
			
			l = DotProduct( temp, temp );
			if ( l < shortLengths[0] ) {
				shortNums[1] = shortNums[0];
				shortLengths[1] = shortLengths[0];
				shortNums[0] = j;
				shortLengths[0] = l;
			} else if ( l < shortLengths[1] ) {
				shortNums[1] = j;
				shortLengths[1] = l;
			}

			if (l > longLengths[0]) {
				longNums[1] = longNums[0];
				longLengths[1] = longLengths[0];
				longNums[0] = j;
				longLengths[0] = l;
			} else if (l > longLengths[1]) {
				longNums[1] = j;
				longLengths[1] = l;
			}
		}

		for ( j = 0 ; j < 2 ; j++ ) {
			v1 = xyz + 4 * edgeVerts[shortNums[j]][0];
			v2 = xyz + 4 * edgeVerts[shortNums[j]][1];

			mid[j][0] = 0.5f * (v1[0] + v2[0]);
			mid[j][1] = 0.5f * (v1[1] + v2[1]);
			mid[j][2] = 0.5f * (v1[2] + v2[2]);
		}

		// find the vector of the major axis
		VectorSubtract( mid[1], mid[0], major );

		// cross this with the view direction to get minor axis
		CrossProduct( major, forward, minor );
		VectorNormalize( minor );

        v1 = xyz + 4 * edgeVerts[shortNums[0]][0];
        v2 = xyz + 4 * edgeVerts[shortNums[0]][1];

        // we need to see which direction this edge
        // is used to determine direction of projection
		if (edgeVerts[shortNums[0]][0] == edgeVerts[longNums[0]][0]
			|| edgeVerts[shortNums[0]][0] == edgeVerts[longNums[0]][1])
		{
			firstOnLeft = qtrue;
		}
		else
		{
			firstOnLeft = qfalse;
        }

        if (edgeVerts[shortNums[1]][0] == edgeVerts[longNums[1]][0]
            || edgeVerts[shortNums[1]][0] == edgeVerts[longNums[1]][1])
        {
            secondOnLeft = qtrue;
        }
        else
        {
			secondOnLeft = qfalse;
        }

		if (firstOnLeft == secondOnLeft)
        {
            v3 = xyz + 4 * edgeVerts[shortNums[1]][0];
            v4 = xyz + 4 * edgeVerts[shortNums[1]][1];
		}
		else
        {
            v3 = xyz + 4 * edgeVerts[shortNums[1]][1];
            v4 = xyz + 4 * edgeVerts[shortNums[1]][0];
		}

        // re-project the points
		edgeLength = sqrt(shortLengths[0]) * 0.5;
        v1[0] = -edgeLength * minor[0] + mid[0][0];
        v1[1] = -edgeLength * minor[1] + mid[0][1];
        v1[2] = -edgeLength * minor[2] + mid[0][2];
		v2[0] = minor[0] * edgeLength + mid[0][0];
        v2[1] = minor[1] * edgeLength + mid[0][1];
        v2[2] = minor[2] * edgeLength + mid[0][2];

        edgeLength = sqrt(shortLengths[1]) * 0.5;
		v3[0] = -edgeLength * minor[0] + mid[1][0];
        v3[1] = -edgeLength * minor[1] + mid[1][1];
        v3[2] = -edgeLength * minor[2] + mid[1][2];
		v4[0] = minor[0] * edgeLength + mid[1][0];
        v4[1] = minor[1] * edgeLength + mid[1][1];
        v4[2] = minor[2] * edgeLength + mid[1][2];
	}
}

static void LightGlowDeform() {
	int i;
	int oldVerts;
	float* xyz;
	vec3_t mid, delta;
	float radius, dist, ofs;
	vec3_t forward, left, up;
	vec3_t leftDir, upDir;

    if (tess.numVertexes & 3) {
        ri.Printf(PRINT_WARNING, "LightGlowDeform shader %s had odd vertex count\n", tess.shader->name);
    }
    if (tess.numIndexes != (tess.numVertexes >> 2) * 6) {
        ri.Printf(PRINT_WARNING, "LightGlowDeform shader %s had odd index count\n", tess.shader->name);
    }

	oldVerts = tess.numVertexes;
	tess.numVertexes = 0;
	tess.numIndexes = 0;

    if (backEnd.currentEntity == &tr.worldEntity)
    {
		VectorCopy(backEnd.viewParms.ori.axis[1], leftDir);
		VectorCopy(backEnd.viewParms.ori.axis[2], upDir);
    }
    else
    {
        GlobalVectorToLocal(backEnd.viewParms.ori.axis[1], leftDir);
        GlobalVectorToLocal(backEnd.viewParms.ori.axis[2], upDir);
    }

	for (i = 0; i < oldVerts; i += 4)
	{
		xyz = tess.xyz[i];

		mid[0] = (xyz[0] + xyz[4] + xyz[8] + xyz[12]) * 0.25f;
		mid[1] = (xyz[1] + xyz[5] + xyz[9] + xyz[13]) * 0.25f;
		mid[2] = (xyz[2] + xyz[6] + xyz[10] + xyz[14]) * 0.25f;

		VectorSubtract(xyz, mid, delta);

		radius = VectorLength(delta) * 0.707f;
		VectorAdd(mid, backEnd.ori.origin, delta);
		VectorSubtract(backEnd.viewParms.ori.origin, delta, forward);

		dist = VectorNormalize(forward) - 4.0;

		VectorScale(forward, radius, forward);
		VectorScale(leftDir, radius, left);
		VectorScale(upDir, radius, up);

		if (backEnd.viewParms.isMirror)
		{
			VectorSubtract(vec3_origin, forward, forward);
			VectorSubtract(vec3_origin, left, left);
		}

		if (backEnd.currentStaticModel || backEnd.currentEntity->e.nonNormalizedAxes)
		{
			float axisLength;

			if (backEnd.currentStaticModel) {
				axisLength = VectorLength(backEnd.currentStaticModel->axis[0]);
			} else {
                axisLength = VectorLength(backEnd.currentEntity->e.axis[0]);
			}

			if (axisLength != 0.0f) {
				VectorScale(forward, axisLength, forward);
				VectorScale(left, axisLength, left);
				VectorScale(up, axisLength, up);
			} else {
				VectorClear(forward);
				VectorClear(left);
				VectorClear(up);
			}
		}

		ofs = VectorLength(forward);
        if (ofs > dist)
        {
            VectorNormalizeFast(forward);
			VectorScale(forward, dist, forward);
        }

		VectorAdd(mid, forward, mid);
        RB_AddQuadStamp(mid, left, up, tess.vertexColors[i]);
	}
}

/*
=====================
RB_DeformTessGeometry

=====================
*/
void RB_DeformTessGeometry( void ) {
	int		i;
	deformStage_t	*ds;

	for ( i = 0 ; i < tess.shader->numDeforms ; i++ ) {
		ds = &tess.shader->deforms[ i ];

		switch ( ds->deformation ) {
        case DEFORM_NONE:
            break;
		case DEFORM_NORMALS:
			RB_CalcDeformNormals( ds );
			break;
		case DEFORM_WAVE:
			RB_CalcDeformVertexes( ds );
			break;
		case DEFORM_BULGE:
			RB_CalcBulgeVertexes( ds );
			break;
		case DEFORM_MOVE:
			RB_CalcMoveVertexes( ds );
			break;
		case DEFORM_AUTOSPRITE:
			AutospriteDeform();
			break;
		case DEFORM_AUTOSPRITE2:
			Autosprite2Deform();
			break;
		case DEFORM_LIGHTGLOW:
			LightGlowDeform();
			break;
		case DEFORM_FLAP_S:
			RB_CalcFlapVertexes(ds, USE_S_COORDS);
			break;
		case DEFORM_FLAP_T:
			RB_CalcFlapVertexes(ds, USE_T_COORDS);
			break;
		}
	}
}

/*
====================================================================

COLORS

====================================================================
*/


/*
** RB_CalcColorFromEntity
*/
void RB_CalcColorFromEntity( unsigned char *dstColors )
{
	int	i;
	int *pColors = ( int * ) dstColors;
	int c;

	if ( !backEnd.currentEntity )
		return;

	c = * ( int * ) backEnd.currentEntity->e.shaderRGBA;

	for ( i = 0; i < tess.numVertexes; i++, pColors++ )
	{
		*pColors = c;
	}
}

/*
** RB_CalcColorFromOneMinusEntity
*/
void RB_CalcColorFromOneMinusEntity( unsigned char *dstColors )
{
	int	i;
	int *pColors = ( int * ) dstColors;
	unsigned char invModulate[4];
	int c;

	if ( !backEnd.currentEntity )
		return;

	invModulate[0] = 255 - backEnd.currentEntity->e.shaderRGBA[0];
	invModulate[1] = 255 - backEnd.currentEntity->e.shaderRGBA[1];
	invModulate[2] = 255 - backEnd.currentEntity->e.shaderRGBA[2];
	invModulate[3] = 255 - backEnd.currentEntity->e.shaderRGBA[3];	// this trashes alpha, but the AGEN block fixes it

	c = * ( int * ) invModulate;

	for ( i = 0; i < tess.numVertexes; i++, pColors++ )
	{
		*pColors = * ( int * ) invModulate;
	}
}

/*
** RB_CalcColorFromConstant
*/
void RB_CalcColorFromConstant(unsigned char* dstColors, unsigned char* constantColor)
{
	int i;

	for (i = 0; i < tess.numVertexes; i++) {
		dstColors[i * 4] = constantColor[0];
		dstColors[i * 4 + 1] = constantColor[1];
		dstColors[i * 4 + 2] = constantColor[2];
		dstColors[i * 4 + 3] = constantColor[3];
	}
}

/*
** RB_CalcRGBFromDot
*/
void RB_CalcRGBFromDot(unsigned char* colors, float alphaMin, float alphaMax)
{
	int i;
	vec3_t viewInModel;

	for (i = 0; i < tess.numVertexes; i++, colors += 4) {
		float f;

		VectorSubtract(backEnd.ori.viewOrigin, tess.xyz[i], viewInModel);
		VectorNormalizeFast(viewInModel);

        f = Square(DotProduct(tess.normal[i], viewInModel));
        f = ((alphaMax - alphaMin) * f + alphaMin) * 255.0;
		f = f - Q_max((int)(f - 255), 0);

		colors[0] = colors[1] = colors[2] = (unsigned char)Q_max(f, 0);
	}
}

/*
** RB_CalcRGBFromOneMinusDot
*/
void RB_CalcRGBFromOneMinusDot(unsigned char* colors, float alphaMin, float alphaMax)
{
	int i;
	vec3_t viewInModel;

	for (i = 0; i < tess.numVertexes; i++, colors += 4) {
		float f;

		VectorSubtract(backEnd.ori.viewOrigin, tess.xyz[i], viewInModel);
		VectorNormalizeFast(viewInModel);

        f = 1.0 - Square(DotProduct(tess.normal[i], viewInModel));
        f = ((alphaMax - alphaMin) * f + alphaMin) * 255.0;
		f = f - Q_max((int)(f - 255), 0);

		colors[0] = colors[1] = colors[2] = (unsigned char)Q_max(f, 0);
	}
}

/*
** RB_CalcAlphaFromConstant
*/
void RB_CalcAlphaFromConstant(unsigned char* dstColors, int constantAlpha)
{
	int i;

	for (i = 0; i < tess.numVertexes; i++) {
		dstColors[i * 4 + 3] = constantAlpha;
	}
}

/*
** RB_CalcAlphaFromDot
*/
void RB_CalcAlphaFromDot(unsigned char* colors, float alphaMin, float alphaMax)
{
	int i;
	vec3_t viewInModel;

	for (i = 0; i < tess.numVertexes; i++, colors += 4) {
		float f;

		VectorSubtract(backEnd.ori.viewOrigin, tess.xyz[i], viewInModel);
		VectorNormalizeFast(viewInModel);

        f = Square(DotProduct(tess.normal[i], viewInModel));
        f = ((alphaMax - alphaMin) * f + alphaMin) * 255.0;
		f = f - Q_max((int)(f - 255), 0);

		colors[0] = colors[1] = colors[2] = (unsigned char)Q_max(f, 0);
	}
}

/*
** RB_CalcAlphaFromHeightFade
*/
void RB_CalcAlphaFromHeightFade(unsigned char* colors, float alphaMin, float alphaMax)
{
	int i;
	float dist;
	float alpha;

	for (i = 0; i < tess.numVertexes; i++) {
		dist = fabs(backEnd.ori.viewOrigin[2] - tess.xyz[i][2]);
		dist = Q_clamp_float(dist, alphaMin, alphaMax);
		// Calculate the alpha height
		alpha = (dist - alphaMin) / (alphaMax - alphaMin);
		colors[i * 4 + 3] = (unsigned char)((1.f - alpha) * 255.0);
	}
}

/*
** RB_CalcAlphaFromOneMinusDot
*/
void RB_CalcAlphaFromOneMinusDot(unsigned char* colors, float alphaMin, float alphaMax)
{
	int i;
	vec3_t viewInModel;

	for (i = 0; i < tess.numVertexes; i++, colors += 4) {
		float f;

		VectorSubtract(backEnd.ori.viewOrigin, tess.xyz[i], viewInModel);
		VectorNormalizeFast(viewInModel);

        f = 1.0 - Square(DotProduct(tess.normal[i], viewInModel));
        f = ((alphaMax - alphaMin) * f + alphaMin) * 255.0;
		f = f - Q_max((int)(f - 255), 0);

		colors[0] = colors[1] = colors[2] = (unsigned char)Q_max(f, 0);
	}
}

/*
** RB_CalcAlphaFromTexCoords
*/
void RB_CalcAlphaFromTexCoords(unsigned char* colors, float alphaMin, float alphaMax, int alphaMinCap, int alphaCap, float sWeight, float tWeight, float* st)
{
	int i;

	for (i = 0; i < tess.numVertexes; i++, colors += 4, st += 2) {
		float f;

        f = sWeight * tess.texCoords[i][0][0] + tWeight * tess.texCoords[i][0][1];
        f = ((alphaMax - alphaMin) * f + alphaMin) * 255.0;
		f = f - Q_max((int)(f - alphaCap), 0);
		f = alphaMinCap + Q_max((int)(f - alphaMinCap), 0);
	
		colors[3] = (unsigned char)f;
	}
}

/*
** RB_CalcRGBFromTexCoords
*/
void RB_CalcRGBFromTexCoords(unsigned char* colors, float alphaMin, float alphaMax, int alphaMinCap, int alphaCap, float sWeight, float tWeight, float* st)
{
	int i;

	for (i = 0; i < tess.numVertexes; i++, colors += 4, st += 2) {
		float f;

        f = sWeight * tess.texCoords[i][0][0] + tWeight * tess.texCoords[i][0][1];
        f = ((alphaMax - alphaMin) * f + alphaMin) * 255.0;
		f = f - Q_max((int)(f - alphaCap), 0);
		f = alphaMinCap + Q_max((int)(f - alphaMinCap), 0);

		colors[0] = colors[1] = colors[2] = (unsigned char)f;
	}
}

/*
** RB_CalcAlphaFromEntity
*/
void RB_CalcAlphaFromEntity( unsigned char *dstColors )
{
	int	i;

	if ( !backEnd.currentEntity )
		return;

	dstColors += 3;

	for ( i = 0; i < tess.numVertexes; i++, dstColors += 4 )
	{
		*dstColors = backEnd.currentEntity->e.shaderRGBA[3];
	}
}

/*
** RB_CalcAlphaFromOneMinusEntity
*/
void RB_CalcAlphaFromOneMinusEntity( unsigned char *dstColors )
{
	int	i;

	if ( !backEnd.currentEntity )
		return;

	dstColors += 3;

	for ( i = 0; i < tess.numVertexes; i++, dstColors += 4 )
	{
		*dstColors = 0xff - backEnd.currentEntity->e.shaderRGBA[3];
	}
}

/*
** RB_CalcWaveColor
*/
void RB_CalcWaveColor(const waveForm_t* wf, unsigned char* dstColors, unsigned char* constantColor)
{
	int i;
	int v;
	float glow;
	int *colors = ( int * ) dstColors;
	byte	color[4];


  if ( wf->func == GF_NOISE ) {
		glow = wf->base + R_NoiseGet4f( 0, 0, 0, ( tess.shaderTime + wf->phase ) * wf->frequency ) * wf->amplitude;
	} else {
		glow = EvalWaveForm( wf ) * tr.identityLight;
	}
	
	if ( glow < 0 ) {
		glow = 0;
	}
	else if ( glow > 1 ) {
		glow = 1;
	}

	if (constantColor)
	{
		color[0] = constantColor[0] * glow;
		color[1] = constantColor[1] * glow;
		color[2] = constantColor[2] * glow;
	}
	else
	{
		v = myftol(255 * glow);
		color[0] = color[1] = color[2] = v;
	}

	color[3] = 255;
	v = *(int*)color;
	
	for ( i = 0; i < tess.numVertexes; i++, colors++ ) {
		*colors = v;
	}
}

/*
** RB_CalcWaveAlpha
*/
void RB_CalcWaveAlpha( const waveForm_t *wf, unsigned char *dstColors )
{
	int i;
	int v;
	float glow;

	glow = EvalWaveFormClamped( wf );

	v = 255 * glow;

	for ( i = 0; i < tess.numVertexes; i++, dstColors += 4 )
	{
		dstColors[3] = v;
	}
}


/*
====================================================================

TEX COORDS

====================================================================
*/

/*
** RB_CalcEnvironmentTexCoords
*/
void RB_CalcEnvironmentTexCoords( float *st ) 
{
	int			i;
	float		*v, *normal;
	vec3_t		viewer, reflected;
	float		d;

	v = tess.xyz[0];
	normal = tess.normal[0];

	for (i = 0 ; i < tess.numVertexes ; i++, v += 4, normal += 4, st += 2 ) 
	{
		VectorSubtract (backEnd.ori.viewOrigin, v, viewer);
		VectorNormalizeFast (viewer);

		d = DotProduct (normal, viewer);

		reflected[0] = normal[0]*2*d - viewer[0];
		reflected[1] = normal[1]*2*d - viewer[1];
		reflected[2] = normal[2]*2*d - viewer[2];

		st[0] = 0.5 + reflected[1] * 0.5;
		st[1] = 0.5 - reflected[2] * 0.5;
	}
}

/*
** RB_CalcEnvironmentTexCoords
*/
void RB_CalcEnvironmentTexCoords2(float* st)
{
    int			i;
    float* v, * normal;
    vec3_t		viewer, reflected, worldReflected;
    float		d;

    v = tess.xyz[0];
    normal = tess.normal[0];

    for (i = 0; i < tess.numVertexes; i++, v += 4, normal += 4, st += 2)
    {
        VectorSubtract(backEnd.ori.viewOrigin, v, viewer);
        VectorNormalizeFast(viewer);

        d = DotProduct(normal, viewer);
		if (d > 0.0f) {
			VectorCopy(viewer, reflected);
		} else {
            d *= -2;
            reflected[0] = normal[0] * d + viewer[0];
            reflected[1] = normal[1] * d + viewer[1];
            reflected[2] = normal[2] * d + viewer[2];
		}

		worldReflected[0] = reflected[0] * backEnd.ori.axis[0][0];
		worldReflected[2] = reflected[0] * backEnd.ori.axis[0][2];
		worldReflected[0] = backEnd.ori.axis[1][0] * reflected[1] + worldReflected[0];
		worldReflected[2] = backEnd.ori.axis[1][2] * reflected[1] + worldReflected[2];
		worldReflected[0] = backEnd.ori.axis[2][0] * reflected[2] + worldReflected[0];
		worldReflected[2] = backEnd.ori.axis[2][2] * reflected[2] + worldReflected[2];

        st[0] = 0.5 + reflected[1] * 0.5;
        st[1] = 0.5 - reflected[2] * 0.5;
    }
}

/*
** RB_CalcSunReflectionTexCoords
*/
void RB_CalcSunReflectionTexCoords(float* st) {
	int i;
	vec3_t viewer;
	vec3_t reflected;
	vec3_t sunReflected;
	float d;
	float* v;
	float* normal;
	vec3_t sunAxis[3];

    v = tess.xyz[0];
    normal = tess.normal[0];
    sunAxis[0][0] = tr.sunDirection[0];
    sunAxis[0][1] = tr.sunDirection[1];
    sunAxis[0][2] = tr.sunDirection[2];
    sunAxis[1][0] = tr.sunDirection[1];
    sunAxis[1][1] = -tr.sunDirection[0];
    sunAxis[1][2] = 0.0;
    VectorNormalizeFast(sunAxis[1]);
    CrossProduct(sunAxis[0], sunAxis[1], sunAxis[2]);

	for (i = 0; i < tess.numVertexes; i++, v += 4, normal += 4, st += 2) {
		VectorSubtract(v, backEnd.ori.viewOrigin, viewer);
		VectorNormalizeFast(viewer);

		d = DotProduct(viewer, normal);
		if (d > 0) {
			VectorCopy(viewer, reflected);
		} else {
			d *= -2;
			VectorMA(viewer, d, normal, reflected);
		}

		VectorScale(sunAxis[0], reflected[0], sunReflected);
		VectorMA(sunReflected, reflected[2], sunAxis[1], sunReflected);
		VectorMA(sunReflected, -reflected[1], sunAxis[2], sunReflected);

		st[0] = sunReflected[1] + 0.5;
		st[1] = sunReflected[2] + 0.5;
	}
}

/*
** RB_CalcTurbulentTexCoords
*/
void RB_CalcTurbulentTexCoords( const waveForm_t *wf, float *st )
{
	int i;
	float now;

	now = ( wf->phase + tess.shaderTime * wf->frequency );

	for ( i = 0; i < tess.numVertexes; i++, st += 2 )
	{
		float s = st[0];
		float t = st[1];

		st[0] = s + tr.sinTable[ ( ( int ) ( ( ( tess.xyz[i][0] + tess.xyz[i][2] )* 1.0/128 * 0.125 + now ) * FUNCTABLE_SIZE ) ) & ( FUNCTABLE_MASK ) ] * wf->amplitude;
		st[1] = t + tr.sinTable[ ( ( int ) ( ( tess.xyz[i][1] * 1.0/128 * 0.125 + now ) * FUNCTABLE_SIZE ) ) & ( FUNCTABLE_MASK ) ] * wf->amplitude;
	}
}

/*
** RB_CalcScaleTexCoords
*/
void RB_CalcScaleTexCoords( const float scale[2], float *st )
{
	int i;

	for ( i = 0; i < tess.numVertexes; i++, st += 2 )
	{
		st[0] *= scale[0];
		st[1] *= scale[1];
	}
}

/*
** RB_CalcOffsetTexCoords
*/
void RB_CalcOffsetTexCoords( const float *offset, float *st )
{
	int i;
	float offsetS, offsetT;

	if (offset[0] != 1234567) {
		offsetS = offset[0];
	} else if (backEnd.currentEntity) {
        offsetS = backEnd.currentEntity->e.shader_data[0];
    } else {
        offsetS = r_static_shaderdata0->value;
    }

	if (offset[0] != 1234567) {
		offsetT = offset[1];
	} else if (backEnd.currentEntity) {
		offsetT = backEnd.currentEntity->e.shader_data[1];
    } else {
		offsetT = r_static_shaderdata1->value;
    }

    if (!backEnd.currentEntity) {
        offsetS = offsetS * r_static_shadermultiplier0->value;
        offsetT = offsetT * r_static_shadermultiplier1->value;
    }

	for (i = 0; i < tess.numVertexes; i++, st += 2) {
		float s, t;

		s = st[0];
		t = st[1];

		st[0] = s + offsetS;
		st[1] = t + offsetT;
	}
}

/*
** RB_CalcParallaxTexCoords
*/
void RB_CalcParallaxTexCoords( const float *rate, float *st )
{
	int i;
	float offsetS, offsetT;

    offsetS = tr.refdef.vieworg[0] * rate[0];
    offsetT = tr.refdef.vieworg[1] * rate[1];
    for (i = 0; i < tess.numVertexes; i++, st += 2) {
		st[0] += offsetS;
		st[1] += offsetT;
    }
}

static vec3_t rb_baseaxis[18];


void RB_TextureAxisFromPlane(const vec3_t normal, vec3_t xv, vec3_t yv)
{
	int bestaxis;
	vec_t dot;
	vec_t best;
	int i;

	if (normal[0] == 1) {
		bestaxis = 2;
	} else if (normal[0] == -1) {
		bestaxis = 3;
	} else if (normal[1] == 1) {
		bestaxis = 4;
	} else if (normal[1] == -1) {
		bestaxis = 5;
	} else if (normal[2] == 1) {
		bestaxis = 0;
	} else if (normal[2] == -1) {
		bestaxis = 1;
	} else {
		best = 0;
		bestaxis = 0;

		for (i = 0; i < 6; i++) {
			dot = DotProduct(rb_baseaxis[i * 3], normal);
			if (dot > best) {
				best = dot;
				bestaxis = i;
			}
		}
	}

	VectorCopy(rb_baseaxis[bestaxis * 3 + 1], xv);
	VectorCopy(rb_baseaxis[bestaxis * 3 + 2], yv);
}

void RB_QuakeTextureVecs(const vec3_t normal, const vec2_t scale, vec3_t mappingVecs[2])
{
	RB_TextureAxisFromPlane(normal, mappingVecs[0], mappingVecs[1]);

	VectorScale(mappingVecs[0], scale[0], mappingVecs[0]);
	VectorScale(mappingVecs[1], scale[1], mappingVecs[1]);
}

/*
** RB_CalcMacroTexCoords
*/
void RB_CalcMacroTexCoords( const  float* rate, float *st )
{
	int i;
	vec3_t vecs[2];
	vec_t* v;
	vec_t* normal;
	float width, height;
    v = tess.xyz[0];

    normal = tess.normal[0];
    width = (float)tess.shader->unfoggedStages[0]->bundle[0].image[0]->uploadWidth;
    height = (float)tess.shader->unfoggedStages[0]->bundle[0].image[0]->uploadHeight;

	for (i = 0; i < tess.numVertexes; i++, v += 4, normal += 4, st += 2) {
		RB_QuakeTextureVecs(normal, rate, vecs);
		st[0] = DotProduct(vecs[0], v) / width;
		st[1] = DotProduct(vecs[1], v) / height;
	}
}

/*
** RB_CalcScrollTexCoords
*/
void RB_CalcScrollTexCoords( const float scrollSpeed[2], float *st )
{
	int i;
	float timeScale = tess.shaderTime;
	float adjustedScrollS, adjustedScrollT;

	adjustedScrollS = scrollSpeed[0] * timeScale;
	adjustedScrollT = scrollSpeed[1] * timeScale;

	// clamp so coordinates don't continuously get larger, causing problems
	// with hardware limits
	adjustedScrollS = adjustedScrollS - floor( adjustedScrollS );
	adjustedScrollT = adjustedScrollT - floor( adjustedScrollT );

	for ( i = 0; i < tess.numVertexes; i++, st += 2 )
	{
		st[0] += adjustedScrollS;
		st[1] += adjustedScrollT;
	}
}

/*
** RB_CalcTransformTexCoords
*/
void RB_CalcTransformTexCoords( const texModInfo_t *tmi, float *st  )
{
	int i;

	for ( i = 0; i < tess.numVertexes; i++, st += 2 )
	{
		float s = st[0];
		float t = st[1];

		st[0] = s * tmi->matrix[0][0] + t * tmi->matrix[1][0] + tmi->translate[0];
		st[1] = s * tmi->matrix[0][1] + t * tmi->matrix[1][1] + tmi->translate[1];
	}
}

/*
** RB_CalcRotateTexCoords
*/
void RB_CalcRotateTexCoords(float degsPerSecond, float degsPerSecondCoef, float* st, float start)
{
	float timeScale = tess.shaderTime;
	float degs;
	int index;
	float sinValue, cosValue;
	texModInfo_t tmi;

    if (degsPerSecond != 1234567) {
		degs = -degsPerSecond * degsPerSecondCoef * tess.shaderTime - start;
	} else if (backEnd.currentEntity) {
		degs = -backEnd.currentEntity->e.shader_data[0] * degsPerSecondCoef * tess.shaderTime - start;
	} else {
		degs = r_static_shaderdata0->value;
	}
	if (!backEnd.currentEntity) {
		degs *= r_static_shadermultiplier0->value;
	}
	index = degs * ( FUNCTABLE_SIZE / 360.0f );

	sinValue = tr.sinTable[ index & FUNCTABLE_MASK ];
	cosValue = tr.sinTable[ ( index + FUNCTABLE_SIZE / 4 ) & FUNCTABLE_MASK ];

	tmi.matrix[0][0] = cosValue;
	tmi.matrix[1][0] = -sinValue;
	tmi.translate[0] = 0.5 - 0.5 * cosValue + 0.5 * sinValue;

	tmi.matrix[0][1] = sinValue;
	tmi.matrix[1][1] = cosValue;
	tmi.translate[1] = 0.5 - 0.5 * sinValue - 0.5 * cosValue;

	RB_CalcTransformTexCoords( &tmi, st );
}






#if id386 && !( (defined __linux__ || defined __FreeBSD__ ) && (defined __i386__ ) ) // rb010123

long myftol( float f ) {
	static int tmp;
	__asm fld f
	__asm fistp tmp
	__asm mov eax, tmp
}

#endif

/*
** RB_CalcSpecularAlpha
**
** Calculates specular coefficient and places it in the alpha channel
*/
vec3_t lightOrigin = { -960, 1980, 96 };		// FIXME: track dynamically

void RB_CalcSpecularAlpha(unsigned char* alphas, float alphaMax, vec3_t lightOrigin) {
	int			i;
	float		*v, *normal;
	vec3_t		viewer,  reflected;
	float		l, d;
	int			b;
	vec3_t		lightDir;
	int			numVertexes;

	v = tess.xyz[0];
	normal = tess.normal[0];

	alphas += 3;

	numVertexes = tess.numVertexes;
	for (i = 0 ; i < numVertexes ; i++, v += 4, normal += 4, alphas += 4) {
		float ilength;

		VectorSubtract( lightOrigin, v, lightDir );
//		ilength = Q_rsqrt( DotProduct( lightDir, lightDir ) );
		VectorNormalizeFast( lightDir );

		// calculate the specular color
		d = DotProduct (normal, lightDir);
//		d *= ilength;

		// we don't optimize for the d < 0 case since this tends to
		// cause visual artifacts such as faceted "snapping"
		reflected[0] = normal[0]*2*d - lightDir[0];
		reflected[1] = normal[1]*2*d - lightDir[1];
		reflected[2] = normal[2]*2*d - lightDir[2];

		VectorSubtract (backEnd.ori.viewOrigin, v, viewer);
		ilength = Q_rsqrt( DotProduct( viewer, viewer ) );
		l = DotProduct (reflected, viewer);
		l *= ilength;

		if (l < 0) {
			b = 0;
		} else {
			l = l*l;
			l = l*l;
			b = l * 255;
			if (b > 255) {
				b = 255;
			}
		}

		*alphas = b;
	}
}

void RB_CalcLightGridColor(unsigned char* colors)
{
	int i;

	if (!backEnd.currentEntity) {
		for (i = 0; i < tess.numVertexes; i++) {
			colors[i * 4] = ((byte*)&backEnd.currentStaticModel->iGridLighting)[0];
			colors[i * 4 + 1] = ((byte*)&backEnd.currentStaticModel->iGridLighting)[1];
			colors[i * 4 + 2] = ((byte*)&backEnd.currentStaticModel->iGridLighting)[2];
			colors[i * 4 + 3] = ((byte*)&backEnd.currentStaticModel->iGridLighting)[3];
		}
	}
	else if (backEnd.currentEntity != &tr.worldEntity) {
		for (i = 0; i < tess.numVertexes; i++) {
			colors[i * 4] = ((byte*)&backEnd.currentEntity->iGridLighting)[0];
			colors[i * 4 + 1] = ((byte*)&backEnd.currentEntity->iGridLighting)[1];
			colors[i * 4 + 2] = ((byte*)&backEnd.currentEntity->iGridLighting)[2];
			colors[i * 4 + 3] = ((byte*)&backEnd.currentEntity->iGridLighting)[3];
		}
	}
	else {
		ri.Printf(PRINT_ALL, 
			"##### shader '%s' incorrectly uses rgbGen lightingGrid or lightingSpherical; was rgbGen vertex intended?\n",
			tess.shader->name);

		for (i = 0; i < tess.numVertexes; i++) {
			colors[i * 4] = 0xFF;
			colors[i * 4 + 1] = 0xFF;
			colors[i * 4 + 2] = 0xFF;
			colors[i * 4 + 3] = 0xFF;
		}
	}
}

void RB_CalcAlphaFromDotView(unsigned char* colors, float alphaMin, float alphaMax)
{
	int i;
	vec3_t viewInModel;

    for (i = 0; i < tess.numVertexes; i++, colors += 4) {
        union {
            float f;
            int i;
        } u;

		VectorCopy(tr.refdef.viewaxis[0], viewInModel);
		VectorNormalizeFast(viewInModel);

        u.f = DotProduct(viewInModel, tess.normal[i]);
        u.f = fabs(u.f);
        ri.Printf(PRINT_ALL, "normal: %f %f %f  dot: %f  i %d\n", tess.normal[i][0], tess.normal[i][1], tess.normal[i][2], u.f, i);

		u.f = ((alphaMax - alphaMin) * u.f + alphaMin) * 255.0;
        u.f = u.f - (float)(~((int)(u.f - 255.0) >> 31) & (int)(u.f - 255.0));

        u.i &= ~(u.i >> 31);
		colors[3] = u.i;
	}
}

void RB_CalcAlphaFromOneMinusDotView(unsigned char* colors, float alphaMin, float alphaMax)
{
	int i;
	vec3_t viewInModel;

    for (i = 0; i < tess.numVertexes; i++, colors += 4) {
        union {
            float f;
            int i;
        } u;

		VectorCopy(tr.refdef.viewaxis[0], viewInModel);
		VectorNormalizeFast(viewInModel);

        u.f = DotProduct(viewInModel, tess.normal[i]);
        u.f = fabs(u.f);
		u.f = 1.0 - u.f;
        ri.Printf(PRINT_ALL, "normal: %f %f %f  dot: %f  i %d\n", tess.normal[i][0], tess.normal[i][1], tess.normal[i][2], u.f, i);
        
        u.f = ((alphaMax - alphaMin) * u.f + alphaMin) * 255.0;
        u.f = u.f - (long double)(~((int)(u.f - 255.0) >> 31) & (int)(u.f - 255.0));

        u.i &= ~(u.i >> 31);
		colors[3] = u.i;
	}
}

/*
** RB_CalcDiffuseColor
**
** The basic vertex lighting calc
*/
void RB_CalcDiffuseColor( unsigned char *colors )
{
	int				i, j;
	float			*v, *normal;
	float			incoming;
	trRefEntity_t	*ent;
	int				ambientLightInt;
	vec3_t			ambientLight;
	vec3_t			lightDir;
	vec3_t			directedLight;
	int				numVertexes;
#if idppc_altivec
	vector unsigned char vSel = (vector unsigned char)(0x00, 0x00, 0x00, 0xff,
							   0x00, 0x00, 0x00, 0xff,
							   0x00, 0x00, 0x00, 0xff,
							   0x00, 0x00, 0x00, 0xff);
	vector float ambientLightVec;
	vector float directedLightVec;
	vector float lightDirVec;
	vector float normalVec0, normalVec1;
	vector float incomingVec0, incomingVec1, incomingVec2;
	vector float zero, jVec;
	vector signed int jVecInt;
	vector signed short jVecShort;
	vector unsigned char jVecChar, normalPerm;
#endif
	ent = backEnd.currentEntity;
	ambientLightInt = ent->ambientLightInt;
#if idppc_altivec
	// A lot of this could be simplified if we made sure
	// entities light info was 16-byte aligned.
	jVecChar = vec_lvsl(0, ent->ambientLight);
	ambientLightVec = vec_ld(0, (vector float *)ent->ambientLight);
	jVec = vec_ld(11, (vector float *)ent->ambientLight);
	ambientLightVec = vec_perm(ambientLightVec,jVec,jVecChar);

	jVecChar = vec_lvsl(0, ent->directedLight);
	directedLightVec = vec_ld(0,(vector float *)ent->directedLight);
	jVec = vec_ld(11,(vector float *)ent->directedLight);
	directedLightVec = vec_perm(directedLightVec,jVec,jVecChar);	 

	jVecChar = vec_lvsl(0, ent->lightDir);
	lightDirVec = vec_ld(0,(vector float *)ent->lightDir);
	jVec = vec_ld(11,(vector float *)ent->lightDir);
	lightDirVec = vec_perm(lightDirVec,jVec,jVecChar);	 

	zero = (vector float)vec_splat_s8(0);
	VectorCopy( ent->lightDir, lightDir );
#else
	VectorCopy( ent->ambientLight, ambientLight );
	VectorCopy( ent->directedLight, directedLight );
	VectorCopy( ent->lightDir, lightDir );
#endif

	v = tess.xyz[0];
	normal = tess.normal[0];

#if idppc_altivec
	normalPerm = vec_lvsl(0,normal);
#endif
	numVertexes = tess.numVertexes;
	for (i = 0 ; i < numVertexes ; i++, v += 4, normal += 4) {
#if idppc_altivec
		normalVec0 = vec_ld(0,(vector float *)normal);
		normalVec1 = vec_ld(11,(vector float *)normal);
		normalVec0 = vec_perm(normalVec0,normalVec1,normalPerm);
		incomingVec0 = vec_madd(normalVec0, lightDirVec, zero);
		incomingVec1 = vec_sld(incomingVec0,incomingVec0,4);
		incomingVec2 = vec_add(incomingVec0,incomingVec1);
		incomingVec1 = vec_sld(incomingVec1,incomingVec1,4);
		incomingVec2 = vec_add(incomingVec2,incomingVec1);
		incomingVec0 = vec_splat(incomingVec2,0);
		incomingVec0 = vec_max(incomingVec0,zero);
		normalPerm = vec_lvsl(12,normal);
		jVec = vec_madd(incomingVec0, directedLightVec, ambientLightVec);
		jVecInt = vec_cts(jVec,0);	// RGBx
		jVecShort = vec_pack(jVecInt,jVecInt);		// RGBxRGBx
		jVecChar = vec_packsu(jVecShort,jVecShort);	// RGBxRGBxRGBxRGBx
		jVecChar = vec_sel(jVecChar,vSel,vSel);		// RGBARGBARGBARGBA replace alpha with 255
		vec_ste((vector unsigned int)jVecChar,0,(unsigned int *)&colors[i*4]);	// store color
#else
		incoming = DotProduct (normal, lightDir);
		if ( incoming <= 0 ) {
			*(int *)&colors[i*4] = ambientLightInt;
			continue;
		} 
		j = myftol( ambientLight[0] + incoming * directedLight[0] );
		if ( j > 255 ) {
			j = 255;
		}
		colors[i*4+0] = j;

		j = myftol( ambientLight[1] + incoming * directedLight[1] );
		if ( j > 255 ) {
			j = 255;
		}
		colors[i*4+1] = j;

		j = myftol( ambientLight[2] + incoming * directedLight[2] );
		if ( j > 255 ) {
			j = 255;
		}
		colors[i*4+2] = j;

		colors[i*4+3] = 255;
#endif
	}
}

