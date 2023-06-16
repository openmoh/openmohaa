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

#include "cm_local.h"
#include "../server/server.h"
#include "../client/client.h"
#include "tiki.h"

#define LOCATION_FAIL			-2
#define MAX_HITLOCATIONS		19

const char *szLocArray[] =
{
	"Bip01 Head",
	"Bip01 Head",
	"Bip01 Neck",
	"Bip01 Spine2",
	"Bip01 Spine1",
	"Bip01 Spine", 
	"Bip01 Pelvis",
	"Bip01 R UpperArm",
	"Bip01 L UpperArm",
	"Bip01 R Thigh",
	"Bip01 L Thigh",
	"Bip01 R Forearm",
	"Bip01 L Forearm",
	"Bip01 R Calf",
	"Bip01 L Calf",
	"Bip01 R Hand",
	"Bip01 L Hand",
	"Bip01 R Foot",
	"Bip01 L Foot"
};

float fLocRadius[] =
{
	5.0,
	5.5,
	4.0,
	9.0,
	8.0,
	9.0,
	9.0,
	7.0,
	7.0,
	8.0,
	8.0,
	5.5,
	5.5,
	7.5,
	7.5,
	6.0,
	6.0,
	6.0,
	6.0,
};

vec3_t vLocOffset[] =
{
	{ 2.5,		3.5,		0.0 },
	{ 8.5,		1.0,		0.0 },
	{ 3.5,		-1.0,		0.0 },
	{ 5.0,		1.0,		3.0 },
	{ 2.0,		1.0,		3.0 },
	{ 1.0,		1.0,		3.0 },
	{ -3.0,		1.0,		4.0 },
	{ 4.0,		0.0,		0.0 },
	{ 4.0,		0.0,		0.0 },
	{ 12.0,		0.0,		0.0 },
	{ 12.0,		0.0,		0.0 },
	{ 5.0,		1.0,		0.0 },
	{ 5.0,		1.0,		0.0 },
	{ 8.0,		0.0,		0.0 },
	{ 8.0,		0.0,		0.0 },
	{ 5.0,		1.0,		0.0 },
	{ 5.0,		1.0,		0.0 },
	{ 3.0,		2.0,		0.0 },
	{ 3.0,		2.0,		0.0 },
};

float fLocRadius_Secondary[] =
{
	0.0,
	5.5,
	4.0,
	9.0,
	8.0,
	9.0,
	9.0,
	6.0,
	6.0,
	8.0,
	8.0,
	5.0,
	5.0,
	6.5,
	6.5,
	0.0,
	0.0,
	5.0,
	5.0,
};

vec3_t vLocOffset_Secondary[] =
{
	{ 0.0,		0.0,		0.0 },
	{ 7.0,		-2.5,		0.0 },
	{ 2.0,		2.0,		0.0 },
	{ 5.0,		1.0,		-3.0 },
	{ 2.0,		1.0,		-3.0 },
	{ 1.0,		1.0,		-3.0 },
	{ -3.0,		1.0,		-4.0 },
	{ 11.5,		0.0,		0.0 },
	{ 11.5,		0.0,		0.0 },
	{ 22.0,		0.0,		0.0 },
	{ 22.0,		0.0,		0.0 },
	{ 11.5,		0.0,		0.0 },
	{ 11.5,		0.0,		0.0 },
	{ 19.0,		0.0,		0.0 },
	{ 19.0,		0.0,		0.0 },
	{ 0.0,		0.0,		0.0 },
	{ 0.0,		0.0,		0.0 },
	{ 4.0,		9.0,		0.0 },
	{ 4.0,		9.0,		0.0 },
};

