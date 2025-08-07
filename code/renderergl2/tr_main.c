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
// tr_main.c -- main control flow for each frame

#include "tr_local.h"

#include <string.h> // memcpy

trGlobals_t		tr;

static float	s_flipMatrix[16] = {
	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	0, 0, -1, 0,
	-1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 1
};


refimport_t	ri;

// entities that will have procedurally generated surfaces will just
// point at this for their sorting surface
surfaceType_t	entitySurface = SF_ENTITY;

/*
================
R_CompareVert
================
*/
qboolean R_CompareVert(srfVert_t * v1, srfVert_t * v2, qboolean checkST)
{
	int             i;

	for(i = 0; i < 3; i++)
	{
		if(floor(v1->xyz[i] + 0.1) != floor(v2->xyz[i] + 0.1))
		{
			return qfalse;
		}

		if(checkST && ((v1->st[0] != v2->st[0]) || (v1->st[1] != v2->st[1])))
		{
			return qfalse;
		}
	}

	return qtrue;
}

/*
=============
R_CalcTexDirs

Lengyel, Eric. �Computing Tangent Space Basis Vectors for an Arbitrary Mesh�. Terathon Software 3D Graphics Library, 2001. http://www.terathon.com/code/tangent.html
=============
*/
void R_CalcTexDirs(vec3_t sdir, vec3_t tdir, const vec3_t v1, const vec3_t v2,
				   const vec3_t v3, const vec2_t w1, const vec2_t w2, const vec2_t w3)
{
	float			x1, x2, y1, y2, z1, z2;
	float			s1, s2, t1, t2, r;

	x1 = v2[0] - v1[0];
	x2 = v3[0] - v1[0];
	y1 = v2[1] - v1[1];
	y2 = v3[1] - v1[1];
	z1 = v2[2] - v1[2];
	z2 = v3[2] - v1[2];

	s1 = w2[0] - w1[0];
	s2 = w3[0] - w1[0];
	t1 = w2[1] - w1[1];
	t2 = w3[1] - w1[1];

	r = s1 * t2 - s2 * t1;
	if (r) r = 1.0f / r;

	VectorSet(sdir, (t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
	VectorSet(tdir, (s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
}

/*
=============
R_CalcTangentSpace

Lengyel, Eric. �Computing Tangent Space Basis Vectors for an Arbitrary Mesh�. Terathon Software 3D Graphics Library, 2001. http://www.terathon.com/code/tangent.html
=============
*/
vec_t R_CalcTangentSpace(vec3_t tangent, vec3_t bitangent, const vec3_t normal, const vec3_t sdir, const vec3_t tdir)
{
	vec3_t n_cross_t;
	vec_t n_dot_t, handedness;

	// Gram-Schmidt orthogonalize
	n_dot_t = DotProduct(normal, sdir);
	VectorMA(sdir, -n_dot_t, normal, tangent);
	VectorNormalize(tangent);

	// Calculate handedness
	CrossProduct(normal, sdir, n_cross_t);
	handedness = (DotProduct(n_cross_t, tdir) < 0.0f) ? -1.0f : 1.0f;

	// Calculate orthogonal bitangent, if necessary
	if (bitangent)
		CrossProduct(normal, tangent, bitangent);

	return handedness;
}

qboolean R_CalcTangentVectors(srfVert_t * dv[3])
{
	int             i;
	float           bb, s, t;
	vec3_t          bary;


	/* calculate barycentric basis for the triangle */
	bb = (dv[1]->st[0] - dv[0]->st[0]) * (dv[2]->st[1] - dv[0]->st[1]) - (dv[2]->st[0] - dv[0]->st[0]) * (dv[1]->st[1] - dv[0]->st[1]);
	if(fabs(bb) < 0.00000001f)
		return qfalse;

	/* do each vertex */
	for(i = 0; i < 3; i++)
	{
		vec4_t tangent;
		vec3_t normal, bitangent, nxt;

		// calculate s tangent vector
		s = dv[i]->st[0] + 10.0f;
		t = dv[i]->st[1];
		bary[0] = ((dv[1]->st[0] - s) * (dv[2]->st[1] - t) - (dv[2]->st[0] - s) * (dv[1]->st[1] - t)) / bb;
		bary[1] = ((dv[2]->st[0] - s) * (dv[0]->st[1] - t) - (dv[0]->st[0] - s) * (dv[2]->st[1] - t)) / bb;
		bary[2] = ((dv[0]->st[0] - s) * (dv[1]->st[1] - t) - (dv[1]->st[0] - s) * (dv[0]->st[1] - t)) / bb;

		tangent[0] = bary[0] * dv[0]->xyz[0] + bary[1] * dv[1]->xyz[0] + bary[2] * dv[2]->xyz[0];
		tangent[1] = bary[0] * dv[0]->xyz[1] + bary[1] * dv[1]->xyz[1] + bary[2] * dv[2]->xyz[1];
		tangent[2] = bary[0] * dv[0]->xyz[2] + bary[1] * dv[1]->xyz[2] + bary[2] * dv[2]->xyz[2];

		VectorSubtract(tangent, dv[i]->xyz, tangent);
		VectorNormalize(tangent);

		// calculate t tangent vector
		s = dv[i]->st[0];
		t = dv[i]->st[1] + 10.0f;
		bary[0] = ((dv[1]->st[0] - s) * (dv[2]->st[1] - t) - (dv[2]->st[0] - s) * (dv[1]->st[1] - t)) / bb;
		bary[1] = ((dv[2]->st[0] - s) * (dv[0]->st[1] - t) - (dv[0]->st[0] - s) * (dv[2]->st[1] - t)) / bb;
		bary[2] = ((dv[0]->st[0] - s) * (dv[1]->st[1] - t) - (dv[1]->st[0] - s) * (dv[0]->st[1] - t)) / bb;

		bitangent[0] = bary[0] * dv[0]->xyz[0] + bary[1] * dv[1]->xyz[0] + bary[2] * dv[2]->xyz[0];
		bitangent[1] = bary[0] * dv[0]->xyz[1] + bary[1] * dv[1]->xyz[1] + bary[2] * dv[2]->xyz[1];
		bitangent[2] = bary[0] * dv[0]->xyz[2] + bary[1] * dv[1]->xyz[2] + bary[2] * dv[2]->xyz[2];

		VectorSubtract(bitangent, dv[i]->xyz, bitangent);
		VectorNormalize(bitangent);

		// store bitangent handedness
		R_VaoUnpackNormal(normal, dv[i]->normal);
		CrossProduct(normal, tangent, nxt);
		tangent[3] = (DotProduct(nxt, bitangent) < 0.0f) ? -1.0f : 1.0f;

		R_VaoPackTangent(dv[i]->tangent, tangent);

		// debug code
		//% Sys_FPrintf( SYS_VRB, "%d S: (%f %f %f) T: (%f %f %f)\n", i,
		//%     stv[ i ][ 0 ], stv[ i ][ 1 ], stv[ i ][ 2 ], ttv[ i ][ 0 ], ttv[ i ][ 1 ], ttv[ i ][ 2 ] );
	}

	return qtrue;
}


/*
=================
R_CullLocalBox

Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
int R_CullLocalBox(vec3_t localBounds[2]) {
#if 0
	int		i, j;
	vec3_t	transformed[8];
	float	dists[8];
	vec3_t	v;
	cplane_t	*frust;
	int			anyBack;
	int			front, back;

	if ( r_nocull->integer ) {
		return CULL_CLIP;
	}

	// transform into world space
	for (i = 0 ; i < 8 ; i++) {
		v[0] = bounds[i&1][0];
		v[1] = bounds[(i>>1)&1][1];
		v[2] = bounds[(i>>2)&1][2];

		VectorCopy( tr.ori.origin, transformed[i] );
		VectorMA( transformed[i], v[0], tr.ori.axis[0], transformed[i] );
		VectorMA( transformed[i], v[1], tr.ori.axis[1], transformed[i] );
		VectorMA( transformed[i], v[2], tr.ori.axis[2], transformed[i] );
	}

	// check against frustum planes
	anyBack = 0;
	for (i = 0 ; i < 4 ; i++) {
		frust = &tr.viewParms.frustum[i];

		front = back = 0;
		for (j = 0 ; j < 8 ; j++) {
			dists[j] = DotProduct(transformed[j], frust->normal);
			if ( dists[j] > frust->dist ) {
				front = 1;
				if ( back ) {
					break;		// a point is in front
				}
			} else {
				back = 1;
			}
		}
		if ( !front ) {
			// all points were behind one of the planes
			return CULL_OUT;
		}
		anyBack |= back;
	}

	if ( !anyBack ) {
		return CULL_IN;		// completely inside frustum
	}

	return CULL_CLIP;		// partially clipped
#else
	int             j;
	vec3_t          transformed;
	vec3_t          v;
	vec3_t          worldBounds[2];

	if(r_nocull->integer)
	{
		return CULL_CLIP;
	}

	// transform into world space
	ClearBounds(worldBounds[0], worldBounds[1]);

	for(j = 0; j < 8; j++)
	{
		v[0] = localBounds[j & 1][0];
		v[1] = localBounds[(j >> 1) & 1][1];
		v[2] = localBounds[(j >> 2) & 1][2];

		R_LocalPointToWorld(v, transformed);

		AddPointToBounds(transformed, worldBounds[0], worldBounds[1]);
	}

	return R_CullBox(worldBounds);
#endif
}

/*
=================
R_CullBox

Returns CULL_IN, CULL_CLIP, or CULL_OUT
=================
*/
int R_CullBox(vec3_t worldBounds[2]) {
	int             i;
	cplane_t       *frust;
	qboolean        anyClip;
	int             r, numPlanes;

	numPlanes = (tr.viewParms.flags & VPF_FARPLANEFRUSTUM) ? 5 : 4;

	// check against frustum planes
	anyClip = qfalse;
	for(i = 0; i < numPlanes; i++)
	{
		frust = &tr.viewParms.frustum[i];

		r = BoxOnPlaneSide(worldBounds[0], worldBounds[1], frust);

		if(r == 2)
		{
			// completely outside frustum
			return CULL_OUT;
		}
		if(r == 3)
		{
			anyClip = qtrue;
		}
	}

	if(!anyClip)
	{
		// completely inside frustum
		return CULL_IN;
	}

	// partially clipped
	return CULL_CLIP;
}

/*
** R_CullLocalPointAndRadius
*/
int R_CullLocalPointAndRadius( const vec3_t pt, float radius )
{
	vec3_t transformed;

	R_LocalPointToWorld( pt, transformed );

	return R_CullPointAndRadius( transformed, radius );
}

/*
** R_CullPointAndRadius
*/
int R_CullPointAndRadiusEx( const vec3_t pt, float radius, const cplane_t* frustum, int numPlanes )
{
	int		i;
	float	dist;
	const cplane_t	*frust;
	qboolean mightBeClipped = qfalse;

	if ( r_nocull->integer ) {
		return CULL_CLIP;
	}

	// check against frustum planes
	for (i = 0 ; i < numPlanes ; i++) 
	{
		frust = &frustum[i];

		dist = DotProduct( pt, frust->normal) - frust->dist;
		if ( dist < -radius )
		{
			return CULL_OUT;
		}
		else if ( dist <= radius ) 
		{
			mightBeClipped = qtrue;
		}
	}

	if ( mightBeClipped )
	{
		return CULL_CLIP;
	}

	return CULL_IN;		// completely inside frustum
}

/*
** R_CullPointAndRadius
*/
int R_CullPointAndRadius( const vec3_t pt, float radius )
{
	return R_CullPointAndRadiusEx(pt, radius, tr.viewParms.frustum, (tr.viewParms.flags & VPF_FARPLANEFRUSTUM) ? 5 : 4);
}

/*
=================
R_LocalNormalToWorld

=================
*/
void R_LocalNormalToWorld (const vec3_t local, vec3_t world) {
	world[0] = local[0] * tr.ori.axis[0][0] + local[1] * tr.ori.axis[1][0] + local[2] * tr.ori.axis[2][0];
	world[1] = local[0] * tr.ori.axis[0][1] + local[1] * tr.ori.axis[1][1] + local[2] * tr.ori.axis[2][1];
	world[2] = local[0] * tr.ori.axis[0][2] + local[1] * tr.ori.axis[1][2] + local[2] * tr.ori.axis[2][2];
}

/*
=================
R_LocalPointToWorld

=================
*/
void R_LocalPointToWorld (const vec3_t local, vec3_t world) {
	world[0] = local[0] * tr.ori.axis[0][0] + local[1] * tr.ori.axis[1][0] + local[2] * tr.ori.axis[2][0] + tr.ori.origin[0];
	world[1] = local[0] * tr.ori.axis[0][1] + local[1] * tr.ori.axis[1][1] + local[2] * tr.ori.axis[2][1] + tr.ori.origin[1];
	world[2] = local[0] * tr.ori.axis[0][2] + local[1] * tr.ori.axis[1][2] + local[2] * tr.ori.axis[2][2] + tr.ori.origin[2];
}

/*
=================
R_WorldToLocal

=================
*/
void R_WorldToLocal (const vec3_t world, vec3_t local) {
	local[0] = DotProduct(world, tr.ori.axis[0]);
	local[1] = DotProduct(world, tr.ori.axis[1]);
	local[2] = DotProduct(world, tr.ori.axis[2]);
}

/*
==========================
R_TransformModelToClip

==========================
*/
void R_TransformModelToClip( const vec3_t src, const float *modelMatrix, const float *projectionMatrix,
							vec4_t eye, vec4_t dst ) {
	int i;

	for ( i = 0 ; i < 4 ; i++ ) {
		eye[i] = 
			src[0] * modelMatrix[ i + 0 * 4 ] +
			src[1] * modelMatrix[ i + 1 * 4 ] +
			src[2] * modelMatrix[ i + 2 * 4 ] +
			1 * modelMatrix[ i + 3 * 4 ];
	}

	for ( i = 0 ; i < 4 ; i++ ) {
		dst[i] = 
			eye[0] * projectionMatrix[ i + 0 * 4 ] +
			eye[1] * projectionMatrix[ i + 1 * 4 ] +
			eye[2] * projectionMatrix[ i + 2 * 4 ] +
			eye[3] * projectionMatrix[ i + 3 * 4 ];
	}
}

/*
==========================
R_TransformClipToWindow

==========================
*/
void R_TransformClipToWindow( const vec4_t clip, const viewParms_t *view, vec4_t normalized, vec4_t window ) {
	normalized[0] = clip[0] / clip[3];
	normalized[1] = clip[1] / clip[3];
	normalized[2] = ( clip[2] + clip[3] ) / ( 2 * clip[3] );

	window[0] = 0.5f * ( 1.0f + normalized[0] ) * view->viewportWidth;
	window[1] = 0.5f * ( 1.0f + normalized[1] ) * view->viewportHeight;
	window[2] = normalized[2];

	window[0] = (int) ( window[0] + 0.5 );
	window[1] = (int) ( window[1] + 0.5 );
}


/*
==========================
myGlMultMatrix

==========================
*/
void myGlMultMatrix( const float *a, const float *b, float *out ) {
	int		i, j;

	for ( i = 0 ; i < 4 ; i++ ) {
		for ( j = 0 ; j < 4 ; j++ ) {
			out[ i * 4 + j ] =
				a [ i * 4 + 0 ] * b [ 0 * 4 + j ]
				+ a [ i * 4 + 1 ] * b [ 1 * 4 + j ]
				+ a [ i * 4 + 2 ] * b [ 2 * 4 + j ]
				+ a [ i * 4 + 3 ] * b [ 3 * 4 + j ];
		}
	}
}

/*
=================
R_RotateForEntity

Generates an orientation for an entity and viewParms
Does NOT produce any GL calls
Called by both the front end and the back end
=================
*/
void R_RotateForEntity( const trRefEntity_t *ent, const viewParms_t *viewParms,
					   orientationr_t *or ) {
	float	glMatrix[16];
	vec3_t	delta;
	float	axisLength;

	if ( ent->e.reType != RT_MODEL ) {
		*or = viewParms->world;
		return;
	}

	VectorCopy( ent->e.origin, or->origin );

	VectorCopy( ent->e.axis[0], or->axis[0] );
	VectorCopy( ent->e.axis[1], or->axis[1] );
	VectorCopy( ent->e.axis[2], or->axis[2] );

	glMatrix[0] = or->axis[0][0];
	glMatrix[4] = or->axis[1][0];
	glMatrix[8] = or->axis[2][0];
	glMatrix[12] = or->origin[0];

	glMatrix[1] = or->axis[0][1];
	glMatrix[5] = or->axis[1][1];
	glMatrix[9] = or->axis[2][1];
	glMatrix[13] = or->origin[1];

	glMatrix[2] = or->axis[0][2];
	glMatrix[6] = or->axis[1][2];
	glMatrix[10] = or->axis[2][2];
	glMatrix[14] = or->origin[2];

	glMatrix[3] = 0;
	glMatrix[7] = 0;
	glMatrix[11] = 0;
	glMatrix[15] = 1;

	Mat4Copy(glMatrix, or->transformMatrix);
	myGlMultMatrix( glMatrix, viewParms->world.modelMatrix, or->modelMatrix );

	// calculate the viewer origin in the model's space
	// needed for fog, specular, and environment mapping
	VectorSubtract( viewParms->ori.origin, or->origin, delta );

	// compensate for scale in the axes if necessary
	if ( ent->e.nonNormalizedAxes ) {
		axisLength = VectorLength( ent->e.axis[0] );
		if ( !axisLength ) {
			axisLength = 0;
		} else {
			axisLength = 1.0f / axisLength;
		}
	} else {
		axisLength = 1.0f;
	}

	or->viewOrigin[0] = DotProduct( delta, or->axis[0] ) * axisLength;
	or->viewOrigin[1] = DotProduct( delta, or->axis[1] ) * axisLength;
	or->viewOrigin[2] = DotProduct( delta, or->axis[2] ) * axisLength;
}

/*
=================
R_RotateForViewer

Sets up the modelview matrix for a given viewParm
=================
*/
void R_RotateForViewer (void) 
{
	float	viewerMatrix[16];
	vec3_t	origin;

	Com_Memset (&tr.ori, 0, sizeof(tr.ori));
	tr.ori.axis[0][0] = 1;
	tr.ori.axis[1][1] = 1;
	tr.ori.axis[2][2] = 1;
	VectorCopy (tr.viewParms.ori.origin, tr.ori.viewOrigin);

	// transform by the camera placement
	VectorCopy( tr.viewParms.ori.origin, origin );

	viewerMatrix[0] = tr.viewParms.ori.axis[0][0];
	viewerMatrix[4] = tr.viewParms.ori.axis[0][1];
	viewerMatrix[8] = tr.viewParms.ori.axis[0][2];
	viewerMatrix[12] = -origin[0] * viewerMatrix[0] + -origin[1] * viewerMatrix[4] + -origin[2] * viewerMatrix[8];

	viewerMatrix[1] = tr.viewParms.ori.axis[1][0];
	viewerMatrix[5] = tr.viewParms.ori.axis[1][1];
	viewerMatrix[9] = tr.viewParms.ori.axis[1][2];
	viewerMatrix[13] = -origin[0] * viewerMatrix[1] + -origin[1] * viewerMatrix[5] + -origin[2] * viewerMatrix[9];

	viewerMatrix[2] = tr.viewParms.ori.axis[2][0];
	viewerMatrix[6] = tr.viewParms.ori.axis[2][1];
	viewerMatrix[10] = tr.viewParms.ori.axis[2][2];
	viewerMatrix[14] = -origin[0] * viewerMatrix[2] + -origin[1] * viewerMatrix[6] + -origin[2] * viewerMatrix[10];

	viewerMatrix[3] = 0;
	viewerMatrix[7] = 0;
	viewerMatrix[11] = 0;
	viewerMatrix[15] = 1;

	// convert from our coordinate system (looking down X)
	// to OpenGL's coordinate system (looking down -Z)
	myGlMultMatrix( viewerMatrix, s_flipMatrix, tr.ori.modelMatrix );

	tr.viewParms.world = tr.ori;

}

/*
** SetFarClip
*/
static void R_SetFarClip( void )
{
	float	farthestCornerDistance = 0;
	int		i;

	// if not rendering the world (icons, menus, etc)
	// set a 2k far clip plane
	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		tr.viewParms.zFar = 2048;
		return;
	}

	//
	// set far clipping planes dynamically
	//
	farthestCornerDistance = 0;
	for ( i = 0; i < 8; i++ )
	{
		vec3_t v;
		vec3_t vecTo;
		float distance;

		if ( i & 1 )
		{
			v[0] = tr.viewParms.visBounds[0][0];
		}
		else
		{
			v[0] = tr.viewParms.visBounds[1][0];
		}

		if ( i & 2 )
		{
			v[1] = tr.viewParms.visBounds[0][1];
		}
		else
		{
			v[1] = tr.viewParms.visBounds[1][1];
		}

		if ( i & 4 )
		{
			v[2] = tr.viewParms.visBounds[0][2];
		}
		else
		{
			v[2] = tr.viewParms.visBounds[1][2];
		}

		VectorSubtract( v, tr.viewParms.ori.origin, vecTo );

		distance = vecTo[0] * vecTo[0] + vecTo[1] * vecTo[1] + vecTo[2] * vecTo[2];

		if ( distance > farthestCornerDistance )
		{
			farthestCornerDistance = distance;
		}
	}
	tr.viewParms.zFar = sqrt( farthestCornerDistance );
}

/*
=================
R_SetupFrustum

Set up the culling frustum planes for the current view using the results we got from computing the first two rows of
the projection matrix.
=================
*/
void R_SetupFrustum (viewParms_t *dest, float xmin, float xmax, float ymax, float zProj, float zFar, float stereoSep)
{
	vec3_t ofsorigin;
	float oppleg, adjleg, length;
	int i;
	
	if(stereoSep == 0 && xmin == -xmax)
	{
		// symmetric case can be simplified
		VectorCopy(dest->ori.origin, ofsorigin);

		length = sqrt(xmax * xmax + zProj * zProj);
		oppleg = xmax / length;
		adjleg = zProj / length;

		VectorScale(dest->ori.axis[0], oppleg, dest->frustum[0].normal);
		VectorMA(dest->frustum[0].normal, adjleg, dest->ori.axis[1], dest->frustum[0].normal);

		VectorScale(dest->ori.axis[0], oppleg, dest->frustum[1].normal);
		VectorMA(dest->frustum[1].normal, -adjleg, dest->ori.axis[1], dest->frustum[1].normal);
	}
	else
	{
		// In stereo rendering, due to the modification of the projection matrix, dest->ori.origin is not the
		// actual origin that we're rendering so offset the tip of the view pyramid.
		VectorMA(dest->ori.origin, stereoSep, dest->ori.axis[1], ofsorigin);
	
		oppleg = xmax + stereoSep;
		length = sqrt(oppleg * oppleg + zProj * zProj);
		VectorScale(dest->ori.axis[0], oppleg / length, dest->frustum[0].normal);
		VectorMA(dest->frustum[0].normal, zProj / length, dest->ori.axis[1], dest->frustum[0].normal);

		oppleg = xmin + stereoSep;
		length = sqrt(oppleg * oppleg + zProj * zProj);
		VectorScale(dest->ori.axis[0], -oppleg / length, dest->frustum[1].normal);
		VectorMA(dest->frustum[1].normal, -zProj / length, dest->ori.axis[1], dest->frustum[1].normal);
	}

	length = sqrt(ymax * ymax + zProj * zProj);
	oppleg = ymax / length;
	adjleg = zProj / length;

	VectorScale(dest->ori.axis[0], oppleg, dest->frustum[2].normal);
	VectorMA(dest->frustum[2].normal, adjleg, dest->ori.axis[2], dest->frustum[2].normal);

	VectorScale(dest->ori.axis[0], oppleg, dest->frustum[3].normal);
	VectorMA(dest->frustum[3].normal, -adjleg, dest->ori.axis[2], dest->frustum[3].normal);
	
	for (i=0 ; i<4 ; i++) {
		dest->frustum[i].type = PLANE_NON_AXIAL;
		dest->frustum[i].dist = DotProduct (ofsorigin, dest->frustum[i].normal);
		SetPlaneSignbits( &dest->frustum[i] );
	}

	if (zFar != 0.0f)
	{
		vec3_t farpoint;

		VectorMA(ofsorigin, zFar, dest->ori.axis[0], farpoint);
		VectorScale(dest->ori.axis[0], -1.0f, dest->frustum[4].normal);

		dest->frustum[4].type = PLANE_NON_AXIAL;
		dest->frustum[4].dist = DotProduct (farpoint, dest->frustum[4].normal);
		SetPlaneSignbits( &dest->frustum[4] );
		dest->flags |= VPF_FARPLANEFRUSTUM;
	}

	//
	// OPENMOHAA-specific stuff
	//=========================
	
	if (dest->isPortalSky) {
		// since 2.0: skybox farplane
		if (r_skybox_farplane->integer) {
			dest->farplane_distance = r_skybox_farplane->value;

            sscanf(
                r_farplane_color->string,
                "%f %f %f",
                &dest->farplane_color[0],
                &dest->farplane_color[1],
                &dest->farplane_color[2]
			);

			if (r_farplane_nocull->integer > 0) {
				dest->farplane_cull = 0;
			} else if (r_farplane_nocull->integer == 0) {
				dest->farplane_cull = 1;
            } else {
                dest->farplane_cull = 2;
            }
		}
	}
	else if (r_farplane->integer)
	{
		dest->farplane_distance = r_farplane->value;
		dest->farplane_bias = r_farplane_bias->value;

		sscanf(
			r_farplane_color->string,
			"%f %f %f",
			&dest->farplane_color[0],
			&dest->farplane_color[1],
			&dest->farplane_color[2]);

			if (r_farplane_nocull->integer > 0) {
				dest->farplane_cull = 0;
			} else if (r_farplane_nocull->integer == 0) {
				dest->farplane_cull = 1;
            } else {
                dest->farplane_cull = 2;
            }
	}

	if (dest->farplane_distance)
	{
		vec3_t farPoint;
		float realPlaneLen;
		float tmp;
		vec4_t fogColor;

		dest->fog.len = dest->farplane_distance;
		dest->fog.oolen = 1.0 / dest->farplane_distance;
		VectorMA(dest->ori.origin, dest->farplane_distance, dest->ori.axis[0], farPoint);

		VectorNegate(dest->ori.axis[0], dest->frustum[4].normal);
		dest->frustum[4].type = PLANE_NON_AXIAL;
		dest->frustum[4].dist = DotProduct(farPoint, dest->frustum[4].normal);
		SetPlaneSignbits(&dest->frustum[4]);

		dest->fog.enabled = r_farplane_nofog->integer == 0;
		if (dest->farplane_cull) {
            // extra fustum for culling
            dest->flags |= VPF_FARPLANEFRUSTUM;
			dest->fog.extrafrustums = 1;
		} else {
			dest->fog.extrafrustums = 0;
		}
	}
	else
	{
		dest->fog.enabled = 0;
		dest->fog.extrafrustums = 0;
    }

    //=========================
}

/*
===============
R_SetupProjection
===============
*/
void R_SetupProjection(viewParms_t *dest, float zProj, float zFar, qboolean computeFrustum)
{
	float	xmin, xmax, ymin, ymax;
	float	width, height, stereoSep = r_stereoSeparation->value;

	/*
	 * offset the view origin of the viewer for stereo rendering 
	 * by setting the projection matrix appropriately.
	 */

	if(stereoSep != 0)
	{
		if(dest->stereoFrame == STEREO_LEFT)
			stereoSep = zProj / stereoSep;
		else if(dest->stereoFrame == STEREO_RIGHT)
			stereoSep = zProj / -stereoSep;
		else
			stereoSep = 0;
	}

	ymax = zProj * tan(dest->fovY * M_PI / 360.0f);
	ymin = -ymax;

	xmax = zProj * tan(dest->fovX * M_PI / 360.0f);
	xmin = -xmax;

	width = xmax - xmin;
	height = ymax - ymin;
	
	dest->projectionMatrix[0] = 2 * zProj / width;
	dest->projectionMatrix[4] = 0;
	dest->projectionMatrix[8] = (xmax + xmin + 2 * stereoSep) / width;
	dest->projectionMatrix[12] = 2 * zProj * stereoSep / width;

	dest->projectionMatrix[1] = 0;
	dest->projectionMatrix[5] = 2 * zProj / height;
	dest->projectionMatrix[9] = ( ymax + ymin ) / height;	// normally 0
	dest->projectionMatrix[13] = 0;

	dest->projectionMatrix[3] = 0;
	dest->projectionMatrix[7] = 0;
	dest->projectionMatrix[11] = -1;
	dest->projectionMatrix[15] = 0;
	
	// Now that we have all the data for the projection matrix we can also setup the view frustum.
	if(computeFrustum)
		R_SetupFrustum(dest, xmin, xmax, ymax, zProj, zFar, stereoSep);
}

/*
===============
R_SetupProjectionZ

Sets the z-component transformation part in the projection matrix
===============
*/
void R_SetupProjectionZ(viewParms_t *dest)
{
	float zNear, zFar, depth;
	
	zNear = r_znear->value;
	zFar	= dest->zFar;

	depth	= zFar - zNear;

	dest->projectionMatrix[2] = 0;
	dest->projectionMatrix[6] = 0;
	dest->projectionMatrix[10] = -( zFar + zNear ) / depth;
	dest->projectionMatrix[14] = -2 * zFar * zNear / depth;

	if (dest->isPortal)
	{
		float	plane[4];
		float	plane2[4];
		vec4_t q, c;

		// transform portal plane into camera space
		plane[0] = dest->portalPlane.normal[0];
		plane[1] = dest->portalPlane.normal[1];
		plane[2] = dest->portalPlane.normal[2];
		plane[3] = dest->portalPlane.dist;

		plane2[0] = -DotProduct (dest->ori.axis[1], plane);
		plane2[1] = DotProduct (dest->ori.axis[2], plane);
		plane2[2] = -DotProduct (dest->ori.axis[0], plane);
		plane2[3] = DotProduct (plane, dest->ori.origin) - plane[3];

		// Lengyel, Eric. "Modifying the Projection Matrix to Perform Oblique Near-plane Clipping".
		// Terathon Software 3D Graphics Library, 2004. http://www.terathon.com/code/oblique.html
		q[0] = (SGN(plane2[0]) + dest->projectionMatrix[8]) / dest->projectionMatrix[0];
		q[1] = (SGN(plane2[1]) + dest->projectionMatrix[9]) / dest->projectionMatrix[5];
		q[2] = -1.0f;
		q[3] = (1.0f + dest->projectionMatrix[10]) / dest->projectionMatrix[14];

		VectorScale4(plane2, 2.0f / DotProduct4(plane2, q), c);

		dest->projectionMatrix[2]  = c[0];
		dest->projectionMatrix[6]  = c[1];
		dest->projectionMatrix[10] = c[2] + 1.0f;
		dest->projectionMatrix[14] = c[3];

	}

}

/*
===============
R_SetupProjectionOrtho
===============
*/
void R_SetupProjectionOrtho(viewParms_t *dest, vec3_t viewBounds[2])
{
	float xmin, xmax, ymin, ymax, znear, zfar;
	//viewParms_t *dest = &tr.viewParms;
	int i;
	vec3_t pop;

	// Quake3:   Projection:
	//
	//    Z  X   Y  Z
	//    | /    | /
	//    |/     |/
	// Y--+      +--X

	xmin  =  viewBounds[0][1];
	xmax  =  viewBounds[1][1];
	ymin  = -viewBounds[1][2];
	ymax  = -viewBounds[0][2];
	znear =  viewBounds[0][0];
	zfar  =  viewBounds[1][0];

	dest->projectionMatrix[0]  = 2 / (xmax - xmin);
	dest->projectionMatrix[4]  = 0;
	dest->projectionMatrix[8]  = 0;
	dest->projectionMatrix[12] = (xmax + xmin) / (xmax - xmin);

	dest->projectionMatrix[1]  = 0;
	dest->projectionMatrix[5]  = 2 / (ymax - ymin);
	dest->projectionMatrix[9]  = 0;
	dest->projectionMatrix[13] = (ymax + ymin) / (ymax - ymin);

	dest->projectionMatrix[2]  = 0;
	dest->projectionMatrix[6]  = 0;
	dest->projectionMatrix[10] = -2 / (zfar - znear);
	dest->projectionMatrix[14] = -(zfar + znear) / (zfar - znear);

	dest->projectionMatrix[3]  = 0;
	dest->projectionMatrix[7]  = 0;
	dest->projectionMatrix[11] = 0;
	dest->projectionMatrix[15] = 1;

	VectorScale(dest->ori.axis[1],  1.0f, dest->frustum[0].normal);
	VectorMA(dest->ori.origin, viewBounds[0][1], dest->frustum[0].normal, pop);
	dest->frustum[0].dist = DotProduct(pop, dest->frustum[0].normal);

	VectorScale(dest->ori.axis[1], -1.0f, dest->frustum[1].normal);
	VectorMA(dest->ori.origin, -viewBounds[1][1], dest->frustum[1].normal, pop);
	dest->frustum[1].dist = DotProduct(pop, dest->frustum[1].normal);

	VectorScale(dest->ori.axis[2],  1.0f, dest->frustum[2].normal);
	VectorMA(dest->ori.origin, viewBounds[0][2], dest->frustum[2].normal, pop);
	dest->frustum[2].dist = DotProduct(pop, dest->frustum[2].normal);

	VectorScale(dest->ori.axis[2], -1.0f, dest->frustum[3].normal);
	VectorMA(dest->ori.origin, -viewBounds[1][2], dest->frustum[3].normal, pop);
	dest->frustum[3].dist = DotProduct(pop, dest->frustum[3].normal);

	VectorScale(dest->ori.axis[0], -1.0f, dest->frustum[4].normal);
	VectorMA(dest->ori.origin, -viewBounds[1][0], dest->frustum[4].normal, pop);
	dest->frustum[4].dist = DotProduct(pop, dest->frustum[4].normal);
	
	for (i = 0; i < 5; i++)
	{
		dest->frustum[i].type = PLANE_NON_AXIAL;
		SetPlaneSignbits (&dest->frustum[i]);
	}

	dest->flags |= VPF_FARPLANEFRUSTUM;
}

/*
=================
R_MirrorPoint
=================
*/
void R_MirrorPoint (vec3_t in, orientation_t *surface, orientation_t *camera, vec3_t out) {
	int		i;
	vec3_t	local;
	vec3_t	transformed;
	float	d;

	VectorSubtract( in, surface->origin, local );

	VectorClear( transformed );
	for ( i = 0 ; i < 3 ; i++ ) {
		d = DotProduct(local, surface->axis[i]);
		VectorMA( transformed, d, camera->axis[i], transformed );
	}

	VectorAdd( transformed, camera->origin, out );
}

void R_MirrorVector (vec3_t in, orientation_t *surface, orientation_t *camera, vec3_t out) {
	int		i;
	float	d;

	VectorClear( out );
	for ( i = 0 ; i < 3 ; i++ ) {
		d = DotProduct(in, surface->axis[i]);
		VectorMA( out, d, camera->axis[i], out );
	}
}


/*
=============
R_PlaneForSurface
=============
*/
void R_PlaneForSurface (surfaceType_t *surfType, cplane_t *plane) {
	srfBspSurface_t	*tri;
	srfPoly_t		*poly;
	srfVert_t		*v1, *v2, *v3;
	vec4_t			plane4;

	if (!surfType) {
		Com_Memset (plane, 0, sizeof(*plane));
		plane->normal[0] = 1;
		return;
	}
	switch (*surfType) {
	case SF_FACE:
		*plane = ((srfBspSurface_t *)surfType)->cullPlane;
		return;
	case SF_TRIANGLES:
		tri = (srfBspSurface_t *)surfType;
		v1 = tri->verts + tri->indexes[0];
		v2 = tri->verts + tri->indexes[1];
		v3 = tri->verts + tri->indexes[2];
		PlaneFromPoints( plane4, v1->xyz, v2->xyz, v3->xyz );
		VectorCopy( plane4, plane->normal ); 
		plane->dist = plane4[3];
		return;
	case SF_POLY:
		poly = (srfPoly_t *)surfType;
		PlaneFromPoints( plane4, poly->verts[0].xyz, poly->verts[1].xyz, poly->verts[2].xyz );
		VectorCopy( plane4, plane->normal ); 
		plane->dist = plane4[3];
		return;
	default:
		Com_Memset (plane, 0, sizeof(*plane));
		plane->normal[0] = 1;		
		return;
	}
}

/*
=================
R_GetPortalOrientation

entityNum is the entity that the portal surface is a part of, which may
be moving and rotating.

Returns qtrue if it should be mirrored
=================
*/
qboolean R_GetPortalOrientations( drawSurf_t *drawSurf, int entityNum, 
							 orientation_t *surface, orientation_t *camera,
							 vec3_t pvsOrigin, qboolean *mirror ) {
	int			i;
	cplane_t	originalPlane, plane;
	trRefEntity_t	*e;
	float		d;
	vec3_t		transformed;

	// create plane axis for the portal we are seeing
	R_PlaneForSurface( drawSurf->surface, &originalPlane );

	// rotate the plane if necessary
	if ( entityNum != REFENTITYNUM_WORLD ) {
		tr.currentEntityNum = entityNum;
		tr.currentEntity = &tr.refdef.entities[entityNum];

		// get the orientation of the entity
		R_RotateForEntity( tr.currentEntity, &tr.viewParms, &tr.ori );

		// rotate the plane, but keep the non-rotated version for matching
		// against the portalSurface entities
		R_LocalNormalToWorld( originalPlane.normal, plane.normal );
		plane.dist = originalPlane.dist + DotProduct( plane.normal, tr.ori.origin );

		// translate the original plane
		originalPlane.dist = originalPlane.dist + DotProduct( originalPlane.normal, tr.ori.origin );
	} else {
		plane = originalPlane;
	}

	VectorCopy( plane.normal, surface->axis[0] );
	PerpendicularVector( surface->axis[1], surface->axis[0] );
	CrossProduct( surface->axis[0], surface->axis[1], surface->axis[2] );

	// locate the portal entity closest to this plane.
	// origin will be the origin of the portal, origin2 will be
	// the origin of the camera
	for ( i = 0 ; i < tr.refdef.num_entities ; i++ ) {
		e = &tr.refdef.entities[i];
		if ( e->e.reType != RT_PORTALSURFACE ) {
			continue;
		}

		d = DotProduct( e->e.origin, originalPlane.normal ) - originalPlane.dist;
		if ( d > 64 || d < -64) {
			continue;
		}

		// get the pvsOrigin from the entity
		VectorCopy( e->e.oldorigin, pvsOrigin );

		// if the entity is just a mirror, don't use as a camera point
		if ( e->e.oldorigin[0] == e->e.origin[0] && 
			e->e.oldorigin[1] == e->e.origin[1] && 
			e->e.oldorigin[2] == e->e.origin[2] ) {
			VectorScale( plane.normal, plane.dist, surface->origin );
			VectorCopy( surface->origin, camera->origin );
			VectorSubtract( vec3_origin, surface->axis[0], camera->axis[0] );
			VectorCopy( surface->axis[1], camera->axis[1] );
			VectorCopy( surface->axis[2], camera->axis[2] );

			*mirror = qtrue;
			return qtrue;
		}

		// project the origin onto the surface plane to get
		// an origin point we can rotate around
		d = DotProduct( e->e.origin, plane.normal ) - plane.dist;
		VectorMA( e->e.origin, -d, surface->axis[0], surface->origin );
			
		// now get the camera origin and orientation
		VectorCopy( e->e.oldorigin, camera->origin );
		AxisCopy( e->e.axis, camera->axis );
		VectorSubtract( vec3_origin, camera->axis[0], camera->axis[0] );
		VectorSubtract( vec3_origin, camera->axis[1], camera->axis[1] );

		// optionally rotate
		if ( e->e.oldframe ) {
			// if a speed is specified
			if ( e->e.frame ) {
				// continuous rotate
				d = (tr.refdef.time/1000.0f) * e->e.frame;
				VectorCopy( camera->axis[1], transformed );
				RotatePointAroundVector( camera->axis[1], camera->axis[0], transformed, d );
				CrossProduct( camera->axis[0], camera->axis[1], camera->axis[2] );
			} else {
				// bobbing rotate, with skinNum being the rotation offset
				d = sin( tr.refdef.time * 0.003f );
				d = e->e.skinNum + d * 4;
				VectorCopy( camera->axis[1], transformed );
				RotatePointAroundVector( camera->axis[1], camera->axis[0], transformed, d );
				CrossProduct( camera->axis[0], camera->axis[1], camera->axis[2] );
			}
		}
		else if ( e->e.skinNum ) {
			d = e->e.skinNum;
			VectorCopy( camera->axis[1], transformed );
			RotatePointAroundVector( camera->axis[1], camera->axis[0], transformed, d );
			CrossProduct( camera->axis[0], camera->axis[1], camera->axis[2] );
		}
		*mirror = qfalse;
		return qtrue;
	}

	// if we didn't locate a portal entity, don't render anything.
	// We don't want to just treat it as a mirror, because without a
	// portal entity the server won't have communicated a proper entity set
	// in the snapshot

	// unfortunately, with local movement prediction it is easily possible
	// to see a surface before the server has communicated the matching
	// portal surface entity, so we don't want to print anything here...

	//ri.Printf( PRINT_ALL, "Portal surface without a portal entity\n" );

	return qfalse;
}

static qboolean IsMirror( const drawSurf_t *drawSurf, int entityNum )
{
	int			i;
	cplane_t	originalPlane, plane;
	trRefEntity_t	*e;
	float		d;

	// create plane axis for the portal we are seeing
	R_PlaneForSurface( drawSurf->surface, &originalPlane );

	// rotate the plane if necessary
	if ( entityNum != REFENTITYNUM_WORLD ) 
	{
		tr.currentEntityNum = entityNum;
		tr.currentEntity = &tr.refdef.entities[entityNum];

		// get the orientation of the entity
		R_RotateForEntity( tr.currentEntity, &tr.viewParms, &tr.ori );

		// rotate the plane, but keep the non-rotated version for matching
		// against the portalSurface entities
		R_LocalNormalToWorld( originalPlane.normal, plane.normal );
		plane.dist = originalPlane.dist + DotProduct( plane.normal, tr.ori.origin );

		// translate the original plane
		originalPlane.dist = originalPlane.dist + DotProduct( originalPlane.normal, tr.ori.origin );
	} 

	// locate the portal entity closest to this plane.
	// origin will be the origin of the portal, origin2 will be
	// the origin of the camera
	for ( i = 0 ; i < tr.refdef.num_entities ; i++ ) 
	{
		e = &tr.refdef.entities[i];
		if ( e->e.reType != RT_PORTALSURFACE ) {
			continue;
		}

		d = DotProduct( e->e.origin, originalPlane.normal ) - originalPlane.dist;
		if ( d > 64 || d < -64) {
			continue;
		}

		// if the entity is just a mirror, don't use as a camera point
		if ( e->e.oldorigin[0] == e->e.origin[0] && 
			e->e.oldorigin[1] == e->e.origin[1] && 
			e->e.oldorigin[2] == e->e.origin[2] ) 
		{
			return qtrue;
		}

		return qfalse;
	}
	return qfalse;
}

/*
** SurfIsOffscreen
**
** Determines if a surface is completely offscreen.
*/
static qboolean SurfIsOffscreen( const drawSurf_t *drawSurf, vec4_t clipDest[128] ) {
	float shortest = 100000000;
	int entityNum;
	int numTriangles;
	shader_t *shader;
	int		fogNum;
	int dlighted;
	int pshadowed;
	vec4_t clip, eye;
	int i;
	unsigned int pointOr = 0;
	unsigned int pointAnd = (unsigned int)~0;
	qboolean bStaticModel;

	R_RotateForViewer();

	R_DecomposeSort( drawSurf->sort, &entityNum, &shader, &fogNum, &dlighted, &pshadowed,
			&bStaticModel );
	RB_BeginSurface( shader, fogNum, drawSurf->cubemapIndex);
	rb_surfaceTable[ *drawSurf->surface ]( drawSurf->surface );

	assert( tess.numVertexes < 128 );

	for ( i = 0; i < tess.numVertexes; i++ )
	{
		int j;
		unsigned int pointFlags = 0;

		R_TransformModelToClip( tess.xyz[i], tr.ori.modelMatrix, tr.viewParms.projectionMatrix, eye, clip );

		for ( j = 0; j < 3; j++ )
		{
			if ( clip[j] >= clip[3] )
			{
				pointFlags |= (1 << (j*2));
			}
			else if ( clip[j] <= -clip[3] )
			{
				pointFlags |= ( 1 << (j*2+1));
			}
		}
		pointAnd &= pointFlags;
		pointOr |= pointFlags;
	}

	// trivially reject
	if ( pointAnd )
	{
		return qtrue;
	}

	// determine if this surface is backfaced and also determine the distance
	// to the nearest vertex so we can cull based on portal range.  Culling
	// based on vertex distance isn't 100% correct (we should be checking for
	// range to the surface), but it's good enough for the types of portals
	// we have in the game right now.
	numTriangles = tess.numIndexes / 3;

	for ( i = 0; i < tess.numIndexes; i += 3 )
	{
		vec3_t normal, tNormal;

		float len;

		VectorSubtract( tess.xyz[tess.indexes[i]], tr.viewParms.ori.origin, normal );

		len = VectorLengthSquared( normal );			// lose the sqrt
		if ( len < shortest )
		{
			shortest = len;
		}

		R_VaoUnpackNormal(tNormal, tess.normal[tess.indexes[i]]);

		if ( DotProduct( normal, tNormal ) >= 0 )
		{
			numTriangles--;
		}
	}
	if ( !numTriangles )
	{
		return qtrue;
	}

	// mirrors can early out at this point, since we don't do a fade over distance
	// with them (although we could)
	if ( IsMirror( drawSurf, entityNum ) )
	{
		return qfalse;
	}

	if ( shortest > (tess.shader->portalRange*tess.shader->portalRange) )
	{
		return qtrue;
	}

	return qfalse;
}

/*
========================
R_MirrorViewBySurface

Returns qtrue if another view has been rendered
========================
*/
qboolean R_MirrorViewBySurface (drawSurf_t *drawSurf, int entityNum) {
	vec4_t			clipDest[128];
	viewParms_t		newParms;
	viewParms_t		oldParms;
	orientation_t	surface, camera;

	// don't recursively mirror
	if (tr.viewParms.isPortal) {
		ri.Printf( PRINT_DEVELOPER, "WARNING: recursive mirror/portal found\n" );
		return qfalse;
	}

	if ( r_noportals->integer || (r_fastsky->integer == 1) ) {
		return qfalse;
	}

	// trivially reject portal/mirror
	if ( SurfIsOffscreen( drawSurf, clipDest ) ) {
		return qfalse;
	}

	// save old viewParms so we can return to it after the mirror view
	oldParms = tr.viewParms;

	newParms = tr.viewParms;
	newParms.isPortal = qtrue;
	newParms.zFar = 0.0f;
	newParms.flags &= ~VPF_FARPLANEFRUSTUM;
	if ( !R_GetPortalOrientations( drawSurf, entityNum, &surface, &camera, 
		newParms.pvsOrigin, &newParms.isMirror ) ) {
		return qfalse;		// bad portal, no portalentity
	}

	// Never draw viewmodels in portal or mirror views.
	newParms.flags |= VPF_NOVIEWMODEL;

	R_MirrorPoint (oldParms.ori.origin, &surface, &camera, newParms.ori.origin );

	VectorSubtract( vec3_origin, camera.axis[0], newParms.portalPlane.normal );
	newParms.portalPlane.dist = DotProduct( camera.origin, newParms.portalPlane.normal );
	
	R_MirrorVector (oldParms.ori.axis[0], &surface, &camera, newParms.ori.axis[0]);
	R_MirrorVector (oldParms.ori.axis[1], &surface, &camera, newParms.ori.axis[1]);
	R_MirrorVector (oldParms.ori.axis[2], &surface, &camera, newParms.ori.axis[2]);

	// OPTIMIZE: restrict the viewport on the mirrored view

	// render the mirror view
	R_RenderView (&newParms);

	tr.viewParms = oldParms;

	return qtrue;
}

/*
=================
R_SpriteFogNum

See if a sprite is inside a fog volume
=================
*/
int R_SpriteFogNum( trRefEntity_t *ent ) {
	int				i, j;
	fog_t			*fog;

	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return 0;
	}

	if ( ent->e.renderfx & RF_CROSSHAIR ) {
		return 0;
	}

	for ( i = 1 ; i < tr.world->numfogs ; i++ ) {
		fog = &tr.world->fogs[i];
		for ( j = 0 ; j < 3 ; j++ ) {
			if ( ent->e.origin[j] - ent->e.radius >= fog->bounds[1][j] ) {
				break;
			}
			if ( ent->e.origin[j] + ent->e.radius <= fog->bounds[0][j] ) {
				break;
			}
		}
		if ( j == 3 ) {
			return i;
		}
	}

	return 0;
}

/*
==========================================================================================

DRAWSURF SORTING

==========================================================================================
*/

/*
===============
R_Radix
===============
*/
static ID_INLINE void R_Radix( int byte, int size, drawSurf_t *source, drawSurf_t *dest )
{
  int           count[ 256 ] = { 0 };
  int           index[ 256 ];
  int           i;
  unsigned char *sortKey = NULL;
  unsigned char *end = NULL;

  sortKey = ( (unsigned char *)&source[ 0 ].sort ) + byte;
  end = sortKey + ( size * sizeof( drawSurf_t ) );
  for( ; sortKey < end; sortKey += sizeof( drawSurf_t ) )
    ++count[ *sortKey ];

  index[ 0 ] = 0;

  for( i = 1; i < 256; ++i )
    index[ i ] = index[ i - 1 ] + count[ i - 1 ];

  sortKey = ( (unsigned char *)&source[ 0 ].sort ) + byte;
  for( i = 0; i < size; ++i, sortKey += sizeof( drawSurf_t ) )
    dest[ index[ *sortKey ]++ ] = source[ i ];
}

/*
===============
R_RadixSort

Radix sort with 4 byte size buckets
===============
*/
static void R_RadixSort( drawSurf_t *source, int size )
{
  static drawSurf_t scratch[ MAX_DRAWSURFS ];
#ifdef Q3_LITTLE_ENDIAN
  R_Radix( 0, size, source, scratch );
  R_Radix( 1, size, scratch, source );
  R_Radix( 2, size, source, scratch );
  R_Radix( 3, size, scratch, source );
#else
  R_Radix( 3, size, source, scratch );
  R_Radix( 2, size, scratch, source );
  R_Radix( 1, size, source, scratch );
  R_Radix( 0, size, scratch, source );
#endif //Q3_LITTLE_ENDIAN
}

//==========================================================================================

/*
=================
R_AddDrawSurf
=================
*/
void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader, 
				   int fogIndex, int dlightMap, int pshadowMap, int cubemap ) {
	int			index;

	// instead of checking for overflow, we just mask the index
	// so it wraps around
	index = tr.refdef.numDrawSurfs & DRAWSURF_MASK;
	// the sort data is packed into a single 32 bit value so it can be
	// compared quickly during the qsorting process
	tr.refdef.drawSurfs[index].sort = (shader->sortedIndex << QSORT_SHADERNUM_SHIFT) 
		| tr.shiftedEntityNum | ( fogIndex << QSORT_FOGNUM_SHIFT ) 
		| ((int)pshadowMap << QSORT_PSHADOW_SHIFT) | (int)dlightMap
		| tr.shiftedIsStatic;
	tr.refdef.drawSurfs[index].cubemapIndex = cubemap;
	tr.refdef.drawSurfs[index].surface = surface;
	tr.refdef.numDrawSurfs++;
}

/*
=================
R_DecomposeSort
=================
*/
void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader, 
					 int *fogNum, int *dlightMap, int *pshadowMap,
                     qboolean *bStaticModel
			) {
	*fogNum = ( sort >> QSORT_FOGNUM_SHIFT ) & 31;
	*shader = tr.sortedShaders[ ( sort >> QSORT_SHADERNUM_SHIFT ) & (MAX_SHADERS-1) ];
	*entityNum = ( sort >> QSORT_REFENTITYNUM_SHIFT ) & REFENTITYNUM_MASK;
	*pshadowMap = (sort >> QSORT_PSHADOW_SHIFT ) & 1;
    *dlightMap = sort & 1;

	//
	// OPENMOHAA-specific stuff
	//=========================
	*fogNum = 0;
    *bStaticModel = sort & (1 << QSORT_STATICMODEL_SHIFT);
    //=========================
}

