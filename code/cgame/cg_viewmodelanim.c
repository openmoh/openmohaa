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

#include "cg_local.h"

#define VIEWMODEL_PITCH_FIX		8.0f

char itemNames[64][64];
void CG_RegisterItemName(int index, const char *str) {
	if(index >= 64 || index < 0) {
		CG_Error("CG_RegisterItemName: item index out of range 64 (%i, %s)\n",index,str);
		return;
	}
	strcpy(itemNames[index],str);
}

const char *CG_GetItemName(int index) {
	if(index >= 64 || index < 0) {
		CG_Error("CG_GetItemName: item index out of range 64 (%i)\n",index);
		return 0;
	}
	return itemNames[index];
}
//papers  colt45  p38 histandard  garand  kar98   kar98sniper springfield thompson    mp40    bar mp44    fraggrenade stielhandgranate    bazooka panzerschreck   shotgun unarmed
//Papers  Colt 45 Walther P38 Hi-Standard Silenced    M1 Garand   Mauser KAR 98K  KAR98 - Sniper  Springfield '03 Sniper  Thompson    MP40    BAR StG 44  Frag Grenade    Stielhandgranate    Bazooka Panzerschreck   Shotgun
const char *CG_GetVMAnimPrefixString(int index) {
	const char *wpn;
	wpn = CG_ConfigString( CS_WEAPONS + index ); //CG_GetItemName(index);
	if(wpn) {
		if(!Q_stricmp(wpn,"Papers"))
			return "papers";
		if(!Q_stricmp(wpn,"Colt 45"))
			return "colt45";
		if(!Q_stricmp(wpn,"Walther P38"))
			return "p38";
		if(!Q_stricmp(wpn,"Hi-Standard Silenced"))
			return "histandard";
		if(!Q_stricmp(wpn,"M1 Garand"))
			return "garand";
		if(!Q_stricmp(wpn,"Mauser KAR 98K"))
			return "kar98";
		if(!Q_stricmp(wpn,"KAR98 - Sniper"))
			return "kar98sniper";
		if(!Q_stricmp(wpn,"Springfield '03 Sniper"))
			return "springfield";
		if(!Q_stricmp(wpn,"Thompson"))
			return "thompson";
		if(!Q_stricmp(wpn,"MP40"))
			return "mp40";
		if(!Q_stricmp(wpn,"BAR"))
			return "bar";
		if(!Q_stricmp(wpn,"StG 44"))
			return "mp44";
		if(!Q_stricmp(wpn,"Frag Grenade"))
			return "fraggrenade";
		if(!Q_stricmp(wpn,"Stielhandgranate"))
			return "stielhandgranate";
		if(!Q_stricmp(wpn,"Bazooka"))
			return "bazooka";
		if(!Q_stricmp(wpn,"Panzerschreck"))
			return "panzerschreck";
		if(!Q_stricmp(wpn,"Shotgun"))
			return "shotgun";
	}
	return "unarmed";
}

