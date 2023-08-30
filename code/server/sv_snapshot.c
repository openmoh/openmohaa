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

#include "server.h"
#include "../qcommon/bg_compat.h"

#define	CULL_IN		0		// completely unclipped
#define	CULL_CLIP	1		// clipped by one or more planes
#define	CULL_OUT	2		// completely outside the clipping planes

/*
=============================================================================

Delta encode a client frame onto the network channel

A normal server packet will look like:

4	sequence number (high bit set if an oversize fragment)
<optional reliable commands>
1	svc_snapshot
4	last client reliable command
4	serverTime
1	lastframe for delta compression
1	snapFlags
1	areaBytes
<areabytes>
<playerstate>
<packetentities>

=============================================================================
*/

/*
=============
SV_EmitPacketEntities

Writes a delta update of an entityState_t list to the message.
=============
*/
static void SV_EmitPacketEntities( clientSnapshot_t *from, clientSnapshot_t *to, msg_t *msg ) {
	entityState_t	*oldent, *newent;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;

	// generate the delta update
	if ( !from ) {
		from_num_entities = 0;
	} else {
		from_num_entities = from->num_entities;
	}

	newent = NULL;
	oldent = NULL;
	newindex = 0;
	oldindex = 0;
	while ( newindex < to->num_entities || oldindex < from_num_entities ) {
		if ( newindex >= to->num_entities ) {
			newnum = 9999;
		} else {
			newent = &svs.snapshotEntities[(to->first_entity+newindex) % svs.numSnapshotEntities];
			newnum = newent->number;
		}

		if ( oldindex >= from_num_entities ) {
			oldnum = 9999;
		} else {
			oldent = &svs.snapshotEntities[(from->first_entity+oldindex) % svs.numSnapshotEntities];
			oldnum = oldent->number;
		}

		if ( newnum == oldnum ) {
			// delta update from old position
			// because the force parm is qfalse, this will not result
			// in any bytes being emited if the entity has not changed at all
			MSG_WriteDeltaEntity (msg, oldent, newent, qfalse, sv.frameTime);
			oldindex++;
			newindex++;
			continue;
		}

		if ( newnum < oldnum ) {
			// this is a new entity, send it from the baseline
			MSG_WriteDeltaEntity (msg, &sv.svEntities[newnum].baseline, newent, qtrue, sv.frameTime);
			newindex++;
			continue;
		}

		if ( newnum > oldnum ) {
			// the old entity isn't present in the new message
			MSG_WriteDeltaEntity (msg, oldent, NULL, qtrue, sv.frameTime);
			oldindex++;
			continue;
		}
	}

	MSG_WriteEntityNum(msg, (MAX_GENTITIES - 1));	// end of packetentities
}