/*
=================
R_SortDrawSurfs
=================
*/
void R_SortDrawSurfs( drawSurf_t *drawSurfs, int numDrawSurfs,
	drawSurf_t *spriteSurfs, int numSpriteSurfs
) {
	shader_t		*shader;
	int				fogNum;
	int				entityNum;
	int				dlighted;
	int             pshadowed;
    int				i;
    qboolean		bStaticModel;

	//ri.Printf(PRINT_ALL, "firstDrawSurf %d numDrawSurfs %d\n", (int)(drawSurfs - tr.refdef.drawSurfs), numDrawSurfs);

	// it is possible for some views to not have any surfaces
	if ( numDrawSurfs < 1 ) {
		// we still need to add it for hyperspace cases
		R_AddDrawSurfCmd( drawSurfs, numDrawSurfs );
		R_AddSpriteSurfCmd( spriteSurfs, numSpriteSurfs );
		return;
	}

	// sort the drawsurfs by sort type, then orientation, then shader
	R_RadixSort( drawSurfs, numDrawSurfs );
	R_RadixSort( spriteSurfs, numSpriteSurfs );

	// skip pass through drawing if rendering a shadow map
	if (tr.viewParms.flags & (VPF_SHADOWMAP | VPF_DEPTHSHADOW))
	{
		R_AddDrawSurfCmd( drawSurfs, numDrawSurfs );
		return;
	}

	// check for any pass through drawing, which
	// may cause another view to be rendered first
	for ( i = 0 ; i < numDrawSurfs ; i++ ) {
		R_DecomposeSort( (drawSurfs+i)->sort, &entityNum, &shader, &fogNum, &dlighted, &pshadowed, &bStaticModel );

		if ( shader->sort > SS_PORTAL ) {
			break;
		}

		// no shader should ever have this sort type
		if ( shader->sort == SS_BAD ) {
			ri.Error (ERR_DROP, "Shader '%s'with sort == SS_BAD", shader->name );
		}

		// if the mirror was completely clipped away, we may need to check another surface
		if ( R_MirrorViewBySurface( (drawSurfs+i), entityNum) ) {
			// this is a debug option to see exactly what is being mirrored
			if ( r_portalOnly->integer ) {
				return;
			}
			break;		// only one mirror view at a time
		}
	}

	R_AddDrawSurfCmd( drawSurfs, numDrawSurfs );
    R_AddSpriteSurfCmd( spriteSurfs, numSpriteSurfs );
}