/*
==================
LineSegmentToSphereIntersect
==================
*/
static qboolean LineSegmentToSphereIntersect( trace_t *pTrace, float fRadius, const vec3_t vCntr, const vec3_t vStart, const vec3_t vEnd )
{
	vec3_t	vDiff, vDir;
	float	fDist;
	float	fLSqrd;
	float	fRSqrd;
	float	fDSqrd;
	float	fLength;
	float	fOfs;
	float	fAngSin;
	vec3_t	vProject, vOfs;

	fRSqrd = fRadius * fRadius;

	VectorSubtract( vEnd, vStart, vDir );
	fLength = VectorNormalize( vDir );

	VectorSubtract( vCntr, vStart, vDiff );
	fDSqrd = DotProduct( vDiff, vDir );
	fDist = VectorLengthSquared( vDiff );

	if( fDSqrd < 0 && fDist > fRSqrd ) {
		return qfalse;
	}

	fLSqrd = fDist - fDSqrd * fDSqrd;
	if( fLSqrd > fRSqrd || fDSqrd > fLength && fDSqrd - fLength > fRadius ) {
		return qfalse;
	}

	vOfs[ 0 ] = vStart[ 0 ] + vDir[ 0 ] * fDSqrd;
	vOfs[ 1 ] = vStart[ 1 ] + vDir[ 1 ] * fDSqrd;
	vOfs[ 2 ] = vStart[ 2 ] + vDir[ 2 ] * fDSqrd;
	VectorSubtract( vCntr, vOfs, vProject );

	fOfs = VectorLength( vProject ) / fRadius;

	if( fOfs >= -0.5 )
	{
		if( fOfs <= 0.5 )
		{
			fAngSin = fOfs * ( -0.531387674508458 * fOfs - 2.11e-14 ) + 1.00086138065435;
		}
		else
		{
			fAngSin = fOfs
				* ( ( ( ( ( -1507.55394345521 * fOfs + 6580.58002318442 ) * fOfs - 11860.0735285953 ) * fOfs
				+ 11290.7510782536 )
				* fOfs
				- 5986.89654545347 )
				* fOfs
				+ 1675.66417006387 )
				- 192.426950291139;
		}
	}
	else
	{
		fAngSin = fOfs
			* ( ( ( ( ( -337.31875783205 * fOfs - 1237.54375255107 ) * fOfs - 1802.11467325687 ) * fOfs - 1303.19904613494 )
			* fOfs
			- 471.347871690988 )
			* fOfs
			- 70.0883838161826 );
	}

	pTrace->fraction = ( fDSqrd - fRadius * fAngSin ) / fLength;
	return qtrue;
}

/*
==================
CM_GetHitLocationInfo
==================
*/
const char *CM_GetHitLocationInfo( int i_iLocation, float *o_fRadius, vec3_t o_vOffset )
{
	if( i_iLocation < MAX_HITLOCATIONS )
	{
		*o_fRadius = fLocRadius[ i_iLocation ];
		VectorCopy( vLocOffset[ i_iLocation ], o_vOffset );
		return szLocArray[ i_iLocation ];
	}
	else
	{
		*o_fRadius = 8.0f;
		VectorClear( o_vOffset );
		return NULL;
	}
}

/*
==================
CM_GetHitLocationInfoSecondary
==================
*/
const char *CM_GetHitLocationInfoSecondary( int i_iLocation, float *o_fRadius, vec3_t o_vOffset )
{
	if( i_iLocation < MAX_HITLOCATIONS )
	{
		*o_fRadius = fLocRadius_Secondary[ i_iLocation ];
		VectorCopy( vLocOffset_Secondary[ i_iLocation ], o_vOffset );
		return szLocArray[ i_iLocation ];
	}
	else
	{
		*o_fRadius = 8.0f;
		VectorClear( o_vOffset );
		return NULL;
	}
}

