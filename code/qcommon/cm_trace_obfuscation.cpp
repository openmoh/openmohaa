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

#include "cm_local.h"

char* CM_NextCsvToken(char** text, qboolean crossline) {
	// FIXME: unimplemented
	return NULL;
}

obfuscation_t* CM_SetupObfuscationMapping() {
	obfuscation_t* list;
	int i;
	
	list = (obfuscation_t*)Hunk_AllocateTempMemory(sizeof(obfuscation_t) * MAX_OBFUSCATIONS);
	for (i = 0; i < MAX_OBFUSCATIONS; i++) {
		list[i].name[0] = 0;
		list[i].heightDensity = 0;
		list[i].widthDensity = 0;
	}
	// FIXME: unimplemented
	return list;
}

void CM_ReleaseObfuscationMapping(obfuscation_t* obfuscation) {
	Hunk_FreeTempMemory(obfuscation);
}

void CM_ObfuscationForShader(obfuscation_t* list, const char* shaderName, float* widthDensity, float* heightDensity) {
	obfuscation_t* current;

	for (current = list; current->name[0]; current++) {
		if (!Q_stricmp(shaderName, current->name)) {
			*widthDensity = current->widthDensity;
			*heightDensity = current->heightDensity;
		}
	}

	Com_Printf("WARNING: using default obfuscation for shader %s\n", shaderName);
	*widthDensity = 1.f / 16.f;
	*heightDensity = 1.f / 1024.f;
}

/*
================
CM_ObfuscationTraceThroughBrush
================
*/
float CM_ObfuscationTraceThroughBrush( traceWork_t *tw, cbrush_t *brush ) {
	int			i;
	cplane_t	*plane, *clipplane, *clipplane2;
	float		dist;
	float		enterFrac, leaveFrac, leaveFrac2;
	float		d1, d2;
	qboolean	getout, startout;
	float		f;
	cbrushside_t	*side, *leadside, *leadside2;
	float		t;
	float		alpha;

	if( !brush->numsides ) {
		return tw->radius * cm.shaders[brush->shaderNum].obfuscationHeightDensity;
	}

	enterFrac = -1.0;
	leaveFrac = 1.0;
	clipplane = NULL;

	c_brush_traces++;

	getout = qfalse;
	startout = qfalse;

	leadside = NULL;
	//
	// compare the trace against all planes of the brush
	// find the latest time the trace crosses a plane towards the interior
	// and the earliest time the trace crosses a plane towards the exterior
	//
	for( i = 0; i < brush->numsides; i++ ) {
		side = brush->sides + i;
		plane = side->plane;

		// adjust the plane distance apropriately for mins/maxs
		dist = plane->dist - DotProduct( tw->offsets[ plane->signbits ], plane->normal );

		d1 = DotProduct( tw->start, plane->normal ) - dist;
		d2 = DotProduct( tw->end, plane->normal ) - dist;

		// if it doesn't cross the plane, the plane isn't relevent
		if( d1 >= 0 && d2 >= 0 ) {
			continue;
		}

		// crosses face
		if( d1 > d2 ) { // enter
			f = ( d1 - SURFACE_CLIP_EPSILON ) / ( d1 - d2 );
			if( f < 0 ) {
				f = 0;
			}
			if( f > enterFrac ) {
				enterFrac = f;
				clipplane = plane;
				leadside = side;
			}
		} else { // leave
			f = ( d1 + SURFACE_CLIP_EPSILON ) / ( d1 - d2 );
			if( f > 1 ) {
				f = 1;
			}
			if( f < leaveFrac ) {
				leaveFrac = f;
			}
		}
	}

	alpha = 1.f;
	// FIXME: unimplemented
	return alpha;
}

/*
================
CM_ObfuscationTraceToLeaf
================
*/
float CM_ObfuscationTraceToLeaf( traceWork_t *tw, cLeaf_t *leaf ) {
	int k;
	cbrush_t *b;
	float total;

	total = 0;
	// test box position against all brushes in the leaf
	for( k = 0; k<leaf->numLeafBrushes; k++ ) {
		b = &cm.brushes[ cm.leafbrushes[ leaf->firstLeafBrush + k ] ];
		if( b->checkcount == cm.checkcount ) {
			continue;	// already checked this brush in another leaf
		}
		b->checkcount = cm.checkcount;

		if( !( b->contents & CONTENTS_DONOTENTER ) ) {
			continue;
		}

		total += CM_ObfuscationTraceThroughBrush( tw, b );
	}

	return total;
}