static void R_AddEntitySurface (int entityNum)
{
	trRefEntity_t	*ent;
	shader_t		*shader;

	tr.currentEntityNum = entityNum;

	ent = tr.currentEntity = &tr.refdef.entities[tr.currentEntityNum];

	ent->needDlights = qfalse;

	// preshift the value we are going to OR into the drawsurf sort
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_REFENTITYNUM_SHIFT;

	//
	// the weapon model must be handled special --
	// we don't want the hacked weapon position showing in 
	// mirrors, because the true body position will already be drawn
	//
	if ( (ent->e.renderfx & RF_FIRST_PERSON) && (tr.viewParms.flags & VPF_NOVIEWMODEL)) {
		return;
	}

	// simple generated models, like sprites and beams, are not culled
	switch ( ent->e.reType ) {
	case RT_PORTALSURFACE:
		break;		// don't draw anything
	case RT_SPRITE:
	case RT_BEAM:
	case RT_LIGHTNING:
	case RT_RAIL_CORE:
	case RT_RAIL_RINGS:
		// self blood sprites, talk balloons, etc should not be drawn in the primary
		// view.  We can't just do this check for all entities, because md3
		// entities may still want to cast shadows from them
		if ( (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal) {
			return;
		}
		shader = R_GetShaderByHandle( ent->e.customShader );
		R_AddDrawSurf( &entitySurface, shader, R_SpriteFogNum( ent ), 0, 0, 0 /*cubeMap*/ );
		break;

	case RT_MODEL:
		// we must set up parts of tr.ori for model culling
		R_RotateForEntity( ent, &tr.viewParms, &tr.ori );

		tr.currentModel = R_GetModelByHandle( ent->e.hModel );
		if (!tr.currentModel) {
			R_AddDrawSurf( &entitySurface, tr.defaultShader, 0, 0, 0, 0 /*cubeMap*/  );
		} else {
			switch ( tr.currentModel->type ) {
			case MOD_MESH:
				R_AddMD3Surfaces( ent );
				break;
			case MOD_MDR:
				R_MDRAddAnimSurfaces( ent );
				break;
			case MOD_IQM:
				R_AddIQMSurfaces( ent );
				break;
			case MOD_BRUSH:
				R_AddBrushModelSurfaces( ent );
				break;
			//
			// OPENMOHAA-specific stuff
			//=========================
			case MOD_SPRITE:
				ri.Printf(PRINT_ALL, "sprite model '%s' being added to renderer!\n", tr.currentModel->name);
                break;
            case MOD_TIKI:
                R_AddSkelSurfaces(ent);
                break;
			//=========================
			case MOD_BAD:		// null model axis
				if ( (ent->e.renderfx & RF_THIRD_PERSON) && !tr.viewParms.isPortal) {
					break;
				}
				R_AddDrawSurf( &entitySurface, tr.defaultShader, 0, 0, 0, 0 );
				break;
			default:
				ri.Error( ERR_DROP, "R_AddEntitySurfaces: Bad modeltype" );
				break;
			}
		}
		break;
	default:
		ri.Error( ERR_DROP, "R_AddEntitySurfaces: Bad reType" );
	}
}

/*
=============
R_AddEntitySurfaces
=============
*/
void R_AddEntitySurfaces (void) {
	int i;

	if ( !r_drawentities->integer ) {
		return;
	}

	//
	// OPENMOHAA-specific stuff
	//=========================
    tr.shiftedIsStatic = 0;
    //=========================

	for ( i = 0; i < tr.refdef.num_entities; i++)
		R_AddEntitySurface(i);
}


/*
====================
R_GenerateDrawSurfs
====================
*/
void R_GenerateDrawSurfs( void ) {
	R_AddWorldSurfaces ();

	R_AddPolygonSurfaces();

	// set the projection matrix with the minimum zfar
	// now that we have the world bounded
	// this needs to be done before entities are
	// added, because they use the projection
	// matrix for lod calculation

	// dynamically compute far clip plane distance
	if (!(tr.viewParms.flags & VPF_SHADOWMAP))
	{
		R_SetFarClip();
	}

	// we know the size of the clipping volume. Now set the rest of the projection matrix.
	R_SetupProjectionZ (&tr.viewParms);

	R_AddEntitySurfaces ();

	//
	// OPENMOHAA-specific stuff
    //=========================
    R_AddSpriteSurfaces();
	//=========================
}

/*
================
R_DebugPolygon
================
*/
void R_DebugPolygon( int color, int numPoints, float *points ) {
	// FIXME: implement this
#if 0
	int		i;

	GL_State( GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );

	// draw solid shade

	qglColor3f( color&1, (color>>1)&1, (color>>2)&1 );
	qglBegin( GL_POLYGON );
	for ( i = 0 ; i < numPoints ; i++ ) {
		qglVertex3fv( points + i * 3 );
	}
	qglEnd();

	// draw wireframe outline
	GL_State( GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE | GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE );
	qglDepthRange( 0, 0 );
	qglColor3f( 1, 1, 1 );
	qglBegin( GL_POLYGON );
	for ( i = 0 ; i < numPoints ; i++ ) {
		qglVertex3fv( points + i * 3 );
	}
	qglEnd();
	qglDepthRange( 0, 1 );
#endif
}

/*
====================
R_DebugGraphics

Visualization aid for movement clipping debugging
====================
*/
void R_DebugGraphics( void ) {
	if ( tr.refdef.rdflags & RDF_NOWORLDMODEL ) {
		return;
	}
	if ( !r_debugSurface->integer ) {
		return;
	}

	R_IssuePendingRenderCommands();

	GL_BindToTMU(tr.whiteImage, TB_COLORMAP);
	GL_Cull( CT_FRONT_SIDED );
	ri.CM_DrawDebugSurface( R_DebugPolygon );
}

#define CIRCLE_LENGTH		25

/*
================
R_DebugCircle
================
*/
void R_DebugCircle(const vec3_t org, float radius, float r, float g, float b, float alpha, qboolean horizontal) {
    int				i;
    float			ang;
	debugline_t* line;
	vec3_t			forward, right;
	vec3_t			pos, lastpos;

	if (!ri.DebugLines || !ri.numDebugLines) {
		return;
	}

	if (horizontal)
	{
		VectorSet(forward, 1, 0, 0);
		VectorSet(right, 0, 1, 0);
	}
	else
	{
		VectorCopy(tr.refdef.viewaxis[1], right);
		VectorCopy(tr.refdef.viewaxis[2], forward);
	}

	VectorClear(pos);
	VectorClear(lastpos);

	for (i = 0; i < CIRCLE_LENGTH; i++) {
		VectorCopy(pos, lastpos);

		ang = DEG2RAD((float)(i * 15));
		pos[0] = (org[0] + sin(ang) * radius * forward[0]) +
			cos(ang) * radius * right[0];
		pos[1] = (org[1] + sin(ang) * radius * forward[1]) +
			cos(ang) * radius * right[1];
		pos[2] = (org[2] + sin(ang) * radius * forward[2]) +
			cos(ang) * radius * right[2];

		if (i > 0)
		{
			if (*ri.numDebugLines >= r_numdebuglines->integer) {
				ri.Printf(PRINT_ALL, "R_DebugCircle: Exceeded MAX_DEBUG_LINES\n");
				return;
			}

			line = &(*ri.DebugLines)[*ri.numDebugLines];
			(*ri.numDebugLines)++;
			VectorCopy(lastpos, line->start);
			VectorCopy(pos, line->end);
			VectorSet(line->color, r, g, b);
			line->alpha = alpha;
			line->width = 1.0;
			line->factor = 1;
			line->pattern = -1;
		}
	}
}

/*
================
R_DebugLine
================
*/
void R_DebugLine(const vec3_t start, const vec3_t end, float r, float g, float b, float alpha) {
	debugline_t* line;

	if (!ri.DebugLines || !ri.numDebugLines) {
		return;
	}

	if (*ri.numDebugLines >= r_numdebuglines->integer) {
		ri.Printf(PRINT_ALL, "R_DebugLine: Exceeded MAX_DEBUG_LINES\n");
		return;
	}

	line = &(*ri.DebugLines)[*ri.numDebugLines];
	(*ri.numDebugLines)++;
	VectorCopy(start, line->start);
	VectorCopy(end, line->end);
	VectorSet(line->color, r, g, b);
	line->alpha = alpha;
	line->width = 1.0;
	line->factor = 1;
	line->pattern = -1;
}

/*
================
R_DrawDebugLines
================
*/
void R_DrawDebugLines(void) {
	// FIXME: unimplemented
#if 0
	debugline_t* line;
	int i;
	float width;
	int factor;
	unsigned short pattern;

	if (!ri.DebugLines || !ri.numDebugLines) {
		return;
	}

	if (!*ri.numDebugLines) {
		return;
	}

	if (tr.refdef.rdflags & RDF_NOWORLDMODEL) {
		return;
	}

	if (tr.viewParms.isPortalSky) {
		return;
	}

	R_IssuePendingRenderCommands();
    GL_BindToTMU(tr.whiteImage, 0);
	if (r_debuglines_depthmask->integer) {
		GL_State(GLS_POLYMODE_LINE | GLS_DEPTHMASK_TRUE);
	} else {
		GL_State(GLS_POLYMODE_LINE);
	}

    qglDisableClientState(GL_COLOR_ARRAY);
    qglDisableClientState(GL_TEXTURE_COORD_ARRAY);

    width = 1.0;
    factor = 4;
    pattern = -1;

    if (r_stipplelines->integer)
    {
        qglEnable(GL_LINE_STIPPLE);
        qglLineStipple(4, -1);
        qglLineWidth(1.0f);
    }

	qglBegin(GL_LINES);

	for (i = 0; i < *ri.numDebugLines; i++) {
		line = &(*ri.DebugLines)[i];

		if (r_stipplelines->integer) {
			if (line->width != width || line->factor != factor || line->pattern != pattern) {
				qglEnd();
				qglLineStipple(line->factor, line->pattern);
                qglLineWidth(line->width);
                qglBegin(GL_LINES);
                factor = line->factor;
                width = line->width;
                pattern = line->pattern;
			}
		}

		qglColor4f(line->color[0], line->color[1], line->color[2], line->alpha);
        qglVertex3fv(line->start);
        qglVertex3fv(line->end);
	}

    qglEnd();

    if (r_stipplelines->integer)
    {
        qglDisable(GL_LINE_STIPPLE);
        qglLineStipple(1, -1);
        qglLineWidth(1.0);
    }

    qglDepthRange(0.0, 1.0);
#endif
}

/*
================
R_DrawDebugLines
================
*/
void R_DrawDebugStrings(void) {
	int i;
	debugstring_t* string;

	if (!ri.DebugStrings || !ri.numDebugStrings) {
		return;
	}

	if (!*ri.numDebugStrings) {
		return;
	}

    if (tr.refdef.rdflags & RDF_NOWORLDMODEL) {
        return;
    }

    if (tr.viewParms.isPortalSky) {
        return;
    }

	for (i = 0; i < *ri.numDebugStrings; i++) {
		string = &(*ri.DebugStrings)[i];

		R_DrawFloatingString(
			tr.pFontDebugStrings,
			string->szText,
			string->pos,
			string->color,
			string->scale,
			64
		);
	}
}


/*
================
R_RenderView

A view may be either the actual camera view,
or a mirror / remote location
================
*/
void R_RenderView (viewParms_t *parms) {
	int		firstDrawSurf;
    int		numDrawSurfs;
    int		firstSpriteSurf;

	if ( parms->viewportWidth <= 0 || parms->viewportHeight <= 0 ) {
		return;
	}

	tr.viewCount++;

	tr.viewParms = *parms;
	tr.viewParms.frameSceneNum = tr.frameSceneNum;
	tr.viewParms.frameCount = tr.frameCount;

    firstDrawSurf = tr.refdef.numDrawSurfs;
    firstSpriteSurf = tr.refdef.numSpriteSurfs;

	tr.viewCount++;

	// set viewParms.world
	R_RotateForViewer ();

	R_SetupProjection(&tr.viewParms, r_zproj->value, tr.viewParms.zFar, qtrue);

	R_GenerateDrawSurfs();

	// if we overflowed MAX_DRAWSURFS, the drawsurfs
	// wrapped around in the buffer and we will be missing
	// the first surfaces, not the last ones
	numDrawSurfs = tr.refdef.numDrawSurfs;
	if ( numDrawSurfs > MAX_DRAWSURFS ) {
		numDrawSurfs = MAX_DRAWSURFS;
	}

	R_SortDrawSurfs( tr.refdef.drawSurfs + firstDrawSurf, numDrawSurfs - firstDrawSurf,
        tr.refdef.spriteSurfs + firstSpriteSurf, tr.refdef.numSpriteSurfs - firstSpriteSurf
	);

	// draw main system development information (surface outlines, etc)
	R_DebugGraphics();
}


void R_RenderDlightCubemaps(const refdef_t *fd)
{
	int i;

	for (i = 0; i < tr.refdef.num_dlights; i++)
	{
		viewParms_t		shadowParms;
		int j;

		// use previous frame to determine visible dlights
		if ((1 << i) & tr.refdef.dlightMask)
			continue;

		Com_Memset( &shadowParms, 0, sizeof( shadowParms ) );

		shadowParms.viewportX = tr.refdef.x;
		shadowParms.viewportY = glConfig.vidHeight - ( tr.refdef.y + PSHADOW_MAP_SIZE );
		shadowParms.viewportWidth = PSHADOW_MAP_SIZE;
		shadowParms.viewportHeight = PSHADOW_MAP_SIZE;
		shadowParms.isPortal = qfalse;
		shadowParms.isMirror = qtrue; // because it is

		shadowParms.fovX = 90;
		shadowParms.fovY = 90;

		shadowParms.flags = VPF_SHADOWMAP | VPF_DEPTHSHADOW | VPF_NOVIEWMODEL;
		shadowParms.zFar = tr.refdef.dlights[i].radius;

		VectorCopy( tr.refdef.dlights[i].origin, shadowParms.ori.origin );

		for (j = 0; j < 6; j++)
		{
			switch(j)
			{
				case 0:
					// -X
					VectorSet( shadowParms.ori.axis[0], -1,  0,  0);
					VectorSet( shadowParms.ori.axis[1],  0,  0, -1);
					VectorSet( shadowParms.ori.axis[2],  0,  1,  0);
					break;
				case 1: 
					// +X
					VectorSet( shadowParms.ori.axis[0],  1,  0,  0);
					VectorSet( shadowParms.ori.axis[1],  0,  0,  1);
					VectorSet( shadowParms.ori.axis[2],  0,  1,  0);
					break;
				case 2: 
					// -Y
					VectorSet( shadowParms.ori.axis[0],  0, -1,  0);
					VectorSet( shadowParms.ori.axis[1],  1,  0,  0);
					VectorSet( shadowParms.ori.axis[2],  0,  0, -1);
					break;
				case 3: 
					// +Y
					VectorSet( shadowParms.ori.axis[0],  0,  1,  0);
					VectorSet( shadowParms.ori.axis[1],  1,  0,  0);
					VectorSet( shadowParms.ori.axis[2],  0,  0,  1);
					break;
				case 4:
					// -Z
					VectorSet( shadowParms.ori.axis[0],  0,  0, -1);
					VectorSet( shadowParms.ori.axis[1],  1,  0,  0);
					VectorSet( shadowParms.ori.axis[2],  0,  1,  0);
					break;
				case 5:
					// +Z
					VectorSet( shadowParms.ori.axis[0],  0,  0,  1);
					VectorSet( shadowParms.ori.axis[1], -1,  0,  0);
					VectorSet( shadowParms.ori.axis[2],  0,  1,  0);
					break;
			}

			R_RenderView(&shadowParms);
			R_AddCapShadowmapCmd( i, j );
		}
	}
}


void R_RenderPshadowMaps(const refdef_t *fd)
{
	viewParms_t		shadowParms;
	int i;

	// first, make a list of shadows
	for ( i = 0; i < tr.refdef.num_entities; i++)
	{
		trRefEntity_t *ent = &tr.refdef.entities[i];

		if((ent->e.renderfx & (RF_FIRST_PERSON | RF_NOSHADOW)))
			continue;

		//if((ent->e.renderfx & RF_THIRD_PERSON))
			//continue;

		if (ent->e.reType == RT_MODEL)
		{
			model_t *model = R_GetModelByHandle( ent->e.hModel );
			pshadow_t shadow;
			float radius = 0.0f;
			float scale = 1.0f;
			vec3_t diff;
			int j;

			if (!model)
				continue;

			if (ent->e.nonNormalizedAxes)
			{
				scale = VectorLength( ent->e.axis[0] );
			}

			switch (model->type)
			{
				case MOD_MESH:
				{
					mdvFrame_t *frame = &model->mdv[0]->frames[ent->e.frame];

					radius = frame->radius * scale;
				}
				break;

				case MOD_MDR:
				{
					// FIXME: never actually tested this
					mdrHeader_t *header = model->modelData;
					int frameSize = (size_t)( &((mdrFrame_t *)0)->bones[ header->numBones ] );
					mdrFrame_t *frame = ( mdrFrame_t * ) ( ( byte * ) header + header->ofsFrames + frameSize * ent->e.frame);

					radius = frame->radius;
				}
				break;
				case MOD_IQM:
				{
					// FIXME: never actually tested this
					iqmData_t *data = model->modelData;
					vec3_t diag;
					float *framebounds;

					framebounds = data->bounds + 6*ent->e.frame;
					VectorSubtract( framebounds+3, framebounds, diag );
					radius = 0.5f * VectorLength( diag );
				}
				break;
				// OPENMOHAA-specific stuff
				//=========================
				case MOD_TIKI:
					radius = ri.TIKI_GlobalRadius(model->d.tiki);
                    break;
				case MOD_SPRITE:
				{
					float w;

					radius = model->d.sprite->width * model->d.sprite->scale * 0.5;
					w = model->d.sprite->height * model->d.sprite->scale * 0.5;

					if (radius <= w) {
						radius = w;
					}
				}
					break;
				//=========================

				default:
					break;
			}

			if (!radius)
				continue;

			// Cull entities that are behind the viewer by more than lightRadius
			VectorSubtract(ent->e.origin, fd->vieworg, diff);
			if (DotProduct(diff, fd->viewaxis[0]) < -r_pshadowDist->value)
				continue;

			memset(&shadow, 0, sizeof(shadow));

			shadow.numEntities = 1;
			shadow.entityNums[0] = i;
			shadow.viewRadius = radius;
			shadow.lightRadius = r_pshadowDist->value;
			VectorCopy(ent->e.origin, shadow.viewOrigin);
			shadow.sort = DotProduct(diff, diff) / (radius * radius);
			VectorCopy(ent->e.origin, shadow.entityOrigins[0]);
			shadow.entityRadiuses[0] = radius;

			for (j = 0; j < MAX_CALC_PSHADOWS; j++)
			{
				pshadow_t swap;

				if (j + 1 > tr.refdef.num_pshadows)
				{
					tr.refdef.num_pshadows = j + 1;
					tr.refdef.pshadows[j] = shadow;
					break;
				}

				// sort shadows by distance from camera divided by radius
				// FIXME: sort better
				if (tr.refdef.pshadows[j].sort <= shadow.sort)
					continue;

				swap = tr.refdef.pshadows[j];
				tr.refdef.pshadows[j] = shadow;
				shadow = swap;
			}
		}
	}

	// next, merge touching pshadows
	for ( i = 0; i < tr.refdef.num_pshadows; i++)
	{
		pshadow_t *ps1 = &tr.refdef.pshadows[i];
		int j;

		for (j = i + 1; j < tr.refdef.num_pshadows; j++)
		{
			pshadow_t *ps2 = &tr.refdef.pshadows[j];
			int k;
			qboolean touch;

			if (ps1->numEntities == 8)
				break;

			touch = qfalse;
			if (SpheresIntersect(ps1->viewOrigin, ps1->viewRadius, ps2->viewOrigin, ps2->viewRadius))
			{
				for (k = 0; k < ps1->numEntities; k++)
				{
					if (SpheresIntersect(ps1->entityOrigins[k], ps1->entityRadiuses[k], ps2->viewOrigin, ps2->viewRadius))
					{
						touch = qtrue;
						break;
					}
				}
			}

			if (touch)
			{
				vec3_t newOrigin;
				float newRadius;

				BoundingSphereOfSpheres(ps1->viewOrigin, ps1->viewRadius, ps2->viewOrigin, ps2->viewRadius, newOrigin, &newRadius);
				VectorCopy(newOrigin, ps1->viewOrigin);
				ps1->viewRadius = newRadius;

				ps1->entityNums[ps1->numEntities] = ps2->entityNums[0];
				VectorCopy(ps2->viewOrigin, ps1->entityOrigins[ps1->numEntities]);
				ps1->entityRadiuses[ps1->numEntities] = ps2->viewRadius;

				ps1->numEntities++;

				for (k = j; k < tr.refdef.num_pshadows - 1; k++)
				{
					tr.refdef.pshadows[k] = tr.refdef.pshadows[k + 1];
				}

				j--;
				tr.refdef.num_pshadows--;
			}
		}
	}

	// cap number of drawn pshadows
	if (tr.refdef.num_pshadows > MAX_DRAWN_PSHADOWS)
	{
		tr.refdef.num_pshadows = MAX_DRAWN_PSHADOWS;
	}

	// next, fill up the rest of the shadow info
	for ( i = 0; i < tr.refdef.num_pshadows; i++)
	{
		pshadow_t *shadow = &tr.refdef.pshadows[i];
		vec3_t up;
		vec3_t ambientLight, directedLight, lightDir;

		VectorSet(lightDir, 0.57735f, 0.57735f, 0.57735f);
#if 1
		R_LightForPoint(shadow->viewOrigin, ambientLight, directedLight, lightDir);

		// sometimes there's no light
		if (DotProduct(lightDir, lightDir) < 0.9f)
			VectorSet(lightDir, 0.0f, 0.0f, 1.0f);
#endif

		if (shadow->viewRadius * 3.0f > shadow->lightRadius)
		{
			shadow->lightRadius = shadow->viewRadius * 3.0f;
		}

		VectorMA(shadow->viewOrigin, shadow->viewRadius, lightDir, shadow->lightOrigin);

		// make up a projection, up doesn't matter
		VectorScale(lightDir, -1.0f, shadow->lightViewAxis[0]);
		VectorSet(up, 0, 0, -1);

		if ( fabsf(DotProduct(up, shadow->lightViewAxis[0])) > 0.9f )
		{
			VectorSet(up, -1, 0, 0);
		}

		CrossProduct(shadow->lightViewAxis[0], up, shadow->lightViewAxis[1]);
		VectorNormalize(shadow->lightViewAxis[1]);
		CrossProduct(shadow->lightViewAxis[0], shadow->lightViewAxis[1], shadow->lightViewAxis[2]);

		VectorCopy(shadow->lightViewAxis[0], shadow->cullPlane.normal);
		shadow->cullPlane.dist = DotProduct(shadow->cullPlane.normal, shadow->lightOrigin);
		shadow->cullPlane.type = PLANE_NON_AXIAL;
		SetPlaneSignbits(&shadow->cullPlane);
	}

	// next, render shadowmaps
	for ( i = 0; i < tr.refdef.num_pshadows; i++)
	{
		int firstDrawSurf;
		int firstSpriteSurf;
		pshadow_t *shadow = &tr.refdef.pshadows[i];
		int j;

		Com_Memset( &shadowParms, 0, sizeof( shadowParms ) );

		if (glRefConfig.framebufferObject)
		{
			shadowParms.viewportX = 0;
			shadowParms.viewportY = 0;
		}
		else
		{
			shadowParms.viewportX = tr.refdef.x;
			shadowParms.viewportY = glConfig.vidHeight - ( tr.refdef.y + PSHADOW_MAP_SIZE );
		}
		shadowParms.viewportWidth = PSHADOW_MAP_SIZE;
		shadowParms.viewportHeight = PSHADOW_MAP_SIZE;
		shadowParms.isPortal = qfalse;
		shadowParms.isMirror = qfalse;

		shadowParms.fovX = 90;
		shadowParms.fovY = 90;

		if (glRefConfig.framebufferObject)
			shadowParms.targetFbo = tr.pshadowFbos[i];

		shadowParms.flags = VPF_DEPTHSHADOW | VPF_NOVIEWMODEL;
		shadowParms.zFar = shadow->lightRadius;

		VectorCopy(shadow->lightOrigin, shadowParms.ori.origin);
		
		VectorCopy(shadow->lightViewAxis[0], shadowParms.ori.axis[0]);
		VectorCopy(shadow->lightViewAxis[1], shadowParms.ori.axis[1]);
		VectorCopy(shadow->lightViewAxis[2], shadowParms.ori.axis[2]);

		{
			tr.viewCount++;

			tr.viewParms = shadowParms;
			tr.viewParms.frameSceneNum = tr.frameSceneNum;
			tr.viewParms.frameCount = tr.frameCount;

            firstDrawSurf = tr.refdef.numDrawSurfs;
            firstSpriteSurf = tr.refdef.numSpriteSurfs;

			tr.viewCount++;

			// set viewParms.world
			R_RotateForViewer ();

			{
				float xmin, xmax, ymin, ymax, znear, zfar;
				viewParms_t *dest = &tr.viewParms;
				vec3_t pop;

				xmin = ymin = -shadow->viewRadius;
				xmax = ymax = shadow->viewRadius;
				znear = 0;
				zfar = shadow->lightRadius;

				dest->projectionMatrix[0] = 2 / (xmax - xmin);
				dest->projectionMatrix[4] = 0;
				dest->projectionMatrix[8] = (xmax + xmin) / (xmax - xmin);
				dest->projectionMatrix[12] =0;

				dest->projectionMatrix[1] = 0;
				dest->projectionMatrix[5] = 2 / (ymax - ymin);
				dest->projectionMatrix[9] = ( ymax + ymin ) / (ymax - ymin);	// normally 0
				dest->projectionMatrix[13] = 0;

				dest->projectionMatrix[2] = 0;
				dest->projectionMatrix[6] = 0;
				dest->projectionMatrix[10] = 2 / (zfar - znear);
				dest->projectionMatrix[14] = 0;

				dest->projectionMatrix[3] = 0;
				dest->projectionMatrix[7] = 0;
				dest->projectionMatrix[11] = 0;
				dest->projectionMatrix[15] = 1;

				VectorScale(dest->ori.axis[1],  1.0f, dest->frustum[0].normal);
				VectorMA(dest->ori.origin, -shadow->viewRadius, dest->frustum[0].normal, pop);
				dest->frustum[0].dist = DotProduct(pop, dest->frustum[0].normal);

				VectorScale(dest->ori.axis[1], -1.0f, dest->frustum[1].normal);
				VectorMA(dest->ori.origin, -shadow->viewRadius, dest->frustum[1].normal, pop);
				dest->frustum[1].dist = DotProduct(pop, dest->frustum[1].normal);

				VectorScale(dest->ori.axis[2],  1.0f, dest->frustum[2].normal);
				VectorMA(dest->ori.origin, -shadow->viewRadius, dest->frustum[2].normal, pop);
				dest->frustum[2].dist = DotProduct(pop, dest->frustum[2].normal);

				VectorScale(dest->ori.axis[2], -1.0f, dest->frustum[3].normal);
				VectorMA(dest->ori.origin, -shadow->viewRadius, dest->frustum[3].normal, pop);
				dest->frustum[3].dist = DotProduct(pop, dest->frustum[3].normal);

				VectorScale(dest->ori.axis[0], -1.0f, dest->frustum[4].normal);
				VectorMA(dest->ori.origin, -shadow->lightRadius, dest->frustum[4].normal, pop);
				dest->frustum[4].dist = DotProduct(pop, dest->frustum[4].normal);

				for (j = 0; j < 5; j++)
				{
					dest->frustum[j].type = PLANE_NON_AXIAL;
					SetPlaneSignbits (&dest->frustum[j]);
				}

				dest->flags |= VPF_FARPLANEFRUSTUM;
			}

			for (j = 0; j < shadow->numEntities; j++)
			{
				R_AddEntitySurface(shadow->entityNums[j]);
			}

			R_SortDrawSurfs( tr.refdef.drawSurfs + firstDrawSurf, tr.refdef.numDrawSurfs - firstDrawSurf,
				tr.refdef.spriteSurfs + firstSpriteSurf, tr.refdef.numSpriteSurfs - firstSpriteSurf
			);

			if (!glRefConfig.framebufferObject)
				R_AddCapShadowmapCmd( i, -1 );
		}
	}
}

static float CalcSplit(float n, float f, float i, float m)
{
	return (n * pow(f / n, i / m) + (f - n) * i / m) / 2.0f;
}


void R_RenderSunShadowMaps(const refdef_t *fd, int level)
{
	viewParms_t		shadowParms;
	vec4_t lightDir, lightCol;
	vec3_t lightViewAxis[3];
	vec3_t lightOrigin;
	float splitZNear, splitZFar, splitBias;
	float viewZNear, viewZFar;
	vec3_t lightviewBounds[2];
	qboolean lightViewIndependentOfCameraView = qfalse;

	if (r_forceSun->integer == 2)
	{
		int scale = 32768;
		float angle = (fd->time % scale) / (float)scale * M_PI;
		lightDir[0] = cos(angle);
		lightDir[1] = sin(35.0f * M_PI / 180.0f);
		lightDir[2] = sin(angle) * cos(35.0f * M_PI / 180.0f);
		lightDir[3] = 0.0f;

		if (1) //((fd->time % (scale * 2)) < scale)
		{
			lightCol[0] = 
			lightCol[1] = 
			lightCol[2] = CLAMP(sin(angle) * 2.0f, 0.0f, 1.0f) * 2.0f;
			lightCol[3] = 1.0f;
		}
		else
		{
			lightCol[0] = 
			lightCol[1] = 
			lightCol[2] = CLAMP(sin(angle) * 2.0f * 0.1f, 0.0f, 0.1f);
			lightCol[3] = 1.0f;
		}

		VectorCopy4(lightDir, tr.refdef.sunDir);
		VectorCopy4(lightCol, tr.refdef.sunCol);
		VectorScale4(lightCol, 0.2f, tr.refdef.sunAmbCol);
	}
	else
	{
		VectorCopy4(tr.refdef.sunDir, lightDir);
	}

	viewZNear = r_shadowCascadeZNear->value;
	viewZFar = r_shadowCascadeZFar->value;
	splitBias = r_shadowCascadeZBias->value;

	switch(level)
	{
		case 0:
		default:
			//splitZNear = r_znear->value;
			//splitZFar  = 256;
			splitZNear = viewZNear;
			splitZFar = CalcSplit(viewZNear, viewZFar, 1, 3) + splitBias;
			break;
		case 1:
			splitZNear = CalcSplit(viewZNear, viewZFar, 1, 3) + splitBias;
			splitZFar = CalcSplit(viewZNear, viewZFar, 2, 3) + splitBias;
			//splitZNear = 256;
			//splitZFar  = 896;
			break;
		case 2:
			splitZNear = CalcSplit(viewZNear, viewZFar, 2, 3) + splitBias;
			splitZFar = viewZFar;
			//splitZNear = 896;
			//splitZFar  = 3072;
			break;
	}
	
	if (level != 3)
		VectorCopy(fd->vieworg, lightOrigin);
	else
		VectorCopy(tr.world->lightGridOrigin, lightOrigin);

	// Make up a projection
	VectorScale(lightDir, -1.0f, lightViewAxis[0]);

	if (level == 3 || lightViewIndependentOfCameraView)
	{
		// Use world up as light view up
		VectorSet(lightViewAxis[2], 0, 0, 1);
	}
	else if (level == 0)
	{
		// Level 0 tries to use a diamond texture orientation relative to camera view
		// Use halfway between camera view forward and left for light view up
		VectorAdd(fd->viewaxis[0], fd->viewaxis[1], lightViewAxis[2]);
	}
	else
	{
		// Use camera view up as light view up
		VectorCopy(fd->viewaxis[2], lightViewAxis[2]);
	}

	// Check if too close to parallel to light direction
	if (fabsf(DotProduct(lightViewAxis[2], lightViewAxis[0])) > 0.9f)
	{
		if (level == 3 || lightViewIndependentOfCameraView)
		{
			// Use world left as light view up
			VectorSet(lightViewAxis[2], 0, 1, 0);
		}
		else if (level == 0)
		{
			// Level 0 tries to use a diamond texture orientation relative to camera view
			// Use halfway between camera view forward and up for light view up
			VectorAdd(fd->viewaxis[0], fd->viewaxis[2], lightViewAxis[2]);
		}
		else
		{
			// Use camera view left as light view up
			VectorCopy(fd->viewaxis[1], lightViewAxis[2]);
		}
	}

	// clean axes
	CrossProduct(lightViewAxis[2], lightViewAxis[0], lightViewAxis[1]);
	VectorNormalize(lightViewAxis[1]);
	CrossProduct(lightViewAxis[0], lightViewAxis[1], lightViewAxis[2]);

	// Create bounds for light projection using slice of view projection
	{
		mat4_t lightViewMatrix;
		vec4_t point, base, lightViewPoint;
		float lx, ly;

		base[3] = 1;
		point[3] = 1;
		lightViewPoint[3] = 1;

		Mat4View(lightViewAxis, lightOrigin, lightViewMatrix);

		ClearBounds(lightviewBounds[0], lightviewBounds[1]);

		if (level != 3)
		{
			// add view near plane
			lx = splitZNear * tan(fd->fov_x * M_PI / 360.0f);
			ly = splitZNear * tan(fd->fov_y * M_PI / 360.0f);
			VectorMA(fd->vieworg, splitZNear, fd->viewaxis[0], base);

			VectorMA(base,   lx, fd->viewaxis[1], point);
			VectorMA(point,  ly, fd->viewaxis[2], point);
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			VectorMA(base,  -lx, fd->viewaxis[1], point);
			VectorMA(point,  ly, fd->viewaxis[2], point);
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			VectorMA(base,   lx, fd->viewaxis[1], point);
			VectorMA(point, -ly, fd->viewaxis[2], point);
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			VectorMA(base,  -lx, fd->viewaxis[1], point);
			VectorMA(point, -ly, fd->viewaxis[2], point);
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);
			

			// add view far plane
			lx = splitZFar * tan(fd->fov_x * M_PI / 360.0f);
			ly = splitZFar * tan(fd->fov_y * M_PI / 360.0f);
			VectorMA(fd->vieworg, splitZFar, fd->viewaxis[0], base);

			VectorMA(base,   lx, fd->viewaxis[1], point);
			VectorMA(point,  ly, fd->viewaxis[2], point);
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			VectorMA(base,  -lx, fd->viewaxis[1], point);
			VectorMA(point,  ly, fd->viewaxis[2], point);
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			VectorMA(base,   lx, fd->viewaxis[1], point);
			VectorMA(point, -ly, fd->viewaxis[2], point);
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			VectorMA(base,  -lx, fd->viewaxis[1], point);
			VectorMA(point, -ly, fd->viewaxis[2], point);
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);
		}
		else
		{
			// use light grid size as level size
			// FIXME: could be tighter
			vec3_t bounds;

			bounds[0] = tr.world->lightGridSize[0] * tr.world->lightGridBounds[0];
			bounds[1] = tr.world->lightGridSize[1] * tr.world->lightGridBounds[1];
			bounds[2] = tr.world->lightGridSize[2] * tr.world->lightGridBounds[2];

			point[0] = tr.world->lightGridOrigin[0];
			point[1] = tr.world->lightGridOrigin[1];
			point[2] = tr.world->lightGridOrigin[2];
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			point[0] = tr.world->lightGridOrigin[0] + bounds[0];
			point[1] = tr.world->lightGridOrigin[1];
			point[2] = tr.world->lightGridOrigin[2];
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			point[0] = tr.world->lightGridOrigin[0];
			point[1] = tr.world->lightGridOrigin[1] + bounds[1];
			point[2] = tr.world->lightGridOrigin[2];
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			point[0] = tr.world->lightGridOrigin[0] + bounds[0];
			point[1] = tr.world->lightGridOrigin[1] + bounds[1];
			point[2] = tr.world->lightGridOrigin[2];
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			point[0] = tr.world->lightGridOrigin[0];
			point[1] = tr.world->lightGridOrigin[1];
			point[2] = tr.world->lightGridOrigin[2] + bounds[2];
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			point[0] = tr.world->lightGridOrigin[0] + bounds[0];
			point[1] = tr.world->lightGridOrigin[1];
			point[2] = tr.world->lightGridOrigin[2] + bounds[2];
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			point[0] = tr.world->lightGridOrigin[0];
			point[1] = tr.world->lightGridOrigin[1] + bounds[1];
			point[2] = tr.world->lightGridOrigin[2] + bounds[2];
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);

			point[0] = tr.world->lightGridOrigin[0] + bounds[0];
			point[1] = tr.world->lightGridOrigin[1] + bounds[1];
			point[2] = tr.world->lightGridOrigin[2] + bounds[2];
			Mat4Transform(lightViewMatrix, point, lightViewPoint);
			AddPointToBounds(lightViewPoint, lightviewBounds[0], lightviewBounds[1]);
		}

		if (!glRefConfig.depthClamp)
			lightviewBounds[0][0] = lightviewBounds[1][0] - 8192;

		// Moving the Light in Texel-Sized Increments
		// from http://msdn.microsoft.com/en-us/library/windows/desktop/ee416324%28v=vs.85%29.aspx
		//
		if (lightViewIndependentOfCameraView)
		{
			float cascadeBound, worldUnitsPerTexel, invWorldUnitsPerTexel;

			cascadeBound = MAX(lightviewBounds[1][0] - lightviewBounds[0][0], lightviewBounds[1][1] - lightviewBounds[0][1]);
			cascadeBound = MAX(cascadeBound, lightviewBounds[1][2] - lightviewBounds[0][2]);
			worldUnitsPerTexel = cascadeBound / tr.sunShadowFbo[level]->width;
			invWorldUnitsPerTexel = 1.0f / worldUnitsPerTexel;

			VectorScale(lightviewBounds[0], invWorldUnitsPerTexel, lightviewBounds[0]);
			lightviewBounds[0][0] = floor(lightviewBounds[0][0]);
			lightviewBounds[0][1] = floor(lightviewBounds[0][1]);
			lightviewBounds[0][2] = floor(lightviewBounds[0][2]);
			VectorScale(lightviewBounds[0], worldUnitsPerTexel, lightviewBounds[0]);

			VectorScale(lightviewBounds[1], invWorldUnitsPerTexel, lightviewBounds[1]);
			lightviewBounds[1][0] = floor(lightviewBounds[1][0]);
			lightviewBounds[1][1] = floor(lightviewBounds[1][1]);
			lightviewBounds[1][2] = floor(lightviewBounds[1][2]);
			VectorScale(lightviewBounds[1], worldUnitsPerTexel, lightviewBounds[1]);
		}

		//ri.Printf(PRINT_ALL, "level %d znear %f zfar %f\n", level, lightviewBounds[0][0], lightviewBounds[1][0]);		
		//ri.Printf(PRINT_ALL, "xmin %f xmax %f ymin %f ymax %f\n", lightviewBounds[0][1], lightviewBounds[1][1], -lightviewBounds[1][2], -lightviewBounds[0][2]);
	}

	{
		int firstDrawSurf;
		int firstSpriteSurf;

		Com_Memset( &shadowParms, 0, sizeof( shadowParms ) );

		if (glRefConfig.framebufferObject)
		{
			shadowParms.viewportX = 0;
			shadowParms.viewportY = 0;
		}
		else
		{
			shadowParms.viewportX = tr.refdef.x;
			shadowParms.viewportY = glConfig.vidHeight - ( tr.refdef.y + tr.sunShadowFbo[level]->height );
		}
		shadowParms.viewportWidth  = tr.sunShadowFbo[level]->width;
		shadowParms.viewportHeight = tr.sunShadowFbo[level]->height;
		shadowParms.isPortal = qfalse;
		shadowParms.isMirror = qfalse;

		shadowParms.fovX = 90;
		shadowParms.fovY = 90;

		if (glRefConfig.framebufferObject)
			shadowParms.targetFbo = tr.sunShadowFbo[level];

		shadowParms.flags = VPF_DEPTHSHADOW | VPF_DEPTHCLAMP | VPF_ORTHOGRAPHIC | VPF_NOVIEWMODEL;
		shadowParms.zFar = lightviewBounds[1][0];

		VectorCopy(lightOrigin, shadowParms.ori.origin);
		
		VectorCopy(lightViewAxis[0], shadowParms.ori.axis[0]);
		VectorCopy(lightViewAxis[1], shadowParms.ori.axis[1]);
		VectorCopy(lightViewAxis[2], shadowParms.ori.axis[2]);

		VectorCopy(lightOrigin, shadowParms.pvsOrigin );

		{
			tr.viewCount++;

			tr.viewParms = shadowParms;
			tr.viewParms.frameSceneNum = tr.frameSceneNum;
			tr.viewParms.frameCount = tr.frameCount;

			firstDrawSurf = tr.refdef.numDrawSurfs;
			firstSpriteSurf = tr.refdef.numSpriteSurfs;

			tr.viewCount++;

			// set viewParms.world
			R_RotateForViewer ();

			R_SetupProjectionOrtho(&tr.viewParms, lightviewBounds);

			R_AddWorldSurfaces ();

			R_AddPolygonSurfaces();

			R_AddEntitySurfaces ();

			R_SortDrawSurfs( tr.refdef.drawSurfs + firstDrawSurf, tr.refdef.numDrawSurfs - firstDrawSurf,
				tr.refdef.spriteSurfs + firstSpriteSurf, tr.refdef.numSpriteSurfs - firstSpriteSurf );
		}

		Mat4Multiply(tr.viewParms.projectionMatrix, tr.viewParms.world.modelMatrix, tr.refdef.sunShadowMvp[level]);
	}
}

