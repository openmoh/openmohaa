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

#include "q_shared.h"
#include "qcommon.h"

/*

packet header
-------------
4	outgoing sequence.  high bit will be set if this is a fragmented message
[2	qport (only for client to server)]
[2	fragment start byte]
[2	fragment length. if < FRAGMENT_SIZE, this is the last fragment]

if the sequence number is -1, the packet should be handled as an out-of-band
message instead of as part of a netcon.

All fragments will have the same sequence numbers.

The qport field is a workaround for bad address translating routers that
sometimes remap the client's source port on a packet during gameplay.

If the base part of the net address matches and the qport matches, then the
channel matches even if the IP port differs.  The IP port should be updated
to the new value before sending out any replies.

*/

#define	MAX_PACKETLEN					10000		// max size of a network packet
#define	MAX_REMOTE_PACKETLEN			1400		// max size of a network packet

#define FRAGMENT_SIZE			(((chan->remoteAddress.type == NA_LOOPBACK) ? MAX_PACKETLEN : MAX_REMOTE_PACKETLEN) - 100)
#define	PACKET_HEADER			10			// two ints and a short

#define	FRAGMENT_BIT	(1<<31)

cvar_t		*showpackets;
cvar_t		*showdrop;
cvar_t		*qport;

static char *netsrcString[2] = {
	"client",
	"server"
};

/*
===============
Netchan_Init

===============
*/
void Netchan_Init( int port ) {
	port &= 0xffff;
	showpackets = Cvar_Get ("showpackets", "0", CVAR_TEMP );
	showdrop = Cvar_Get ("showdrop", "0", CVAR_TEMP );
	qport = Cvar_Get ("net_qport", va("%i", port), CVAR_INIT );
}

/*
==============
Netchan_Setup

called to open a channel to a remote system
==============
*/
void Netchan_Setup(netsrc_t sock, netchan_t *chan, netadr_t adr, int qport, int challenge, qboolean compat)
{
	Com_Memset (chan, 0, sizeof(*chan));
	
	chan->sock = sock;
	chan->remoteAddress = adr;
	chan->qport = qport;
	chan->incomingSequence = 0;
	chan->outgoingSequence = 1;
	chan->challenge = challenge;

#ifdef LEGACY_PROTOCOL
	chan->compat = compat;
#endif
}

/*
==============
NetProfileAddPacket
==============
*/
void NetProfileAddPacket(netprofpacketlist_t* list, size_t size, int type)
{
    list->index = (list->index + 1) % 64;
    list->packets[list->index].updateTime = Com_Milliseconds();
    list->packets[list->index].size = size;
    list->packets[list->index].flags = type;
}

/*
==============
NetProfileSetPacketFlags
==============
*/
void NetProfileSetPacketFlags(netprofpacketlist_t* list, int flags)
{
	list->packets[list->index].flags |= flags;
}

/*
==============
NetProfileCalcStats
==============
*/
void NetProfileCalcStats(netprofpacketlist_t* list, int time)
{
    netprofpacket_t* packet;
    int i;
    float frequency;

	if (list->updateTime < time + list->lastCalcTime) {
		return;
	}

    list->lastCalcTime = list->updateTime;
    list->lowestUpdateTime = list->updateTime;
    list->highestUpdateTime = 0;
    list->totalProcessed = 0;
    list->totalSize = 0;
    list->numFragmented = 0;
    list->numDropped = 0;
    list->numConnectionLess = 0;
    list->totalLengthConnectionLess = 0;

	for (i = 0; i < ARRAY_LEN(list->packets); i++) {
		packet = &list->packets[i];
		if (!packet->size) {
			continue;
		}

		if (list->updateTime > packet->updateTime + 1000) {
			packet->size = 0;
			continue;
		}

		if (list->lowestUpdateTime > packet->updateTime) {
			list->lowestUpdateTime = packet->updateTime;
		}
		if (list->highestUpdateTime < packet->updateTime) {
			list->highestUpdateTime = packet->updateTime;
		}

		list->totalProcessed++;
		list->totalSize += packet->size;

		if (packet->flags & NETPROF_PACKET_FRAGMENTED) {
			list->numFragmented++;
		}
		if (packet->flags & NETPROF_PACKET_DROPPED) {
			list->numDropped++;
		}
		if (packet->flags & NETPROF_PACKET_MESSAGE) {
			list->numConnectionLess++;
			list->totalLengthConnectionLess += packet->size;
		}
	}

    if (!list->totalProcessed) {
        list->packetsPerSec = 0;
        list->highestUpdateTime = list->lowestUpdateTime;
        list->bytesPerSec = 0;
        list->percentFragmented = 0;
        list->percentDropped = 0;
        list->percentConnectionLess = 0;

		return;
	}

	if (list->lowestUpdateTime == list->highestUpdateTime) {
		list->latency = 0.0;
		frequency = 1.0;
	} else {
		list->latency = (list->highestUpdateTime - list->lowestUpdateTime) / 1000.0;
		frequency = 1.0 / list->latency;
	}

	list->packetsPerSec = list->totalProcessed * frequency;
	list->bytesPerSec = list->totalSize * frequency;

	if (list->numFragmented) {
		list->percentFragmented = (float)list->numFragmented / list->totalProcessed * 100.0;
	} else {
		list->percentFragmented = 0;
	}

	if (list->numDropped) {
		list->percentDropped = (float)list->numDropped / list->totalProcessed * 100.0;
	} else {
		list->percentDropped = 0;
	}

	if (list->totalLengthConnectionLess) {
		list->percentConnectionLess = (float)list->totalLengthConnectionLess / list->totalSize * 100.0;
	}
}

