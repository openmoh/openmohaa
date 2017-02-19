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
// sv_game.c -- interface to the game dll

#include "server.h"

#include "../botlib/botlib.h"
#include "../client/snd_public.h"
#include "../client/client.h"
#include <localization.h>
#include <crc.h>

debugline_t *DebugLines;
int numDebugLines;
debugstring_t *DebugStrings;
int numDebugStrings;

botlib_export_t	*botlib_export;
static int modelUserCount[ MAX_MODELS ];

void SV_GameError( const char *string ) {
	Com_Error( ERR_DROP, "%s", string );
}

void SV_GamePrint( const char *string ) {
	Com_Printf( "%s", string );
}

// these functions must be used instead of pointer arithmetic, because
// the game allocates gentities with private information after the server shared part
int	SV_NumForGentity( gentity_t *ent ) {
	int		num;

	num = ( (byte *)ent - (byte *)sv.gentities ) / sv.gentitySize;

	return num;
}

gentity_t *SV_GentityNum( int num ) {
	gentity_t *ent;

	ent = (gentity_t *)((byte *)sv.gentities + sv.gentitySize*(num));

	return ent;
}

playerState_t *SV_GameClientNum( int num ) {
	playerState_t	*ps;

	ps = (playerState_t *)((byte *)sv.gameClients + sv.gameClientSize*(num));

	return ps;
}

svEntity_t	*SV_SvEntityForGentity( gentity_t *gEnt ) {
	if ( !gEnt || gEnt->s.number < 0 || gEnt->s.number >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	return &sv.svEntities[ gEnt->s.number ];
}

gentity_t *SV_GEntityForSvEntity( svEntity_t *svEnt ) {
	int		num;

	num = svEnt - sv.svEntities;
	return SV_GentityNum( num );
}

// su44: MoHAA cg messages sending
typedef struct {
	byte *data;
	int cursize;
	byte *datatypes;
	int dtindex;
} cgm_t;
cgm_t g_CGMessages[MAX_CLIENTS];
qboolean g_CGMRecieve[MAX_CLIENTS];
void SV_ClearCGMessage (int iClient)
{
	g_CGMessages[iClient].cursize = 0;
	g_CGMessages[iClient].dtindex = 0;
	if(g_CGMessages[iClient].data) {
		Z_Free(g_CGMessages[iClient].data);
		g_CGMessages[iClient].data = 0;
	}
	if(g_CGMessages[iClient].datatypes) {
		Z_Free(g_CGMessages[iClient].datatypes);
		g_CGMessages[iClient].datatypes = 0;
	}
	g_CGMRecieve[iClient] = 0;
} 
void SV_ClearAllCGMessages ()
{
	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for(i = 0; i < MAX_CLIENTS; i++,pCGM++) {
		pCGM->cursize = 0;
		pCGM->dtindex = 0;
		if(pCGM->data) {
			Z_Free(pCGM->data);
			pCGM->data = 0;
		}
		if(pCGM->datatypes) {
			Z_Free(pCGM->datatypes);
			pCGM->datatypes = 0;
		}
		g_CGMRecieve[i] = 0;
	}
}
#define CGM_DATA_SIZE 4096
#define CGM_DATATYPES_SIZE 8192
void SV_InitCGMessage (int iClient)
{
	cgm_t *pCGM;

	pCGM = &g_CGMessages[iClient];

	if ( pCGM->data == 0 )
		pCGM->data = Z_Malloc(CGM_DATA_SIZE);

	if ( pCGM->datatypes == 0 )
		pCGM->datatypes = Z_Malloc(CGM_DATA_SIZE);

	pCGM->cursize = 0;
	pCGM->dtindex = 0;

	g_CGMRecieve[iClient] = 0;
}

void SV_InitAllCGMessages ()
{
	int i;

	SV_ClearAllCGMessages();
	for ( i = 0; i < svs.iNumClients; ++i )
		SV_InitCGMessage(i);
}

static void MSG_WriteCGMBits (cgm_t *pCGM, int value, int bits)
{
	if(CGM_DATA_SIZE - pCGM->cursize <= 3) {
		Com_DPrintf("CGM buffer for client %i overflowed\n", pCGM - g_CGMessages);
		return;
	}
	if(CGM_DATATYPES_SIZE - pCGM->dtindex <= 0) {
		Com_DPrintf("CGM buffer for client %i overflowed number of datum\n", pCGM - g_CGMessages);
		return;
	}

    if (bits > 32)
        Com_Error(1, "PF_MSG_WriteBits: bad bits %i", bits);

	if(bits < 0)
		bits = -bits;

	if(bits <= 8) {
		// append single byte
		pCGM->data[pCGM->cursize] = value;
		pCGM->cursize++;
	} else if(bits <= 16) {
		// append short (two bytes)
		*((short*)(&pCGM->data[pCGM->cursize])) = LittleShort(value);
		pCGM->cursize+=2;
	} else if(bits <= 32) {
		// append integer (4 bytes)
		*((int*)(&pCGM->data[pCGM->cursize])) = LittleLong(value);	
		pCGM->cursize+=4;
	}
	// save datatype
	pCGM->datatypes[pCGM->dtindex] = bits;
	pCGM->dtindex++;
}
void PF_MSG_WriteBits (int value, int bits)
{
	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for( i = 0; i < svs.iNumClients; i++, pCGM++ ) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM,value,bits);
	}
}