/*
==================
CM_TraceDeepSimple
==================
*/
static qboolean CM_TraceDeepSimple( trace_t *results, const vec3_t vStart, const vec3_t vEnd, int iBrushMask, int iEntContents, const vec3_t vEntMins, const vec3_t vEntMaxs, const vec3_t vEntAngles, vec3_t vTransStart, vec3_t vTransEnd )
{
	vec3_t	vCntr;
	vec3_t	vTemp;
	vec3_t	vNewMins, vNewMaxs;
	vec3_t	vForward, vLeft, vUp;
	clipHandle_t	clipHandle;

	vNewMins[ 0 ] = vEntMins[ 0 ] - 40.0;
	vNewMins[ 1 ] = vEntMins[ 1 ] - 40.0;
	vNewMins[ 2 ] = vEntMins[ 2 ] - 40.0;
	vNewMaxs[ 0 ] = vEntMaxs[ 0 ] + 40.0;
	vNewMaxs[ 1 ] = vEntMaxs[ 1 ] + 40.0;
	vNewMaxs[ 2 ] = vEntMaxs[ 2 ] + 40.0;

	clipHandle = CM_TempBoxModel( vNewMins, vNewMaxs, iEntContents );
	AngleVectorsLeft( vEntAngles, vForward, vLeft, vUp );

	VectorCopy( vTransStart, vTemp );
	vTransStart[ 0 ] = DotProduct( vTemp, vForward );
	vTransStart[ 1 ] = DotProduct( vTemp, vLeft );
	vTransStart[ 2 ] = DotProduct( vTemp, vUp );

	VectorCopy( vTransEnd, vTemp );
	vTransEnd[ 0 ] = DotProduct( vTemp, vForward );
	vTransEnd[ 1 ] = DotProduct( vTemp, vLeft );
	vTransEnd[ 2 ] = DotProduct( vTemp, vUp );

	// don't use mins and maxs
	VectorClear( vCntr );
	CM_BoxTrace( results, vTransStart, vTransEnd, vCntr, vCntr, clipHandle, iBrushMask, qfalse );

	if( results->fraction == 1.0 && !results->startsolid && !results->allsolid )
	{
		VectorCopy( vEnd, results->endpos );
		return qfalse;
	}

	return qtrue;
}

/*
==================
CM_TraceDeepSimple2
==================
*/
static qboolean CM_TraceDeepSimple2( trace_t *results, const orientation_t *orPosition, vec3_t vOffset, float fRad, const char *pszTagName, int iLocation, const vec3_t vTransStart, const vec3_t vTransEnd )
{
	vec3_t	vCntr;

	VectorCopy( orPosition->origin, vCntr );

	if( vOffset[ 0 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 0 ], orPosition->axis[ 0 ], vCntr );
	}
	if( vOffset[ 1 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 1 ], orPosition->axis[ 1 ], vCntr );
	}
	if( vOffset[ 2 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 2 ], orPosition->axis[ 2 ], vCntr );
	}

	if( LineSegmentToSphereIntersect( results, fRad, vCntr, vTransStart, vTransEnd ) ) {
		return qtrue;
	}

	// get the secondary location if any
	if( !CM_GetHitLocationInfoSecondary( iLocation, &fRad, vOffset ) ) {
		return qfalse;
	}

	VectorCopy( orPosition->origin, vCntr );

	if( vOffset[ 0 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 0 ], orPosition->axis[ 0 ], vCntr );
	}
	if( vOffset[ 1 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 1 ], orPosition->axis[ 1 ], vCntr );
	}
	if( vOffset[ 2 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 2 ], orPosition->axis[ 2 ], vCntr );
	}

	if( LineSegmentToSphereIntersect( results, fRad, vCntr, vTransStart, vTransEnd ) ) {
		return qtrue;
	}

	return qfalse;
}

/*
==================
CM_TraceDeepFail
==================
*/
static void CM_TraceDeepFail( trace_t *results, const vec3_t vEnd )
{
	results->fraction	= 1.0;
	results->allsolid	= qfalse;
	results->startsolid	= qfalse;
	VectorCopy( vEnd, results->endpos );
	results->entityNum	= ENTITYNUM_NONE;
	results->location	= LOCATION_FAIL;
	results->ent		= NULL;
}

