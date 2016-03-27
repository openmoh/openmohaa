/*
===========================================================================
Copyright (C) 2009-2010 su44

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
//translate bone_s b to pos/rot coordinates space. (treat pos/rot as parent bone orientation)
void CG_BoneLocal2World(bone_t *b, vec3_t pos, vec3_t rot, vec3_t outPos, vec3_t outRot) {
	quat_t q,qpos,res,tmp;
	quat_t outQuat;
#if 0
	QuatFromAngles(q,rot[0],rot[1],rot[2]);
#else
	matrix_t m;
	MatrixFromAngles(m,rot[0],rot[1],rot[2]);
	QuatFromMatrix(q,m);
#endif
	VectorCopy(b->p,qpos);
	qpos[3] = 0;


	QuaternionMultiply(tmp,qpos,q);

	QuatInverse(q);
	QuaternionMultiply(res,q,tmp);
	QuatInverse(q);

	outPos[0] = res[0] + pos[0];
	outPos[1] = res[1] + pos[1];
	outPos[2] = res[2] + pos[2];

	QuaternionMultiply(outQuat,b->q,q);
	QuatNormalize(outQuat);
	if(outRot) {
#if 0
		QuatToAngles(outQuat, outRot);
#else
		MatrixFromQuat(m,outQuat);
		MatrixToAngles(m,outRot);
#endif
	}
}
void CG_CentBoneLocal2World(bone_t *b, centity_t *cent, vec3_t outPos, vec3_t outRot) {
	CG_BoneLocal2World(b,cent->lerpOrigin,cent->lerpAngles,outPos,outRot);
}
void CG_CentBoneIndexLocal2World(int boneIndex, centity_t *cent, vec3_t outPos, vec3_t outRot) {
	CG_BoneLocal2World(cent->bones+boneIndex,cent->lerpOrigin,cent->lerpAngles,outPos,outRot);
}
int CG_TIKI_BoneIndexForName(tiki_t *tiki, char *name) {
	int nameIndex;
	int i;
	nameIndex = cgi.TIKI_GetBoneNameIndex(name);
	for(i = 0; i < tiki->numBones; i++) {
		if(tiki->boneNames[i] == nameIndex) {
			return i;
		}
	}
	return -1;
}
void CG_AttachEntity(refEntity_t *e, centity_t *parent, int boneIndex, vec3_t outAngles) {
	bone_t *b;
	vec3_t a;
	if(parent->bones==0)
		return;
	b = &parent->bones[boneIndex];
	CG_BoneLocal2World(b,parent->lerpOrigin,parent->lerpAngles,e->origin,a);
	AnglesToAxis(a,e->axis);
	if(outAngles)
		VectorCopy(a,outAngles);
}
qboolean CG_TIKI_BoneOnGround(centity_t *cent, tiki_t *tiki, int boneIndex)
{
	if(cent->bones == 0)
		return qfalse;
	//CG_Printf("Z: %f\n",cent->bones[boneIndex].p[2]);
	if(cent->bones[boneIndex].p[2] < 6)
		return qfalse;
	return qtrue;
}
void CG_ModelAnim( centity_t *cent )
{
	refEntity_t ent;
	entityState_t *s1;
	tiki_t *tiki;
	int i;
	qboolean attachedToViewmodel;
	s1 = &cent->currentState;

	attachedToViewmodel = qfalse;

	memset( &ent, 0, sizeof( ent ) );

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		frameInfo_t *frame = &cent->currentState.frameInfo[ i ];
		frameInfo_t *nextframe = &cent->nextState.frameInfo[ i ];

		if( frame->index != nextframe->index || !cent->interpolate ) {
			continue;
		}

		if( frame->time <= nextframe->time ) {
			frame->time += cg.frametime / 1000.0f;
		} else {
			frame->time -= cg.frametime / 1000.0f;
		}
	}

	if(s1->tag_num != -1 && s1->parent != 1023) {
		if(s1->parent == cg.clientNum && !cg.renderingThirdPerson) {
			ent.renderfx |= RF_THIRD_PERSON;
			attachedToViewmodel = qtrue;
		} else {
			CG_AttachEntity(&ent,&cg_entities[s1->parent],s1->tag_num,cent->lerpAngles);
			// save pos/rot for events
			VectorCopy(ent.origin,cent->lerpOrigin);
		}

	} else {
		AnglesToAxis(cent->lerpAngles,ent.axis);
		VectorCopy(cent->lerpOrigin,ent.origin);
	}

	if(s1->eType == ET_MOVER) {
		ent.model = cgs.inlineDrawModel[ s1->modelindex ];
	} else {
		ent.model = cgs.gameModels[ s1->modelindex ];
	}

	tiki = cgs.gameTIKIs[ s1->modelindex ];

	if(tiki && tiki->numAnims) {
		int idleIndex = 0;
		ent.bones = cgi.TIKI_GetBones(tiki->numBones);
		cent->bones = ent.bones;
#if 0
		for(i = 0; i < tiki->numAnims; i++) {
			if(!Q_stricmp(tiki->anims[i]->alias,"unarmed_stand_idle")) {
			idleIndex = i;
				break;
			}
		}
		if(idleIndex!=0) {
			cgi.TIKI_SetChannels(tiki,idleIndex,0,0,ent.bones);
		} else 
#endif


		{
			frameInfo_t *fi = s1->frameInfo;
			ClearBounds(ent.bounds[0],ent.bounds[1]);
			ent.radius = 0;
			for(i = 0; i < 16; i++)	{
				if(fi->weight!=0) {
					cgi.TIKI_AppendFrameBoundsAndRadius(tiki,fi->index,fi->time,&ent.radius,ent.bounds);
					cgi.TIKI_SetChannels(tiki,fi->index,fi->time,fi->weight,ent.bones);
				}
				fi++;
			}
		}

		/*if( cent->currentState.number == cg.clientNum )
		{
			vec3_t angles;

			QuatToAngles( ent.bones[ 0 ].q, angles );

			angles[ 0 ] -= 90.0f;
			angles[ 2 ] -= 90.0f;

			EulerToQuat( angles, ent.bones[ 0 ].q );
		}*/