/*
==================
SV_WriteSnapshotToClient
==================
*/
static void SV_WriteSnapshotToClient( client_t *client, msg_t *msg ) {
	clientSnapshot_t	*frame, *oldframe;
	int					lastframe;
	int					i;
	int					snapFlags;

	// this is the snapshot we are creating
	frame = &client->frames[ client->netchan.outgoingSequence & PACKET_MASK ];

	// try to use a previous frame as the source for delta compressing the snapshot
	if ( client->deltaMessage <= 0 || client->state != CS_ACTIVE ) {
		// client is asking for a retransmit
		oldframe = NULL;
		lastframe = 0;
	} else if ( client->netchan.outgoingSequence - client->deltaMessage 
		>= (PACKET_BACKUP - 3) ) {
		// client hasn't gotten a good message through in a long time
		Com_DPrintf ("%s: Delta request from out of date packet.\n", client->name);
		oldframe = NULL;
		lastframe = 0;
	} else {
		// we have a valid snapshot to delta from
		oldframe = &client->frames[ client->deltaMessage & PACKET_MASK ];
		lastframe = client->netchan.outgoingSequence - client->deltaMessage;

		// the snapshot's entities may still have rolled off the buffer, though
		if ( oldframe->first_entity <= svs.nextSnapshotEntities - svs.numSnapshotEntities ) {
			Com_DPrintf ("%s: Delta request from out of date entities.\n", client->name);
			oldframe = NULL;
			lastframe = 0;
		}
	}

	MSG_WriteSVC(msg, svc_snapshot);

	// NOTE, MRE: now sent at the start of every message from server to client
	// let the client know which reliable clientCommands we have received
	//MSG_WriteLong( msg, client->lastClientCommand );

	// send over the current server time so the client can drift
	// its view of time to try to match
	if( client->oldServerTime ) {
		// The server has not yet got an acknowledgement of the
		// new gamestate from this client, so continue to send it
		// a time as if the server has not restarted. Note from
		// the client's perspective this time is strictly speaking
		// incorrect, but since it'll be busy loading a map at
		// the time it doesn't really matter.
		MSG_WriteLong (msg, svs.time + client->oldServerTime);
	} else {
		// WOMBAT: note that MOHAA always goes into this else.
		// therefore we are deviating from the MOHAA protocol but i don't think this is a problem
		MSG_WriteLong (msg, svs.time);
	}

	if ( sv.timeResidual > 254 )
		MSG_WriteByte( msg, 255 );
	else MSG_WriteByte( msg, sv.timeResidual );

	// what we are delta'ing from
	MSG_WriteByte (msg, lastframe);

	snapFlags = svs.snapFlagServerBit;
	if ( client->rateDelayed ) {
		snapFlags |= SNAPFLAG_RATE_DELAYED;
	}
	if ( client->state != CS_ACTIVE ) {
		snapFlags |= SNAPFLAG_NOT_ACTIVE;
	}

	MSG_WriteByte (msg, snapFlags);

	// send over the areabits
	if ( frame->areabytes > 255 ) {
		Com_DPrintf( "WARNING: area bytes exceeds 255!  Bad!  Bad!" ); // 2015 actually had humour
		MSG_WriteByte( msg, 0 );
	} else {
		MSG_WriteByte (msg, frame->areabytes);
		MSG_WriteData (msg, frame->areabits, frame->areabytes);
	}

	// delta encode the playerstate
	if ( oldframe ) {
		MSG_WriteDeltaPlayerstate( msg, &oldframe->ps, &frame->ps, sv.frameTime);
	} else {
		MSG_WriteDeltaPlayerstate( msg, NULL, &frame->ps, sv.frameTime);
	}

	// delta encode the entities
	SV_EmitPacketEntities (oldframe, frame, msg);

	MSG_WriteSounds( msg, client->server_sounds, client->number_of_server_sounds );

	if ( client->stringToPrint[0] ) {
		if ( client->locprint ) {
			MSG_WriteSVC( msg, svc_locprint );
			MSG_WriteShort( msg, client->XOffset );
			MSG_WriteShort( msg, client->YOffset );
			MSG_WriteScrambledString( msg, client->stringToPrint );
		}
		else {
			MSG_WriteSVC( msg, svc_centerprint );
			MSG_WriteScrambledString( msg, client->stringToPrint);
		}
	}

	// padding for rate debugging
	if ( sv_padPackets->integer ) {
		for ( i = 0 ; i < sv_padPackets->integer ; i++ ) {
			MSG_WriteSVC(msg, svc_nop);
		}
	}
}


/*
==================
SV_UpdateServerCommandsToClient

(re)send all server commands the client hasn't acknowledged yet
==================
*/
void SV_UpdateServerCommandsToClient( client_t *client, msg_t *msg ) {
	int		i;

	// write any unacknowledged serverCommands
	for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
		MSG_WriteSVC( msg, svc_serverCommand );
		MSG_WriteLong( msg, i );
		MSG_WriteScrambledString( msg, client->reliableCommands[ i & (MAX_RELIABLE_COMMANDS-1) ] );
	}
	client->reliableSent = client->reliableSequence;
}

/*
=============================================================================

Build a client snapshot structure

=============================================================================
*/

#define	MAX_SNAPSHOT_ENTITIES	1024
typedef struct {
	int		numSnapshotEntities;
	int		snapshotEntities[MAX_SNAPSHOT_ENTITIES];	
} snapshotEntityNumbers_t;

/*
=======================
SV_QsortEntityNumbers
=======================
*/
static int QDECL SV_QsortEntityNumbers( const void *a, const void *b ) {
	int	*ea, *eb;

	ea = (int *)a;
	eb = (int *)b;

	if ( *ea == *eb ) {
		Com_Error( ERR_DROP, "SV_QsortEntityStates: duplicated entity" );
	}

	if ( *ea < *eb ) {
		return -1;
	}

	return 1;
}

static void SV_AddNonPVSSound(client_t* client, gentity_t* ent) {
	int i;

    for (i = 0; i < ent->r.num_nonpvs_sounds; i++) {
        SV_ClientSound(
            client,
            &ent->s.origin,
            ENTITYNUM_NONE,
            1,
            ent->r.nonpvs_sounds[i].index,
            ent->r.nonpvs_sounds[i].volume,
            ent->r.nonpvs_sounds[i].minDist,
            ent->r.nonpvs_sounds[i].pitch,
            ent->r.nonpvs_sounds[i].maxDist,
            qfalse
        );
	}
}

