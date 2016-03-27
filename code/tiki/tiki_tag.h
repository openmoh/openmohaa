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

// tiki_tag.h : TIKI Tag

#ifndef __TIKI_TAG_H__
#define __TIKI_TAG_H__

#ifdef __cplusplus
extern "C" {
#endif

int TIKI_Tag_NameToNum( dtiki_t *pmdl, const char *name );
const char *TIKI_Tag_NumToName( dtiki_t *pmdl, int iTagNum );
SkelMat4 *TIKI_TransformInternal( dtiki_t *tiki, int entnum, int tagnum );
qboolean TIKI_IsOnGroundInternal( dtiki_t *tiki, int entnum, int tagnum, float threshold );
orientation_t TIKI_OrientationInternal( dtiki_t *tiki, int entnum, int tagnum, float scale );
void TIKI_SetPoseInternal( void *skeletor, const frameInfo_t *frameInfo, int *bone_tag, vec4_t *bone_quat, float actionWeight );
float TIKI_GetRadiusInternal( dtiki_t *tiki, int entnum, float scale );
float TIKI_GetCentroidRadiusInternal( dtiki_t *tiki, int entnum, float scale, float *centroid );
void TIKI_GetFrameInternal( dtiki_t *tiki, int entnum, skelAnimFrame_t *newFrame );
void TIKI_SetEyeTargetPos( dtiki_t *tiki, int entnum, vec3_t pos );

#ifdef __cplusplus
}
#endif

#endif // __TIKI_TAG_H__