#if 0
		for(i = 0; i < 5; i++)	{
			if(s1->bone_tag[i] != -1) {
				matrix_t m;
				matrix_t m2;
				matrix_t mf;
				quat_t q;
				//				CG_Printf("i %i of 5, tag %i, angles %f %f %f\n",i,s1->bone_tag[i],
				//					s1->bone_angles[i][0],s1->bone_angles[i][1],s1->bone_angles[i][2]);
				//bone_quat is always 0 0 0 0?
				if(s1->bone_quat[i][0] != 0 || s1->bone_quat[i][1] != 0 || s1->bone_quat[i][2] != 0 || s1->bone_quat[i][3] != 0) {
					CG_Printf("i %i of 5, tag %i, quat %f %f %f %f\n",i,s1->bone_tag[i],
						s1->bone_quat[i][0],s1->bone_quat[i][1],s1->bone_quat[i][2],s1->bone_quat[i][3]);
					__asm int 3
				}
				// s1->bone_angles[i][0] changes when looking up / down (PITCH)
				// s1->bone_angles[i][2] changes when leaning left/right (ROLL)
				//AnglesToMatrix(s1->bone_angles[i],m);
				// ...?
				MatrixFromAngles(m,-s1->bone_angles[i][2],-s1->bone_angles[i][0],0);
				MatrixFromQuat(m2,ent.bones[s1->bone_tag[i]].q);
				Matrix4x4Multiply(m,m2,mf);
				QuatFromMatrix(ent.bones[s1->bone_tag[i]].q,mf);
			}
	}
