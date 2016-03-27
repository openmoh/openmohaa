/*
===========================================================================
Copyright (C) 2011 su44

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
// cg_parsemsg.c

#include "cg_local.h"

#define MAX_HDELEMENTS 256

hdelement_t hdelements[MAX_HDELEMENTS];

typedef struct {
	float i_vBarrel[3];
	float i_vStart[3];
	float (*i_vEnd)[3];
	int i_iNumBullets;
	qboolean iLarge;
	int iTracerVisible;
	qboolean bIgnoreEntities;
} bullet_tracer_t;

#define MAX_IMPACTS 64
#define MAX_BULLER_TRACERS 32
#define MAX_BULLET_TRACE_BULLETS 1024

// bullet tracers
static bullet_tracer_t bullet_tracers[MAX_BULLER_TRACERS];
static int bullet_tracers_count;
static float bullet_tracer_bullets[MAX_BULLET_TRACE_BULLETS][3];
static int bullet_tracer_bullets_count;

// wall impacts
static vec3_t wall_impact_pos[MAX_IMPACTS];
static vec3_t wall_impact_norm[MAX_IMPACTS];
static int wall_impact_large[MAX_IMPACTS];
static int wall_impact_type[MAX_IMPACTS]; // 0,2,3,4,5
static int wall_impact_count;
// flesh impacts
static vec3_t flesh_impact_pos[MAX_IMPACTS];
static vec3_t flesh_impact_norm[MAX_IMPACTS];
static int flesh_impact_large[MAX_IMPACTS];
static int flesh_impact_type[MAX_IMPACTS];
static int flesh_impact_count;

static int current_entity_number;

void CG_MakeBulletHole(float *i_vPos, float *i_vNorm, int iLarge,
	trace_t *pPreTrace, qboolean bMakeSound) {
	char *s;

#if 0
	// su44: make an "explosion" effect here so I can debug bullet holes coordinates
	CG_MakeExplosion(i_vPos,vec3_origin,0,cgi.R_RegisterShader("heavy_pipe"),250,qtrue);
#endif
	s = "bhole_";

	// TODO: choose an appropriate decal shader and play hit sound

	CG_ImpactMark(cgi.R_RegisterShader("bhole_wood"),i_vPos,i_vNorm,0,1,1,1,1,qfalse,8,qfalse);
}

void CG_AddBulletImpacts() {
	int i;
	char *s;
	int type;

	for(i = 0; i < wall_impact_count; i++) {
		CG_MakeBulletHole(wall_impact_pos[i],wall_impact_norm[i],
			wall_impact_large[i],0,qtrue);

		type = wall_impact_type[i];
		if(type) {
			if ( wall_impact_type[i] < 2 || wall_impact_type[i] > 3 )
				s = "snd_bh_metal";
			else
				s = "snd_bh_wood";
			//CG_PlaySound(s,wall_impact_pos[i],-1, 
		}
	}
	for(i = 0; i < flesh_impact_count; i++) {

	}
}

static void CG_MakeBubbleTrail(float *i_vStart, float *i_vEnd, int iLarge) {
	// TODO
}
// this is called from CG_MakeBulletTracerInternal
static void CG_BulletTracerEffect(float *i_vStart, float *i_vEnd)
{
	byte fTracerColor[4]; // su44: in MoHAA it was "float[4]"
	int iLife;
	vec3_t vNewStart;
	int v2;

	VectorCopy(i_vStart,vNewStart);
	
	v2 = 1;

	iLife = 20;
	fTracerColor[0] = 255;
	fTracerColor[1] = 255;
	fTracerColor[2] = 255;
	fTracerColor[3] = 255;
	CG_CreateBeam(
			vNewStart, // start
			vec3_origin, // dir
			0, // owner
			1, // model
			1.0, // alpha
			1.0, // scale
			8192, // flags
			1000.0, // len
			iLife, // life
			1, // qboolean create
			i_vEnd, // endpointvec
			0, // min_ofs
			0, // maxs_ofs
			0, // overlap
			1, // numSubdivions
			0, // delay
			"tracer", // beamshadername
			fTracerColor, // modulate (in FAKK that's a byte[4], in MoH that's float[4])
			0, // numSphereBeams
			0.0, // sphereradius
			v2, //toggleDelay
			1.0, //endapha
			0, // renderfx
			"tracereffect"); // name
}

static void CG_MakeBulletTracerInternal(float *i_vBarrel, float *i_vStart,
	float (*i_vEnd)[3], int i_iNumBullets, qboolean iLarge,
	int iTracerVisible, qboolean bIgnoreEntities) {
	trace_t tr;
	int i;

	for(i = 0; i < i_iNumBullets; i++) {
		CG_BulletTracerEffect(i_vBarrel,i_vEnd[i]);
		CG_Trace(&tr,i_vBarrel,vec3_origin,vec3_origin,i_vEnd[i],-1,MASK_SHOT);
		if(tr.fraction != 1.f && tr.allsolid == qfalse) {
			CG_MakeBulletHole(tr.endpos,tr.plane.normal,iLarge,&tr,qfalse);
		} else {
			// su44: it happens quite often. Is it a bug?
			//CG_Printf("CG_MakeBulletTracerInternal: missed\n");
		}
	}

}

static void CG_MakeBulletTracer(float *i_vBarrel, float *i_vStart,
	float (*i_vEnd)[3], int i_iNumBullets, qboolean iLarge,
	int iTracerVisible, qboolean bIgnoreEntities) {
	bullet_tracer_t *bt;
	int i;
	if(bullet_tracers_count < MAX_BULLER_TRACERS) {
		if(i_iNumBullets + bullet_tracer_bullets_count < MAX_BULLET_TRACE_BULLETS) {
			bt = bullet_tracers + bullet_tracers_count;
			VectorCopy(i_vBarrel,bt->i_vBarrel);
			VectorCopy(i_vStart,bt->i_vStart);
			bt->i_vEnd = &bullet_tracer_bullets[bullet_tracer_bullets_count];
			bt->i_iNumBullets = i_iNumBullets;
			bt->iLarge = iLarge;
			bt->iTracerVisible = iTracerVisible;
			bt->bIgnoreEntities = bIgnoreEntities;
			bullet_tracers_count++;
			for(i = 0; i < i_iNumBullets; i++) {
				VectorCopy(i_vEnd[i],bullet_tracer_bullets[bullet_tracer_bullets_count]);
				bullet_tracer_bullets_count++;
			}
		} else {
			CG_Printf("CG_MakeBulletTracerInternal: MAX_BULLET_TRACE_BULLETS exceeded\n");
		}
	} else {
		CG_Printf("CG_MakeBulletTracer: MAX_BULLER_TRACERS exceeded\n");
	}
}

// su44: this should be called after CG_AddPacketEntities and before CG_DrawActive
void CG_AddBulletTracers() {
	int i;
	bullet_tracer_t *bt;

	bt = bullet_tracers;
	for(i = 0; i < bullet_tracers_count; i++,bt++) {
		CG_MakeBulletTracerInternal(bt->i_vBarrel,bt->i_vStart,bt->i_vEnd,bt->i_iNumBullets,
			bt->iLarge,bt->iTracerVisible,bt->bIgnoreEntities);
	}
	bullet_tracers_count = 0;
	bullet_tracer_bullets_count = 0;
}

static void CG_MakeExplosionEffect(float *vPos, int iType) {
	CG_MakeExplosion(vPos,vec3_origin,0,cgi.R_RegisterShader("gren_explosion"),1000,qtrue);
}

static void CG_MakeEffect_Normal(int iEffect, vec3_t vPos, vec3_t vNormal) {
	// TODO
}

static void CG_SpawnEffectModel(const char *model, vec3_t origin) {
	localEntity_t *le;
	vec3_t spawnAngles = { 0, 0, 0 };
	qhandle_t h;

	le = CG_AllocLocalEntity();
	le->endTime = cg.time + 5000;
	le->leType = LE_SKIP;
	le->pos.trTime = cg.time - (rand()&15);
	VectorCopy( origin, le->pos.trDelta );
	VectorCopy(origin,le->refEntity.origin);
	VectorCopy( spawnAngles, le->angles.trDelta );
	AnglesToAxis(spawnAngles,le->refEntity.axis);

	h = cgi.R_RegisterModel(model);
	le->refEntity.model = h;
	le->leType = LE_FRAGMENT;
	le->tiki = cgi.TIKI_RegisterModel(model);
}

void CG_HudDrawElements() {
	int i;
	hdelement_t *hdi;

	//if ( cg_huddraw_force.integer || cg_hud.integer )
	{
		hdi = hdelements;
		for ( i = 0; i < MAX_HDELEMENTS; i++, hdi++ ) {
			if ( hdi->hShader || hdi->string[0] ) {
				if ( hdi->vColor[3] != 0.f ) {
					float x = hdi->iX;
					float y = hdi->iY;
					float w = hdi->iWidth;
					float h = hdi->iHeight;

					if ( hdi->iHorizontalAlign == 1 ) {
						if ( hdi->bVirtualScreen )
							x = 320.f - w * 0.5f + x;
						else
							x = cgs.glconfig.vidWidth * 0.5f - w * 0.5f + x;
					} else {
						if ( hdi->iHorizontalAlign == 2 ) {
							if ( hdi->bVirtualScreen )
								x = x + 640.f;
							else
								x = cgs.glconfig.vidWidth + x;
						}
					}

					if ( hdi->iVerticalAlign == 1 ) {
						if ( hdi->bVirtualScreen )
							y = 240.f - h * 0.5f + y;
						else
							y = cgs.glconfig.vidHeight * 0.5f - h * 0.5f + y;
					} else {
						if ( hdi->iVerticalAlign == 2 ) {
							if ( hdi->bVirtualScreen )
								y = y + 480.f;
							else
								y = cgs.glconfig.vidHeight + y;
						}
					}

					cgi.R_SetColor(hdi->vColor);

					// su44: that's wrong
					if(hdi->bVirtualScreen) {
						if(x > 640) 
							x -= 640;
						if(x < 0)
							x += 640;
						if(y > 480) 
							y -= 480;
						if(y < 0)
							y += 480;
					}

					if ( hdi->string[0] ) {
						if ( hdi->pFont.glyphs[0].glyph ) {
							cgi.R_Text_Paint(&hdi->pFont, x, y, 1, 1, hdi->string,0,-1, qfalse, hdi->bVirtualScreen);
						} else {
							cgi.R_Text_Paint(&cgs.media.verdana, x, y, 1, 1, hdi->string,0,-1, qfalse, hdi->bVirtualScreen);
						}
					} else {
						if ( hdi->bVirtualScreen )
							CG_AdjustFrom640(&x, &y, &w, &h);

						cgi.R_DrawStretchPic(x, y, w, h, 0.f, 0.f, 1.f, 1.f, hdi->hShader);
					}
				}
			}
		}
	}
}

static void CG_HudDrawShader (int iInfo) {
	hdelement_t *hdi = hdelements + iInfo;
	if(hdi->shaderName[0]) {
		hdi->hShader = cgi.R_RegisterShaderNoMip(hdi->shaderName);
	} else {
		hdi->hShader = 0;
	}
}

static void CG_HudDrawFont (int iInfo) {
	hdelement_t *hdi = hdelements + iInfo;
	if(hdi->fontName[0]) {
		cgi.R_RegisterFont(hdi->fontName,0,&hdi->pFont);
	} else {
		memset(&hdi->pFont,0,sizeof(hdi->pFont));
	}
}

static void CG_PlaySound(char *sound_name, float *origin, int channel,
	float volume, float min_distance, float pitch, int argstype) {
	ubersound_t* sound;

	sound = CG_GetUbersound(sound_name);

	if(sound)
		cgi.S_StartSound( origin, -1, sound->channel, sound->sfxHandle );


}

/*
=====================
CG_ParseCGMessage
=====================
*/
void CG_ParseCGMessage() {
	int msgtype;
	vec3_t vecStart, vecTmp, vecEnd, vecArray[64];
	int iCount, iLarge;
	int i, iTemp;
	char *s;
	hdelement_t *hde;

	// thats a hack, it should be done somewhere else [?]
	wall_impact_count = 0;
	flesh_impact_count = 0;

	do {
		msgtype = cgi.MSG_ReadBits( 6 );
		if(cg_debugCGMessages->integer) {
			Com_Printf( "CG_ParseCGMessage: command type %i\n", msgtype );
		}
		switch ( msgtype ) {
			case 1: // BulletTracer (visible?)
				vecTmp[0] = cgi.MSG_ReadCoord();
				vecTmp[1] = cgi.MSG_ReadCoord();
				vecTmp[2] = cgi.MSG_ReadCoord();
			case 2: // BulletTracer (invisible?)
			case 5: // BubbleTrail
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();
				if ( msgtype != 1 ) {
					vecTmp[0] = vecStart[0];
					vecTmp[1] = vecStart[1];
					vecTmp[2] = vecStart[2];
				}
				vecArray[0][0] = cgi.MSG_ReadCoord();
				vecArray[0][1] = cgi.MSG_ReadCoord();
				vecArray[0][2] = cgi.MSG_ReadCoord();

				iLarge = cgi.MSG_ReadBits( 1 );
				if(msgtype==5) {
					CG_MakeBubbleTrail(vecStart,vecArray[0],iLarge);
				} else if(msgtype==1) {
					// su44: I am not sure about the last parameters...
					CG_MakeBulletTracer(vecTmp, vecStart, vecArray, 1, iLarge, 1, 1);
				} else { //msgtype == 2
					CG_MakeBulletTracer(vecTmp, vecStart, vecArray, 1, iLarge, 0, 1);
				}
				break;
			case 3: // BulletTracer multiple times
				vecTmp[0] = cgi.MSG_ReadCoord();
				vecTmp[1] = cgi.MSG_ReadCoord();
				vecTmp[2] = cgi.MSG_ReadCoord();
				iTemp = cgi.MSG_ReadBits( 6 );
			case 4: // BulletTracer multiple times (shotgun shot)
				if ( msgtype == 4 )
					iTemp = 0;
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				iLarge = cgi.MSG_ReadBits( 1 );
				iCount = cgi.MSG_ReadBits( 6 );

				// this check is missing in MOHAA code, so this has buffer overflow risk in AA
				if ( iCount > 64 )
					Com_Error( ERR_DROP, "CG message type 4 sent too many data.\n" );
				for (i=0;i<iCount;i++) {
					vecArray[i][0] = cgi.MSG_ReadCoord();
					vecArray[i][1] = cgi.MSG_ReadCoord();
					vecArray[i][2] = cgi.MSG_ReadCoord();
				}
				CG_MakeBulletTracer(vecTmp, vecStart, vecArray, iCount, iLarge, iTemp, 1);
				break;
			case 6: // wall impact
			case 7: // flesh impact
			case 8: // flesh impact (?)
			case 9: // wall impact (?)
			case 10: // wall impact (?)
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				cgi.MSG_ReadDir( vecEnd );
				iLarge = cgi.MSG_ReadBits( 1 );
				switch (msgtype) {
					case 6:
						if(wall_impact_count < MAX_IMPACTS) {
							VectorCopy(vecStart,wall_impact_pos[wall_impact_count]);
							VectorCopy(vecEnd,wall_impact_norm[wall_impact_count]);
							wall_impact_large[wall_impact_count] = iLarge;
							wall_impact_type[wall_impact_count] = 0;
							wall_impact_count++;
						}
						break;
					case 7:
						if(flesh_impact_count < MAX_IMPACTS) {
							// inverse hit normal, I dont know why,
							// but that's done by MoHAA
							VectorInverse(vecEnd);
							VectorCopy(vecStart,flesh_impact_pos[flesh_impact_count]);
							VectorCopy(vecEnd,flesh_impact_norm[flesh_impact_count]);
							flesh_impact_large[flesh_impact_count] = iLarge;
							flesh_impact_count++;
						}
						break;
					case 8:
						if(flesh_impact_count < MAX_IMPACTS) {
							// same here?
							VectorInverse(vecEnd);
							VectorCopy(vecStart,flesh_impact_pos[flesh_impact_count]);
							VectorCopy(vecEnd,flesh_impact_norm[flesh_impact_count]);
							flesh_impact_large[flesh_impact_count] = iLarge;
							flesh_impact_count++;
						}
						break;
					case 9:
						if(wall_impact_count < MAX_IMPACTS) {
							VectorCopy(vecStart,wall_impact_pos[wall_impact_count]);
							VectorCopy(vecEnd,wall_impact_norm[wall_impact_count]);
							wall_impact_large[wall_impact_count] = iLarge;
							if(iLarge) {
								wall_impact_type[wall_impact_count] = 3;
							} else {
								wall_impact_type[wall_impact_count] = 2;
							}
							wall_impact_count++;
						}
						break;
					case 10:
						if(wall_impact_count < MAX_IMPACTS) {
							VectorCopy(vecStart,wall_impact_pos[wall_impact_count]);
							VectorCopy(vecEnd,wall_impact_norm[wall_impact_count]);
							wall_impact_large[wall_impact_count] = iLarge;
							if(iLarge) {
								wall_impact_type[wall_impact_count] = 5;
							} else {
								wall_impact_type[wall_impact_count] = 4;
							}
							wall_impact_count++;
						}
						break;
					default:
						break;
				}
				break;
			case 11:
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();
				vecEnd[0] = cgi.MSG_ReadCoord();
				vecEnd[1] = cgi.MSG_ReadCoord();
				vecEnd[2] = cgi.MSG_ReadCoord();
				CG_MeleeImpact(vecStart,vecEnd);
				break;
			case 12: // m1 frag/stiel grenade explosion
			case 13: // bazooka/panzershrek projectile explosion
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();
				CG_MakeExplosionEffect(vecStart,msgtype);
				break;
			default: //unknown message
				Com_Error(ERR_DROP, "CG_ParseCGMessage: Unknown CG Message %i", msgtype);
				break;
			case 14:
			case 15: // MakeEffect
			case 16:
			case 17:
			case 18:
			case 19: // oil barrel effect
			case 20:
			case 21: // oil barrel effect top
			case 22: // oil barrel effect top - first hit
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();
				cgi.MSG_ReadDir( vecEnd );
				CG_MakeEffect_Normal(msgtype/* + 67*/,vecStart,vecEnd);
				break;
			case 23: // broke crate
			case 24: // broke glass window
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();
				i = cgi.MSG_ReadByte();
				if(msgtype == 23) {
					s = va("models/fx/crates/debris_%i.tik",i);
				} else {
					s = va("models/fx/windows/debris_%i.tik",i);
				}
				CG_SpawnEffectModel(s, vecStart);
				break;
			case 25: // Bullet tracer
				vecTmp[0] = cgi.MSG_ReadCoord();
				vecTmp[1] = cgi.MSG_ReadCoord();
				vecTmp[2] = cgi.MSG_ReadCoord();
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				vecArray[0][0] = cgi.MSG_ReadCoord();
				vecArray[0][1] = cgi.MSG_ReadCoord();
				vecArray[0][2] = cgi.MSG_ReadCoord();

				iLarge = cgi.MSG_ReadBits( 1 );

				CG_MakeBulletTracer(vecTmp,vecStart,vecArray,1,iLarge,0,1);				
				break;
			case 26: // Bullet tracer
				vecTmp[0] = 0;
				vecTmp[1] = 0;
				vecTmp[2] = 0;
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				vecArray[0][0] = cgi.MSG_ReadCoord();
				vecArray[0][1] = cgi.MSG_ReadCoord();
				vecArray[0][2] = cgi.MSG_ReadCoord();

				iLarge = cgi.MSG_ReadBits( 1 );
				CG_MakeBulletTracer(vecTmp,vecStart,vecArray,1,iLarge,0,1);
				break;
			case 27: // HUD drawing...
				i = cgi.MSG_ReadByte();
				hde = hdelements + i;
				Q_strncpyz( hde->shaderName, cgi.MSG_ReadString(), sizeof(hde->shaderName) );
				hde->string[0] = 0;
				memset(&hde->pFont,0,sizeof(hde->pFont));
				hde->fontName[0] = 0;	
				
				CG_HudDrawShader(i);
				break;
			case 28: // HUD drawing...
				i = cgi.MSG_ReadByte();
				hde = hdelements + i;
				hde->iHorizontalAlign = cgi.MSG_ReadBits( 2 );
				hde->iHorizontalAlign = cgi.MSG_ReadBits( 2 );
				break;
			case 29:
				i = cgi.MSG_ReadByte();
				hde = hdelements + i;
				hde->iX = cgi.MSG_ReadShort();
				hde->iY = cgi.MSG_ReadShort();
				hde->iWidth = cgi.MSG_ReadShort();
				hde->iHeight = cgi.MSG_ReadShort();
				break;
			case 30:
				i = cgi.MSG_ReadByte();
				hde = hdelements + i;
				hde->bVirtualScreen = cgi.MSG_ReadBits( 1 );
				break;
			case 31: // huddraw_color
				i = cgi.MSG_ReadByte();
				hde = hdelements + i;
				hde->vColor[0] = cgi.MSG_ReadByte()*0.003921568859368563;
				hde->vColor[1] = cgi.MSG_ReadByte()*0.003921568859368563;
				hde->vColor[2] = cgi.MSG_ReadByte()*0.003921568859368563;
				break;
			case 32: // huddraw_alpha
				i = cgi.MSG_ReadByte();
				hde = hdelements + i;
				hde->vColor[3] = cgi.MSG_ReadByte()*0.003921568859368563;
				break;
			case 33: // huddraw_string
				i = cgi.MSG_ReadByte();
				hde = hdelements + i;
				hde->hShader = 0;
				Q_strncpyz( hde->string, cgi.MSG_ReadString(), sizeof(hde->string) );
				break;
			case 34: // huddraw_font
				i = cgi.MSG_ReadByte();
				hde = hdelements + i;
				Q_strncpyz( hde->fontName, cgi.MSG_ReadString(), sizeof(hde->fontName) );
				hde->hShader = 0;
				hde->shaderName[0] = 0;
				CG_HudDrawFont(i);
				break;
			case 35:
			case 36:
				// TODO - play sound (?)
				//CG_PlaySound(s, 0, 5, 2.0, -1.0, vecStart, 1);
				break;
			case 37: // voicechat message (squad command, taunt, etc...)
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				iTemp = cgi.MSG_ReadBits( 1 );
				// read client index
				i = cgi.MSG_ReadBits( 6 );
				// read voicechat sound alias
				s = cgi.MSG_ReadString();

				if(iTemp) {
					current_entity_number = i;

				}
				//CG_Printf("Case 37: iTemp %i, i %i, s %s\n",iTemp,i,s);
				// play an aliased sound from uberdialog.scr
				// wombat: only if there is a sound to be played
				if ( *s )
					CG_PlaySound(s,vecStart,5,-1.0,-1.0,-1.0,0);
				break;
		}
	} while ( cgi.MSG_ReadBits(1) );
}