void PF_MSG_WriteChar (int c)
{
	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		// write 8 bit character
		MSG_WriteCGMBits(pCGM,c,8);
	}
} 
void PF_MSG_WriteByte (int c)
{
	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		// write 8 bit unsigned byte
		MSG_WriteCGMBits(pCGM,c,8);
	}
}
void PF_MSG_WriteSVC (int c)
{
	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM,c,8);
	}
}
void PF_MSG_WriteShort (int c)
{
	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM,c,16);
	}
}
void PF_MSG_WriteLong (int c)
{
 	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM,c,32);
	}
}
void PF_MSG_WriteFloat (float f)
{
	union {
		float fl;
		int l;
	} dat;
 	cgm_t *pCGM;
	int i;
	dat.fl = f;
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM,dat.l,32);
	}
}
void PF_MSG_WriteString (const char *s)
{
 	cgm_t *pCGM;
	int i;
	size_t j, l;
	l = strlen(s);
	l+=1; // include trailing zero
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		for(j = 0; j < l; j++) {
			MSG_WriteCGMBits(pCGM,s[j],8);
		}
	}
}
void PF_MSG_WriteAngle8 (float f)
{
 	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM,f * 256.0 / 360.0,8);
	}
}
void PF_MSG_WriteAngle16 (float f)
{
 	cgm_t *pCGM;
	int i;
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM,ANGLE2SHORT(f),16);
	}
}
void PF_MSG_WriteCoord (float f)
{
 	cgm_t *pCGM;
	int i;

	// 19th bit is a sign
	int write = abs(f * 16.f);
	if(f < 0) {
		write |= 262144;
	} else {
		write &= ~262144;
	}

	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM, write, 19);
	}
}
void PF_MSG_WriteDir (vec_t *dir)
{
 	cgm_t *pCGM;
	int i;
	byte b;
	b = DirToByte(dir);
	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		MSG_WriteCGMBits(pCGM,b,8);
	}
}
void PF_MSG_StartCGM (int type)
{
    cgm_t *pCGM;
    int i;

	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		if(g_CGMRecieve[i] == 0) 
			continue;
		if(pCGM->cursize <= 0) {
			// always put "svc_cgameMessage" byte at the beginning of CGM block
			MSG_WriteCGMBits(pCGM,svc_cgameMessage,8);
		} else {
			// single bit means that there is one more message in block
			MSG_WriteCGMBits(pCGM, 1, 1);
		}
		// write CG message type
		MSG_WriteCGMBits(pCGM, type, 6);
	}
}
void PF_MSG_EndCGM ()
{
	memset(g_CGMRecieve,0,sizeof(g_CGMRecieve));
}
void PF_MSG_SetClient (int iClient)
{
	memset(g_CGMRecieve,0,sizeof(g_CGMRecieve));
	if(g_CGMessages[iClient].data && (g_CGMessages[iClient].cursize <= 3967))
	{
		g_CGMRecieve[iClient] = 1;
	}
}

void MSG_SetBroadcastAll ()
{
	cgm_t *pCGM;
	int i;

	pCGM = g_CGMessages;
	for(i = 0; i < svs.iNumClients; i++,pCGM++) {
		g_CGMRecieve[i] = qtrue;
	}
}
void MSG_SetBroadcastVisible( const vec_t *vPos, const vec_t *vPosB)
{
    byte *clientpvs;
    int posBcluster;
    int poscluster;
    int posBarea;
    int posarea;
    int posBleaf;
    int posleaf;
    int clientcluster;
    int clientarea;
    int leafnum;
    client_t *pClient;
    int i;
	cgm_t *pCGM;

#if 0
	MSG_SetBroadcastAll();
#else
	// dont check the same pos twice
	if(vPosB && VectorCompare(vPos,vPosB)) {
		vPosB = 0;
	}

	pCGM = g_CGMessages;
	pClient = svs.clients;
	for(i = 0; i < svs.iNumClients; i++,pCGM++,pClient++) {
		if(pClient->state != CS_ACTIVE) {
			g_CGMRecieve[i] = qfalse;
			continue;
		}
		leafnum = CM_PointLeafnum(pClient->gentity->s.origin);
		clientcluster = CM_LeafCluster(leafnum);
		clientarea = CM_LeafArea(leafnum);
		clientpvs = CM_ClusterPVS(clientcluster);

		posleaf = CM_PointLeafnum(vPos);
		poscluster = CM_LeafCluster(posleaf);
		posarea = CM_LeafArea(posleaf);

		if ( clientpvs[poscluster >> 3] & (1 << (poscluster&7) ) ) {
			// visible.
			g_CGMRecieve[i] = qtrue;
			continue;
		}
	
		if(vPosB) {
			posBleaf = CM_PointLeafnum(vPosB);
			posBcluster = CM_LeafCluster(posBleaf);
			posBarea = CM_LeafArea(posBleaf);

			if ( clientpvs[posBcluster >> 3] & (1 << (posBcluster&7) ) ) {
				// visible.
				g_CGMRecieve[i] = qtrue;
				continue;
			}
		}
		// not visible
		g_CGMRecieve[i] = qfalse;
	}
#endif
}
void MSG_SetBroadcastHearable ( const vec_t *vPos, const vec_t *vPosB)
{
	// su44: in MoHAA's, MSG_SetBroadcastAll is used here,
	// but I'd rather use MSG_SetBroadcastVisible with
	// PHS instead of PVS data
	MSG_SetBroadcastAll();
}
void SV_WriteCGMToClient (client_t *client, msg_t *msg)
{
//	byte *pBuffer;
	cgm_t *pCGMMsg;
//	int i;
	int clientNum;

	clientNum = client - svs.clients;;
	pCGMMsg = &g_CGMessages[clientNum];
	if ( pCGMMsg->data )
	{
		if ( pCGMMsg->cursize > 0 )
		{
			int curDTIndex;
			byte *data;
			// write end of message - single false bit
			MSG_WriteCGMBits(pCGMMsg, 0, 1);
			data = pCGMMsg->data;
			curDTIndex = 0;
			for(curDTIndex = 0; curDTIndex < pCGMMsg->dtindex; curDTIndex++) {
				int bits = pCGMMsg->datatypes[curDTIndex];
				if(bits <= 8) {
					MSG_WriteBits(msg, *data, bits);
					data++;
				} else if(bits <= 16) {
					MSG_WriteBits(msg, *(short*)data, bits);
					data+=2;
				} else if(bits <= 32) {
					MSG_WriteBits(msg, *(int*)data, bits);
					data+=4;
				}
			
			}
			pCGMMsg->cursize = 0;
			pCGMMsg->dtindex = 0;
		}
	}
}

