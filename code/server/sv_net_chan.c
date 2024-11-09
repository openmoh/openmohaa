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

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "server.h"

#ifdef LEGACY_PROTOCOL
/*
==============
SV_Netchan_Encode

	// first four bytes of the data are always:
	long reliableAcknowledge;

==============
*/
static void SV_Netchan_Encode(client_t *client, msg_t *msg, const char *clientCommandString)
{
	long i, index;
	byte key, *string;
	int	srdc, sbit;
	qboolean soob;

	if ( msg->cursize < SV_ENCODE_START ) {
		return;
	}

	srdc = msg->readcount;
	sbit = msg->bit;
	soob = msg->oob;

	msg->bit = 0;
	msg->readcount = 0;
	msg->oob = qfalse;

	/* reliableAcknowledge = */ MSG_ReadLong(msg);

	msg->oob = soob;
	msg->bit = sbit;
	msg->readcount = srdc;

	string = (byte *) clientCommandString;
	index = 0;
	// xor the client challenge with the netchan sequence number
	key = client->challenge ^ client->netchan.outgoingSequence;
	for (i = SV_ENCODE_START; i < msg->cursize; i++) {
		// modify the key with the last received and with this message acknowledged client command
		if (!string[index])
			index = 0;
		if (string[index] > 127 || string[index] == '%') {
			key ^= '.' << (i & 1);
		}
		else {
			key ^= string[index] << (i & 1);
		}
		index++;
		// encode the data with this key
		*(msg->data + i) = *(msg->data + i) ^ key;
	}
}

/*
==============
SV_Netchan_Decode

	// first 12 bytes of the data are always:
	long serverId;
	long messageAcknowledge;
	long reliableAcknowledge;

==============
*/
static void SV_Netchan_Decode( client_t *client, msg_t *msg ) {
	int serverId, messageAcknowledge, reliableAcknowledge;
	int i, index, srdc, sbit;
	qboolean soob;
	byte key, *string;

	srdc = msg->readcount;
	sbit = msg->bit;
	soob = msg->oob;

	msg->oob = qfalse;

	serverId = MSG_ReadLong(msg);
	messageAcknowledge = MSG_ReadLong(msg);
	reliableAcknowledge = MSG_ReadLong(msg);

	msg->oob = soob;
	msg->bit = sbit;
	msg->readcount = srdc;

	string = (byte *)client->reliableCommands[ reliableAcknowledge & (MAX_RELIABLE_COMMANDS-1) ];
	index = 0;
	//
	key = client->challenge ^ serverId ^ messageAcknowledge;
	for (i = msg->readcount + SV_DECODE_START; i < msg->cursize; i++) {
		// modify the key with the last sent and acknowledged server command
		if (!string[index])
			index = 0;
		if (string[index] > 127 || string[index] == '%') {
			key ^= '.' << (i & 1);
		}
		else {
			key ^= string[index] << (i & 1);
		}
		index++;
		// decode the data with this key
		*(msg->data + i) = *(msg->data + i) ^ key;
	}
}
#endif



/*
=================
SV_Netchan_FreeQueue
=================
*/
void SV_Netchan_FreeQueue(client_t *client)
{
	netchan_buffer_t *netbuf, *next;
	
	for(netbuf = client->netchan_start_queue; netbuf; netbuf = next)
	{
		next = netbuf->next;
		Z_Free(netbuf);
	}
	
	client->netchan_start_queue = NULL;
	client->netchan_end_queue = &client->netchan_start_queue;
}

/*
=================
SV_Netchan_TransmitNextInQueue
=================
*/
void SV_Netchan_TransmitNextInQueue(client_t *client)
{
	netchan_buffer_t *netbuf;
		
	Com_DPrintf("#462 Netchan_TransmitNextFragment: popping a queued message for transmit\n");
	netbuf = client->netchan_start_queue;

#ifdef LEGACY_PROTOCOL
	if(client->compat)
		SV_Netchan_Encode(client, &netbuf->msg, netbuf->clientCommandString);
#endif

    Netchan_Transmit(&client->netchan, netbuf->msg.cursize, netbuf->msg.data, sv_netprofile->integer ? &client->netprofile.outPackets : NULL);

	// pop from queue
	client->netchan_start_queue = netbuf->next;
	if(!client->netchan_start_queue)
	{
		Com_DPrintf("#462 Netchan_TransmitNextFragment: emptied queue\n");
		client->netchan_end_queue = &client->netchan_start_queue;
	}
	else
		Com_DPrintf("#462 Netchan_TransmitNextFragment: remaining queued message\n");

	Z_Free(netbuf);
}

/*
=================
SV_Netchan_TransmitNextFragment
Transmit the next fragment and the next queued packet
Return number of ms until next message can be sent based on throughput given by client rate,
-1 if no packet was sent.
=================
*/

