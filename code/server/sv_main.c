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
#include "../gamespy/sv_gamespy.h"
#include "../gamespy/sv_gqueryreporting.h"

cvar_t			*sv_mapname;
serverStatic_t	svs;				// persistant server info
server_t		sv;					// local server
//vm_t			*gvm = NULL;				// game virtual machine
game_export_t	*ge = NULL;

cvar_t	*sv_fps;				// time rate for running non-clients
cvar_t	*sv_timeout;			// seconds without any message
cvar_t	*sv_zombietime;			// seconds to sink messages after disconnect
cvar_t	*sv_rconPassword;		// password for remote server commands
cvar_t	*sv_privatePassword;	// password for the privateClient slots
cvar_t	*sv_allowDownload;
cvar_t	*sv_maxclients;

cvar_t	*sv_privateClients;		// number of clients reserved for password
cvar_t	*sv_hostname;
cvar_t	*sv_master[ MAX_MASTER_SERVERS ];		// master server ip address
cvar_t	*sv_reconnectlimit;		// minimum seconds between connect messages
cvar_t	*sv_showloss;			// report when usercmds are lost
cvar_t	*sv_padPackets;			// add nop bytes to messages
cvar_t	*sv_killserver;			// menu system can set to 1 to shut server down
cvar_t	*sv_mapChecksum;
cvar_t	*sv_serverid;
cvar_t	*sv_minRate;
cvar_t	*sv_maxRate;
cvar_t	*sv_dlRate;
cvar_t	*sv_minPing;
cvar_t	*sv_maxPing;
cvar_t	*sv_pure;
cvar_t	*sv_floodProtect;
cvar_t	*sv_maplist;
cvar_t	*sv_drawentities;
cvar_t	*sv_deeptracedebug;
cvar_t	*sv_netprofile;
cvar_t	*sv_netprofileoverlay;
cvar_t	*sv_netoptimize;
cvar_t	*sv_netoptimize_vistime;
cvar_t	*g_netoptimize;
cvar_t	*g_gametype;
cvar_t	*g_gametypestring;
cvar_t	*sv_chatter;
cvar_t	*sv_gamename;
cvar_t	*sv_location;
cvar_t	*sv_debug_gamespy;
cvar_t	*sv_gamespy;

// FIXME: use another global network ?
//cvar_t	*sv_debug_gamepsy;
//cvar_t	*sv_gamespy;
cvar_t	*sv_lanForceRate; // dedicated 1 (LAN) server forces local client rates to 99999 (bug #491)
#ifndef STANDALONE
cvar_t	*sv_strictAuth;
#endif
cvar_t	*sv_banFile;

serverBan_t serverBans[SERVER_MAXBANS];
int serverBansCount = 0;

/*
=============================================================================

EVENT MESSAGES

=============================================================================
*/

/*
===============
SV_ExpandNewlines

Converts newlines to "\n" so a line prints nicer
===============
*/
char	*SV_ExpandNewlines( char *in ) {
	static	char	string[1024];
	int		l;

	l = 0;
	while ( *in && l < sizeof(string) - 3 ) {
		if ( *in == '\n' ) {
			string[l++] = '\\';
			string[l++] = 'n';
		} else {
			string[l++] = *in;
		}
		in++;
	}
	string[l] = 0;

	return string;
}

/*
======================
SV_ReplacePendingServerCommands

  This is ugly
======================
*/
int SV_ReplacePendingServerCommands( client_t *client, const char *cmd ) {
	int i, index, csnum1, csnum2;

	for ( i = client->reliableSent+1; i <= client->reliableSequence; i++ ) {
		index = i & ( MAX_RELIABLE_COMMANDS - 1 );
		//
		if ( !Q_strncmp(cmd, client->reliableCommands[ index ], strlen("cs")) ) {
			sscanf(cmd, "cs %i", &csnum1);
			sscanf(client->reliableCommands[ index ], "cs %i", &csnum2);
			if ( csnum1 == csnum2 ) {
				Q_strncpyz( client->reliableCommands[ index ], cmd, sizeof( client->reliableCommands[ index ] ) );
				/*
				if ( client->netchan.remoteAddress.type != NA_BOT ) {
					Com_Printf( "WARNING: client %i removed double pending config string %i: %s\n", client-svs.clients, csnum1, cmd );
				}
				*/
				return qtrue;
			}
		}
	}
	return qfalse;
}