void R_RenderCubemapSide( int cubemapIndex, int cubemapSide, qboolean subscene )
{
	refdef_t refdef;
	viewParms_t	parms;

	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = 0;
	VectorCopy(tr.cubemaps[cubemapIndex].origin, refdef.vieworg);

	switch(cubemapSide)
	{
		case 0:
			// -X
			VectorSet( refdef.viewaxis[0], -1,  0,  0);
			VectorSet( refdef.viewaxis[1],  0,  0, -1);
			VectorSet( refdef.viewaxis[2],  0,  1,  0);
			break;
		case 1: 
			// +X
			VectorSet( refdef.viewaxis[0],  1,  0,  0);
			VectorSet( refdef.viewaxis[1],  0,  0,  1);
			VectorSet( refdef.viewaxis[2],  0,  1,  0);
			break;
		case 2: 
			// -Y
			VectorSet( refdef.viewaxis[0],  0, -1,  0);
			VectorSet( refdef.viewaxis[1],  1,  0,  0);
			VectorSet( refdef.viewaxis[2],  0,  0, -1);
			break;
		case 3: 
			// +Y
			VectorSet( refdef.viewaxis[0],  0,  1,  0);
			VectorSet( refdef.viewaxis[1],  1,  0,  0);
			VectorSet( refdef.viewaxis[2],  0,  0,  1);
			break;
		case 4:
			// -Z
			VectorSet( refdef.viewaxis[0],  0,  0, -1);
			VectorSet( refdef.viewaxis[1],  1,  0,  0);
			VectorSet( refdef.viewaxis[2],  0,  1,  0);
			break;
		case 5:
			// +Z
			VectorSet( refdef.viewaxis[0],  0,  0,  1);
			VectorSet( refdef.viewaxis[1], -1,  0,  0);
			VectorSet( refdef.viewaxis[2],  0,  1,  0);
			break;
	}

	refdef.fov_x = 90;
	refdef.fov_y = 90;

	refdef.x = 0;
	refdef.y = 0;
	refdef.width = tr.renderCubeFbo->width;
	refdef.height = tr.renderCubeFbo->height;

	refdef.time = 0;

	if (!subscene)
	{
		RE_BeginScene(&refdef);

		// FIXME: sun shadows aren't rendered correctly in cubemaps
		// fix involves changing r_FBufScale to fit smaller cubemap image size, or rendering cubemap to framebuffer first
		if(0) //(glRefConfig.framebufferObject && r_sunlightMode->integer && (r_forceSun->integer || tr.sunShadows))
		{
			R_RenderSunShadowMaps(&refdef, 0);
			R_RenderSunShadowMaps(&refdef, 1);
			R_RenderSunShadowMaps(&refdef, 2);
			R_RenderSunShadowMaps(&refdef, 3);
		}
	}

	{
		vec3_t ambient, directed, lightDir;
		float scale;

		R_LightForPoint(tr.refdef.vieworg, ambient, directed, lightDir);
		scale = directed[0] + directed[1] + directed[2] + ambient[0] + ambient[1] + ambient[2] + 1.0f;

		// only print message for first side
		if (scale < 1.0001f && cubemapSide == 0)
		{
			ri.Printf(PRINT_ALL, "cubemap %d %s (%f, %f, %f) is outside the lightgrid or inside a wall!\n", cubemapIndex, tr.cubemaps[cubemapIndex].name, tr.refdef.vieworg[0], tr.refdef.vieworg[1], tr.refdef.vieworg[2]);
		}
	}

	Com_Memset( &parms, 0, sizeof( parms ) );

	parms.viewportX = 0;
	parms.viewportY = 0;
	parms.viewportWidth = tr.renderCubeFbo->width;
	parms.viewportHeight = tr.renderCubeFbo->height;
	parms.isPortal = qfalse;
	parms.isMirror = qtrue;
	parms.flags =  VPF_NOVIEWMODEL | VPF_NOCUBEMAPS;

	parms.fovX = 90;
	parms.fovY = 90;

	VectorCopy( refdef.vieworg, parms.ori.origin );
	VectorCopy( refdef.viewaxis[0], parms.ori.axis[0] );
	VectorCopy( refdef.viewaxis[1], parms.ori.axis[1] );
	VectorCopy( refdef.viewaxis[2], parms.ori.axis[2] );

	VectorCopy( refdef.vieworg, parms.pvsOrigin );

	// FIXME: sun shadows aren't rendered correctly in cubemaps
	// fix involves changing r_FBufScale to fit smaller cubemap image size, or rendering cubemap to framebuffer first
	if (0) //(r_depthPrepass->value && ((r_forceSun->integer) || tr.sunShadows))
	{
		parms.flags = VPF_USESUNLIGHT;
	}

	parms.targetFbo = tr.renderCubeFbo;
	parms.targetFboLayer = cubemapSide;
	parms.targetFboCubemapIndex = cubemapIndex;

	R_RenderView(&parms);

	if (!subscene)
		RE_EndScene();
}