// TTimo: unused, commenting out to make gcc happy
#if 0
/*
==============
Netchan_ScramblePacket

A probably futile attempt to make proxy hacking somewhat
more difficult.
==============
*/
#define	SCRAMBLE_START	6
static void Netchan_ScramblePacket( msg_t *buf ) {
	unsigned	seed;
	int			i, j, c, mask, temp;
	int			seq[MAX_PACKETLEN];

	seed = ( LittleLong( *(unsigned *)buf->data ) * 3 ) ^ ( buf->cursize * 123 );
	c = buf->cursize;
	if ( c <= SCRAMBLE_START ) {
		return;
	}
	if ( c > MAX_PACKETLEN ) {
		Com_Error( ERR_DROP, "MAX_PACKETLEN" );
	}

	// generate a sequence of "random" numbers
	for (i = 0 ; i < c ; i++) {
		seed = (119 * seed + 1);
		seq[i] = seed;
	}

	// transpose each character
	for ( mask = 1 ; mask < c-SCRAMBLE_START ; mask = ( mask << 1 ) + 1 ) {
	}
	mask >>= 1;
	for (i = SCRAMBLE_START ; i < c ; i++) {
		j = SCRAMBLE_START + ( seq[i] & mask );
		temp = buf->data[j];
		buf->data[j] = buf->data[i];
		buf->data[i] = temp;
	}

	// byte xor the data after the header
	for (i = SCRAMBLE_START ; i < c ; i++) {
		buf->data[i] ^= seq[i];
	}
}

static void Netchan_UnScramblePacket( msg_t *buf ) {
	unsigned	seed;
	int			i, j, c, mask, temp;
	int			seq[MAX_PACKETLEN];

	seed = ( LittleLong( *(unsigned *)buf->data ) * 3 ) ^ ( buf->cursize * 123 );
	c = buf->cursize;
	if ( c <= SCRAMBLE_START ) {
		return;
	}
	if ( c > MAX_PACKETLEN ) {
		Com_Error( ERR_DROP, "MAX_PACKETLEN" );
	}

	// generate a sequence of "random" numbers
	for (i = 0 ; i < c ; i++) {
		seed = (119 * seed + 1);
		seq[i] = seed;
	}

	// byte xor the data after the header
	for (i = SCRAMBLE_START ; i < c ; i++) {
		buf->data[i] ^= seq[i];
	}

	// transpose each character in reverse order
	for ( mask = 1 ; mask < c-SCRAMBLE_START ; mask = ( mask << 1 ) + 1 ) {
	}
	mask >>= 1;
	for (i = c-1 ; i >= SCRAMBLE_START ; i--) {
		j = SCRAMBLE_START + ( seq[i] & mask );
		temp = buf->data[j];
		buf->data[j] = buf->data[i];
		buf->data[i] = temp;
	}
}
#endif