int SV_Netchan_TransmitNextFragment(client_t *client)
{
	if(client->netchan.unsentFragments)
	{
		Netchan_TransmitNextFragment(&client->netchan, sv_netprofile->integer ? &client->netprofile.outPackets : NULL);
		return SV_RateMsec(client);
	}
	else if(client->netchan_start_queue)
	{
		SV_Netchan_TransmitNextInQueue(client);
		return SV_RateMsec(client);
	}
	
	return -1;
}


/*
===============
SV_Netchan_Transmit
TTimo
https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=462
if there are some unsent fragments (which may happen if the snapshots
and the gamestate are fragmenting, and collide on send for instance)
then buffer them and make sure they get sent in correct order
================
*/

void SV_Netchan_Transmit( client_t *client, msg_t *msg)
{
	MSG_WriteByte( msg, svc_EOF );

	if(client->netchan.unsentFragments || client->netchan_start_queue)
	{
		netchan_buffer_t *netbuf;
		Com_DPrintf("#462 SV_Netchan_Transmit: unsent fragments, stacked\n");
		netbuf = (netchan_buffer_t *) Z_Malloc(sizeof(netchan_buffer_t));
		// store the msg, we can't store it encoded, as the encoding depends on stuff we still have to finish sending
		MSG_Copy(&netbuf->msg, netbuf->msgBuffer, sizeof( netbuf->msgBuffer ), msg);
#ifdef LEGACY_PROTOCOL
		if(client->compat)
		{
			Q_strncpyz(netbuf->clientCommandString, client->lastClientCommandString,
				   sizeof(netbuf->clientCommandString));
		}
#endif
		netbuf->next = NULL;
		// insert it in the queue, the message will be encoded and sent later
		*client->netchan_end_queue = netbuf;
		client->netchan_end_queue = &(*client->netchan_end_queue)->next;
	}
	else
	{
#ifdef LEGACY_PROTOCOL
		if(client->compat)
			SV_Netchan_Encode(client, msg, client->lastClientCommandString);
#endif
        Netchan_Transmit(&client->netchan, msg->cursize, msg->data, sv_netprofile->integer ? &client->netprofile.outPackets : NULL);
	}
}

/*
=================
Netchan_SV_Process
=================
*/
qboolean SV_Netchan_Process( client_t *client, msg_t *msg ) {
	int ret;
	ret = Netchan_Process( &client->netchan, msg, sv_netprofile->integer ? &client->netprofile.inPackets : NULL );
	if (!ret)
		return qfalse;

#ifdef LEGACY_PROTOCOL
	if(client->compat)
		SV_Netchan_Decode(client, msg);
#endif

	return qtrue;
}

/*
=================
SV_NET_OutOfBandPrint
=================
*/
void SV_NET_OutOfBandPrint(netprofclient_t* netprof, netadr_t adr, const char* format, ...) {
    va_list	argptr;
    char	string[MAX_MSGLEN];

    va_start(argptr, format);
    Q_vsnprintf(string, sizeof(string), format, argptr);
    va_end(argptr);

	NET_OutOfBandPrint(NS_SERVER, adr, "%s", string);

	if (sv_netprofile->integer && netprof) {
		NetProfileAddPacket(&netprof->outPackets, strlen(string), NETPROF_PACKET_MESSAGE);
	}
}

/*
=================
SV_NET_UpdateClientNetProfileInfo
=================
*/
void SV_NET_UpdateClientNetProfileInfo(netprofclient_t* netprofile, int rate) {
	int time;

	if (!netprofile->initialized) {
		memset(netprofile, 0, sizeof(*netprofile));
		netprofile->initialized = qtrue;
	}

	if (netprofile->rate != rate) {
		netprofile->rate = rate;
	}

	// set the update time
    time = Com_Milliseconds();

    netprofile->outPackets.updateTime = time;
    netprofile->inPackets.updateTime = time;
}

/*
=================
SV_NET_UpdateAllNetProfileInfo
=================
*/
void SV_NET_UpdateAllNetProfileInfo() {
	client_t *client;
	int i;

	if (!sv_netprofile->integer) {
		//
		// Clear all netprofile fields
		//

		svs.netprofile.initialized = qfalse;

		for (i = 0; i < svs.iNumClients; i++) {
			svs.clients[i].netprofile.initialized = qfalse;
		}

		return;
	}

	SV_NET_UpdateClientNetProfileInfo(&svs.netprofile, sv_maxRate->integer);

    for (i = 0; i < svs.iNumClients; i++) {
		client = &svs.clients[i];
		if (client->state != CS_ACTIVE || !client->gentity) {
			client->netprofile.initialized = qfalse;
			continue;
		}

		SV_NET_UpdateClientNetProfileInfo(&client->netprofile, client->rate);
    }
}