/*
==================
CM_TraceDeepSuccess
==================
*/
static void CM_TraceDeepSuccess( trace_t *results, const vec3_t vStart, const vec3_t vEnd, int iEntNum, int iEntContents, int iLocation )
{
	results->endpos[ 0 ] = vStart[ 0 ] + results->fraction * ( vEnd[ 0 ] - vStart[ 0 ] );
	results->endpos[ 1 ] = vStart[ 1 ] + results->fraction * ( vEnd[ 1 ] - vStart[ 1 ] );
	results->endpos[ 2 ] = vStart[ 2 ] + results->fraction * ( vEnd[ 2 ] - vStart[ 2 ] );
	results->allsolid = qfalse;
	results->startsolid = qfalse;
	results->contents = iEntContents;
	results->entityNum = iEntNum;
	results->location = iLocation;
	results->ent = NULL;
}

/*
==================
SV_TraceDeep_DebugDraw
==================
*/
void SV_TraceDeep_DebugDraw( gentity_t *touch, orientation_t& orTag, int iLocation, qboolean bHit )
{
	int		i;
	float	fRad;
	vec3_t	vOffset;
	vec3_t	vCntr;
	orientation_t	orEnt;
	orientation_t	orPosition;
	const char		*pszLocationName;

	if( !sv_deeptracedebug->integer ) {
		return;
	}

	VectorCopy( touch->s.origin, orEnt.origin );
	AngleVectors( touch->s.angles, orEnt.axis[ 0 ], orEnt.axis[ 1 ], orEnt.axis[ 2 ] );
	VectorInverse( orEnt.axis[ 1 ] );

	pszLocationName = CM_GetHitLocationInfo( iLocation, &fRad, vOffset );

	if( bHit ) {
		Com_Printf( "Hit %i '%s'\n", iLocation, pszLocationName );
	}

	// get the orientation
	VectorCopy( orEnt.origin, orPosition.origin );

	for( i = 0; i < 3; i++ )
	{
		VectorMA( orPosition.origin, orEnt.origin[ i ], orEnt.axis[ i ], orPosition.origin );
	}
	MatrixMultiply( orTag.axis, orEnt.axis, orPosition.axis );

	VectorCopy( orPosition.origin, vCntr );

	if( vOffset[ 0 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 0 ], orPosition.axis[ 0 ], vCntr );
	}
	if( vOffset[ 1 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 1 ], orPosition.axis[ 1 ], vCntr );
	}
	if( vOffset[ 2 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 2 ], orPosition.axis[ 2 ], vCntr );
	}

	if( bHit ) {
		ge->DebugCircle( vCntr, fRad, 1.0, 0.5, 0.0, 1.0, qfalse );
	} else {
		ge->DebugCircle( vCntr, fRad, 0.0, 0.5, 1.0, 1.0, qfalse );
	}

	// get the secondary location if any
	if( !CM_GetHitLocationInfoSecondary( iLocation, &fRad, vOffset ) ) {
		return;
	}

	VectorCopy( orPosition.origin, vCntr );

	if( vOffset[ 0 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 0 ], orPosition.axis[ 0 ], vCntr );
	}
	if( vOffset[ 1 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 1 ], orPosition.axis[ 1 ], vCntr );
	}
	if( vOffset[ 2 ] != 0.0 ) {
		VectorMA( vCntr, vOffset[ 2 ], orPosition.axis[ 2 ], vCntr );
	}

	if( bHit ) {
		ge->DebugCircle( vCntr, fRad, 1.0, 0.0, 0.5, 1.0, qfalse );
	} else {
		ge->DebugCircle( vCntr, fRad, 0.5, 0.0, 1.0, 1.0, qfalse );
	}
}