/*
======================
SV_AddServerCommand

The given command will be transmitted to the client, and is guaranteed to
not have future snapshot_t executed before it is executed
======================
*/
void SV_AddServerCommand( client_t *client, const char *cmd ) {
	int		index, i;

	// this is very ugly but it's also a waste to for instance send multiple config string updates
	// for the same config string index in one snapshot
//	if ( SV_ReplacePendingServerCommands( client, cmd ) ) {
//		return;
//	}

	// do not send commands until the gamestate has been sent
	if( client->state < CS_PRIMED )
		return;

	client->reliableSequence++;
	// if we would be losing an old command that hasn't been acknowledged,
	// we must drop the connection
	// we check == instead of >= so a broadcast print added by SV_DropClient()
	// doesn't cause a recursive drop client
	if ( client->reliableSequence - client->reliableAcknowledge == MAX_RELIABLE_COMMANDS + 1 ) {
		Com_Printf( "===== pending server commands =====\n" );
		for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
			Com_Printf( "cmd %5d: %s\n", i, client->reliableCommands[ i & (MAX_RELIABLE_COMMANDS-1) ] );
		}
		Com_Printf( "cmd %5d: %s\n", i, cmd );
		SV_DropClient( client, "Server command overflow" );
		return;
	}
	index = client->reliableSequence & ( MAX_RELIABLE_COMMANDS - 1 );
	Q_strncpyz( client->reliableCommands[ index ], cmd, sizeof( client->reliableCommands[ index ] ) );
}


/*
=================
SV_SendServerCommand

Sends a reliable command string to be interpreted by 
the client game module: "cp", "print", "chat", etc
A NULL client will broadcast to all clients
=================
*/
void QDECL SV_SendServerCommand(client_t *cl, const char *fmt, ...) {
	va_list		argptr;
	byte		message[MAX_MSGLEN];
	client_t	*client;
	int			j;

	if (!svs.clients) {
		return;
	}
	
	va_start (argptr,fmt);
	Q_vsnprintf ((char *)message, sizeof(message), fmt,argptr);
	va_end (argptr);

	// Fix to http://aluigi.altervista.org/adv/q3msgboom-adv.txt
	// The actual cause of the bug is probably further downstream
	// and should maybe be addressed later, but this certainly
	// fixes the problem for now
	if ( strlen ((char *)message) + 1 >= MAX_STRING_CHARS ) {
		return;
	}

	if ( cl != NULL ) {
		SV_AddServerCommand( cl, (char *)message );
		return;
	}

	// hack to echo broadcast prints to console
	if ( com_dedicated->integer && !strncmp( (char *)message, "print", 5) ) {
		Com_Printf ("broadcast: %s\n", SV_ExpandNewlines((char *)message) );
	}

	// send the data to all relevent clients
	for (j = 0, client = svs.clients; j < sv_maxclients->integer ; j++, client++) {
		SV_AddServerCommand( client, (char *)message );
	}
}


/*
==============================================================================

MASTER SERVER FUNCTIONS

==============================================================================
*/

/*
================
SV_MasterHeartbeat

Send a message to the masters every few minutes to
let it know we are alive, and log information.
We will also have a heartbeat sent when a server
changes from empty to non-empty, and full to non-full,
but not on every player enter or exit.
================
*/
void SV_MasterHeartbeat( void ) {
	if (!sv_gamespy->integer) {
		return;
	}

	if (svs.time >= svs.nextHeartbeatTime)
	{
		svs.nextHeartbeatTime = svs.time + MAX_HEARTBEAT_TIME;
		SV_GamespyHeartbeat();
	}
}