/*
===============
SV_WorldTrace
===============
*/
qboolean SV_WorldTrace(const vec3_t start, const vec3_t end, int mask)
{
	trace_t trace = { 0 };

    CM_BoxTrace(&trace, start, end, vec3_origin, vec3_origin, 0, mask, qfalse);
    return trace.fraction == 1;
}

/*
===============
SV_ClientIsVisibleTrace
===============
*/
qboolean SV_ClientIsVisibleTrace(const vec3_t fromOrigin, const vec3_t toOrigin, float height, float dot) {
	vec3_t dir;
	vec3_t end;

	VectorSubtract(toOrigin, fromOrigin, dir);
    VectorNormalize(dir);
    //VectorMA(toOrigin, -height, dir, end);
	VectorCopy(toOrigin, end);

	if (SV_WorldTrace(fromOrigin, end, (CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_SOLID))) {
		return qtrue;
	}

	if (dot < 0) {
		return qfalse;
    }

	end[2] -= height;
	return SV_WorldTrace(fromOrigin, end, (CONTENTS_SLIME | CONTENTS_LAVA | CONTENTS_SOLID));
}

/*
===============
SV_ClientIsVisible
===============
*/
qboolean SV_ClientIsVisible(int toNum, int fromNum, int distCheck, const vec3_t forward, const vec3_t right) {
	client_t* fromClient;
	playerState_t *fromPs, *toPs;
	vec3_t dir;
	vec3_t fromOrigin, toOrigin;
	vec3_t toRight;
	float dot;
	float speed;
	
	if (!g_netoptimize->integer || !sv_netoptimize->integer) {
		return qtrue;
	}

	if (toNum >= svs.iNumClients) {
		return qtrue;
	}

	fromClient = &svs.clients[fromNum];
	if (sv_netoptimize->integer == NETO_CULLED && distCheck == CULL_IN) {
		fromClient->lastVisCheckTime[toNum] = svs.time + sv_netoptimize_vistime->integer;
		return qtrue;
	}

	if (fromClient->lastVisCheckTime[toNum] > svs.time) {
		return qtrue;
	}

    fromPs = SV_GameClientNum(fromNum);
    toPs = SV_GameClientNum(toNum);

	if (fromPs->fLeanAngle == 0) {
		VectorCopy(fromPs->vEyePos, fromOrigin);
	} else if (fromPs->fLeanAngle >= 0) {
		VectorMA(fromPs->vEyePos, 30, right, fromOrigin);
	} else {
		VectorMA(fromPs->vEyePos, -30, right, fromOrigin);
    }
    VectorCopy(fromPs->vEyePos, fromOrigin);

	if (toPs->fLeanAngle == 0) {
		VectorCopy(toPs->vEyePos, toOrigin);
	} else if (toPs->fLeanAngle >= 0) {
		AngleVectors(toPs->viewangles, toRight, NULL, NULL);
		VectorMA(toPs->vEyePos, 30, toRight, toOrigin);
	} else {
		AngleVectors(toPs->viewangles, toRight, NULL, NULL);
		VectorMA(toPs->vEyePos, -30, toRight, toOrigin);
    }
    VectorCopy(toPs->vEyePos, toOrigin);

	VectorSubtract(toOrigin, fromOrigin, dir);
	VectorNormalize(dir);

	dot = DotProduct(forward, dir);
	if (SV_ClientIsVisibleTrace(fromOrigin, toOrigin, toPs->viewheight / 2, dot)) {
		fromClient->lastVisCheckTime[toNum] = svs.time + sv_netoptimize_vistime->integer;
		return qtrue;
	}

	speed = VectorLength(toPs->velocity);
	if (speed <= 0) {
		return qfalse;
	}

	//
	// check with velocity prediction
	//
	VectorMA(fromOrigin, sv.frameTime * 3, fromPs->velocity, fromOrigin);
	VectorMA(toOrigin, sv.frameTime * 3, toPs->velocity, toOrigin);

	if (SV_ClientIsVisibleTrace(fromOrigin, toOrigin, toPs->viewheight / 2, dot)) {
        fromClient->lastVisCheckTime[toNum] = svs.time + sv_netoptimize_vistime->integer;
        return qtrue;
	}

	// not visible
	return qfalse;
}