/*
=================
Netchan_TransmitNextFragment

Send one fragment of the current message
=================
*/
void Netchan_TransmitNextFragment( netchan_t *chan, netprofpacketlist_t *packetlist ) {
	msg_t		send;
	byte		send_buf[MAX_PACKETLEN];
	size_t		fragmentLength;

	// write the packet header
	MSG_InitOOB (&send, send_buf, chan->remoteAddress.type == NA_LOOPBACK ? MAX_PACKETLEN : MAX_REMOTE_PACKETLEN); // <-- only do the oob here

	MSG_WriteLong( &send, chan->outgoingSequence | FRAGMENT_BIT );

	// send the qport if we are a client
	if ( chan->sock == NS_CLIENT ) {
		MSG_WriteShort( &send, qport->integer );
    }

#ifdef LEGACY_PROTOCOL
    if (!chan->compat)
#endif
        MSG_WriteLong(&send, NETCHAN_GENCHECKSUM(chan->challenge, chan->outgoingSequence));

	// copy the reliable message to the packet first
	fragmentLength = FRAGMENT_SIZE;
	if ( chan->unsentFragmentStart  + fragmentLength > chan->unsentLength ) {
		fragmentLength = chan->unsentLength - chan->unsentFragmentStart;
	}

	MSG_WriteLong( &send, (int)chan->unsentFragmentStart );
	MSG_WriteShort( &send, (short)fragmentLength );
	MSG_WriteData( &send, chan->unsentBuffer + chan->unsentFragmentStart, fragmentLength );

	// send the datagram
	NET_SendPacket( chan->sock, send.cursize, send.data, chan->remoteAddress );
	
	// Store send time and size of this packet for rate control
	chan->lastSentTime = Sys_Milliseconds();
	chan->lastSentSize = send.cursize;

	if ( showpackets->integer ) {
		Com_Printf ("%s send %4zu : s=%i fragment=%zu,%zu\n"
			, netsrcString[ chan->sock ]
			, send.cursize
			, chan->outgoingSequence
			, chan->unsentFragmentStart, fragmentLength);
	}

	if (packetlist) {
		NetProfileAddPacket(packetlist, send.cursize, NETPROF_PACKET_FRAGMENTED);
	}

	chan->unsentFragmentStart += fragmentLength;

	// this exit condition is a little tricky, because a packet
	// that is exactly the fragment length still needs to send
	// a second packet of zero length so that the other side
	// can tell there aren't more to follow
	if ( chan->unsentFragmentStart == chan->unsentLength && fragmentLength != FRAGMENT_SIZE ) {
		chan->outgoingSequence++;
		chan->unsentFragments = qfalse;
	}
}


/*
===============
Netchan_Transmit

Sends a message to a connection, fragmenting if necessary
A 0 length will still generate a packet.
================
*/
void Netchan_Transmit( netchan_t *chan, size_t length, const byte *data, netprofpacketlist_t *packetlist ) {
	msg_t			send;
	byte			send_buf[MAX_PACKETLEN];

	if ( length > MAX_MSGLEN ) {
		Com_Error( ERR_DROP, "Netchan_Transmit: length = %zu", length );
	}
	chan->unsentFragmentStart = 0;

	// fragment large reliable messages
	if ( length >= FRAGMENT_SIZE ) {
		chan->unsentFragments = qtrue;
		chan->unsentLength = length;
		Com_Memcpy( chan->unsentBuffer, data, length );

		// only send the first fragment now
		Netchan_TransmitNextFragment( chan, packetlist );

		return;
	}

	// write the packet header
	MSG_InitOOB (&send, send_buf, chan->remoteAddress.type == NA_LOOPBACK ? MAX_PACKETLEN : MAX_REMOTE_PACKETLEN);

	MSG_WriteLong( &send, chan->outgoingSequence );

	// send the qport if we are a client
	if(chan->sock == NS_CLIENT)
		MSG_WriteShort(&send, qport->integer);

#ifdef LEGACY_PROTOCOL
	if(!chan->compat)
#endif
		MSG_WriteLong(&send, NETCHAN_GENCHECKSUM(chan->challenge, chan->outgoingSequence));

	chan->outgoingSequence++;

	MSG_WriteData( &send, data, length );

	// send the datagram
	NET_SendPacket( chan->sock, send.cursize, send.data, chan->remoteAddress );

	// Store send time and size of this packet for rate control
	chan->lastSentTime = Sys_Milliseconds();
	chan->lastSentSize = send.cursize;

	if ( showpackets->integer ) {
		Com_Printf( "%s send %4zu : s=%i ack=%i\n"
			, netsrcString[ chan->sock ]
			, send.cursize
			, chan->outgoingSequence - 1
			, chan->incomingSequence );
	}

	if (packetlist) {
		NetProfileAddPacket(packetlist, send.cursize, 0);
	}
}