/*
=================
SV_inPVS

Also checks portalareas so that doors block sight
=================
*/
qboolean SV_inPVS (const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);
	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return qfalse;
	if (!CM_AreasConnected (area1, area2))
		return qfalse;		// a door blocks sight
	return qtrue;
}


/*
=================
SV_inPVSIgnorePortals

Does NOT check portalareas
=================
*/
qboolean SV_inPVSIgnorePortals( const vec3_t p1, const vec3_t p2)
{
	int		leafnum;
	int		cluster;
	int		area1, area2;
	byte	*mask;

	leafnum = CM_PointLeafnum (p1);
	cluster = CM_LeafCluster (leafnum);
	area1 = CM_LeafArea (leafnum);
	mask = CM_ClusterPVS (cluster);

	leafnum = CM_PointLeafnum (p2);
	cluster = CM_LeafCluster (leafnum);
	area2 = CM_LeafArea (leafnum);

	if ( mask && (!(mask[cluster>>3] & (1<<(cluster&7)) ) ) )
		return qfalse;

	return qtrue;
}

/*
===============
PF_NumAnims
===============
*/
int PF_NumAnims( dtiki_t *tiki )
{
	return TIKI_NumAnims( tiki );
}

/*
===============
PF_NumSurfaces
===============
*/
int PF_NumSurfaces( dtiki_t *tiki )
{
	return TIKI_NumSurfaces( tiki );
}

/*
===============
PF_NumTags
===============
*/
int PF_NumTags( dtiki_t *tiki )
{
	return TIKI_NumTags( tiki );
}

/*
===============
PF_Cross_Time
===============
*/
float PF_Cross_Time( dtiki_t *tiki, int anim )
{
	return TIKI_Anim_CrossblendTime( tiki, anim );
}

/*
===============
PF_CalculateBounds
===============
*/
void PF_CalculateBounds( dtiki_t *tiki, float scale, vec3_t mins, vec3_t maxs )
{
	TIKI_CalculateBounds( tiki, scale, mins, maxs );
}

/*
===============
PF_Anim_NameForNum
===============
*/
const char *PF_Anim_NameForNum( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_NameForNum( tiki, animnum );
}

/*
===============
PF_Anim_NumForName
===============
*/
int PF_Anim_NumForName( dtiki_t *tiki, const char *name )
{
	return TIKI_Anim_NumForName( tiki, name );
}

/*
===============
PF_Anim_Random
===============
*/
int PF_Anim_Random( dtiki_t *tiki, const char *name )
{
	return TIKI_Anim_Random( tiki, name );
}

/*
===============
PF_Anim_NumFrames
===============
*/
int PF_Anim_NumFrames( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_NumFrames( tiki, animnum );
}

/*
===============
PF_Anim_Time
===============
*/
float PF_Anim_Time( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_Time( tiki, animnum );
}

/*
===============
PF_Anim_Frametime
===============
*/
float PF_Anim_Frametime( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_Frametime( tiki, animnum );
}

/*
===============
PF_Anim_Delta
===============
*/
void PF_Anim_Delta( dtiki_t *tiki, int animnum, vec3_t delta )
{
	TIKI_Anim_Delta( tiki, animnum, delta );
}

/*
===============
PF_Anim_HasDelta
===============
*/
qboolean PF_Anim_HasDelta( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_HasDelta( tiki, animnum );
}

/*
===============
PF_Anim_DeltaOverTime
===============
*/
void PF_Anim_DeltaOverTime( dtiki_t *tiki, int iAnimnum, float fTime1, float fTime2, vec3_t vDelta )
{
	TIKI_Anim_DeltaOverTime( tiki, iAnimnum, fTime1, fTime2, vDelta );
}

/*
===============
PF_Anim_Flags
===============
*/
int PF_Anim_Flags( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_Flags( tiki, animnum );
}

/*
===============
PF_Anim_FlagsSkel
===============
*/
int PF_Anim_FlagsSkel( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_FlagsSkel( tiki, animnum );
}

/*
===============
PF_Anim_HasCommands
===============
*/
qboolean PF_Anim_HasCommands( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_HasServerCommands( tiki, animnum );
}

/*
===============
PF_ModelTiki
===============
*/
dtiki_t *PF_ModelTiki( const char *name )
{
	return TIKI_RegisterTikiFlags( name, qtrue );
}