/*
===============
SV_AddEntToSnapshot
===============
*/
static void SV_AddEntToSnapshot( svEntity_t *svEnt, gentity_t *gEnt, snapshotEntityNumbers_t *eNums, svEntity_t* portalEnt, qboolean portalsky) {
	// if we have already added this entity to this snapshot, don't add again
	if ( svEnt->snapshotCounter == sv.snapshotCounter ) {
		return;
	}
	svEnt->snapshotCounter = sv.snapshotCounter;

	// if we are full, silently discard entities
	if ( eNums->numSnapshotEntities == MAX_SNAPSHOT_ENTITIES ) {
		return;
	}

	eNums->snapshotEntities[ eNums->numSnapshotEntities ] = gEnt->s.number;
	eNums->numSnapshotEntities++;
}

/*
===============
EntityDistCheck
===============
*/
int EntityDistCheck(const vec3_t origin, const vec3_t forward, const gentity_t* ent, float farplane, float fov) {
	vec3_t dir;
	float farplaneMax;
	float length;
	float fovCheck;
	float dot;

	VectorSubtract(origin, ent->r.centroid, dir);
	length = VectorNormalize(dir) - ent->r.radius;

	farplaneMax = farplane + 128;
	if (length >= farplaneMax) {
		return CULL_OUT;
	}

	fovCheck = fov / 80 * length;

	if (ent->s.number < svs.iNumClients && VectorLength(ent->s.pos.trDelta) < 5) {
		fovCheck *= 0.25f;
	}

	if (fovCheck <= 640) {
		return CULL_IN;
	}

	dot = DotProduct(forward, dir) + 0.5f;
	if (dot < 0) {
		dot = 0;
	}

	if (fovCheck * (dot * 2.f + 1.f) >= farplaneMax) {
		// outside the farplane
		return CULL_OUT;
	}

	return CULL_CLIP;
}