/*
=================
Netchan_Process

Returns qfalse if the message should not be processed due to being
out of order or a fragment.

Msg must be large enough to hold MAX_MSGLEN, because if this is the
final fragment of a multi-part message, the entire thing will be
copied out.
=================
*/
qboolean Netchan_Process( netchan_t *chan, msg_t *msg, netprofpacketlist_t *packetlist ) {
	int			sequence;
	int			qport;
	int			fragmentStart, fragmentLength;
	qboolean	fragmented;

	// XOR unscramble all data in the packet after the header
//	Netchan_UnScramblePacket( msg );

	// get sequence numbers		
	MSG_BeginReadingOOB( msg );
	sequence = MSG_ReadLong( msg );

	// check for fragment information
	if ( sequence & FRAGMENT_BIT ) {
		sequence &= ~FRAGMENT_BIT;
		fragmented = qtrue;
	} else {
		fragmented = qfalse;
	}

	// read the qport if we are a server
	if ( chan->sock == NS_SERVER ) {
		qport = MSG_ReadShort( msg );
	}

#ifdef LEGACY_PROTOCOL
	if(!chan->compat)
#endif
	{
		int checksum = MSG_ReadLong(msg);

		// UDP spoofing protection
		if(NETCHAN_GENCHECKSUM(chan->challenge, sequence) != checksum)
			return qfalse;
	}

	// read the fragment information
	if ( fragmented ) {
		fragmentStart = MSG_ReadLong( msg );
		fragmentLength = MSG_ReadShort( msg );
	} else {
		fragmentStart = 0;		// stop warning message
		fragmentLength = 0;
	}

	if ( showpackets->integer ) {
		if ( fragmented ) {
			Com_Printf( "%s recv %4zu : s=%u fragment=%i,%i\n"
				, netsrcString[ chan->sock ]
				, msg->cursize
				, sequence
				, fragmentStart, fragmentLength );
		} else {
			Com_Printf( "%s recv %4zu : s=%i\n"
				, netsrcString[ chan->sock ]
				, msg->cursize
				, sequence );
		}
	}

	if (packetlist) {
		NetProfileAddPacket(packetlist, msg->cursize, fragmented ? NETPROF_PACKET_FRAGMENTED : 0);
	}

	//
	// discard out of order or duplicated packets
	//
	if ( sequence <= chan->incomingSequence ) {
		if ( showdrop->integer || showpackets->integer ) {
			Com_Printf( "%s:Out of order packet %i at %i\n"
				, NET_AdrToString( chan->remoteAddress )
				,  sequence
				, chan->incomingSequence );
		}
		return qfalse;
	}

	//
	// dropped packets don't keep the message from being used
	//
	chan->dropped = sequence - (chan->incomingSequence+1);
	if ( chan->dropped > 0 ) {
		if ( showdrop->integer || showpackets->integer ) {
			Com_Printf( "%s:Dropped %i packets at %i\n"
			, NET_AdrToString( chan->remoteAddress )
			, chan->dropped
			, sequence );
		}
	}
	

	//
	// if this is the final framgent of a reliable message,
	// bump incoming_reliable_sequence 
	//
	if ( fragmented ) {
		// TTimo
		// make sure we add the fragments in correct order
		// either a packet was dropped, or we received this one too soon
		// we don't reconstruct the fragments. we will wait till this fragment gets to us again
		// (NOTE: we could probably try to rebuild by out of order chunks if needed)
		if ( sequence != chan->fragmentSequence ) {
			chan->fragmentSequence = sequence;
			chan->fragmentLength = 0;
		}

		// if we missed a fragment, dump the message
		if ( fragmentStart != chan->fragmentLength ) {
			if ( showdrop->integer || showpackets->integer ) {
				Com_Printf( "%s:Dropped a message fragment\n"
				, NET_AdrToString( chan->remoteAddress ));
			}
			// we can still keep the part that we have so far,
			// so we don't need to clear chan->fragmentLength
			return qfalse;
		}

		// copy the fragment to the fragment buffer
		if ( fragmentLength < 0 || msg->readcount + fragmentLength > msg->cursize ||
			chan->fragmentLength + fragmentLength > sizeof( chan->fragmentBuffer ) ) {
			if ( showdrop->integer || showpackets->integer ) {
				Com_Printf ("%s:illegal fragment length\n"
				, NET_AdrToString (chan->remoteAddress ) );
			}
			return qfalse;
		}

		Com_Memcpy( chan->fragmentBuffer + chan->fragmentLength, 
			msg->data + msg->readcount, fragmentLength );

		chan->fragmentLength += fragmentLength;

		// if this wasn't the last fragment, don't process anything
		if ( fragmentLength == FRAGMENT_SIZE ) {
			return qfalse;
		}

		if ( chan->fragmentLength > msg->maxsize ) {
			Com_Printf( "%s:fragmentLength %i > msg->maxsize\n"
				, NET_AdrToString (chan->remoteAddress ),
				chan->fragmentLength );
			return qfalse;
		}

		// copy the full message over the partial fragment

		// make sure the sequence number is still there
		*(int *)msg->data = LittleLong( sequence );

		Com_Memcpy( msg->data + 4, chan->fragmentBuffer, chan->fragmentLength );
		msg->cursize = chan->fragmentLength + 4;
		chan->fragmentLength = 0;
		msg->readcount = 4;	// past the sequence number
		msg->bit = 32;	// past the sequence number

		// TTimo
		// clients were not acking fragmented messages
		chan->incomingSequence = sequence;
		
		return qtrue;
	}

	//
	// the message can now be read from the current message pointer
	//
	chan->incomingSequence = sequence;

	return qtrue;
}