/*
===============
PF_ModelTikiAnim
===============
*/
dtikianim_t *PF_ModelTikiAnim( const char *name )
{
	return TIKI_RegisterTikiAnimFlags( name, qtrue );
}

/*
===============
PF_NumHeadModels
===============
*/
int PF_NumHeadModels( const char *model )
{
	dtikianim_t *tiki = PF_ModelTikiAnim( model );
	return TIKI_NumHeadModels( tiki );
}

/*
===============
PF_GetHeadModel
===============
*/
void PF_GetHeadModel( const char *model, int num, char *name )
{
	dtikianim_t *tiki = PF_ModelTikiAnim( model );
	TIKI_GetHeadModel( tiki, num, name );
}

/*
===============
PF_NumHeadSkins
===============
*/
int PF_NumHeadSkins( const char *model )
{
	dtikianim_t *tiki = PF_ModelTikiAnim( model );
	return TIKI_NumHeadSkins( tiki );
}

/*
===============
PF_GetHeadSkin
===============
*/
void PF_GetHeadSkin( const char *model, int num, char *name )
{
	dtikianim_t *tiki = PF_ModelTikiAnim( model );
	TIKI_GetHeadSkin( tiki, num, name );
}

/*
===============
PF_Anim_HasClientCommands
===============
*/
qboolean PF_Anim_HasClientCommands( dtiki_t *tiki, int animnum )
{
	return TIKI_Anim_HasClientCommands( tiki, animnum );
}

/*
===============
PF_Frame_Commands
===============
*/
qboolean PF_Frame_Commands( dtiki_t *tiki, int animnum, int framenum, tiki_cmd_t *tiki_cmds )
{
	return TIKI_Frame_Commands_Server( tiki, animnum, framenum, tiki_cmds );
}

/*
===============
PF_Surface_NameToNum
===============
*/
int PF_Surface_NameToNum( dtiki_t *tiki, const char *name )
{
	return TIKI_Surface_NameToNum( tiki, name );
}

/*
===============
PF_Surface_NumToName
===============
*/
const char *PF_Surface_NumToName( dtiki_t *tiki, int num )
{
	return TIKI_Surface_NumToName( tiki, num );
}

/*
===============
PF_Tag_NameToNum
===============
*/
int PF_Tag_NameToNum( dtiki_t *tiki, const char *name )
{
	return TIKI_Tag_NameToNum( tiki, name );
}

/*
===============
PF_Tag_NumToName
===============
*/
const char *PF_Tag_NumToName( dtiki_t *tiki, int num )
{
	return TIKI_Tag_NumToName( tiki, num );
}

/*
===============
PF_TIKI_OrientationInternal
===============
*/
orientation_t PF_TIKI_OrientationInternal( dtiki_t *tiki, int entnum, int num, float scale )
{
	return TIKI_OrientationInternal( tiki, entnum, num, scale );
}

/*
===============
PF_TIKI_TransformInternal
===============
*/
void *PF_TIKI_TransformInternal( dtiki_t *tiki, int entnum, int num )
{
	return TIKI_TransformInternal( tiki, entnum, num );
}

/*
===============
PF_TIKI_IsOnGroundInternal
===============
*/
qboolean PF_TIKI_IsOnGroundInternal( dtiki_t *tiki, int entnum, int num, float threshold )
{
	return TIKI_IsOnGroundInternal( tiki, entnum, num, threshold );
}

/*
===============
PF_SetPoseInternal
===============
*/
void PF_SetPoseInternal( dtiki_t *tiki, int entnum, const frameInfo_t *frameInfo, int *bone_tag, vec4_t *bone_quat, float actionWeight )
{
	TIKI_SetPoseInternal( TIKI_GetSkeletor( tiki, entnum ), frameInfo, bone_tag, bone_quat, actionWeight );
}

/*
===============
PF_Alias_Add

===============
*/
qboolean PF_Alias_Add( dtiki_t *pmdl, const char *alias, const char *name, const char *parameters )
{
	if( !pmdl->a->alias_list ) pmdl->a->alias_list = AliasList_New( pmdl->a->name );
	return Alias_ListAdd( pmdl->a->alias_list, alias, name, parameters );
}

/*
===============
PF_Alias_FindRandom

===============
*/
const char *PF_Alias_FindRandom( dtiki_t *tiki, const char *alias, AliasListNode_t **ret )
{
	AliasList_t *alias_list = tiki->a->alias_list;

	alias_list = ( AliasList_t * )tiki->a->alias_list;
	if( alias_list )
	{
		return Alias_ListFindRandom( alias_list, alias, ret );
	}
	else
	{
		return NULL;
	}
}

/*
===============
PF_Alias_UpdateDialog

===============
*/
void PF_Alias_UpdateDialog( dtikianim_t *tiki, const char *alias )
{
	AliasList_t *alias_list = tiki->alias_list;

	if( alias_list )
	{
		Alias_ListUpdateDialog( alias_list, alias );
	}
}

/*
===============
PF_Alias_Dump

===============
*/
void PF_Alias_Dump( dtiki_t *tiki )
{
	AliasList_t *alias_list = tiki->a->alias_list;

	if( alias_list )
	{
		Alias_ListDump( tiki->a->alias_list );
	}
}

/*
===============
PF_Alias_Clear

===============
*/
void PF_Alias_Clear( dtiki_t *tiki )
{
	PF_Alias_Dump( tiki );
}