/*
===============
SV_AddEntitiesVisibleFromPoint
===============
*/
static void SV_AddEntitiesVisibleFromPoint(const vec3_t origin, clientSnapshot_t* frame, snapshotEntityNumbers_t* eNums, svEntity_t* portalEnt, qboolean portalsky, client_t* client, const vec3_t angles ) {
	int		e, i;
	gentity_t *ent;
	gentity_t *parentEnt;
	playerState_t *ps;
	svEntity_t	*svEnt, *svCheckEnt;
	int		l;
	int		clientarea, clientcluster;
	int		leafnum;
	int		c_fullsend;
	byte	*clientpvs;
	byte	*bitvector;
	gentity_t* skyorigin = NULL;
	int		num;
	int		check = 0;
	vec3_t	forward, right;

	// during an error shutdown message we may need to transmit
	// the shutdown message after the server has shutdown, so
	// specfically check for it
	if ( !sv.state ) {
		return;
	}

	ps = SV_GameClientNum(client - svs.clients);

	num = 1;
	if (sv_drawentities->integer) {
		num = sv.num_entities;
	}

	leafnum = CM_PointLeafnum (origin);
	clientarea = CM_LeafArea (leafnum);
	clientcluster = CM_LeafCluster (leafnum);

	// calculate the visible areas
	frame->areabytes = CM_WriteAreaBits( frame->areabits, clientarea );

	clientpvs = CM_ClusterPVS (clientcluster);

	AngleVectors(angles, forward, right, NULL);

	c_fullsend = 0;

	for ( e = 0 ; e < sv.num_entities ; e++ ) {
		ent = SV_GentityNum(e);

		// never send unused entities
		if (!ent->inuse) {
			continue;
		}

		// mark the entity as sent
		ent->r.svFlags |= SVF_SENT;

		// never send entities that aren't linked in
		if ( !ent->r.linked ) {
			continue;
		}
		// entities can be flagged to explicitly not be sent to the client
		if ( ent->r.svFlags & SVF_NOCLIENT ) {
			continue;
		}
		// entities can be flagged to be sent to only one client
		if ( ent->r.svFlags & SVF_SINGLECLIENT ) {
			if ( ent->r.singleClient != frame->ps.clientNum ) {
				continue;
			}
		}
		// entities can be flagged to be sent to everyone but one client
		if ( ent->r.svFlags & SVF_NOTSINGLECLIENT ) {
			if ( ent->r.singleClient == frame->ps.clientNum ) {
				continue;
			}
		}
		// entities can be flagged to be sent to a given mask of clients
		if ( ent->r.svFlags & SVF_CLIENTMASK ) {
			if (frame->ps.clientNum >= 32)
				Com_Error( ERR_DROP, "SVF_CLIENTMASK: clientNum > 32\n" );
			if (~ent->r.singleClient & (1 << frame->ps.clientNum))
				continue;
		}

		parentEnt = NULL;
		if (ent->s.parent != ENTITYNUM_NONE) {
			parentEnt = SV_GentityNum(ent->s.parent);
			// parents that will not send to clients will be skipped
			if (parentEnt && parentEnt->r.svFlags & SVF_NOCLIENT) {
				continue;
			}
		}

		svEnt = SV_SvEntityForGentity( ent );

		// don't double add an entity through portals
		if ( svEnt->snapshotCounter == sv.snapshotCounter ) {
			continue;
		}

		if (ent->s.renderfx & RF_SKYORIGIN) {
			if (sv.skyportal && !portalsky && !portalEnt) {
				if (skyorigin) {
					Com_Error(ERR_DROP, "SV_AddEntitiesVisibleFromPoint: duplicate sky origin");
				}
				skyorigin = ent;
				SV_AddEntToSnapshot(svEnt, ent, eNums, NULL, qfalse);
			}
			continue;
		}

		// broadcast entities are always sent
		// or broadcast entities that are sent once
		if ( (ent->r.svFlags & SVF_BROADCAST) || (ent->r.svFlags & SVF_SENDONCE)  ) {
			SV_AddEntToSnapshot( svEnt, ent, eNums, NULL, qfalse);
			continue;
		}

		if (parentEnt) {
			svEntity_t* parentSvEnt = SV_SvEntityForGentity(parentEnt);
			if (parentSvEnt->snapshotCounter == sv.snapshotCounter) {
				SV_AddEntToSnapshot(svEnt, ent, eNums, portalEnt, portalsky);
				continue;
			} else if (g_gametype->integer != GT_SINGLE_PLAYER && ent->s.parent < svs.iNumClients) {
				SV_AddNonPVSSound(client, ent);
				continue;
			}

			svCheckEnt = parentSvEnt;
		} else {
			svCheckEnt = svEnt;
		}

		if (!(ent->r.svFlags & SVF_PORTAL) && !ent->s.modelindex && !ent->s.loopSound) {
			// don't send entities that have nothing to draw
			continue;
		}

		if ((ent->s.loopSound && ent->s.loopSoundMinDist == 10000) || ent->s.renderfx & RF_VIEWMODEL) {
            // loopsound entities should be sent regardless
			SV_AddEntToSnapshot(svEnt, ent, eNums, portalEnt, portalsky);
            continue;
		}

		// ignore if not touching a PV leaf
		// check area
		if ( !CM_AreasConnected( clientarea, svCheckEnt->areanum ) ) {
			// doors can legally straddle two areas, so
			// we may need to check another one
			if ( !CM_AreasConnected( clientarea, svCheckEnt->areanum2 ) ) {
				continue;		// blocked by a door
			}
		}

		if (g_gametype->integer != GT_SINGLE_PLAYER && !(ent->r.svFlags & SVF_NOFARPLANE)) {
			float farplane = sv.farplane;

			if (farplane < 1) farplane = 1;
			if (farplane > 12000) farplane = 12000;

			check = EntityDistCheck(origin, forward, parentEnt ? parentEnt : ent, farplane, ps->fov);
			if (check == CULL_OUT) {
				continue;
			}
		}

		// check individual leafs
		if( !svEnt->numClusters ) {
			continue;
		}

		bitvector = clientpvs;

		l = 0;
		for ( i=0 ; i < svCheckEnt->numClusters ; i++ ) {
			l = svCheckEnt->clusternums[i];
			if ( bitvector[l >> 3] & (1 << (l&7) ) ) {
				break;
			}
		}

		// if we haven't found it to be visible,
		// check overflow clusters that coudln't be stored
		if ( i == svCheckEnt->numClusters ) {
			if ( svCheckEnt->lastCluster ) {
				for ( ; l <= svCheckEnt->lastCluster ; l++ ) {
					if ( bitvector[l >> 3] & (1 << (l&7) ) ) {
						break;
					}
				}
				if ( l == svCheckEnt->lastCluster ) {
					continue;	// not visible
				}
			} else {
				continue;
			}
		}

		if (svEnt->snapshotCounter == sv.snapshotCounter) {
			ent->s.renderfx &= ~(RF_WRAP_FRAMES | RF_SHADOW_PLANE);
			ent->s.renderfx |= RF_WRAP_FRAMES;
			continue;
		}

		if (g_gametype->integer != GT_SINGLE_PLAYER && ent->s.number < svs.iNumClients) {
			if (!SV_ClientIsVisible(ent->s.number, client - svs.clients, check, forward, right)) {
				SV_AddNonPVSSound(client, ent);
				continue;
			}
		}

		// add it
		SV_AddEntToSnapshot( svEnt, ent, eNums, NULL, qfalse);

		// if its a portal entity, add everything visible from its camera position
		if ( ent->r.svFlags & SVF_PORTAL && svEnt != portalEnt ) {
			SV_AddEntitiesVisibleFromPoint( ent->s.origin2, frame, eNums, svEnt, qfalse, client, angles  );
		}
	}

	if (!portalsky && skyorigin && !portalEnt) {
		SV_AddEntitiesVisibleFromPoint(skyorigin->s.origin, frame, eNums, NULL, qtrue, client, angles);
	}
}