/*
==================
SV_TraceDeep
==================
*/
void SV_TraceDeep( trace_t *results, const vec3_t vStart, const vec3_t vEnd, int iBrushMask, gentity_t *touch )
{
	int				iLocation, iBoneNum;
	float			fRad;
	vec3_t			vOffset;
	orientation_t	orPosition;
	const char		*pszTagName;
	dtiki_t			*tiki;
	vec3_t			vTransStart, vTransEnd;

	// subtract the origin
	VectorSubtract( vStart, touch->s.origin, vTransStart );
	VectorSubtract( vEnd, touch->s.origin, vTransEnd );

	// check if something was not hit first
	if( !CM_TraceDeepSimple( results,
		vStart, vEnd,
		iBrushMask, touch->r.contents,
		touch->r.mins, touch->r.maxs, touch->s.angles,
		vTransStart, vTransEnd
		) ) {
		return;
	}

	tiki = touch->tiki;

	// trace through each locations
	for( iLocation = 0; iLocation < MAX_HITLOCATIONS; iLocation++ )
	{
		pszTagName = CM_GetHitLocationInfo( iLocation, &fRad, vOffset );
		iBoneNum = tiki->GetBoneNumFromName( pszTagName );

		if( iBoneNum == -1 ) {
			continue;
		}

		// retrieve the orientation from the game dll
		orPosition = ge->TIKI_Orientation( touch, iBoneNum );

		// check if the tag was hit
		if( CM_TraceDeepSimple2( results,
			&orPosition, vOffset, fRad,
			pszTagName, iLocation,
			vTransStart, vTransEnd ) )
		{
			SV_TraceDeep_DebugDraw( touch, orPosition, iLocation, qtrue );

			// set the location
			CM_TraceDeepSuccess( results, vStart, vEnd, touch->s.number, touch->r.contents, iLocation );
			return;
		}

		SV_TraceDeep_DebugDraw( touch, orPosition, iLocation, qfalse );
	}

	// fail
	CM_TraceDeepFail( results, vEnd );
}

/*
==================
CL_CheckWeights
==================
*/
qboolean CL_CheckWeights( refEntity_t *ent )
{
	int i;

	if( !ent->tiki )
	{
		return qtrue;
	}

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		if( ent->frameInfo[ i ].weight > 0.0f )
		{
			return qtrue;
		}
	}

	return qfalse;
}

#ifdef CLIENT

/*
==================
CL_TraceDeep
==================
*/
qboolean CL_TraceDeep( trace_t *results, const vec3_t vStart, const vec3_t vEnd, int iBrushMask, int iEntNum, int iEntContents, const vec3_t vEntMins, const vec3_t vEntMaxs, const vec3_t vEntOrigin, const vec3_t vEntAngles, refEntity_t *model )
{
	int				iLocation, iBoneNum;
	float			fRad;
	vec3_t			vOffset;
	orientation_t	orPosition;
	const char		*pszTagName;
	dtiki_t			*tiki;
	vec3_t			vTransStart, vTransEnd;

	// subtract the origin
	VectorSubtract( vStart, vEntOrigin, vTransStart );
	VectorSubtract( vEnd, vEntOrigin, vTransEnd );

	// check if something was not hit first
	if( !CM_TraceDeepSimple( results,
		vStart, vEnd,
		iBrushMask, iEntContents,
		vEntMins, vEntMaxs, vEntAngles,
		vTransStart, vTransEnd
		) ) {
		return qfalse;
	}

	tiki = model->tiki;

	// trace through each locations
	for( iLocation = 0; iLocation < MAX_HITLOCATIONS; iLocation++ )
	{
		pszTagName = CM_GetHitLocationInfo( iLocation, &fRad, vOffset );
		iBoneNum = tiki->GetBoneNumFromName( pszTagName );

		if( iBoneNum == -1 ) {
			continue;
		}

		// retrieve the orientation from the game dll
		orPosition = re.TIKI_Orientation( model, iBoneNum );

		// check if the tag was hit
		if( CM_TraceDeepSimple2( results,
			&orPosition, vOffset, fRad,
			pszTagName, iLocation,
			vTransStart, vTransEnd ) )
		{
			// set the location
			CM_TraceDeepSuccess( results, vStart, vEnd, iEntNum, iEntContents, iLocation );
			return qtrue;
		}
	}

	// fail
	CM_TraceDeepFail( results, vEnd );
	return qfalse;
}

#endif