//
// OPENMOHAA-specific stuff
//

void R_AdjustVisBoundsForSprite(refSprite_t* ent, viewParms_t* viewParms, orientationr_t* or )
{
	if (tr.viewParms.visBounds[0][0] > ent->origin[0] - ent->scale) {
		tr.viewParms.visBounds[0][0] = ent->origin[0] - ent->scale;
    }
    if (tr.viewParms.visBounds[0][1] > ent->origin[1] - ent->scale) {
        tr.viewParms.visBounds[0][1] = ent->origin[1] - ent->scale;
    }
    if (tr.viewParms.visBounds[0][2] > ent->origin[2] - ent->scale) {
        tr.viewParms.visBounds[0][2] = ent->origin[2] - ent->scale;
    }

	if (tr.viewParms.visBounds[1][0] < ent->origin[0] + ent->scale) {
        tr.viewParms.visBounds[1][0] = ent->origin[0] + ent->scale;
    }
    if (tr.viewParms.visBounds[1][1] < ent->origin[1] + ent->scale) {
        tr.viewParms.visBounds[1][1] = ent->origin[1] + ent->scale;
    }
    if (tr.viewParms.visBounds[1][2] < ent->origin[2] + ent->scale) {
        tr.viewParms.visBounds[1][2] = ent->origin[2] + ent->scale;
    }
}