/*
===============
PF_NameForNum
===============
*/
const char *PF_NameForNum( dtiki_t *tiki )
{
	return TIKI_Name( tiki );
}

/*
===============
SV_ClearModelUserCounts

Clears model's reference
===============
*/
void SV_ClearModelUserCounts()
{
	memset( modelUserCount, 0, sizeof( modelUserCount ) );
}

/*
===============
PF_RegisterTiki
===============
*/
dtiki_t *PF_RegisterTiki( const char *path )
{
	modelUserCount[ SV_ModelIndex( path ) ]++;
	return TIKI_RegisterTiki( path );
}

static int g_usageIndex = 0;

/*
===============
PF_setmodel
===============
*/
qboolean PF_setmodel( gentity_t *ent, const char *name )
{
	dtiki_t *tiki;
	int newModelIndex;

	g_usageIndex++;
	if( g_usageIndex >= 65535 ) {
		g_usageIndex = 1;
	}

	ent->s.usageIndex = g_usageIndex;

	if( !*name )
	{
		tiki = NULL;
	}
	else
	{
		tiki = PF_ModelTiki( name );
		if( !tiki ) {
			return qfalse;
		}

		newModelIndex = SV_ModelIndex( name );
		if( !newModelIndex ) {
			return qfalse;
		}

		// don't reference the model again
		if( ent->s.modelindex == newModelIndex ) {
			return qtrue;
		}
	}

	if( ent->tiki && ent->s.modelindex )
	{
		modelUserCount[ ent->s.modelindex ]--;
		if( !modelUserCount[ ent->s.modelindex ] )
			SV_ClearModel( ent->s.modelindex );
		ent->s.modelindex = 0;
	}

	ent->tiki = tiki;
	ent->r.bmodel = qfalse;

	if( ent->tiki )
	{
		ent->s.modelindex = newModelIndex;
		modelUserCount[ newModelIndex ]++;
	}

	return qtrue;
}

/*
===============
PF_clearmodel
===============
*/
void PF_clearmodel( gentity_t *ent )
{
	PF_setmodel( ent, "" );
}

/*
===============
PF_GetSkeletor
===============
*/
void *PF_GetSkeletor( dtiki_t *tiki, int entnum )
{
	return TIKI_GetSkeletor( tiki, entnum );
}

/*
===============
SV_GameSendServerCommand

Sends a command string to a client
===============
*/
void SV_GameSendServerCommand( int clientNum, const char *text, ... )
{
	va_list va;
	char buffer[ 500 ];

	va_start( va, text );
	vsprintf( buffer, text, va );
	va_end( va );

	if ( clientNum == -1 ) {
		SV_SendServerCommand( NULL, "%s", buffer );
	} else {
		if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
			return;
		}
		SV_SendServerCommand( svs.clients + clientNum, "%s", buffer );
	}
}

/*
===============
SV_GameDropClient

Disconnects the client with a message
===============
*/
void SV_GameDropClient( int clientNum, const char *reason ) {
	if ( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		return;
	}
	SV_DropClient( svs.clients + clientNum, reason );	
}

/*
=================
SV_SetBrushModel

sets mins and maxs for inline bmodels
=================
*/
void SV_SetBrushModel( gentity_t *ent, const char *name ) {
	clipHandle_t	h;
	vec3_t			mins, maxs;

	if( !name ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: NULL" );
	}

	if( name[ 0 ] != '*' ) {
		Com_Error( ERR_DROP, "SV_SetBrushModel: %s isn't a brush model", name );
	}

	ent->s.modelindex = atoi( name + 1 );
	h = CM_InlineModel( ent->s.modelindex );
	CM_ModelBounds( h, mins, maxs );
	VectorCopy( mins, ent->r.mins );
	VectorCopy( maxs, ent->r.maxs );
	ent->r.bmodel = qtrue;

	if( !ent->r.contents )
		ent->r.contents = -1; // we don't know exactly what is in the brushes

	SV_LinkEntity( ent );
}

/*
========================
SV_AdjustAreaPortalState
========================
*/
void SV_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	svEntity_t	*svEnt;

	svEnt = SV_SvEntityForGentity( ent );
	if( svEnt->areanum2 == -1 ) {
		return;
	}
	CM_AdjustAreaPortalState( svEnt->areanum, svEnt->areanum2, open );
}

/*
========================
SV_EntityContact
========================
*/
qboolean SV_EntityContact( vec3_t mins, vec3_t maxs, gentity_t *gEnt )
{
	clipHandle_t ch;
	trace_t trace;

	ch = SV_ClipHandleForEntity( gEnt );
	CM_TransformedBoxTrace( &trace, vec3_origin, vec3_origin, mins, maxs, ch, -1, gEnt->s.origin, gEnt->r.currentAngles, qfalse );
	return trace.startsolid;
}


/*
===============
SV_GetServerinfo

===============
*/
void SV_GetServerinfo( char *buffer, int bufferSize ) {
	if( bufferSize < 1 ) {
		Com_Error( ERR_DROP, "SV_GetServerinfo: bufferSize == %i", bufferSize );
	}
	Q_strncpyz( buffer, Cvar_InfoString( CVAR_SERVERINFO ), bufferSize );
}

/*
===============
SV_LocateGameData

===============
*/
void SV_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
	playerState_t *clients, int sizeofGameClient ) {
	sv.gentities = ( gentity_t * )gEnts;
	sv.gentitySize = sizeofGEntity_t;
	sv.num_entities = numGEntities;

	sv.gameClients = clients;
	sv.gameClientSize = sizeofGameClient;
}