/*
=================
SV_MasterShutdown

Informs all masters that this server is going down
=================
*/
void SV_MasterShutdown( void ) {
	// send a hearbeat right now
	svs.nextHeartbeatTime = -9999;
	SV_MasterHeartbeat();

	// send it again to minimize chance of drops
	svs.nextHeartbeatTime = -9999;
	SV_MasterHeartbeat();

	// when the master tries to poll the server, it won't respond, so
	// it will be removed from the list
}


/*
==============================================================================

CONNECTIONLESS COMMANDS

==============================================================================
*/

/*
================
SVC_Status

Responds with all the info that qplug or qspy can see about the server
and all connected players.  Used for getting detailed information after
the simple info query.
================
*/
void SVC_Status( netadr_t from ) {
	char	player[1024];
	char	status[MAX_MSGLEN];
	int		i;
	client_t	*cl;
	playerState_t	*ps;
	size_t	statusLength;
	size_t	playerLength;
	char	infostring[MAX_INFO_STRING];

	// ignore if we are in single player
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER ) {
		return;
	}

	strcpy( infostring, Cvar_InfoString( CVAR_SERVERINFO ) );

	// echo back the parameter to status. so master servers can use it as a challenge
	// to prevent timed spoofed reply packets that add ghost servers
	Info_SetValueForKey( infostring, "challenge", Cmd_Argv(1) );

	status[0] = 0;
	statusLength = 0;

	for (i=0 ; i < sv_maxclients->integer ; i++) {
		cl = &svs.clients[i];
		if ( cl->state >= CS_CONNECTED ) {
			ps = SV_GameClientNum( i );
			Com_sprintf (player, sizeof(player), "%i \"%s\"\n", 
			// su44: ps->persistant is not avaible in MoHAA
			//	ps->persistant[PERS_SCORE], cl->ping, cl->name);
				cl->ping, cl->name);
			playerLength = strlen(player);
			if (statusLength + playerLength >= sizeof(status) ) {
				break;		// can't hold any more
			}
			strcpy (status + statusLength, player);
			statusLength += playerLength;
		}
	}

	NET_OutOfBandPrint( NS_SERVER, from, "statusResponse\n%s\n%s", infostring, status );
}

/*
================
SVC_Info

Responds with a short info message that should be enough to determine
if a user is interested in a server to do a full status
================
*/
void SVC_Info( netadr_t from ) {
	int		i, count;
	char	*gamedir;
	char	infostring[MAX_INFO_STRING];

	// ignore if we are in single player
	if ( Cvar_VariableValue( "g_gametype" ) == GT_SINGLE_PLAYER || Cvar_VariableValue("ui_singlePlayerActive")) {
		return;
	}

	/*
	 * Check whether Cmd_Argv(1) has a sane length. This was not done in the original Quake3 version which led
	 * to the Infostring bug discovered by Luigi Auriemma. See http://aluigi.altervista.org/ for the advisory.
	 */

	// A maximum challenge length of 128 should be more than plenty.
	if(strlen(Cmd_Argv(1)) > 128)
		return;

	// don't count privateclients
	count = 0;
	for ( i = sv_privateClients->integer ; i < svs.iNumClients ; i++ ) {
		if ( svs.clients[i].state >= CS_CONNECTED ) {
			count++;
		}
	}

	infostring[0] = 0;

	// echo back the parameter to status. so servers can use it as a challenge
	// to prevent timed spoofed reply packets that add ghost servers
	Info_SetValueForKey( infostring, "challenge", Cmd_Argv(1) );

	Info_SetValueForKey( infostring, "protocol", va("%i", com_protocol->integer) );
	Info_SetValueForKey( infostring, "hostname", sv_hostname->string );
	Info_SetValueForKey( infostring, "mapname", sv_mapname->string );
	Info_SetValueForKey( infostring, "clients", va("%i", count) );
	Info_SetValueForKey( infostring, "sv_maxclients", 
		va("%i", svs.iNumClients - sv_privateClients->integer ) );
	Info_SetValueForKey( infostring, "gametype", va("%i", g_gametype->integer ) );
	Info_SetValueForKey( infostring, "gametypestring", g_gametypestring->string );
	Info_SetValueForKey( infostring, "pure", va("%i", sv_pure->integer ) );

	if( sv_minPing->integer ) {
		Info_SetValueForKey( infostring, "minPing", va("%i", sv_minPing->integer) );
	}
	if( sv_maxPing->integer ) {
		Info_SetValueForKey( infostring, "maxPing", va("%i", sv_maxPing->integer) );
	}
	gamedir = Cvar_VariableString( "fs_game" );
	if( *gamedir ) {
		Info_SetValueForKey( infostring, "game", gamedir );
	}

	Info_SetValueForKey(infostring, "gamever", com_target_version->string);
	Info_SetValueForKey(infostring, "serverType", va("%i", com_target_game->integer));

	NET_OutOfBandPrint( NS_SERVER, from, "infoResponse\n%s", infostring );
}