/*
=================
R_AddSpriteSurf
=================
*/
void R_AddSpriteSurf(surfaceType_t* surface, shader_t* shader, float zDistance)
{
	int index;

	if (zDistance > MAX_SPRITE_DIST_SQUARED) {
		zDistance = MAX_SPRITE_DIST_SQUARED;
	}

	index = tr.refdef.numSpriteSurfs % MAX_SPRITES;
    tr.refdef.spriteSurfs[index].sort = (int)(MAX_SPRITE_DIST_SQUARED - zDistance) | (shader->sortedIndex << QSORT_SHADERNUM_SHIFT);
    tr.refdef.spriteSurfs[index].surface = surface;
    tr.refdef.numSpriteSurfs++;
}

void R_AddSpriteSurfaces()
{
	refSprite_t* sprite;
	vec3_t delta;

	if (!r_drawsprites->integer) {
		return;
	}

	for (tr.currentSpriteNum = 0; tr.currentSpriteNum < tr.refdef.num_sprites; ++tr.currentSpriteNum)
	{
		sprite = &tr.refdef.sprites[tr.currentSpriteNum];

        if (tr.viewParms.isPortalSky) {
			if (!(sprite->renderfx & RF_SKYENTITY)) {
				continue;
			}
        } else {
			if (sprite->renderfx & RF_SKYENTITY) {
				continue;
			}
		}
        
		if (tr.viewParms.isPortal) {
			if (!(sprite->renderfx & (RF_SHADOW_PLANE | RF_WRAP_FRAMES))) {
				continue;
			}
		} else {
			if (sprite->renderfx & RF_SHADOW_PLANE) {
				continue;
			}
		}

		tr.currentEntityNum = ENTITYNUM_WORLD;
        tr.shiftedEntityNum = tr.currentEntityNum << QSORT_REFENTITYNUM_SHIFT;
		if (sprite->hModel && sprite->hModel < tr.numModels) {
			tr.currentModel = tr.models[sprite->hModel];
		} else {
			tr.currentModel = tr.models[0];
		}

		if (tr.currentModel->type != MOD_SPRITE) {
			continue;
		}

		R_AdjustVisBoundsForSprite(&tr.refdef.sprites[tr.currentSpriteNum], &tr.viewParms, &tr.ori );
		sprite->shaderNum = tr.currentModel->d.sprite->shader->sortedIndex;

		VectorSubtract(sprite->origin, tr.refdef.vieworg, delta);
		R_AddSpriteSurf(&sprite->surftype, tr.currentModel->d.sprite->shader, VectorLengthSquared(delta));
	}
}