char	*viewModelAnimTypes[] = {
	"idle", // 0
	"charge", // 1
	"fire", // 2
	"fire_secondary", // 3
	"rechamber", // 4
	"reload", // 5
	"reload_single", // 6
	"reload_end", // 7
	"pullout", // 8
	"putaway", // 9
	"ladderstep" // 10
};
const char *CG_GetVMTypeString(int index) {
	if(index < 0 || index > VMA_NUMANIMATIONS) {
		return "idle";
	}
	return viewModelAnimTypes[index];
}
void CG_AddViewModelAnimAttachment( refEntity_t *ent, centity_t *cent ) {
	vec3_t outRot;
	vec3_t angles;

	int boneName;
	int i;
	if(cg.renderingThirdPerson || cgi.anim->ref.bones == 0)
		return;
	if(cg.snap->ps.stats[STAT_INZOOM])
		return;
	ent->renderfx = RF_FIRST_PERSON;
#if 0 //doesnt work, tag_num is incorrect..
	CG_BoneLocal2World(cg.viewModelEnt.bones+cent->currentState.tag_num,cg.viewModelEnt.origin,cg.refdefViewAngles,ent->origin,outRot);
#else
	if(cgs.gameTIKIs[cg_entities[cent->currentState.parent].currentState.modelindex]==0) {
		//CG_Printf("CG_AddViewModelAnimAttachment: entity %i has null tiki; cannot attach %i\n",cent->currentState.parent, cent->currentState.number);
		return;
	}
	boneName = cgs.gameTIKIs[cg_entities[cent->currentState.parent].currentState.modelindex]->boneNames[cent->currentState.tag_num];
	for(i = 0; i < cgi.anim->tiki->numBones; i++) {
		if( boneName == cgi.anim->tiki->boneNames[ i ] )
			break;
	}
	if( i == cgi.anim->tiki->numBones )
		return;

	VectorCopy( cg.refdefViewAngles, angles );

	angles[ PITCH ] += VIEWMODEL_PITCH_FIX;

	CG_BoneLocal2World(cgi.anim->ref.bones+i,cgi.anim->ref.origin,angles,ent->origin,outRot);
#endif
	VectorCopy(ent->origin,cent->lerpOrigin);
	VectorCopy(outRot,cent->lerpAngles);

	AnglesToAxis(outRot,ent->axis);

	cgi.R_AddRefEntityToScene(ent);
}
void CG_CalcViewModelMovement(float fViewBobPhase, float fViewBobAmp, 
		float *vVelocity, float *vMovement ) {
	float tmp,tmp2;
	float frac;
	vec3_t vmOfs;
	float vMovementLen;
	vec3_t vMovementNormalized;
	float *curAnimVMPosOfs;
	int i;

	vMovement[1] = sin(fViewBobPhase + 0.3141592700403172) * fViewBobAmp * vm_sway_side->value;
	vMovement[0] = vMovement[1] * vm_sway_front->value;
	tmp = sin(fViewBobPhase - 0.9424778335276408 + fViewBobPhase - 0.9424778335276408 + 3.141592653589793)
	 + sin((fViewBobPhase - 0.9424778335276408) * 4.0 + 1.570796326794897) * 0.125;
	vMovement[2] = fViewBobAmp * tmp * vm_sway_up->value;

	//if(!cg.predictedPlayerState.walking) {
	if(cg.predictedPlayerState.groundEntityNum == ENTITYNUM_NONE) {
		// player is in air
		vmOfs[0] = vm_offset_air_front->value;
		vmOfs[1] = vm_offset_air_side->value;
		vmOfs[2] = vm_offset_air_up->value;
		// append offset depending on Z velocity
		if(vVelocity[2] != 0.f) {
			vmOfs[2] -= vVelocity[2] * vm_offset_upvel->value;
		}
	} else {
		// player is walking
		if ( cg.predictedPlayerState.viewheight == 48 ) {
			// and crouching?

			/*
			// special cases for shotgun and rocked launchers - TODO
			if(using shotgun) {
				vmOfs[0] = vm_offset_shotguncrouch_front->value;
				vmOfs[1] = vm_offset_shotguncrouch_side->value;
				vmOfs[2] = vm_offset_shotguncrouch_up->value;
			} else if(using rockets) {
				vmOfs[0] = vm_offset_rocketcrouch_front->value;
				vmOfs[1] = vm_offset_rocketcrouch_side->value;
				vmOfs[2] = vm_offset_rocketcrouch_up->value;
			} else
			*/
			{
				vmOfs[0] = vm_offset_crouch_front->value;
				vmOfs[1] = vm_offset_crouch_side->value;
				vmOfs[2] = vm_offset_crouch_up->value;
			}
		} else {
			// player is standing
			VectorClear(vmOfs);
		}
		tmp = VectorLength(vVelocity) - vm_offset_vel_base->value;

		if(tmp > 0) {
			tmp2 = 250.0 - vm_offset_vel_base->value;
			if( tmp > tmp2 )
				tmp = tmp2;
			frac = tmp / tmp2;	

			vmOfs[0] += frac * vm_offset_vel_front->value;
			vmOfs[1] += frac * vm_offset_vel_side->value;
			vmOfs[2] += frac * vm_offset_vel_up->value;
		}



	}


	// for each coordinate
	curAnimVMPosOfs = &cgi.anim->currentVMPosOffset[0];
	for(i = 0; i < 3; i++) {
		float ofsDelta;
		ofsDelta = vmOfs[i] - curAnimVMPosOfs[i];
		curAnimVMPosOfs[i] += (float)cg.frametime
			* 0.001
			* ofsDelta
			* vm_offset_speed->value;
		//if(ofsDelta < 0) {
		//	if ( vmOfs[i] <= curAnimVMPosOfs[i] ) {

		//	} else {
		//		curAnimVMPosOfs[i] = vmOfs[i];
		//	}
		//} else if(ofsDelta > 0) {
		//	if ( vmOfs[i] < curAnimVMPosOfs[i] ) {

		//	} else {
		//		curAnimVMPosOfs[i] = vmOfs[i];
		//	}
		//}
	}

	vMovement[ 0 ] = vMovement[ 0 ] + cgi.anim->currentVMPosOffset[ 0 ];
	vMovement[ 1 ] = vMovement[ 1 ] + cgi.anim->currentVMPosOffset[ 1 ];
	vMovement[ 2 ] = vMovement[ 2 ] + cgi.anim->currentVMPosOffset[ 2 ];

	// add lean offset
	if(cg.predictedPlayerState.fLeanAngle != 0.f) {
		vMovement[2] -= fabs(cg.predictedPlayerState.fLeanAngle) * vm_lean_lower->value;
	}
	vMovementLen = VectorNormalize2(vMovement, vMovementNormalized);
	if(vm_offset_max->value < vMovementLen) {
		vMovement[0] = vm_offset_max->value * vMovementNormalized[0];
		vMovement[1] = vm_offset_max->value * vMovementNormalized[1];
		vMovement[2] = vm_offset_max->value * vMovementNormalized[2];
	}



}