/*
===============
SV_SetFarPlane
===============
*/
void SV_SetFarPlane( int farplane )
{
	if( farplane )
	{
		sv.farplane = ( farplane + 32 ) * ( farplane + 32 );
	}
	else
	{
		sv.farplane = 0;
	}
}

/*
===============
SV_SetSkyPortal
===============
*/
void SV_SetSkyPortal( qboolean skyportal )
{
	sv.skyportal = skyportal;
}

/*
===============
SV_GetUsercmd
===============
*/
void SV_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	if( clientNum < 0 || clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_GetUsercmd: bad clientNum:%i", clientNum );
	}
	*cmd = svs.clients[ clientNum ].lastUsercmd;
}

/*
===============
PF_centerprintf
===============
*/
void PF_centerprintf( gentity_t *ent, const char *fmt, ... )
{
	va_list va;
	char msg[ 2048 ];

	if( ent->s.number > svs.iNumClients )
	{
		return;
	}

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );

	if( strlen( msg ) > 256 ) {
		Com_DPrintf( "Centerprint text exceeds buffer size\n" );
	}

	strncpy( svs.clients[ ent->s.number ].centerprint, msg, 256 );
}

/*
===============
PF_locationprintf
===============
*/
void PF_locationprintf( gentity_t *ent, int x, int y, const char *fmt, ... )
{
	va_list va;
	char msg[ 2048 ];

	if( ent->s.number > svs.iNumClients )
	{
		return;
	}

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );

	if( strlen( msg ) > 256 ) {
		Com_DPrintf( "Locationprint text exceeds buffer size\n" );
	}

	strncpy( svs.clients[ ent->s.number ].centerprint, msg, 256 );
	svs.clients[ ent->s.number ].XOffset = x;
	svs.clients[ ent->s.number ].YOffset = y;
	svs.clients[ ent->s.number ].locprint = qtrue;
}

/*
=====================
SV_AddGameCommand
=====================
*/
void SV_AddGameCommand( const char *cmdName, xcommand_t function ) {
	Cmd_AddCommand( cmdName, function );
}

/*
===============
SV_Malloc
===============
*/
void *SV_Malloc( size_t size )
{
	return Z_TagMalloc( size, TAG_GAME );
}

void SV_Free( void* ptr )
{
	Z_Free( ptr );
}

/*
===============
SV_ShutdownGameProgs

Called every time a map changes
===============
*/
void SV_ShutdownGameProgs( void ) {
	if( !ge ) {
		return;
	}

	ge->Shutdown();
	Sys_UnloadGame();

	ge = NULL;
}

/*
===============
PF_UI_PopMenu
===============
*/
void PF_UI_PopMenu( int iClient, int i )
{
	SV_GameSendServerCommand( iClient, "stufftext \"popmenu %i\"\n", i );
}

/*
===============
PF_UI_ShowMenu
===============
*/
void PF_UI_ShowMenu( int iClient, const char *name, qboolean bForce )
{
	SV_GameSendServerCommand( iClient, "stufftext \"showmenu %s %i\"\n", name,  bForce );
}

/*
===============
PF_UI_HideMenu
===============
*/
void PF_UI_HideMenu( int iClient, const char *name, qboolean bForce )
{
	SV_GameSendServerCommand( iClient, "stufftext \"hidemenu %s %i\"\n", name, bForce );
}

/*
===============
PF_UI_PushMenu
===============
*/
void PF_UI_PushMenu( int iClient, const char *name )
{
	SV_GameSendServerCommand( iClient, "stufftext \"pushmenu %s\"\n", name );
}

/*
===============
PF_UI_HideMouse_f
===============
*/
void PF_UI_HideMouse_f( int iClient )
{
	SV_GameSendServerCommand( iClient, "stufftext \"ui_hidemouse\"\n" );
}

/*
===============
PF_UI_ShowMouse_f
===============
*/
void PF_UI_ShowMouse_f( int iClient )
{
	SV_GameSendServerCommand( iClient, "stufftext \"ui_showmouse\"\n" );
}

/*
===============
PF_Key_StringToKeynum
===============
*/
int PF_Key_StringToKeynum( const char *str )
{
	if( com_cl_running->integer ) {
		return PF_Key_StringToKeynum( str );
	} else {
		return 0;
	}
}

/*
===============
PF_Key_KeynumToBindString
===============
*/
const char *PF_Key_KeynumToBindString( int keynum )
{	if( com_cl_running->integer ) {
		return Key_KeynumToBindString( keynum );
	} else {
		return 0;
	}
}

/*
===============
PF_Key_GetKeysForCommand
===============
*/
void PF_Key_GetKeysForCommand( const char *command, int *key1, int *key2 )
{
	if( com_cl_running->integer ) {
		Key_GetKeysForCommand( command, key1, key2 );
	} else {
		*key1 = 0;
		*key2 = 0;
	}
}