/*
=================
R_RotateForStaticModel

Generates an orientation for a static model and viewParms
=================
*/
void R_RotateForStaticModel( cStaticModelUnpacked_t *SM, const viewParms_t *viewParms,
					   orientationr_t *ori ) {
	float	glMatrix[16];
	vec3_t	delta;
	float	tiki_scale;

	tiki_scale = SM->tiki->load_scale * SM->scale;

	VectorCopy( SM->origin, ori->origin );

	VectorCopy( SM->axis[0], ori->axis[0] );
	VectorCopy( SM->axis[1], ori->axis[1] );
	VectorCopy( SM->axis[2], ori->axis[2] );

	glMatrix[0]  = ori->axis[0][0] * tiki_scale;
	glMatrix[4]  = ori->axis[1][0] * tiki_scale;
	glMatrix[8]  = ori->axis[2][0] * tiki_scale;
	glMatrix[12] = ori->origin[0];

	glMatrix[1]  = ori->axis[0][1] * tiki_scale;
	glMatrix[5]  = ori->axis[1][1] * tiki_scale;
	glMatrix[9]  = ori->axis[2][1] * tiki_scale;
	glMatrix[13] = ori->origin[1];

	glMatrix[2]  = ori->axis[0][2] * tiki_scale;
	glMatrix[6]  = ori->axis[1][2] * tiki_scale;
	glMatrix[10] = ori->axis[2][2] * tiki_scale;
	glMatrix[14] = ori->origin[2];

	glMatrix[3]  = 0;
	glMatrix[7]  = 0;
	glMatrix[11] = 0;
	glMatrix[15] = 1;

	myGlMultMatrix( glMatrix, viewParms->world.modelMatrix, ori->modelMatrix );

	// calculate the viewer origin in the model's space
	// needed for fog, specular, and environment mapping
	VectorSubtract( viewParms->ori.origin, ori->origin, delta );

	ori->viewOrigin[0] = DotProduct( delta, ori->axis[0] );
	ori->viewOrigin[1] = DotProduct( delta, ori->axis[1] );
	ori->viewOrigin[2] = DotProduct( delta, ori->axis[2] );
}