/*
=============
SV_ClearNonPVSClient

Clears the client's radar.
=============
*/
void SV_ClearNonPVSClient(client_t* client) {
	client->radarInfo = client - svs.clients;
}

/*
=============
SV_InitRadar

Initializes radar for all clients.
=============
*/
void SV_InitRadar() {
	int i;

	for (i = 0; i < svs.iNumClients; i++) {
		SV_ClearNonPVSClient(&svs.clients[i]);
	}
}

/*
=============
SV_PackNonPVSClient

Pack radar information into a 32-bit integer for a client.
=============
*/
void SV_PackNonPVSClient(radarUnpacked_t* unpacked, int* packed) {
	float inv;
	float x, y;
	float length;
	int packedX, packedY;
	int valid;

	if (com_radar_range && com_radar_range->value) {
		inv = 1.f / com_radar_range->value;
	} else {
		inv = 0;
	}

	x = inv * unpacked->x;
	y = inv * unpacked->y;

	length = sqrt(x * x + y * y);
	if (length > 0) {
		valid = 1;
		x *= 1.f / length;
		y *= 1.f / length;
	} else {
		valid = 0;
	}

	packedX = (int)((x * 63.f) + 63.5f);
	packedY = (int)((y * 63.f) + 63.5f);
	if (packedX < 0) {
		packedX = 0;
	} else if (packedX > 126) {
		packedX = 126;
	}

	if (packedY < 0) {
		packedY = 0;
	} else if (packedY > 126) {
		packedY = 126;
	}

	*packed = (packedY << 13) | unpacked->clientNum | (packedX << 6) | (((valid << 5) | (int)(unpacked->yaw
		* 0.088f
		+ 32.5f) & 0x1F) << 20);
}

/*
=============
SV_SetNonPVSClient

Pack an invisible client (other) into the client's radar information.
=============
*/
void SV_SetNonPVSClient(client_t* client, client_t* other) {
	radarUnpacked_t radar;

	radar.clientNum = other - svs.clients;
	radar.x = other->gentity->s.origin[0] - client->gentity->s.origin[0];
	radar.y = other->gentity->s.origin[1] - client->gentity->s.origin[1];
	radar.yaw = other->gentity->s.angles[1];

	SV_PackNonPVSClient(&radar, &client->radarInfo);
	client->lastRadarTime[radar.clientNum] = svs.time;
}

/*
=============
SV_InTeamGame

Returns true if the client is in game and joined a team.
=============
*/
qboolean SV_InTeamGame(client_t* client) {
	return (client->state != CS_FREE && client->gentity && client->gentity->s.solid && client->gentity->s.eFlags & EF_ANY_TEAM);
}

/*
=============
SV_SameTeam

Returns true if both clients are on the same team.
=============
*/
qboolean SV_SameTeam(client_t* client1, client_t* client2) {
	return (client1->gentity->s.eFlags & EF_ANY_TEAM) == (client2->gentity->s.eFlags & EF_ANY_TEAM);
}

/*
=============
SV_IsTeamGame

Returns true if it's a team game.
=============
*/
qboolean SV_IsTeamGame() {
	return g_gametype->integer >= GT_TEAM;
}