#endif

		cgi.TIKI_Animate( tiki, ent.bones );

		for( i = 0; i < NUM_BONE_CONTROLLERS; i++ )
		{
			int tag = s1->bone_tag[ i ];

			//if( tag == cgi.Tag_NumForName( tiki, "Bip01 Pelvis" ) ) {
			//	continue;
			//}

			if( tag >= 0 )
			{
				matrix_t m;
				matrix_t m2;
				matrix_t mf;
				quat_t quat;

				MatrixFromAngles( m, s1->bone_angles[ i ][ 0 ] * 2.0f, s1->bone_angles[ i ][ 1 ], s1->bone_angles[ i ][ 2 ] * 4.0f );
				MatrixFromQuat( m2, ent.bones[ tag ].q );
				Matrix4x4Multiply( m, m2, mf );
				QuatFromMatrix( quat, mf );

				/*MatrixFromAngles( m, -cent->nextState.bone_angles[ i ][ 2 ] * 4.0f, -cent->nextState.bone_angles[ i ][ 0 ], cent->nextState.bone_angles[ i ][ 1 ] );
				MatrixFromQuat( m2, ent.bones[ tag ].q );
				Matrix4x4Multiply( m, m2, mf );
				QuatFromMatrix( nextquat, mf );

				if( ( cent->interpolate ) && ( cent->nextState.bone_tag[ i ] == cent->currentState.bone_tag[ i ] ) )
				{
					QuatSlerp( quat, nextquat,
						cg.frameInterpolation, ent.bones[ tag ].q );
				}
				else*/
				{
					ent.bones[ tag ].q[ 0 ] = quat[ 0 ];
					ent.bones[ tag ].q[ 1 ] = quat[ 1 ];
					ent.bones[ tag ].q[ 2 ] = quat[ 2 ];
					ent.bones[ tag ].q[ 3 ] = quat[ 3 ];
				}
			}
		}
	}

	// player model
	if( cent->currentState.number == cg.clientNum ) {
#if 1 //calculate eye pos/rot for usereyes_t
		if( tiki ) {
			vec3_t pos, rot;
#if 0
			VectorSet( eyePos, 0, 0, cg.predictedPlayerState.viewheight );

			VectorCopy( cg.predictedPlayerState.viewangles, eyeRot );
			cgi.SetEyeInfo( eyePos, eyeRot );
#else
			int eyeBoneName;
			int torso_tag;
			eyeBoneName = cgi.TIKI_GetBoneNameIndex( "eyes bone" );//("tag_weapon_right");
			torso_tag = cgi.TIKI_GetBoneNameIndex( "Bip01 Spine1" );
			for( i = 0; i < tiki->numBones; i++ ) {
				if( tiki->boneNames[ i ] == eyeBoneName ) {
					CG_BoneLocal2World( ent.bones + i, ent.origin, cent->lerpAngles, pos, rot );

					VectorCopy( cg.predictedPlayerState.viewangles, rot );
					VectorSubtract( pos, cent->lerpOrigin, pos );
					cgi.SetEyeInfo( pos, rot );
				}
				/*else if( tiki->boneNames[ i ] == torso_tag )
				{
					bone_t *bone = ent.bones + i;

					CG_BoneLocal2World( ent.bones + i, ent.origin, cent->lerpAngles, pos, rot );

					VectorCopy( s1->bone_angles[ 1 ], rot );
					VectorSubtract( pos, cent->lerpOrigin, pos );

					VectorCopy( rot, bone->p );
					EulerToQuat( rot, bone->q );
				}*/
			}
#endif
		}
#endif
		if( !cg.renderingThirdPerson ) {
			ent.renderfx = RF_THIRD_PERSON;			// only draw in mirrors
		} else {
			if( cg_cameraMode->integer ) {
				return;
			}
		}

		CG_ViewModelAnim(); // maybe I should put it somewhere else..
	}

	if(cent->currentState.groundEntityNum == ENTITYNUM_NONE) {
		cent->bFootOnGround_Right = 0;
		cent->bFootOnGround_Left = 0;
	} else if(tiki) {
		int tagNum;

		tagNum = CG_TIKI_BoneIndexForName(tiki,"Bip01 L Foot");
		if(tagNum != -1) {
			if(CG_TIKI_BoneOnGround(cent,tiki,tagNum)) {
				if(!cent->bFootOnGround_Left) {
					CG_Footstep("Bip01 L Foot",cent,1,0);
				}
				cent->bFootOnGround_Left = 1;
			} else {
				cent->bFootOnGround_Left = 0;
			}
		}

		tagNum = CG_TIKI_BoneIndexForName(tiki,"Bip01 R Foot");
		if(tagNum != -1) {
			if(CG_TIKI_BoneOnGround(cent,tiki,tagNum)) {
				if(!cent->bFootOnGround_Right) {
					CG_Footstep("Bip01 R Foot",cent,1,0);
				}
				cent->bFootOnGround_Right = 1;
			} else {
				cent->bFootOnGround_Right = 0;
			}
		}
	}

	for( i = 0; i < 32; i++ )
	{
		if( cent->currentState.surfaces[ i ] & MDL_SURFACE_NODRAW )
		{
			ent.surfaceBits |= 1 << i;
		}
	}

	cgi.R_AddRefEntityToScene(&ent);
	if(attachedToViewmodel) {
		CG_AddViewModelAnimAttachment(&ent,cent);
	}

}