int R_DistanceCullLocalPointAndRadius(float fDist, const vec3_t pt, float radius) {
	vec3_t transformed;

	R_LocalPointToWorld(pt, transformed);

	return R_DistanceCullPointAndRadius(fDist, transformed, radius);
}

int R_DistanceCullPointAndRadius(float fDist, const vec3_t pt, float radius) {
	if (!r_nocull->integer)
	{
		vec3_t vDelta;
		float fLengthSquared;
		float fTotalDistSquared;

		VectorSubtract(pt, tr.viewParms.ori.origin, vDelta);
		fLengthSquared = VectorLengthSquared(vDelta);
		fTotalDistSquared = Square(fDist + radius);

		if (Square(fDist + radius) < Square(vDelta[2]) + fLengthSquared) {
			return CULL_OUT;
		} else if (Square(fDist - radius) > Square(vDelta[2]) + fLengthSquared) {
			return CULL_IN;
		}
	}

	return CULL_CLIP;
}

/*
** SurfIsOffscreen
**
** Determines if a surface is completely offscreen.
*/
qboolean SurfIsOffscreen2(const srfBspSurface_t* surface, shader_t* shader, int entityNum) {
	float shortest = 100000000;
	int numTriangles;
	qboolean doRange;
	orientationr_t surfOr;
	unsigned int* indices;
	vec4_t clip, eye;
	int i;
	unsigned int pointOr = 0;
	unsigned int pointAnd = (unsigned int)~0;

	if (surface->surfaceType != SF_FACE) {
		ri.Printf(PRINT_WARNING, "WARNING: SurfIsOffscreen called on non-bmodel!\n");
		return qfalse;
	}
	
	if ( glConfig.smpActive ) {		// FIXME!  we can't do RB_BeginSurface/RB_EndSurface stuff with smp!
		return qfalse;
	}

	if (entityNum == ENTITYNUM_WORLD) {
		surfOr = tr.viewParms.world;
	} else {
		R_RotateForEntity(&tr.refdef.entities[entityNum], &tr.viewParms, &surfOr);
	}

	for ( i = 0; i < surface->numVerts; i++ )
	{
		int j;
		unsigned int pointFlags = 0;

		R_TransformModelToClip( surface->verts[i].xyz, surfOr.modelMatrix, tr.viewParms.projectionMatrix, eye, clip );

		for ( j = 0; j < 3; j++ )
		{
			if ( clip[j] >= clip[3] )
			{
				pointFlags |= (1 << (j*2));
			}
			else if ( clip[j] <= -clip[3] )
			{
				pointFlags |= ( 1 << (j*2+1));
			}
		}
		pointAnd &= pointFlags;
		pointOr |= pointFlags;
	}

	// trivially reject
	if ( pointAnd )
	{
		return qtrue;
	}

	// determine if this surface is backfaced and also determine the distance
	// to the nearest vertex so we can cull based on portal range.  Culling
	// based on vertex distance isn't 100% correct (we should be checking for
	// range to the surface), but it's good enough for the types of portals
	// we have in the game right now.
	numTriangles = surface->numIndexes / 3;

	for ( i = 0; i < surface->numIndexes; i += 3 )
	{
		vec3_t normal;
		float dot;
		float len;
		unsigned* indices;

		indices = surface->indexes; // (unsigned*)(((char*)surface) + surface->ofsIndices);

		VectorSubtract( surface->verts[indices[i]].xyz, surfOr.viewOrigin, normal);

		if (shader->fDistRange > 0) {
			len = VectorLengthSquared(normal);			// lose the sqrt
			if (len < shortest)
			{
				shortest = len;
			}
		}

		if ( ( dot = DotProduct( normal, surface->cullPlane.normal ) ) >= 0 )
		{
			numTriangles--;
		}
	}
	if ( !numTriangles )
	{
		return qtrue;
	}

	if (shader->fDistRange > 0.0)
	{
		if (shortest > Square(shader->fDistRange))
		{
			return qtrue;
		}
	}

	return qfalse;
}

static qboolean DrawSurfIsOffscreen(drawSurf_t* drawSurf) {
	// FIXME: unimplemented (GL2)
#if 0
	int entityNum;
	shader_t* shader;
	int dlighted;
	qboolean bStaticModel;

	R_DecomposeSort(drawSurf->sort, &entityNum, &shader, &dlighted, &bStaticModel);
	return SurfIsOffscreen2((srfBspSurface_t*)drawSurf->surface, shader, entityNum);
#endif
}