/*
===============
SV_InitGameProgs

Called on a normal map change, not on a map_restart
===============
*/
void SV_InitGameProgs( void ) {
	gameImport_t import;
	const char *err;
	int i;

	import.Printf						= Com_Printf;
	import.DPrintf						= Com_DPrintf;
	import.DPrintf2						= Com_DPrintf;
	import.Error						= Com_Error;
	import.GetArchiveFileName			= Com_GetArchiveFileName;
	import.Milliseconds					= Sys_Milliseconds;
	import.LV_ConvertString				= Sys_LV_CL_ConvertString;

	import.SendServerCommand			= SV_GameSendServerCommand;
	import.DropClient					= SV_GameDropClient;

	import.MSG_WriteBits				= PF_MSG_WriteBits;
	import.MSG_WriteChar				= PF_MSG_WriteChar;
	import.MSG_WriteByte				= PF_MSG_WriteByte;
	import.MSG_WriteSVC					= PF_MSG_WriteSVC;
	import.MSG_WriteShort				= PF_MSG_WriteShort;
	import.MSG_WriteLong				= PF_MSG_WriteLong;
	import.MSG_WriteFloat				= PF_MSG_WriteFloat;
	import.MSG_WriteString				= PF_MSG_WriteString;
	import.MSG_WriteAngle8				= PF_MSG_WriteAngle8;
	import.MSG_WriteAngle16				= PF_MSG_WriteAngle16;
	import.MSG_WriteCoord				= PF_MSG_WriteCoord;
	import.MSG_WriteDir					= PF_MSG_WriteDir;
	import.MSG_StartCGM					= PF_MSG_StartCGM;
	import.MSG_EndCGM					= PF_MSG_EndCGM;
	import.MSG_SetClient				= PF_MSG_SetClient;
	import.SetBroadcastVisible			= MSG_SetBroadcastVisible;
	import.SetBroadcastHearable			= MSG_SetBroadcastHearable;
	import.SetBroadcastAll				= MSG_SetBroadcastAll;

	import.LinkEntity					= SV_LinkEntity;
	import.UnlinkEntity					= SV_UnlinkEntity;

	import.AreaEntities					= SV_AreaEntities;
	
	import.SightTraceEntity				= SV_SightTraceEntity;
	import.SightTrace					= SV_SightTrace;
	import.Trace						= SV_Trace;
	import.GetShader					= SV_GetShaderPointer;
	import.PointContents				= SV_PointContents;
	import.PointBrushnum				= CM_PointBrushNum;
	import.SetBrushModel				= SV_SetBrushModel;
	import.ModelBoundsFromName			= CM_ModelBoundsFromName;
	import.ClipToEntity					= SV_ClipToEntity;
	import.SetConfigstring				= SV_SetConfigstring;
	import.GetConfigstring				= SV_GetConfigstring;
	import.GetUserinfo					= SV_GetUserinfo;
	import.SetUserinfo					= SV_SetUserinfo;

	import.Malloc						= SV_Malloc;
	import.Free							= SV_Free;

	import.Cvar_Get						= Cvar_Get;
	import.Cvar_Set						= Cvar_Set;
	import.cvar_set2					= Cvar_Set2;
	import.NextCvar						= Cvar_Next;

	import.Argc							= Cmd_Argc;
	import.Args							= Cmd_Args;
	import.Argv							= Cmd_Argv;

	import.AddCommand					= SV_AddGameCommand;
	import.SendConsoleCommand			= Cbuf_AddText;

	import.FS_ReadFile					= FS_ReadFileEx;
	import.FS_FreeFile					= FS_FreeFile;
	import.FS_WriteFile					= FS_WriteFile;
	import.FS_FOpenFileWrite			= FS_FOpenFileWrite;
	import.FS_FOpenFileAppend			= FS_FOpenFileAppend;
	import.FS_PrepFileWrite				= FS_PrepFileWrite;
	import.FS_Write						= FS_Write;
	import.FS_Read						= FS_Read;
	import.FS_FCloseFile				= FS_FCloseFile;
	import.FS_Tell						= FS_FTell;
	import.FS_Seek						= FS_Seek;
	import.FS_Flush						= FS_ForceFlush;
	import.FS_FileNewer					= FS_FileNewer;
	import.FS_CanonicalFilename			= FS_CanonicalFilename;
	import.FS_ListFiles					= FS_ListFiles;
	import.FS_FreeFileList				= FS_FreeFileList;

	import.DebugGraph					= SCR_DebugGraph;
	import.AdjustAreaPortalState		= SV_AdjustAreaPortalState;
	import.AreasConnected				= CM_AreasConnected;
	import.AreaForPoint					= CM_AreaForPoint;
	import.InPVS						= CM_inPVS;
	import.GameDir						= FS_Gamedir;

	import.setmodel						= PF_setmodel;
	import.clearmodel					= PF_clearmodel;
	import.TIKI_NumAnims				= PF_NumAnims;
	import.NumSurfaces					= PF_NumSurfaces;
	import.NumTags						= PF_NumTags;
	import.CalculateBounds				= PF_CalculateBounds;
	import.TIKI_GetSkeletor				= PF_GetSkeletor;
	import.Anim_NameForNum				= PF_Anim_NameForNum;
	import.Anim_NumForName				= PF_Anim_NumForName;
	import.Anim_Random					= PF_Anim_Random;
	import.Anim_NumFrames				= PF_Anim_NumFrames;
	import.Anim_Time					= PF_Anim_Time;
	import.Anim_Frametime				= PF_Anim_Frametime;
	import.Anim_CrossTime				= PF_Cross_Time;
	import.Anim_Delta					= PF_Anim_Delta;
	import.Anim_HasDelta				= PF_Anim_HasDelta;
	import.Anim_DeltaOverTime			= PF_Anim_DeltaOverTime;
	import.Anim_Flags					= PF_Anim_Flags;
	import.Anim_FlagsSkel				= PF_Anim_FlagsSkel;
	import.Anim_HasCommands				= PF_Anim_HasCommands;
	import.NumHeadModels				= PF_NumHeadModels;
	import.GetHeadModel					= PF_GetHeadModel;
	import.NumHeadSkins					= PF_NumHeadSkins;
	import.GetHeadSkin					= PF_GetHeadSkin;
	import.Frame_Commands				= PF_Frame_Commands;
	import.Surface_NameToNum			= PF_Surface_NameToNum;
	import.Surface_NumToName			= PF_Surface_NumToName;
	import.Tag_NumForName				= PF_Tag_NameToNum;
	import.Tag_NameForNum				= PF_Tag_NumToName;
	import.TIKI_OrientationInternal		= PF_TIKI_OrientationInternal;
	import.TIKI_TransformInternal		= PF_TIKI_TransformInternal;
	import.TIKI_IsOnGroundInternal		= PF_TIKI_IsOnGroundInternal;
	import.TIKI_SetPoseInternal			= PF_SetPoseInternal;

	import.CM_GetHitLocationInfo			= CM_GetHitLocationInfo;
	import.CM_GetHitLocationInfoSecondary	= CM_GetHitLocationInfoSecondary;

	import.Alias_Add					= PF_Alias_Add;
	import.Alias_FindRandom				= PF_Alias_FindRandom;
	import.Alias_Dump					= PF_Alias_Dump;
	import.Alias_Clear					= PF_Alias_Clear;
	import.Alias_UpdateDialog			= PF_Alias_UpdateDialog;
	import.GlobalAlias_Add				= Alias_Add;
	import.GlobalAlias_FindRandom		= Alias_FindRandom;
	import.GlobalAlias_Dump				= Alias_Dump;
	import.GlobalAlias_Clear			= Alias_Clear;

	import.TIKI_NameForNum				= PF_NameForNum;
	import.TIKI_RegisterModel			= PF_RegisterTiki;
	import.modeltiki					= PF_ModelTiki;
	import.modeltikianim				= PF_ModelTikiAnim;
	import.soundindex					= SV_SoundIndex;
	import.imageindex					= SV_ImageIndex;
	import.itemindex					= SV_ItemIndex;
	import.SetLightStyle				= SV_SetLightStyle;

	import.DebugLines					= &DebugLines;
	import.numDebugLines				= &numDebugLines;
	import.DebugStrings					= &DebugStrings;
	import.numDebugStrings				= &numDebugStrings;

	import.CalcCRC						= CRC_Block;

	import.Sound						= SV_Sound;
	import.StopSound					= SV_StopSound;
	import.S_IsSoundPlaying				= S_IsSoundPlaying;
	import.centerprintf					= PF_centerprintf;
	import.locationprintf				= PF_locationprintf;
	import.LocateGameData				= SV_LocateGameData;
	import.SetFarPlane					= SV_SetFarPlane;
	import.SetSkyPortal					= SV_SetSkyPortal;

	import.Popmenu						= PF_UI_PopMenu;
	import.Showmenu						= PF_UI_ShowMenu;
	import.Hidemenu						= PF_UI_HideMenu;
	import.Pushmenu						= PF_UI_PushMenu;
	import.HideMouseCursor				= PF_UI_HideMouse_f;
	import.ShowMouseCursor				= PF_UI_ShowMouse_f;
	import.MapTime						= CM_MapTime;
	import.LoadResource					= UI_LoadResource;
	import.ClearResource				= UI_ClearResource;

	import.Key_StringToKeynum			= PF_Key_StringToKeynum;
	import.Key_KeynumToBindString		= PF_Key_KeynumToBindString;
	import.Key_GetKeysForCommand		= PF_Key_GetKeysForCommand;
	import.ArchiveLevel					= SV_ArchiveLevel;
	import.AddSvsTimeFixup				= SV_AddSvsTimeFixup;
	import.HudDrawShader				= SV_HudDrawShader;
	import.HudDrawAlign					= SV_HudDrawAlign;
	import.HudDrawRect					= SV_HudDrawRect;
	import.HudDrawVirtualSize			= SV_HudDrawVirtualSize;
	import.HudDrawColor					= SV_HudDrawColor;
	import.HudDrawAlpha					= SV_HudDrawAlpha;
	import.HudDrawString				= SV_HudDrawString;
	import.HudDrawFont					= SV_HudDrawFont;
	import.SanitizeName					= Com_SanitizeName;
	import.fsDebug						= fs_debug;

	ge = Sys_GetGameAPI( &import );

	if( !ge ) {
		Com_Error( ERR_DROP, "failed to load game DLL" );
	}

	if( ge->apiversion != GAME_API_VERSION )
	{
		Com_Error( ERR_DROP, "game is version %i, not %i", ge->apiversion,
			GAME_API_VERSION );
	}

	ge->Init( svs.startTime, Com_Milliseconds() );

	err = ge->errorMessage;
	if( err )
	{
		ge->errorMessage = NULL;
		Com_Error( ERR_DROP, err );
	}

	for( i = 0; i < svs.iNumClients; i++ )
	{
		svs.clients[ i ].gentity = NULL;
	}
}


/*
====================
SV_GameCommand

See if the current console command is claimed by the game
====================
*/
qboolean SV_GameCommand( void ) {
	if ( sv.state != SS_GAME ) {
		return qfalse;
	}

	return ge->ConsoleCommand();
}