/*
==================
CM_ObfuscationTraceThroughTree

Traverse all the contacted leafs from the start to the end position.
If the trace is a point, they will be exactly in order, but for larger
trace volumes it is possible to hit something in a later leaf with
a smaller intercept fraction.
==================
*/
float CM_ObfuscationTraceThroughTree( traceWork_t *tw, int num, float p1f, float p2f, vec3_t p1, vec3_t p2) {
	cNode_t		*node;
	cplane_t	*plane;
	float		t1, t2, offset;
	float		frac, frac2;
	float		idist;
	vec3_t		mid;
	int			side;
	float		midf;

	// FIXME: unimplemented

	// if < 0, we are in a leaf node
	if (num < 0) {
		return CM_ObfuscationTraceToLeaf( tw, &cm.leafs[-1-num] );
	}

	//
	// find the point distances to the seperating plane
	// and the offset for the size of the box
	//
	node = cm.nodes + num;
	plane = node->plane;

	// adjust the plane distance apropriately for mins/maxs
	if ( plane->type < 3 ) {
		t1 = p1[plane->type] - plane->dist;
		t2 = p2[plane->type] - plane->dist;
		offset = tw->extents[plane->type];
	} else {
		t1 = DotProduct (plane->normal, p1) - plane->dist;
		t2 = DotProduct (plane->normal, p2) - plane->dist;
		if ( tw->isPoint ) {
			offset = 0;
		} else {
			// this is silly
			offset = 2048;
		}
	}

	// see which sides we need to consider
	if ( t1 >= offset + 1 && t2 >= offset + 1 ) {
		return CM_ObfuscationTraceThroughTree( tw, node->children[0], p1f, p2f, p1, p2 );
	}
	if ( t1 < -offset - 1 && t2 < -offset - 1 ) {
		return CM_ObfuscationTraceThroughTree( tw, node->children[1], p1f, p2f, p1, p2 );
	}

	// put the crosspoint SURFACE_CLIP_EPSILON pixels on the near side
	if ( t1 < t2 ) {
		idist = 1.0/(t1-t2);
		side = 1;
		frac2 = (t1 + offset + SURFACE_CLIP_EPSILON)*idist;
		frac = (t1 - offset + SURFACE_CLIP_EPSILON)*idist;
	} else if (t1 > t2) {
		idist = 1.0/(t1-t2);
		side = 0;
		frac2 = (t1 - offset - SURFACE_CLIP_EPSILON)*idist;
		frac = (t1 + offset + SURFACE_CLIP_EPSILON)*idist;
	} else {
		side = 0;
		frac = 1;
		frac2 = 0;
	}

	// move up to the node
	if ( frac < 0 ) {
		frac = 0;
	}
	if ( frac > 1 ) {
		frac = 1;
	}

	midf = p1f + (p2f - p1f)*frac;

	mid[0] = p1[0] + frac*(p2[0] - p1[0]);
	mid[1] = p1[1] + frac*(p2[1] - p1[1]);
	mid[2] = p1[2] + frac*(p2[2] - p1[2]);

	CM_ObfuscationTraceThroughTree( tw, node->children[side], p1f, midf, p1, mid );


	// go past the node
	if ( frac2 < 0 ) {
		frac2 = 0;
	}
	if ( frac2 > 1 ) {
		frac2 = 1;
	}

	midf = p1f + (p2f - p1f)*frac2;

	mid[0] = p1[0] + frac2*(p2[0] - p1[0]);
	mid[1] = p1[1] + frac2*(p2[1] - p1[1]);
	mid[2] = p1[2] + frac2*(p2[2] - p1[2]);

	return CM_ObfuscationTraceThroughTree( tw, node->children[side^1], midf, p2f, mid, p2 );
}

float CM_ObfuscationTrace(const vec3_t start, const vec3_t end, clipHandle_t model) {
	// FIXME: unimplemented
	return 0.f;
}

float CM_VisualObfuscation(const vec3_t start, const vec3_t end) {	// FIXME: unimplemented
	return 0.f;
}