/*
================
SVC_FlushRedirect

================
*/
void SV_FlushRedirect( char *outputbuf ) {
	NET_OutOfBandPrint( NS_SERVER, svs.redirectAddress, "print\n%s", outputbuf );
}

/*
===============
SVC_RemoteCommand

An rcon packet arrived from the network.
Shift down the remaining args
Redirect all printfs
===============
*/
void SVC_RemoteCommand( netadr_t from, msg_t *msg ) {
	qboolean	valid;
	unsigned int time;
	char		remaining[1024];
	// TTimo - scaled down to accumulate, but not overflow anything network wise, print wise etc.
	// (OOB messages are the bottleneck here)
#define SV_OUTPUTBUF_LENGTH (1024 - 16)
	char		sv_outputbuf[SV_OUTPUTBUF_LENGTH];
	static unsigned int lasttime = 0;
	char *cmd_aux;

	// TTimo - https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=534
	time = Com_Milliseconds();
	if ( (unsigned)( time - lasttime ) < 500u ) {
		return;
	}
	lasttime = time;

	if ( !strlen( sv_rconPassword->string ) ||
		strcmp (Cmd_Argv(1), sv_rconPassword->string) ) {
		valid = qfalse;
		Com_Printf ("Bad rcon from %s:\n%s\n", NET_AdrToString (from), Cmd_Argv(2) );
	} else {
		valid = qtrue;
		Com_Printf ("Rcon from %s:\n%s\n", NET_AdrToString (from), Cmd_Argv(2) );
	}

	// start redirecting all print outputs to the packet
	svs.redirectAddress = from;
	Com_BeginRedirect (sv_outputbuf, SV_OUTPUTBUF_LENGTH, SV_FlushRedirect);

	if ( !strlen( sv_rconPassword->string ) ) {
		Com_Printf ("No rconpassword set on the server.\n");
	} else if ( !valid ) {
		Com_Printf ("Bad rconpassword.\n");
	} else {
		remaining[0] = 0;
		
		// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=543
		// get the command directly, "rcon <pass> <command>" to avoid quoting issues
		// extract the command by walking
		// since the cmd formatting can fuckup (amount of spaces), using a dumb step by step parsing
		cmd_aux = Cmd_Cmd();
		cmd_aux+=4;
		while(cmd_aux[0]==' ')
			cmd_aux++;
		while(cmd_aux[0] && cmd_aux[0]!=' ') // password
			cmd_aux++;
		while(cmd_aux[0]==' ')
			cmd_aux++;
		
		Q_strcat( remaining, sizeof(remaining), cmd_aux);
		
		Cmd_ExecuteString (remaining);

	}

	Com_EndRedirect ();
}

