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
// cg_eventSystem.c

#include "cg_local.h"
#include "../qcommon/tiki_local.h"

typedef struct event_s {
	centity_t *ent;
	char buffer[1024];
	int execTime; // event should be executed and fried if cg.time >= ev->execTime
	struct event_s *next,*prev; // linked list
} event_t;

#define MAX_PENDING_EVENTS 512

static event_t events[MAX_PENDING_EVENTS];
static event_t cg_pendingEvents;	// double linked list
static event_t *cg_freeEvents;		// single linked list

void CG_InitEventSystem() {
	int		i;

	memset( events, 0, sizeof( events ) );
	cg_pendingEvents.next = &cg_pendingEvents;
	cg_pendingEvents.prev = &cg_pendingEvents;
	cg_freeEvents = events;
	for ( i = 0 ; i < MAX_PENDING_EVENTS - 1 ; i++ ) {
		events[i].next = &events[i+1];
	}
}
/*
Generates a random float component for MoHAA events.
If random is specified, the component will range from 0 to specified value.
If crandom is specified, the component will range from -specified to +specified value.
If range is specified, the component needs two values; it will randomly pick a number in the range
from the first number to the first number plus the second number.
This is used by "randvel", "randvelaxis", "offset", "offsetalongaxis", "angles", MoHAA events
*/
float CG_ParseEventFloatParm(char **text) {
	char *token;
	float value;
	token = COM_ParseExt( text, qfalse );
	if(!Q_stricmp(token,"random")) {
		token = COM_ParseExt( text, qfalse );
		value = random()*atof(token);
	} else if(!Q_stricmp(token,"crandom")) {
		token = COM_ParseExt( text, qfalse );
		value = crandom()*atof(token);
	} else if(!Q_stricmp(token,"range")) {
		float range;
		token = COM_ParseExt( text, qfalse );
		value = atof(token);	

		token = COM_ParseExt( text, qfalse );
		range = atof(token);	

		value += random()*range;
	} else {
#if 0
		CG_Printf("CG_ParseEventFloatParm: unknown first parm, expected 'random', 'crandom' or 'range', found %s\n",token);
		value = 0;
#else
		value = atof(token);
#endif
	}
	return value;
}
// this is used by both "tagspawn" and "originspawn" events
void CG_ProcessSpawnEvent(centity_t *ent, vec3_t spawnOrigin, vec3_t spawnAngles, char *text) {
	char *token;
	qhandle_t h;
	tiki_t *tiki;
	vec3_t v;
	localEntity_t *le;
	char *dot;

	// skip optional paramter(s) ? see rechamber anim of weapons/kar98.tik
	do {
		token = COM_ParseExt( &text, qtrue );
		if(token[0]==0) {
			CG_Printf("'***spawn' event without parameters block\n");
			return;			
		}
	} while(token[0] != '(');
	if(token[1] == 0) {
		token = COM_ParseExt( &text, qtrue );
	} else {
		token++;
	}

	le = CG_AllocLocalEntity();
	le->endTime = cg.time + 10000;
	le->leType = LE_SKIP;
	//le->leFlags = LEF_TUMBLE;
	le->pos.trTime = cg.time - (rand()&15);
	VectorCopy( spawnOrigin, le->pos.trDelta );
	VectorCopy(spawnOrigin,le->refEntity.origin);
	VectorCopy( spawnAngles, le->angles.trDelta );
	AnglesToAxis(spawnAngles,le->refEntity.axis);
	tiki = le->tiki;
	// parse parameters ( ... ) block
	while(token[0] != ')') {
		if(token[0]==0) {
			CG_Printf("'***spawn' missing ')' in tiki %s\n",tiki->name);
			return;			
		}
		Q_strlwr(token);
		if(!strcmp(token,"spawnrange")) {
			token = COM_ParseExt( &text, qtrue );

		} else if(!strcmp(token,"spawnrate")) {
			token = COM_ParseExt( &text, qtrue );

		} else if(!strcmp(token,"count")) {
			token = COM_ParseExt( &text, qtrue );

		} else if(!strcmp(token,"model")) {
			// both .tik and .spr models are used here
			token = COM_ParseExt( &text, qtrue );
			h = cgi.R_RegisterModel(token);
			le->refEntity.model = h;
			le->leType = LE_FRAGMENT;
			dot = strchr(token,'.');
			if(dot && !Q_stricmp(dot+1,"tik")) {
				le->tiki = cgi.TIKI_RegisterModel(token);
			}
		} else if(!strcmp(token,"color")) {
			token = COM_ParseExt( &text, qfalse );
			token = COM_ParseExt( &text, qfalse );
			token = COM_ParseExt( &text, qfalse );

		} else if(!strcmp(token,"varycolor")) {
			token = COM_ParseExt( &text, qfalse );
			
		} else if(!strcmp(token,"alpha")) {
			token = COM_ParseExt( &text, qfalse );

		} else if(!strcmp(token,"offset")) {
			token = COM_ParseExt( &text, qfalse ); // x
			token = COM_ParseExt( &text, qfalse ); // y
			token = COM_ParseExt( &text, qfalse ); // z

		} else if(!strcmp(token,"offsetalongaxis")) {
			v[0] = CG_ParseEventFloatParm(&text); // x
			v[1] = CG_ParseEventFloatParm(&text); // y
			v[2] = CG_ParseEventFloatParm(&text); // z

		} else if(!strcmp(token,"randvelaxis")) {
			v[0] = CG_ParseEventFloatParm(&text); // x
			v[1] = CG_ParseEventFloatParm(&text); // y
			v[2] = CG_ParseEventFloatParm(&text); // z

		} else if(!strcmp(token,"scale")) {
			token = COM_ParseExt( &text, qtrue );

		} else if(!strcmp(token,"scalemin")) {
			token = COM_ParseExt( &text, qtrue );

		} else if(!strcmp(token,"scalemax")) {
			token = COM_ParseExt( &text, qtrue );

		} else if(!strcmp(token,"scalerate")) {
			token = COM_ParseExt( &text, qtrue );

		} else if(!strcmp(token,"velocity")) {
			token = COM_ParseExt( &text, qtrue ); // forwardVelocity 

		} else if(!strcmp(token,"angles")) {
			v[0] = CG_ParseEventFloatParm(&text);
			v[1] = CG_ParseEventFloatParm(&text);
			v[2] = CG_ParseEventFloatParm(&text);
			VectorCopy( v, le->angles.trDelta );
			AnglesToAxis(v,le->refEntity.axis);

		} else if(!strcmp(token,"randvel")) {
			v[0] = CG_ParseEventFloatParm(&text);
			v[1] = CG_ParseEventFloatParm(&text);
			v[2] = CG_ParseEventFloatParm(&text);
			VectorCopy(v,le->pos.trDelta);
		} else if(!strcmp(token,"emitterangles")) {
			token = COM_ParseExt( &text, qfalse ); // pitchofs
			token = COM_ParseExt( &text, qfalse ); // yawofs
			token = COM_ParseExt( &text, qfalse ); // rollofs
		} else if(!strcmp(token,"avelocity")) {
			// crandom keyword is used for avelocity in weapons/mp40.tik
			v[YAW] = CG_ParseEventFloatParm(&text); // yawVel
			v[PITCH] = CG_ParseEventFloatParm(&text); // pitchVel
			v[ROLL] = CG_ParseEventFloatParm(&text); // rollVel

			//VectorCopy(v,le->apos.trDelta);

		} else if(!strcmp(token,"accel")) {
			// NOTE: This acceleration is applied using the world axis
			token = COM_ParseExt( &text, qfalse ); // accelX
			token = COM_ParseExt( &text, qfalse ); // accelY
			token = COM_ParseExt( &text, qfalse ); // accelZ

		} else if(!strcmp(token,"physicsrate")) {
			token = COM_ParseExt( &text, qtrue );
			if(!Q_stricmp(token,"every")) {

			}
		} else if(!strcmp(token,"life")) {
			token = COM_ParseExt( &text, qtrue );
			le->endTime = cg.time + atof(token)*1000;
		} else if(!strcmp(token,"fadedelay")) {
			token = COM_ParseExt( &text, qtrue );

		} else if(!strcmp(token,"alignstretch")) {
			token = COM_ParseExt( &text, qfalse ); // scaleMultiplier 

		} else if(!strcmp(token,"alignonce")) {

		} else if(!strcmp(token,"align")) {

		} else if(!strcmp(token,"collision")) {

		} else if(!strcmp(token,"fade")) {

		} else if(!strcmp(token,"randomroll")) {

		} else if(!strcmp(token,"spritegridlighting")) {

		} else if(!strcmp(token,"volumetric")) {	
		// set the effect to spawn volumetric sources rather than tempmodels

		} else if(!strcmp(token,"wateronly")) {
		// makes the temp model remove itself if it leaves water

		} else if(!strcmp(token,"wind")) {
		// makes the temp model be affected by wind

		} else if(!strcmp(token,"bouncefactor")) {
			token = COM_ParseExt( &text, qtrue );
			le->bounceFactor  = atof(token);
		} else if(!strcmp(token,"bouncesoundonce")) {
			token = COM_ParseExt( &text, qtrue );


		} else if(!strcmp(token,"friction")) {
			token = COM_ParseExt( &text, qtrue );

		} else {
			//CG_Printf("Uknown '***spawn' parameter %s\n",token);
		}
		COM_ParseExt( &text, qtrue );
	}



}