/*
=============================================================================

LOOPBACK BUFFERS FOR LOCAL PLAYER

=============================================================================
*/

// there needs to be enough loopback messages to hold a complete
// gamestate of maximum size
#define	MAX_LOOPBACK	32

typedef struct {
	byte	data[MAX_PACKETLEN];
	size_t	datalen;
} loopmsg_t;

typedef struct {
	loopmsg_t	msgs[MAX_LOOPBACK];
	int			get, send;
} loopback_t;

loopback_t	loopbacks[2];


qboolean	NET_GetLoopPacket (netsrc_t sock, netadr_t *net_from, msg_t *net_message)
{
	int		i;
	loopback_t	*loop;

	loop = &loopbacks[sock];

	if (loop->send - loop->get > MAX_LOOPBACK)
		loop->get = loop->send - MAX_LOOPBACK;

	if (loop->get >= loop->send)
		return qfalse;

	i = loop->get & (MAX_LOOPBACK-1);
	loop->get++;

	Com_Memcpy (net_message->data, loop->msgs[i].data, loop->msgs[i].datalen);
	net_message->cursize = loop->msgs[i].datalen;
	Com_Memset (net_from, 0, sizeof(*net_from));
	net_from->type = NA_LOOPBACK;
	return qtrue;

}


void NET_SendLoopPacket (netsrc_t sock, size_t length, const void *data, netadr_t to)
{
	int		i;
	loopback_t	*loop;

	loop = &loopbacks[sock^1];

	i = loop->send & (MAX_LOOPBACK-1);
	loop->send++;

	Com_Memcpy (loop->msgs[i].data, data, length);
	loop->msgs[i].datalen = length;
}

//=============================================================================

typedef struct packetQueue_s {
        struct packetQueue_s *next;
		size_t length;
        byte *data;
        netadr_t to;
        int release;
} packetQueue_t;

packetQueue_t *packetQueue = NULL;

static void NET_QueuePacket( size_t length, const void *data, netadr_t to,
	int offset )
{
	packetQueue_t *new, *next = packetQueue;

	if(offset > 999)
		offset = 999;

	new = Z_Malloc( sizeof( packetQueue_t ) );
	new->data = Z_Malloc( length );
	Com_Memcpy(new->data, data, length);
	new->length = length;
	new->to = to;
	new->release = Sys_Milliseconds() + offset;	
	new->next = NULL;

	if(!packetQueue) {
		packetQueue = new;
		return;
	}
	while(next) {
		if(!next->next) {
			next->next = new;
			return;
		}
		next = next->next;
	}
}

void NET_FlushPacketQueue(void)
{
	packetQueue_t *last;
	int now;

	while(packetQueue) {
		now = Sys_Milliseconds();
		if(packetQueue->release >= now)
			break;
		Sys_SendPacket(packetQueue->length, packetQueue->data,
			packetQueue->to);
		last = packetQueue;
		packetQueue = packetQueue->next;
		Z_Free(last->data);
		Z_Free(last);
	}
}