/*
=================
SV_ConnectionlessPacket

A connectionless packet has four leading 0xff
characters to distinguish it from a game channel.
Clients that are in the game can still send
connectionless packets.
=================
*/
void SV_ConnectionlessPacket( netadr_t from, msg_t *msg ) {
	char	*s;
	char	*c;

	MSG_BeginReadingOOB( msg );
	MSG_ReadLong( msg );		// skip the -1 marker

	MSG_ReadByte( msg );		// skip the direction byte

	if (!Q_strncmp("connect", (char *) &msg->data[5], 7)) {
		Huff_Decompress(msg, 13);
	}

	s = MSG_ReadStringLine( msg );
	Cmd_TokenizeString( s );

	c = Cmd_Argv(0);
	Com_DPrintf ("SV packet %s : %s\n", NET_AdrToString(from), c);

	if (!Q_stricmp(c, "getstatus")) {
		SVC_Status( from  );
	} else if (!Q_stricmp(c, "getinfo")) {
		SVC_Info( from );
	} else if (!Q_stricmp(c, "getchallenge")) {
		SV_GetChallenge( from );
	} else if (!Q_stricmp(c, "connect")) {
		SV_DirectConnect( from );
	} else if (!Q_stricmp(c, "authorizeThis")) {
		SV_GamespyAuthorize( from, Cmd_Argv(1) );
	} else if (!Q_stricmp(c, "rcon")) {
		SVC_RemoteCommand( from, msg );
	} else if (!Q_stricmp(c, "disconnect")) {
		// if a client starts up a local server, we may see some spurious
		// server disconnect messages when their new server sees our final
		// sequenced messages to the old client
	} else {
		Com_DPrintf ("bad connectionless packet from %s:\n%s\n"
		, NET_AdrToString (from), s);
	}
}

//============================================================================

/*
=================
SV_ReadPackets
=================
*/
void SV_PacketEvent( netadr_t from, msg_t *msg ) {
	int			i;
	client_t	*cl;
	int			qport;

	// check for connectionless packet (0xffffffff) first
	if ( msg->cursize >= 4 && *(int *)msg->data == -1) {
		SV_ConnectionlessPacket( from, msg );
		return;
	}

	// read the qport out of the message so we can fix up
	// stupid address translating routers
	MSG_BeginReadingOOB( msg );
	i=MSG_ReadLong( msg );				// sequence number
	qport = MSG_ReadShort( msg ) & 0xffff;

	// find which client the message is from
	for (i=0, cl=svs.clients ; i < svs.iNumClients; i++,cl++) {
		if (cl->state == CS_FREE) {
			continue;
		}
		if ( !NET_CompareBaseAdr( from, cl->netchan.remoteAddress ) ) {
			continue;
		}
		// it is possible to have multiple clients from a single IP
		// address, so they are differentiated by the qport variable
		if (cl->netchan.qport != qport) {
			continue;
		}

		// the IP port can't be used to differentiate them, because
		// some address translating routers periodically change UDP
		// port assignments
		if (cl->netchan.remoteAddress.port != from.port) {
			Com_Printf( "SV_PacketEvent: fixing up a translated port\n" );
			cl->netchan.remoteAddress.port = from.port;
		}

		// make sure it is a valid, in sequence packet
		if (SV_Netchan_Process(cl, msg)) {
			// zombie clients still need to do the Netchan_Process
			// to make sure they don't need to retransmit the final
			// reliable message, but they don't do any other processing
			if (cl->state != CS_ZOMBIE) {
				cl->lastPacketTime = svs.lastTime;	// don't timeout
				SV_ExecuteClientMessage( cl, msg );
			}
		}
		return;
	}
	
	// if we received a sequenced packet from an address we don't recognize,
	// send an out of band disconnect packet to it
	NET_OutOfBandPrint( NS_SERVER, from, "disconnect" );
}


/*
===================
SV_CalcPings

Updates the cl->ping variables
===================
*/
void SV_CalcPings( void ) {
	int			i, j;
	client_t	*cl;
	int			total, count;
	int			delta;
	playerState_t	*ps;

	for (i=0 ; i < sv_maxclients->integer ; i++) {
		cl = &svs.clients[i];
		if ( cl->state != CS_ACTIVE ) {
			cl->ping = 999;
			continue;
		}
		if ( !cl->gentity ) {
			cl->ping = 999;
			continue;
		}
		if ( cl->gentity->r.svFlags & SVF_MONSTER ) {
			cl->ping = 0;
			continue;
		}

		total = 0;
		count = 0;
		for ( j = 0 ; j < PACKET_BACKUP ; j++ ) {
			if ( cl->frames[j].messageAcked <= 0 ) {
				continue;
			}
			delta = cl->frames[j].messageAcked - cl->frames[j].messageSent;
			count++;
			total += delta;
		}
		if (!count) {
			cl->ping = 999;
		} else {
			cl->ping = total/count;
			if ( cl->ping > 999 ) {
				cl->ping = 999;
			}
		}

		// let the game dll know about the ping
		ps = SV_GameClientNum( i );
		ps->ping = cl->ping;
	}
}