/*
=============
SV_UpdateRadar

Updates the radar information for the specified client.
=============
*/
void SV_UpdateRadar(client_t* client) {
	client_t* other;
	client_t* mate;
	float deltaX, deltaY;
	float dist;
	int deltaTime;
	int bestTime;
	int i;

	mate = NULL;

	if (!SV_IsTeamGame()) {
		SV_ClearNonPVSClient(client);
		return;
	}

	if (!SV_InTeamGame(client)) {
		SV_ClearNonPVSClient(client);
		return;
	}

	bestTime = svs.time;

	for (i = 0; i < svs.iNumClients; i++) {
		other = &svs.clients[i];

		if (other == client) {
			continue;
		}

		if (!SV_InTeamGame(other)) {
			continue;
		}

		if (!SV_SameTeam(client, other)) {
			continue;
		}

		deltaX = other->gentity->s.origin[0] - client->gentity->s.origin[0];
		deltaY = other->gentity->s.origin[1] - client->gentity->s.origin[1];
		deltaTime = svs.time - client->lastRadarTime[i];
		dist = sqrt(deltaX * deltaX + deltaY * deltaY);

		if (dist > com_radar_range->value) {
			if (deltaTime < 1000) {
				continue;
			}

			deltaTime /= 2;
		}

		if (deltaTime > svs.time - bestTime) {
			bestTime = client->lastRadarTime[i];
			mate = other;
		}
	}

	if (!mate) {
		SV_ClearNonPVSClient(client);
		return;
	}

	SV_SetNonPVSClient(client, mate);
}

/*
=============
SV_BuildClientSnapshot

Decides which entities are going to be visible to the client, and
copies off the playerstate and areabits.

This properly handles multiple recursive portals, but the render
currently doesn't.

For viewing through other player's eyes, clent can be something other than client->gentity
=============
*/
static void SV_BuildClientSnapshot( client_t *client ) {
	vec3_t						org;
	vec3_t						ang;
	clientSnapshot_t			*frame;
	snapshotEntityNumbers_t		entityNumbers;
	int							i;
	gentity_t					*ent;
	entityState_t				*state;
	svEntity_t					*svEnt;
	gentity_t					*clent;
	int							clientNum;
	playerState_t				*ps;

	// bump the counter used to prevent double adding
	sv.snapshotCounter++;

	// this is the frame we are creating
	frame = &client->frames[ client->netchan.outgoingSequence & PACKET_MASK ];

	// clear everything in this snapshot
	entityNumbers.numSnapshotEntities = 0;
	Com_Memset( frame->areabits, 0, sizeof( frame->areabits ) );

  // https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=62
	frame->num_entities = 0;
	
	clent = client->gentity;
	if ( !clent || client->state == CS_ZOMBIE ) {
		return;
	}

	// grab the current playerState_t
	ps = SV_GameClientNum( client - svs.clients );
	frame->ps = *ps;
    frame->ps.net_pm_flags = CPT_DenormalizePlayerStateFlags(ps->pm_flags);
    frame->ps.iNetViewModelAnim = CPT_DenormalizeViewModelAnim(ps->iViewModelAnim);

	//SV_SvEntityForGentity
	// never send client's own entity, because it can
	// be regenerated from the playerstate
	clientNum = frame->ps.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_GENTITIES ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}
	svEnt = &sv.svEntities[ clientNum ];
	
	// su44: that's not done in MoHAA
	//svEnt->snapshotCounter = sv.snapshotCounter;

	// find the client's viewpoint
	if (ps->pm_flags & PMF_CAMERA_VIEW)
	{
		VectorCopy(ps->camera_origin, org);
		VectorCopy(ps->camera_angles, ang);
	}
	else
	{
		VectorCopy(ps->vEyePos, org);
		VectorCopy(ps->viewangles, ang);
	}

	SV_AddEntToSnapshot(svEnt, SV_GentityNum(client - svs.clients), &entityNumbers, NULL, qfalse);

	// add all the entities directly visible to the eye, which
	// may include portal entities that merge other viewpoints
	SV_AddEntitiesVisibleFromPoint( org, frame, &entityNumbers, NULL, qfalse, client, ang );

	// if there were portals visible, there may be out of order entities
	// in the list which will need to be resorted for the delta compression
	// to work correctly.  This also catches the error condition
	// of an entity being included twice.
	qsort( entityNumbers.snapshotEntities, entityNumbers.numSnapshotEntities, 
		sizeof( entityNumbers.snapshotEntities[0] ), SV_QsortEntityNumbers );

	// now that all viewpoint's areabits have been OR'd together, invert
	// all of them to make it a mask vector, which is what the renderer wants
	for ( i = 0 ; i < MAX_MAP_AREA_BYTES/4 ; i++ ) {
		((int *)frame->areabits)[i] = ((int *)frame->areabits)[i] ^ -1;
	}

	
	// copy the entity states out
	frame->num_entities = 0;
	frame->first_entity = svs.nextSnapshotEntities;
	for ( i = 0 ; i < entityNumbers.numSnapshotEntities ; i++ ) {
		ent = SV_GentityNum(entityNumbers.snapshotEntities[i]);
		if (ent->client) {
			client->lastRadarTime[ent->s.number + 1] = svs.time;
		}

		state = &svs.snapshotEntities[svs.nextSnapshotEntities % svs.numSnapshotEntities];
		*state = ent->s;
		svs.nextSnapshotEntities++;
		// this should never hit, map should always be restarted first in SV_Frame
		if ( svs.nextSnapshotEntities >= 0x7FFFFFFE ) {
			Com_Error(ERR_FATAL, "svs.nextSnapshotEntities wrapped");
		}
		frame->num_entities++;
	}

	SV_UpdateRadar(client);
    frame->ps.radarInfo = client->radarInfo;
}