void CG_ViewModelAnim() {
	char anim[128];
	char tmp[128];
	int i;
	char *ptr;
	tiki_t *fps, *tiki;
	refEntity_t *ent;
	bone_t *bone;
	vec3_t v;
	int boneName;
	matrix_t m;
	vec3_t vMovement, angles;
	int slot;
	int index;
	float crossblend;
	float time, weight;
	qboolean m_bAnimChange = qfalse;

	if( cg.renderingThirdPerson )
		return;

	if( cg.snap->ps.stats[ STAT_INZOOM ] )
		return;

	slot = cgi.anim->currentVMAnimSlot;

	if( slot < 0 || slot > MAX_FRAMEINFOS )
	{
		slot = 0;
	}

	ent = &cgi.anim->ref;

	VectorCopy( cg.refdefViewAngles, angles );

	angles[ PITCH ] += VIEWMODEL_PITCH_FIX;

	//CG_Printf("vma changed %i\n",cg.predictedPlayerState.iViewModelAnimChanged);

	// mp40_reload, mp40_fire
	if(cg.predictedPlayerState.activeItems[ITEM_WEAPON]!=-1)	{
		sprintf(anim,"%s_%s",CG_GetVMAnimPrefixString(cg.predictedPlayerState.activeItems[ITEM_WEAPON]),CG_GetVMTypeString(cg.predictedPlayerState.iViewModelAnim));
		//CG_Printf("VMA %s \n",anim);
	} else
		return;
#if 0
	for(i = 0; i < 8; i++) {
		if(cg.predictedPlayerState.activeItems[i]!= -1 && cg.predictedPlayerState.activeItems[i]) {
			CG_Printf("Item %i of 8, %s (indx %i)\n",i,itemNames[cg.predictedPlayerState.activeItems[i]],cg.predictedPlayerState.activeItems[i]);
		}
		/*
item 0 "pistol"
item 1 "Walther P38"
item 2 "Binoculars"

item 0 "mg"
item 1 "MP44"
item 2 "Binoculars"
		*/
	}
#endif
	//tiki = cgs.gameTIKIs[cg.predictedPlayerEntity.currentState.modelindex]; // doesnt work... currentState.modelindex == 0
	tiki = cgs.gameTIKIs[cg_entities[cg.clientNum].currentState.modelindex];
	if(!tiki) {
		if(cg.snap->ps.stats[STAT_TEAM]==TEAM_AXIS) {
			tiki = cgi.TIKI_RegisterModel("models/player/german_wehrmacht_soldier.tik");
		} else {
			tiki = cgi.TIKI_RegisterModel("models/player/american_army.tik");
		}
		if(!tiki)
			return;
	}
	strcpy(tmp,tiki->name);
	ptr = strchr(tmp,'.');

	if( !ptr )
		return;

	*ptr = 0;
	strcat(tmp,"_fps.tik");
	fps = cgi.TIKI_RegisterModel(tmp);

	if(!fps) {
		CG_Printf("CG_ViewModelAnim: error, cannot register first person player model for %s\n",tiki->name);
		return;
	}

	cgi.anim->tiki = fps;

	if( cgi.anim->lastVMAnim == -1 )
	{
		index = cgi.Anim_Random( fps, anim );

		if( index < 0 )
		{
			index = cgi.Anim_Random( fps, "idle" );

			if( index == -1 ) {
				index = 0;
			}

			cgi.Printf( "Warning: #1 Couldn't find view model animation \"%s\"\n", anim );
		}

		cgi.anim->vmFrameInfo[ slot ].index = index;

		cgi.anim->vmFrameInfo[ slot ].time = 0.0f;
		cgi.anim->vmFrameInfo[ slot ].weight = 1.0f;
		cgi.anim->lastVMAnim = VMA_IDLE;
	}

	if( cg.snap->ps.iViewModelAnimChanged != cgi.anim->lastVMAnimChanged )
	{
		m_bAnimChange = qtrue;

		cgi.anim->lastVMAnim = cg.snap->ps.iViewModelAnim;
		cgi.anim->lastVMAnimChanged = cg.snap->ps.iViewModelAnimChanged;
	}

	if( m_bAnimChange )
	{
		crossblend = cgi.Anim_CrossblendTime( tiki, cgi.anim->vmFrameInfo[ slot ].index );
		time = cgi.anim->currentVMDuration / 1000.0f;

		if( time < crossblend )
		{
			weight = time / crossblend;

			for( i = 0; i < MAX_FRAMEINFOS; i++ )
			{
				float w;

				w = cgi.anim->vmFrameInfo[ i ].weight;

				if( w == 0.0f )
				{
					continue;
				}

				if( i == slot )
				{
					cgi.anim->vmFrameInfo[ i ].weight = weight;
				}
				else
				{
					w = ( 1.0f - weight ) * cgi.anim->vmFrameInfo[ i ].weight;
					cgi.anim->vmFrameInfo[ i ].weight = w;
				}
			}
		}

		slot = ( slot + 1 ) % MAX_FRAMEINFOS;
		cgi.anim->currentVMAnimSlot = slot;

		index = cgi.Anim_Random( fps, anim );

		if( index < 0 )
		{
			cgi.Printf( "Warning: #2 Couldn't find view model animation \"%s\"\n", anim );

			sprintf( anim, "%s_idle", CG_GetVMAnimPrefixString( cg.predictedPlayerState.activeItems[ ITEM_WEAPON ] ) );

			index = cgi.Anim_Random( fps, anim );

			if( index < 0 ) {
				index = 0;
			}
		}

		cgi.anim->vmFrameInfo[ slot ].index = index;
		cgi.anim->vmFrameInfo[ slot ].time = 0.0f;
		cgi.anim->vmFrameInfo[ slot ].weight = 1.0f;

		cgi.anim->currentVMDuration = 0;

		crossblend = cgi.Anim_CrossblendTime( tiki, index );

		if( crossblend == 0.0f )
		{
			for( i = 0; i < MAX_FRAMEINFOS; i++ )
			{
				if( i != slot ) {
					cgi.anim->vmFrameInfo[ i ].weight = 0.0f;
				}
			}

			cgi.anim->crossBlending = qfalse;
		}
		else
		{
			cgi.anim->crossBlending = qtrue;
		}
	}

	cgi.anim->currentVMDuration += cg.frametime / 2;

	if( cgi.anim->crossBlending )
	{
		crossblend = cgi.Anim_CrossblendTime( tiki, cgi.anim->vmFrameInfo[ slot ].index );
		time = cgi.anim->currentVMDuration / 1000.0f;

		if( time >= crossblend )
		{
			for( i = 0; i < MAX_FRAMEINFOS; i++ )
			{
				if( slot != i ) {
					cgi.anim->vmFrameInfo[ i ].weight = 0.0f;
				}
			}

			cgi.anim->crossBlending = qfalse;
		}
		else
		{
			weight = time / crossblend;
		}
	}

	// su44: ok, we have chosen the proper viewmodelanim
	// but ent->origin and ent->axis still needs to be adjusted
	memset( ent, 0, sizeof( *ent ) );

	ent->renderfx = RF_FIRST_PERSON;
	ent->model = cgi.R_RegisterModel( tmp );
	ent->bones = cgi.TIKI_GetBones( fps->numBones );

	// dont bother culling hands model
	VectorSet( ent->bounds[ 0 ], -99999, -99999, -99999 );
	VectorSet( ent->bounds[ 1 ], 99999, 99999, 99999 );

	ent->radius = 99999;

	//ClearBounds( ent->bounds[ 0 ], ent->bounds[ 1 ] );
	//ent->radius = 0;

	// su44: kinda strange, but thats how its done by MoHAA
	// we have two different hand surfaces - one is visible only while carrying rifles,
	// second is used for the rest of weapons
	ptr = itemNames[ cg.predictedPlayerState.activeItems[ ITEM_WEAPON ] ];

	if( !Q_stricmp( ptr, "M1 Garand" ) || !Q_stricmp( ptr, "Springfield '03 Sniper" )
		|| !Q_stricmp( ptr, "KAR98 - Sniper" ) || !Q_stricmp( ptr, "Mauser KAR 98K" ) )
	{
		ent->surfaceBits |= ( 1 << TIKI_GetSurfaceIndex( fps, "lefthand" ) ); // hide "lefthand" surface
	}
	else
	{
		ent->surfaceBits |= ( 1 << TIKI_GetSurfaceIndex( fps, "garandhand" ) ); // hide "garandhand" surface
	}

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		float w, frametime;

		if( cgi.anim->vmFrameInfo[ i ].index >= 0 && cgi.anim->vmFrameInfo[ i ].weight != 0.0f )
		{
			//cgi.TIKI_AppendFrameBoundsAndRadius( fps, cgi.anim->vmFrameInfo[ i ].index, cgi.anim->vmFrameInfo[ i ].time, &ent->radius, ent->bounds );
			cgi.TIKI_SetChannels( fps, cgi.anim->vmFrameInfo[ i ].index, cgi.anim->vmFrameInfo[ i ].time, cgi.anim->vmFrameInfo[ i ].weight, ent->bones );

			w = cgi.anim->vmFrameInfo[ i ].weight;

			if( w == 0.0f )
			{
				cgi.anim->vmFrameInfo[ i ].index = 0;
				cgi.anim->vmFrameInfo[ i ].time = 0.0f;
				cgi.anim->vmFrameInfo[ i ].weight = 0.0f;

				continue;
			}

			frametime = cgi.Anim_Frametime( tiki, cgi.anim->vmFrameInfo[ i ].index );

			cgi.anim->vmFrameInfo[ i ].time += cg.frametime / 2000.0f;

			if( cgi.anim->crossBlending )
			{
				if( slot == i )
				{
					cgi.anim->vmFrameInfo[ i ].weight = weight;
				}
				else
				{
					cgi.anim->vmFrameInfo[ i ].weight = ( 1.0f - weight ) * cgi.anim->vmFrameInfo[ i ].weight;
				}
			}
			else
			{
				cgi.anim->vmFrameInfo[ i ].weight = 1.0f;
			}
		}
	}

	cgi.TIKI_Animate( fps, ent->bones );

	CG_CalcViewModelMovement(cg.fCurrentViewBobPhase,cg.fCurrentViewBobAmp,cg.predictedPlayerState.velocity,vMovement);
	//CG_Printf("Movement: %f %f %f\n",vMovement[0],vMovement[1],vMovement[2]);

	// in MoHAA "eyes bone" is used here
	bone = 0;
	boneName = cgi.TIKI_GetBoneNameIndex("eyes bone"); 

	for(i = 0; i < fps->numBones; i++)
	{
		if( fps->boneNames[ i ] == boneName )
		{
			bone = ent->bones+i;
			break;
		}
	}

	if( bone == 0 )
	{
		CG_Printf("CG_OffsetFirstPersonView warning: Couldn't find 'eyes bone' for player\n");
	}