/*
==================
SV_CheckTimeouts

If a packet has not been received from a client for timeout->integer 
seconds, drop the conneciton.  Server time is used instead of
realtime to avoid dropping the local client while debugging.

When a client is normally dropped, the client_t goes into a zombie state
for a few seconds to make sure any final reliable message gets resent
if necessary
==================
*/
void SV_CheckTimeouts( void ) {
	int		i;
	client_t	*cl;
	int			droppoint;
	int			zombiepoint;

	droppoint = svs.time - 1000 * sv_timeout->integer;
	zombiepoint = svs.time - 1000 * sv_zombietime->integer;

	for (i=0,cl=svs.clients ; i < sv_maxclients->integer ; i++,cl++) {
		// message times may be wrong across a changelevel
		if (cl->lastPacketTime > svs.time) {
			cl->lastPacketTime = svs.time;
		}

		if (cl->state == CS_ZOMBIE
		&& cl->lastPacketTime < zombiepoint) {
			// using the client id cause the cl->name is empty at this point
			Com_DPrintf( "Going from CS_ZOMBIE to CS_FREE for client %d\n", i );
			cl->state = CS_FREE;	// can now be reused
			continue;
		}
		if ( cl->state >= CS_CONNECTED && cl->lastPacketTime < droppoint) {
			// wait several frames so a debugger session doesn't
			// cause a timeout
			if ( ++cl->timeoutCount > 5 ) {
				SV_DropClient (cl, "timed out"); 
				cl->state = CS_FREE;	// don't bother with zombie state
			}
		} else {
			cl->timeoutCount = 0;
		}
	}
}


/*
==================
SV_CheckPaused
==================
*/
qboolean SV_CheckPaused( void ) {
	int		count;
	client_t	*cl;
	int		i;

	if ( !paused->integer && !SV_DoSaveGame() ) {
		return qfalse;
	}

	if( !ge->AllowPaused() ) {
		Com_Unpause();
		return qfalse;
	}

	// only pause if there is just a single client connected
	count = 0;
	for (i=0,cl=svs.clients ; i < svs.iNumClients ; i++,cl++) {
		if ( cl->state >= CS_CONNECTED && cl->netchan.remoteAddress.type != NA_BOT ) {
			count++;
		}
	}

	if ( count > 1 ) {
		Com_Unpause();
	}

	return qtrue;
}

/*
==================
SV_SetFrameNumber
==================
*/
void SV_SetFrameNumber(int frameNumber) {
	if (ge) {
		ge->SetFrameNumber(frameNumber);
	}
}

/*
==================
SV_FrameMsec
Return time in millseconds until processing of the next server frame.
==================
*/
int SV_FrameMsec()
{
	if(sv_fps)
	{
		int frameMsec;
		
		frameMsec = 1000.0f / sv_fps->value;
		
		if(frameMsec < sv.timeResidual)
			return 0;
		else
			return frameMsec - sv.timeResidual;
	}
	else
		return 1;
}