void CG_ProcessEventText(centity_t *ent, const char *eventText) {
	char str[128];
	char *text;
	char *token;
	float f;
	int i;
	int boneName;
	tiki_t *tiki;
	vec3_t v,a;


	text = (char*)eventText;
again:
	// if(cg_printEvents.integer)
	//	CG_Printf("CG_ProcessEventText: event %s\n",eventText);

	// get event name
	token = COM_ParseExt( &text, qtrue );
	Q_strlwr(token);
	if(!strcmp(token,"sound")) {
	// syntax: sound( String soundName,
	// [ String channelName ], [ Float volume ],
	// [ Float min_distance ], [ Float pitch ],
	// [ Float randompitch ], [ String randomvolume ] )
		char *name;
		//float volume, minDist, pitch, randompitch;
		soundChannel_t channel;
		ubersound_t* sound;
		name = COM_ParseExt( &text, qtrue );
		sound = CG_GetUbersound(name);

		if (!sound)
			return;
		token = COM_ParseExt( &text, qtrue );

		if ( token[0] ) {
			if ( !Q_strncmp( token, "auto", MAX_QPATH ) )
				channel = CHAN_AUTO;
			else if ( !Q_strncmp( token, "body", MAX_QPATH ) )
				channel = CHAN_BODY;
			else if ( !Q_strncmp( token, "item", MAX_QPATH ) )
				channel = CHAN_ITEM;
			else if ( !Q_strncmp( token, "weaponidle", MAX_QPATH ) )
				channel = CHAN_WEAPONIDLE;
			else if ( !Q_strncmp( token, "voice", MAX_QPATH ) )
				channel = CHAN_VOICE;
			else if ( !Q_strncmp( token, "local", MAX_QPATH ) )
				channel = CHAN_LOCAL;
			else if ( !Q_strncmp( token, "weapon", MAX_QPATH ) )
				channel = CHAN_WEAPON;
			else if ( !Q_strncmp( token, "dialog_secondary", MAX_QPATH ) )
				channel = CHAN_DIALOG_SECONDARY;
			else if ( !Q_strncmp( token, "dialog", MAX_QPATH ) )
				channel = CHAN_DIALOG;
			else if ( !Q_strncmp( token, "menu", MAX_QPATH ) )
				channel = CHAN_MENU;
			else {
				//CG_Printf( "Event sound unrecognized channel %s for %s\n", token, name );
				channel = sound->channel;
			}
			//token = COM_ParseExt( &text, qtrue );
		} else channel = sound->channel;

/*		if ( token[0] ) {
			volume = atof( token );
			token = COM_ParseExt( &text, qtrue );
		}
		minDist		= atof( COM_ParseExt( &text, qtrue ) );
		pitch		= atof( COM_ParseExt( &text, qtrue ) );
		randompitch	= atof( COM_ParseExt( &text, qtrue ) );
		randomvolume = COM_ParseExt( &text, qtrue );*/
#if 0
		cgi.S_StartSound( ent->currentState.origin, -1, channel, sound->sfxHandle );
#else
		cgi.S_StartSound( 0, ent->currentState.number, channel, sound->sfxHandle );
#endif
	} else if(!strcmp(token,"stopsound")) {
		// syntax: stopsound( String channelName )
		// TODO
	} else if(!strcmp(token,"loopsound")) {
		// syntax: loopsound( String soundName, [ Float volume ],
		// [ Float min_distance ], Float pitch )
		char *name;
		//float volume, minDist, pitch;
		ubersound_t* snd;
		name = COM_ParseExt( &text, qtrue );
		snd = CG_GetUbersound(name);
/*		volume	= atof( COM_ParseExt( &text, qtrue ) );
		minDist	= atof( COM_ParseExt( &text, qtrue ) );
		pitch	= atof( COM_ParseExt( &text, qtrue ) );
*/
		if (snd)
			cgi.S_AddLoopingSound( ent->currentState.number, ent->currentState.origin, vec3_origin, snd->sfxHandle );
	} else if(!strcmp(token,"stoploopsound")) {
		cgi.S_StopLoopingSound( ent->currentState.number );
	} else if(!strcmp(token,"stopaliaschannel")) {
		token = COM_ParseExt( &text, qtrue );
	} else if(!strcmp(token,"viewkick")) {
	// viewkick( Float pitchmin, Float pitchmax,
	// Float yawmin, Float yawmax, Float recenterspeed,
	// String patters, Float pitchmax, Float yawmax )
		// TODO
	} else if(!strcmp(token,"cache")) {
		token = COM_ParseExt( &text, qtrue );
		//CG_Printf("Caching %s...\n",token);
		cgi.R_RegisterModel(token);
	} else if(!strcmp(token,"commanddelay")) {
		token = COM_ParseExt( &text, qtrue );
		f = atof(token);
		CG_PostEvent(ent,text,f*1000);
	} else if(!strcmp(token,"originspawn")) {
		CG_ProcessSpawnEvent(ent,ent->lerpOrigin, ent->lerpAngles, text);
	} else if(!strcmp(token,"tagspawn")) {
		tiki = cgs.gameTIKIs[ent->currentState.modelindex];
		if(!tiki) {
			CG_Printf("'tagspawn' event cast on entity with null TIKI\n");
			return;
		}
		// parse tagname
		token = COM_ParseExt( &text, qtrue );
		// find tag in tiki
		boneName = cgi.TIKI_GetBoneNameIndex(token); 
		for(i = 0; i < tiki->numBones; i++) {
			if(tiki->boneNames[i] == boneName) {
				break;
			}
		}
		if(i == tiki->numBones) {
			CG_Printf("Cant find bone %s in tiki %s for 'tagspawn' event\n",token,tiki->name);
			return;
		}
		if(ent->bones == 0) {
			CG_Printf("'tagspawn' event cast on entity with null bones ptr\n");
			return;
		}
		CG_CentBoneIndexLocal2World(i,ent,v,a);
		CG_ProcessSpawnEvent(ent,v,a,text);
	} else if(!strcmp(token,"tagspawnlinked")) {
		return; // FIXME!
	} else if(!strcmp(token,"tagdlight")) {
		float r,g,b,intensity;
	// tagdlight( String tagName, Float red, Float green,
	//			Float blue, Float intensity, Float life, 
	//			[ String intvel ], [ String type1 ] )
	/*
	Spawn a dynamic light from the specified tag
	The red,green,blue parms are the color of the light
	The intensity is the radius of the light
	type is the type of light to create (lensflare,viewlensflare,additive)
	*/
		tiki = cgs.gameTIKIs[ent->currentState.modelindex];
		if(!tiki) {
			CG_Printf("'tagdlight' event cast on entity with null TIKI\n");
			return;
		}
		// parse tagname
		token = COM_ParseExt( &text, qtrue );
		// find tag in tiki
		boneName = cgi.TIKI_GetBoneNameIndex(token); 
		for(i = 0; i < tiki->numBones; i++) {
			if(tiki->boneNames[i] == boneName) {
				break;
			}
		}
		if(i == tiki->numBones) {
			CG_Printf("Cant find bone %s in tiki %s for 'tagdlight' event\n",token,tiki->name);
			return;
		}
		if(ent->bones == 0) {
			CG_Printf("'tagdlight' event cast on entity with null bones ptr\n");
			return;
		}
		token = COM_ParseExt( &text, qtrue );
		r = atof(token);
		token = COM_ParseExt( &text, qtrue );
		g = atof(token);
		token = COM_ParseExt( &text, qtrue );
		b = atof(token);
		token = COM_ParseExt( &text, qtrue );
		intensity = atof(token);
		// optional parameters
		token = COM_ParseExt( &text, qtrue );
		if(token[0]) {
			// TODO
			token = COM_ParseExt( &text, qtrue );
			if(token[0]) {
				if(!Q_stricmp(token,"addictive")) {

				} else if(!Q_stricmp(token,"lensflare")) {

				} else if(!Q_stricmp(token,"viewlensflare")) {

				}
			}
		}
		CG_CentBoneIndexLocal2World(i,ent,v,0);
		cgi.R_AddLightToScene(v,intensity,r,g,b);
	} else if(!strcmp(token,"sfx")) {
		goto again;

	} else if(!strcmp(token,"bodyfall")) {

	} else if(!strcmp(token,"footstep")) {
		int iRunning;
		int iEquipment;
	//footstep( String tag, String sRunning, [ Integer iEquipment ] )
	// plays a footstep sound that is appropriate to the surface
	// we are currently stepping on.
	// sRunning should be set to run, walk, or ladder
		tiki = cgs.gameTIKIs[ent->currentState.modelindex];
		if(!tiki) {
			CG_Printf("'footstep' event cast on entity with null TIKI\n");
			return;
		}
		// parse tagname
		token = COM_ParseExt( &text, qfalse );
		strcpy(str,token);
		/*
		// find tag in tiki
		boneName = cgi.TIKI_GetBoneNameIndex(token); 
		for(i = 0; i < tiki->numBones; i++) {
			if(tiki->boneNames[i] == boneName) {
				break;
			}
		}
		if(i == tiki->numBones) {
			CG_Printf("Cant find bone %s in tiki %s for 'footstep' event\n",token,tiki->name);
			return;
		}
		if(ent->bones == 0) {
			CG_Printf("'footstep' event cast on entity with null bones ptr\n");
			return;
		}	
		*/
		// sRunning
		token = COM_ParseExt( &text, qfalse );
		if(!Q_stricmp(token,"run")) {
			//iRunning = ??;
		} else if(!Q_stricmp(token,"walk")) {
			//iRunning = ??;
		} else if(!Q_stricmp(token,"ladder")) {
			//iRunning = ??;
		} else if(!Q_stricmp(token,"R")) {
			//iRunning = ??;
		} else if(!Q_stricmp(token,"L")) {
			//iRunning = ??;
		} else {
			//iRunning = ??;
			CG_Printf("'footstep' event: unknown sRunning type %s\n",token);
		}
		// Integer iEquipment
		token = COM_ParseExt( &text, qfalse );
		iEquipment = 0;
		if(token[0]) {
			iEquipment = atoi(token);
		}

		iRunning = 0;
		CG_Footstep(str,ent,/*0,*/iRunning,iEquipment);

	} else if(!strcmp(token,"emitteroff")) {

	} else if(!strcmp(token,"emitteron")) {

	} else {
		CG_Printf("CG_ProcessEventText: unknown event %s\n",token);
	}
	

	
}

