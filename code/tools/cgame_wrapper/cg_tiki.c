/*
===========================================================================
Copyright (C) 2012 Michael Rieder

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "qcommon.h"

struct dtiki_s *R_Model_GetHandle ( qhandle_t handle ) {
	struct dtiki_s *ret;

	ret = cgi.R_Model_GetHandle( handle );
	if ( ret )
		return ret;
	else
		return ret;
}

struct dtiki_s *TIKI_FindTiki ( char *path ) {
	return cgi.TIKI_FindTiki(path);
}

// su44: detours for ALL tiki functions
int TIKI_NumAnims( dtiki_t *pmdl ) {
	int ret = cgi.TIKI_NumAnims;
	return ret;
}
void TIKI_CalculateBounds( dtiki_t *pmdl, float scale, float *mins, float *maxs ) {
	cgi.TIKI_CalculateBounds(pmdl,scale,mins,maxs);
}
char *TIKI_Name( dtiki_t *pmdl ) {
	const char *ret = cgi.TIKI_Name(pmdl);
	return ret;
}
skeletor_c *TIKI_GetSkeletor( dtiki_t *tiki, int entnum ) {
	skeletor_c *ret = cgi.TIKI_GetSkeletor(tiki,entnum);
	return ret;
}
void TIKI_SetEyeTargetPos( dtiki_t *tiki, int entnum, float *pos ) {
	cgi.TIKI_SetEyeTargetPos(tiki,entnum,pos);
}
char *Anim_NameForNum( dtiki_t *pmdl, int animnum ) {
	const char *ret = cgi.Anim_NameForNum(pmdl,animnum);
	return ret;
}
int Anim_NumForName( dtiki_t *pmdl, char *name ) {
	int ret = cgi.Anim_NumForName(pmdl,name);
	return ret;
}
int Anim_Random( dtiki_t *pmdl, char *name ) {
	int ret = cgi.Anim_Random(pmdl,name);
	return ret;
}
int Anim_NumFrames( dtiki_t *pmdl, int animnum ) {
	int ret = cgi.Anim_NumFrames(pmdl,animnum);
	return ret;
}
float Anim_Time( dtiki_t *pmdl, int animnum ) {
	float ret = cgi.Anim_Time(pmdl,animnum);
	return ret;
}
float Anim_Frametime( dtiki_t *pmdl, int animnum ) {
	float ret = cgi.Anim_Frametime(pmdl,animnum);
	return ret;
}
// WARNING: Anim_Delta might be NULL pointer in MOHAA
void Anim_Delta( dtiki_t *pmdl, int animnum, float *delta ) {
	cgi.Anim_Delta(pmdl,animnum,delta);
}
int Anim_Flags( dtiki_t *pmdl, int animnum ) {
	int ret = cgi.Anim_Flags( pmdl, animnum );
	return ret;
}
int Anim_FlagsSkel( dtiki_t *pmdl, int animnum ) {
	int ret = cgi.Anim_FlagsSkel( pmdl, animnum );
	return ret;
}
float Anim_CrossblendTime( dtiki_t *pmdl, int animnum ) {
	float ret = cgi.Anim_CrossblendTime( pmdl, animnum );
	return ret;
}
qboolean Anim_HasCommands( dtiki_t *pmdl, int animnum ) {
	qboolean ret = cgi.Anim_HasCommands( pmdl, animnum );
	return ret;
}
qboolean Frame_Commands( dtiki_t *pmdl, int animnum, int framenum, tiki_cmd_t *tiki_cmd ) {
	qboolean ret = cgi.Frame_Commands( pmdl, animnum, framenum, tiki_cmd );
	return ret;
}
qboolean Frame_CommandsTime( dtiki_t *pmdl, int animnum, float start, float end, tiki_cmd_t *tiki_cmd ) {
	qboolean ret = cgi.Frame_CommandsTime( pmdl, animnum, start, end, tiki_cmd );
	return ret;
}
int Surface_NameToNum( dtiki_t *pmdl, char *name ) {
	int ret = cgi.Surface_NameToNum( pmdl, name );
	return ret;
}
int Tag_NumForName( dtiki_t *pmdl, char *name ) {
	int ret = cgi.Tag_NumForName( pmdl, name );
	return ret;
}
char *Tag_NameForNum( dtiki_t *pmdl, int iTagNum ) {
	const char* ret = cgi.Tag_NameForNum( pmdl, iTagNum );
	return ret;
}
void ForceUpdatePose( refEntity_t *model ) {
	cgi.ForceUpdatePose(model);
}
// this might call "getDirtyTransform" from our cg_skeletor.c
// if model bones are not up to date...
orientation_t TIKI_Orientation( refEntity_t *model, int tagnum ) {
	orientation_t ret = cgi.TIKI_Orientation(model,tagnum);
	return ret;
}
// su44: this is called for "Bip01 R Foot" and "Bip01 L Foot" bones.
qboolean TIKI_IsOnGround( refEntity_t *model, int tagnum, float threshold ) {
	qboolean ret;
	const char *tagName;
	tagName = cgi.Tag_NameForNum(model->tiki,tagnum);
	ret = cgi.TIKI_IsOnGround(model,tagnum,threshold);
	return ret;
}