/*
==================
SV_Frame

Player movement occurs as a result of packet events, which
happen before SV_Frame is called
==================
*/
void SV_Frame( int msec ) {
	int		frameMsec;
	int		startTime;

	// the menu kills the server with this cvar
	if ( sv_killserver->integer ) {
		SV_Shutdown ("Server was killed");
		Cvar_Set( "sv_killserver", "0" );
		return;
	}

	if( !com_sv_running->integer )
	{
		// Running as a server, but no map loaded
		return;
	}

	// allow pause if only the local client is connected
	if ( SV_CheckPaused() ) {
		return;
	}

	// if it isn't time for the next frame, do nothing
	if ( sv_fps->integer < 1 ) {
		Cvar_Set( "sv_fps", "20" );
	}

	frameMsec = 1000 / sv_fps->integer;
	sv.timeResidual += msec;

	if ( com_dedicated->integer && sv.timeResidual < frameMsec ) {
		// NET_Sleep will give the OS time slices until either get a packet
		// or time enough for a server frame has gone by
		NET_Sleep( frameMsec - sv.timeResidual );
		return;
	}

	// if time is about to hit the 32nd bit, kick all clients
	// and clear sv.time, rather
	// than checking for negative time wraparound everywhere.
	// 2giga-milliseconds = 23 days, so it won't be too often
	if ( svs.time > 0x70000000 ) {
		SV_Shutdown( "Restarting server due to time wrapping" );
		Cbuf_AddText( va( "map %s\n", Cvar_VariableString( "mapname" ) ) );
		return;
	}
	// this can happen considerably earlier when lots of clients play and the map doesn't change
	if ( svs.nextSnapshotEntities >= 0x7FFFFFFE - svs.numSnapshotEntities ) {
		SV_Shutdown( "Restarting server due to numSnapshotEntities wrapping" );
		Cbuf_AddText( va( "map %s\n", Cvar_VariableString( "mapname" ) ) );
		return;
	}

	// update infostrings if anything has been changed
	if ( cvar_modifiedFlags & CVAR_SERVERINFO ) {
		SV_SetConfigstring( CS_SERVERINFO, Cvar_InfoString( CVAR_SERVERINFO ) );
		cvar_modifiedFlags &= ~CVAR_SERVERINFO;
	}
	if ( cvar_modifiedFlags & CVAR_SYSTEMINFO ) {
		SV_SetConfigstring( CS_SYSTEMINFO, Cvar_InfoString( CVAR_SYSTEMINFO ) );
		cvar_modifiedFlags &= ~CVAR_SYSTEMINFO;
	}

	if ( com_speeds->integer ) {
		startTime = Sys_Milliseconds ();
	} else {
		startTime = 0;	// quite a compiler warning
	}

	if( ( sv_fps->integer * msec > 1100 ) && ( svs.time > svs.serverLagTime + 2500 ) )
	{
		svs.serverLagTime = svs.time;
		SV_SendServerCommand( NULL, "svlag" );
	}

	// update ping based on the all received frames
	SV_CalcPings();

	// run the game simulation in chunks
	while ( sv.timeResidual >= frameMsec ) {
		sv.timeResidual -= frameMsec;
		svs.time += frameMsec;

		if( sv.state == SS_GAME )
		{
			const char *err;

			// let everything in the world think and move
			ge->RunFrame( svs.time, frameMsec );

			err = ge->errorMessage;
			if( err )
			{
				ge->errorMessage = NULL;
				Com_Error(ERR_DROP, "%s", err);
			}
		}
	}

	if ( com_speeds->integer ) {
		time_game = Sys_Milliseconds () - startTime;
	}

	// check timeouts
	SV_CheckTimeouts();

	// send messages back to the clients
	SV_SendClientMessages();

	// send a heartbeat to the master if needed
	SV_MasterHeartbeat();

	// process all gamespy queries
	SV_ProcessGamespyQueries();

	svs.lastTime = svs.time;
}

/*
==================
Com_Pause
==================
*/
void Com_Pause()
{
	if( deathmatch->integer ) {
		return;
	}

	if( !com_sv_running->integer ) {
		return;
	}

	if( sv.state != SS_GAME ) {
		return;
	}

	if( paused->integer == 1 ) {
		return;
	}

	SetBelowNormalThreadPriority();
	Cvar_Set( "paused", "1" );
}

/*
==================
Com_Unpause
==================
*/
void Com_Unpause()
{
	if( paused->integer )
	{
		if( paused->integer == 1 ) {
			SetNormalThreadPriority();
		}

		Cvar_Set( "paused", "0" );
		CL_ClearButtons();
	}
}

/*
==================
Com_FakePause
==================
*/
void Com_FakePause()
{
	if( !paused->integer ) {
		Cvar_Set( "paused", "2" );
	}
}