void NET_SendPacket( netsrc_t sock, size_t length, const void *data, netadr_t to ) {

	// sequenced packets are shown in netchan, so just show oob
	if ( showpackets->integer && *(int *)data == -1 )	{
		Com_Printf( "%s send OOB packet %4zu\n", netsrcString[ sock ], length );
	}

	if ( to.type == NA_LOOPBACK ) {
		NET_SendLoopPacket (sock, length, data, to);
		return;
	}
	if ( to.type == NA_BOT ) {
		return;
	}
	if ( to.type == NA_BAD ) {
		return;
	}

	if( sock == NS_CLIENT && cl_packetdelay->integer > 0 ) {
		NET_QueuePacket( length, data, to, cl_packetdelay->integer );
	} else if( sock == NS_SERVER && sv_packetdelay->integer > 0 ) {
		NET_QueuePacket( length, data, to, sv_packetdelay->integer );
	} else {
		Sys_SendPacket( length, data, to );
	}
}

/*
===============
NET_OutOfBandPrint

Sends a text message in an out-of-band datagram
================
*/
void QDECL NET_OutOfBandPrint( netsrc_t sock, netadr_t adr, const char *format, ... ) {
	va_list		argptr;
	char		string[MAX_MSGLEN];


	// set the header
	string[0] = -1;
	string[1] = -1;
	string[2] = -1;
	string[3] = -1;

    // wombat: MOHAA OOB packets carry another byte to indicate the direction
    if (sock == NS_SERVER) {
        string[4] = 1;
    } else {
        string[4] = 2;
    }

	va_start( argptr, format );
	Q_vsnprintf( string+5, sizeof(string)-5, format, argptr );
	va_end( argptr );

	// send the datagram
	NET_SendPacket( sock, strlen( string ), string, adr );
}

/*
===============
NET_OutOfBandPrint

Sends a data message in an out-of-band datagram (only used for "connect")
================
*/
void QDECL NET_OutOfBandData( netsrc_t sock, netadr_t adr, byte *format, int len ) {
	byte		string[MAX_MSGLEN*2];
	int			i;
	msg_t		mbuf;

	// set the header
	string[0] = 0xff;
	string[1] = 0xff;
	string[2] = 0xff;
	string[3] = 0xff;

    // wombat: MOHAA OOB packets carry another byte to indicate the direction
    if (sock == NS_SERVER) {
        string[4] = 1;
	} else {
        string[4] = 2;
	}

	for(i=0;i<len;i++) {
		string[i+(4+1)] = format[i];
	}

	mbuf.data = string;
	mbuf.cursize = len+(4+1);
	Huff_Compress( &mbuf, (12+1));
	// send the datagram
	NET_SendPacket( sock, mbuf.cursize, mbuf.data, adr );
}

/*
=============
NET_StringToAdr

Traps "localhost" for loopback, passes everything else to system
=============
*/
int NET_StringToAdr( const char *s, netadr_t *a, netadrtype_t family )
{
	char	base[MAX_STRING_CHARS], *search;
	char	*port = NULL;

	if (!strcmp (s, "localhost")) {
		Com_Memset (a, 0, sizeof(*a));
		a->type = NA_LOOPBACK;
// as NA_LOOPBACK doesn't require ports report port was given.
		return 1;
	}

	Q_strncpyz( base, s, sizeof( base ) );
	
	if(*base == '[' || Q_CountChar(base, ':') > 1)
	{
		// This is an ipv6 address, handle it specially.
		search = strchr(base, ']');
		if(search)
		{
			*search = '\0';
			search++;

			if(*search == ':')
				port = search + 1;
		}
		
		if(*base == '[')
			search = base + 1;
		else
			search = base;
	}
	else
	{
		// look for a port number
		port = strchr( base, ':' );
		
		if ( port ) {
			*port = '\0';
			port++;
		}
		
		search = base;
	}

	if(!Sys_StringToAdr(search, a, family))
	{
		a->type = NA_BAD;
		return 0;
	}

	if(port)
	{
		a->port = BigShort((short) atoi(port));
		return 1;
	}
	else
	{
		a->port = BigShort(PORT_SERVER);
		return 2;
	}
}