/*
=================
SV_NET_CalcTotalNetProfile
=================
*/
void SV_NET_CalcTotalNetProfile(netprofclient_t* netprofile, qboolean server) {
	client_t *client;
	int i;
	int numValidClients;

	numValidClients = 0;
	memset(netprofile, 0, sizeof(*netprofile));

    if (server) {
        NetProfileCalcStats(&svs.netprofile.outPackets, 0);
        NetProfileCalcStats(&svs.netprofile.inPackets, 0);
    } else {
        NetProfileCalcStats(&svs.netprofile.outPackets, 500);
        NetProfileCalcStats(&svs.netprofile.inPackets, 500);
    }

	for (i = 0; i < svs.iNumClients; i++) {
		client = &svs.clients[i];
		if (client->state != CS_ACTIVE || !client->gentity) {
			client->netprofile.initialized = qfalse;
			continue;
		}

		if (server) {
			NetProfileCalcStats(&client->netprofile.outPackets, 0);
			NetProfileCalcStats(&client->netprofile.inPackets, 0);
		} else {
			NetProfileCalcStats(&client->netprofile.outPackets, 500);
			NetProfileCalcStats(&client->netprofile.inPackets, 500);
		}

		numValidClients++;

		netprofile->rate += client->netprofile.rate;
		netprofile->outPackets.totalSize += client->netprofile.outPackets.totalSize;
		netprofile->outPackets.bytesPerSec += client->netprofile.outPackets.bytesPerSec;
		netprofile->outPackets.numDropped += client->netprofile.outPackets.numDropped;
		netprofile->outPackets.percentDropped += client->netprofile.outPackets.percentDropped;
		netprofile->outPackets.percentFragmented += client->netprofile.outPackets.percentFragmented;
		netprofile->outPackets.numFragmented += client->netprofile.outPackets.numFragmented;
		netprofile->outPackets.totalLengthConnectionLess += client->netprofile.outPackets.totalLengthConnectionLess;
		netprofile->outPackets.percentConnectionLess += client->netprofile.outPackets.percentConnectionLess;
		netprofile->outPackets.totalProcessed += client->netprofile.outPackets.totalProcessed;
		netprofile->outPackets.packetsPerSec += client->netprofile.outPackets.packetsPerSec;
		netprofile->inPackets.totalSize += client->netprofile.inPackets.totalSize;
		netprofile->inPackets.bytesPerSec += client->netprofile.inPackets.bytesPerSec;
		netprofile->inPackets.numDropped += client->netprofile.inPackets.numDropped;
		netprofile->inPackets.percentDropped += client->netprofile.inPackets.percentDropped;
		netprofile->inPackets.percentFragmented += client->netprofile.inPackets.percentFragmented;
		netprofile->inPackets.numFragmented += client->netprofile.inPackets.numFragmented;
		netprofile->inPackets.totalLengthConnectionLess += client->netprofile.inPackets.totalLengthConnectionLess;
		netprofile->inPackets.percentConnectionLess += client->netprofile.inPackets.percentConnectionLess;
		netprofile->inPackets.totalProcessed += client->netprofile.inPackets.totalProcessed;
		netprofile->inPackets.packetsPerSec += client->netprofile.inPackets.packetsPerSec;
	}

	if (numValidClients && netprofile->rate) {
		netprofile->rate /= numValidClients;
	}

    netprofile->outPackets.totalSize += svs.netprofile.outPackets.totalSize;
    netprofile->outPackets.bytesPerSec += svs.netprofile.outPackets.bytesPerSec;
    netprofile->outPackets.numDropped += svs.netprofile.outPackets.numDropped;
    netprofile->outPackets.percentDropped += svs.netprofile.outPackets.percentDropped;
    netprofile->outPackets.percentFragmented += svs.netprofile.outPackets.percentFragmented;
    netprofile->outPackets.numFragmented += svs.netprofile.outPackets.numFragmented;
    netprofile->outPackets.totalLengthConnectionLess += svs.netprofile.outPackets.totalLengthConnectionLess;
    netprofile->outPackets.percentConnectionLess += svs.netprofile.outPackets.percentConnectionLess;
    netprofile->outPackets.totalProcessed += svs.netprofile.outPackets.totalProcessed;
    netprofile->outPackets.packetsPerSec += svs.netprofile.outPackets.packetsPerSec;
    netprofile->inPackets.totalSize += svs.netprofile.inPackets.totalSize;
    netprofile->inPackets.bytesPerSec += svs.netprofile.inPackets.bytesPerSec;
    netprofile->inPackets.numDropped += svs.netprofile.inPackets.numDropped;
    netprofile->inPackets.percentDropped += svs.netprofile.inPackets.percentDropped;
    netprofile->inPackets.percentFragmented += svs.netprofile.inPackets.percentFragmented;
    netprofile->inPackets.numFragmented += svs.netprofile.inPackets.numFragmented;
    netprofile->inPackets.totalLengthConnectionLess += svs.netprofile.inPackets.totalLengthConnectionLess;
    netprofile->inPackets.percentConnectionLess += svs.netprofile.inPackets.percentConnectionLess;
    netprofile->inPackets.totalProcessed += svs.netprofile.inPackets.totalProcessed;
    netprofile->inPackets.packetsPerSec += svs.netprofile.inPackets.packetsPerSec;
}

