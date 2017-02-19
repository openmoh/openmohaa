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

// tiki_tag.cpp : TIKI Tag

#include "q_shared.h"
#include "qcommon.h"
#include "../skeletor/skeletor.h"
#include "tiki_cache.h"
#include "dbgheap.h"

/*
===============
TIKI_Tag_NameToNum
===============
*/
int TIKI_Tag_NameToNum( dtiki_t *pmdl, const char *name )
{
	return pmdl->GetBoneNumFromName( name );
}

/*
===============
TIKI_Tag_NumToName
===============
*/
const char *TIKI_Tag_NumToName( dtiki_t *pmdl, int iTagNum )
{
	return pmdl->GetBoneNameFromNum( iTagNum );
}

/*
===============
TIKI_TransformInternal
===============
*/
SkelMat4 *TIKI_TransformInternal( dtiki_t *tiki, int entnum, int tagnum )
{
	skeletor_c *skeletor = ( skeletor_c * )TIKI_GetSkeletor( tiki, entnum );
	return &skeletor->GetBoneFrame( tagnum );
}

/*
===============
TIKI_IsOnGroundInternal
===============
*/
qboolean TIKI_IsOnGroundInternal( dtiki_t *tiki, int entnum, int tagnum, float threshold )
{
	skeletor_c *skeletor = ( skeletor_c * )TIKI_GetSkeletor( tiki, entnum );
	return skeletor->IsBoneOnGround( tagnum, threshold );
}

/*
===============
TIKI_OrientationInternal
===============
*/
orientation_t TIKI_OrientationInternal( dtiki_t *tiki, int entnum, int tagnum, float scale )
{
	orientation_t or;
	skeletor_c *skeletor;
	SkelMat4 pTransform;

	skeletor = ( skeletor_c * )TIKI_GetSkeletor( tiki, entnum );
	pTransform = skeletor->GetBoneFrame( tagnum );

	or.origin[ 0 ] = ( pTransform.val[ 3 ][ 0 ] + tiki->load_origin[ 0 ] ) * ( scale * tiki->load_scale );
	or.origin[ 1 ] = ( pTransform.val[ 3 ][ 1 ] + tiki->load_origin[ 1 ] ) * ( scale * tiki->load_scale );
	or.origin[ 2 ] = ( pTransform.val[ 3 ][ 2 ] + tiki->load_origin[ 2 ] ) * ( scale * tiki->load_scale );
	memcpy( or.axis, pTransform.val, sizeof( or.axis ) );

	return or;
}

/*
===============
TIKI_SetPoseInternal
===============
*/
void TIKI_SetPoseInternal( void *skeletor, const frameInfo_t *frameInfo, int *bone_tag, vec4_t *bone_quat, float actionWeight )
{
	skeletor_c *skel = ( skeletor_c * )skeletor;
	skel->SetPose( frameInfo, bone_tag, bone_quat, actionWeight );
}

/*
===============
TIKI_GetRadiusInternal
===============
*/
float TIKI_GetRadiusInternal( dtiki_t *tiki, int entnum, float scale )
{
	skeletor_c *skeletor = ( skeletor_c * )TIKI_GetSkeletor( tiki, entnum );
	return skeletor->GetRadius() * tiki->load_scale * scale;
}

/*
===============
TIKI_GetCentroidRadiusInternal
===============
*/
float TIKI_GetCentroidRadiusInternal( dtiki_t *tiki, int entnum, float scale, float *centroid )
{
	skeletor_c *skeletor = ( skeletor_c * )TIKI_GetSkeletor( tiki, entnum );
	return skeletor->GetCentroidRadius( centroid ) * tiki->load_scale * scale;
}

/*
===============
TIKI_GetFrameInternal
===============
*/
void TIKI_GetFrameInternal( dtiki_t *tiki, int entnum, skelAnimFrame_t *newFrame )
{
	skeletor_c *skeletor = ( skeletor_c * )TIKI_GetSkeletor( tiki, entnum );
	skeletor->GetFrame( newFrame );
}

/*
===============
TIKI_SetEyeTargetPos
===============
*/
void TIKI_SetEyeTargetPos( dtiki_t *tiki, int entnum, vec3_t pos )
{
	skeletor_c *skeletor = ( skeletor_c * )TIKI_GetSkeletor( tiki, entnum );
	skeletor->SetEyeTargetPos( pos );
}