/*
==================
Com_FakeUnpause
==================
*/
void Com_FakeUnpause()
{
	if( paused->integer >= 2 )
	{
		Cvar_Set( "paused", "0" );
		CL_ClearButtons();
	}
}

/*
==================
Com_Pause_f
==================
*/
void Com_Pause_f()
{
	if( paused->integer == 1 ) {
		Com_Unpause();
	} else {
		Com_Pause();
	}
}

/*
==================
SV_SoundCallback
==================
*/
void SV_SoundCallback( int entnum, int channel_number, const char *name )
{
	if( ge ) {
		ge->SoundCallback( entnum, channel_number, name );
	} else {
		Com_Printf( "^~^~^ Failed SV_SoundCallback for (entnum %d, channel %d, sound '%s')\n", entnum, channel_number, name );
	}
}

//============================================================================

/*
====================
SV_RateMsec

Return the number of msec until another message can be sent to
a client based on its rate settings
====================
*/

#define UDPIP_HEADER_SIZE 28
#define UDPIP6_HEADER_SIZE 48

int SV_RateMsec(client_t* client)
{
    int rate, rateMsec;
    int messageSize;

    messageSize = client->netchan.lastSentSize;
    rate = client->rate;

    if (sv_maxRate->integer)
    {
        if (sv_maxRate->integer < 1000)
            Cvar_Set("sv_MaxRate", "1000");
        if (sv_maxRate->integer < rate)
            rate = sv_maxRate->integer;
    }

    if (sv_minRate->integer)
    {
        if (sv_minRate->integer < 1000)
            Cvar_Set("sv_minRate", "1000");
        if (sv_minRate->integer > rate)
            rate = sv_minRate->integer;
    }

    if (client->netchan.remoteAddress.type == NA_IP6)
        messageSize += UDPIP6_HEADER_SIZE;
    else
        messageSize += UDPIP_HEADER_SIZE;

    rateMsec = messageSize * 1000 / ((int)(rate * com_timescale->value));
    rate = Sys_Milliseconds() - client->netchan.lastSentTime;

    if (rate > rateMsec)
        return 0;
    else
        return rateMsec - rate;
}

/*
====================
SV_SendQueuedPackets

Send download messages and queued packets in the time that we're idle, i.e.
not computing a server frame or sending client snapshots.
Return the time in msec until we expect to be called next
====================
*/

int SV_SendQueuedPackets()
{
    int numBlocks;
    int dlStart, deltaT, delayT;
    static int dlNextRound = 0;
    int timeVal = INT_MAX;

    // Send out fragmented packets now that we're idle
    delayT = SV_SendQueuedMessages();
    if (delayT >= 0)
        timeVal = delayT;

    if (sv_dlRate->integer)
    {
        // Rate limiting. This is very imprecise for high
        // download rates due to millisecond timedelta resolution
        dlStart = Sys_Milliseconds();
        deltaT = dlNextRound - dlStart;

        if (deltaT > 0)
        {
            if (deltaT < timeVal)
                timeVal = deltaT + 1;
        }
        else
        {
            numBlocks = SV_SendDownloadMessages();

            if (numBlocks)
            {
                // There are active downloads
                deltaT = Sys_Milliseconds() - dlStart;

                delayT = 1000 * numBlocks * MAX_DOWNLOAD_BLKSIZE;
                delayT /= sv_dlRate->integer * 1024;

                if (delayT <= deltaT + 1)
                {
                    // Sending the last round of download messages
                    // took too long for given rate, don't wait for
                    // next round, but always enforce a 1ms delay
                    // between DL message rounds so we don't hog
                    // all of the bandwidth. This will result in an
                    // effective maximum rate of 1MB/s per user, but the
                    // low download window size limits this anyways.
                    if (timeVal > 2)
                        timeVal = 2;

                    dlNextRound = dlStart + deltaT + 1;
                }
                else
                {
                    dlNextRound = dlStart + delayT;
                    delayT -= deltaT;

                    if (delayT < timeVal)
                        timeVal = delayT;
                }
            }
        }
    }
    else
    {
        if (SV_SendDownloadMessages())
            timeVal = 0;
    }

    return timeVal;
}