/*
=======================
SV_SendMessageToClient

Called by SV_SendClientSnapshot and SV_SendClientGameState
=======================
*/
void SV_SendMessageToClient( msg_t *msg, client_t *client ) {
	// record information about the message
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSize = msg->cursize;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSent = svs.time;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageAcked = -1;

	// send the datagram
	SV_Netchan_Transmit(client, msg);
}


/*
=======================
SV_SendClientSnapshot

Also called by SV_FinalMessage

=======================
*/
void SV_SendClientSnapshot( client_t *client ) {
	byte		msg_buf[MAX_MSGLEN];
	msg_t		msg;

	// build the snapshot
	SV_BuildClientSnapshot( client );

	// bots need to have their snapshots build, but
	// the query them directly without needing to be sent
	if ( client->gentity && client->gentity->r.svFlags & SVF_MONSTER ) {
		return;
	}

	MSG_Init (&msg, msg_buf, sizeof(msg_buf));
	msg.allowoverflow = qtrue;

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand );

	// (re)send any reliable server commands
	SV_UpdateServerCommandsToClient( client, &msg );

	// send over all the relevant entityState_t
	// and the playerState_t
	SV_WriteSnapshotToClient( client, &msg );

	// clear the sounds on the client, preventing them to be sent each at packet
	SV_ClearSounds( client );

	// clear center print, preventing it to be sent at each packet
	client->stringToPrint[ 0 ] = 0;

	// su44: write any pending MoHAA cg messages
	SV_WriteCGMToClient( client, &msg );

	// Add any download data if the client is downloading
	SV_WriteDownloadToClient( client, &msg );

	// check for overflow
	if ( msg.overflowed ) {
		Com_Printf ("WARNING: msg overflowed for %s\n", client->name);
		MSG_Clear (&msg);
	}

	SV_SendMessageToClient( &msg, client );
}


/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages( void ) {
	int			i;
	int			rate;
	client_t	*c;

	// send a message to each connected client
	for (i=0, c = svs.clients ; i < sv_maxclients->integer ; i++, c++) {
		if (!c->state) {
			continue;		// not connected
		}

		if(svs.time - c->lastSnapshotTime < c->snapshotMsec * com_timescale->value)
			continue;		// It's not time yet

		if (!SV_IsValidSnapshotClient(c))
			continue; // not valid snapshot

		if(*c->downloadName)
			continue;		// Client is downloading, don't send snapshots

		if(c->netchan.unsentFragments || c->netchan_start_queue)
		{
			c->rateDelayed = qtrue;
			continue;		// Drop this snapshot if the packet queue is still full or delta compression will break
		}

		rate = SV_RateMsec(c);

		if(!(c->netchan.remoteAddress.type == NA_LOOPBACK ||
		     (sv_lanForceRate->integer && Sys_IsLANAddress(c->netchan.remoteAddress))))
		{
			// rate control for clients not on LAN 
			if(rate > 0)
			{
				// Not enough time since last packet passed through the line
				c->rateDelayed = qtrue;
				continue;
			}
		}

		// generate and send a new message
        SV_SendClientSnapshot(c);
        c->lastSnapshotTime = svs.time;
		c->rateDelayed = qfalse;
    }

    for (i = 0; i < sv.num_entities; i++) {
        gentity_t* ent = SV_GentityNum(i);
		ent->r.num_nonpvs_sounds = 0;
    }
}

qboolean SV_IsValidSnapshotClient(client_t* client) {
	if (client->deltaMessage <= 0) {
		return qtrue;
	}

	if (client->state != CS_ACTIVE) {
		return qtrue;
	}

	if (client->lastPacketTime >= svs.lastTime) {
		return qtrue;
	}

	if (client->netchan.outgoingSequence - client->deltaMessage < 29) {
		return qtrue;
	}

	return qfalse;
}