void CG_FreeEvent(event_t *ev) {
	if ( !ev->prev ) {
		CG_Error( "CG_FreeEvent: not active" );
	}

	// remove from the doubly linked active list
	ev->prev->next = ev->next;
	ev->next->prev = ev->prev;

	// the free list is only singly linked
	ev->next = cg_freeEvents;
	cg_freeEvents = ev;
}
event_t	*CG_AllocEvent() {
	event_t	*ev;

	if ( !cg_freeEvents ) {
		CG_Printf("CG_AllocEvent: no free events\n");
		return 0;
	}

	ev = cg_freeEvents;
	cg_freeEvents = cg_freeEvents->next;

	memset( ev, 0, sizeof( *ev ) );

	// link into the active list
	ev->next = cg_pendingEvents.next;
	ev->prev = &cg_pendingEvents;
	cg_pendingEvents.next->prev = ev;
	cg_pendingEvents.next = ev;
	return ev;
}

void CG_PostEvent(centity_t *ent, const char *eventText, int eventDelay) {
	event_t	*ev;
	ev = CG_AllocEvent();
	if(!ev)
		return; // failed to alloc an event
	ev->ent = ent;
	strcpy(ev->buffer,eventText);
	ev->execTime = cg.time + eventDelay;
}

void CG_ProcessEvent(event_t *ev) {
	CG_ProcessEventText(ev->ent,ev->buffer);
	CG_FreeEvent(ev);
}

void CG_ProcessPendingEvents() {
	event_t	*ev, *next;
	ev = cg_pendingEvents.next;
	for ( ; ev != &cg_pendingEvents ; ev = next ) {
		// grab next now, so if the event is freed we
		// still have it
		next = ev->next;
		if ( cg.time >= ev->execTime ) {
			// execute it
			CG_ProcessEventText(ev->ent, ev->buffer);
			// and free
			CG_FreeEvent(ev);
			continue;
		}
	}

}