#if 1

	bone->p[ 0 ] += 4.55f; // front
	bone->p[ 1 ] += 0.15f; // side
	bone->p[ 2 ] -= 1.25f; // up

	VectorSubtract( bone->p, vMovement, bone->p );


#if 0
	CG_BoneLocal2World( bone, vec3_origin, cg.refdefViewAngles, v, a );
#else
	MatrixFromAngles( m, cg.refdefViewAngles[ 0 ], cg.refdefViewAngles[ 1 ], cg.refdefViewAngles[ 2 ] );
	MatrixTransformPoint( m, bone->p, v );
#endif
	VectorSubtract( cg.refdef.vieworg, v, ent->origin );
#else
	VectorCopy(cg.predictedPlayerState.origin,ent->origin);
	ent->origin[2] += cg.predictedPlayerState.viewheight;
	//ent->origin[2] += bone->p[2];
	// transform vMovement from players model space to worldspace
	MatrixFromAngles(m,cg.refdefViewAngles[0],cg.refdefViewAngles[1],cg.refdefViewAngles[2]);
	MatrixTransformPoint(m,vMovement,v);
	// add it
	VectorAdd(ent->origin,v,ent->origin);
#endif

	VectorMA( ent->origin, cg_gun_x->value, cg.refdef.viewaxis[ 0 ], ent->origin );
	VectorMA( ent->origin, cg_gun_y->value, cg.refdef.viewaxis[ 1 ], ent->origin );
	VectorMA( ent->origin, cg_gun_z->value, cg.refdef.viewaxis[ 2 ], ent->origin );

	AnglesToAxis( angles, ent->axis );

	cgi.R_AddRefEntityToScene( ent );
}
