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

static huffman_t		msgHuff;

static qboolean			msgInit = qfalse;

int pcount[256];

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

int oldsize = 0;

void MSG_initHuffman( void );

void MSG_Init( msg_t *buf, byte *data, size_t length ) {
	if (!msgInit) {
		MSG_initHuffman();
	}
	Com_Memset (buf, 0, sizeof(*buf));
	buf->data = data;
	buf->maxsize = length;
}

void MSG_InitOOB( msg_t *buf, byte *data, size_t length ) {
	if (!msgInit) {
		MSG_initHuffman();
	}
	Com_Memset (buf, 0, sizeof(*buf));
	buf->data = data;
	buf->maxsize = length;
	buf->oob = qtrue;
}

void MSG_Clear( msg_t *buf ) {
	buf->cursize = 0;
	buf->overflowed = qfalse;
	buf->bit = 0;					//<- in bits
}


void MSG_Bitstream( msg_t *buf ) {
	buf->oob = qfalse;
}

void MSG_BeginReading( msg_t *msg ) {
	msg->readcount = 0;
	msg->bit = 0;
	msg->oob = qfalse;
}

void MSG_BeginReadingOOB( msg_t *msg ) {
	msg->readcount = 0;
	msg->bit = 0;
	msg->oob = qtrue;
}

void MSG_Copy(msg_t *buf, byte *data, int length, msg_t *src)
{
	if (length<src->cursize) {
		Com_Error( ERR_DROP, "MSG_Copy: can't copy into a smaller msg_t buffer");
	}
	Com_Memcpy(buf, src, sizeof(msg_t));
	buf->data = data;
	Com_Memcpy(buf->data, src->data, src->cursize);
}

/*
=============================================================================

bit functions
  
=============================================================================
*/

int	overflows;

// negative bit values include signs
void MSG_WriteBits( msg_t *msg, int value, int bits ) {
	int	i;
//	FILE*	fp;

	oldsize += bits;

	// this isn't an exact overflow check, but close enough
	if ( msg->maxsize - msg->cursize < 4 ) {
		msg->overflowed = qtrue;
		return;
	}

	if ( bits == 0 || bits < -31 || bits > 32 ) {
		Com_Error( ERR_DROP, "MSG_WriteBits: bad bits %i", bits );
	}

	// check for overflows
	if ( bits != 32 ) {
		if ( bits > 0 ) {
			if ( value > ( ( 1 << bits ) - 1 ) || value < 0 ) {
				overflows++;
			}
		} else {
			int	r;

			r = 1 << (bits-1);

			if ( value >  r - 1 || value < -r ) {
				overflows++;
			}
		}
	}
	if ( bits < 0 ) {
		bits = -bits;
	}
	if (msg->oob) {
		if (bits==8) {
			msg->data[msg->cursize] = value;
			msg->cursize += 1;
			msg->bit += 8;
		} else if (bits==16) {
			unsigned short *sp = (unsigned short *)&msg->data[msg->cursize];
			*sp = LittleShort(value);
			msg->cursize += 2;
			msg->bit += 16;
		} else if (bits==32) {
			unsigned int *ip = (unsigned int *)&msg->data[msg->cursize];
			*ip = LittleLong(value);
			msg->cursize += 4;
			msg->bit += 32;
		} else {
			Com_Error(ERR_DROP, "can't read %d bits\n", bits);
		}
	} else {
//		fp = fopen("c:\\netchan.bin", "a");
		value &= (0xffffffff>>(32-bits));
		if (bits&7) {
			int nbits;
			nbits = bits&7;
			for(i=0;i<nbits;i++) {
				Huff_putBit((value&1), msg->data, &msg->bit);
				value = (value>>1);
			}
			bits = bits - nbits;
		}
		if (bits) {
			for(i=0;i<bits;i+=8) {
//				fwrite(bp, 1, 1, fp);
				Huff_offsetTransmit (&msgHuff.compressor, (value&0xff), msg->data, &msg->bit);
				value = (value>>8);
			}
		}
		msg->cursize = (msg->bit>>3)+1;
//		fclose(fp);
	}
}

int MSG_ReadBits( msg_t *msg, int bits ) {
	int			value;
	int			get;
	qboolean	sgn;
	int			i, nbits;
//	FILE*	fp;

	value = 0;

	if ( bits < 0 ) {
		bits = -bits;
		sgn = qtrue;
	} else {
		sgn = qfalse;
	}

	if (msg->oob) {
		if (bits==8) {
			value = msg->data[msg->readcount];
			msg->readcount += 1;
			msg->bit += 8;
		} else if (bits==16) {
			unsigned short *sp = (unsigned short *)&msg->data[msg->readcount];
			value = LittleShort(*sp);
			msg->readcount += 2;
			msg->bit += 16;
		} else if (bits==32) {
			unsigned int *ip = (unsigned int *)&msg->data[msg->readcount];
			value = LittleLong(*ip);
			msg->readcount += 4;
			msg->bit += 32;
		} else {
			Com_Error(ERR_DROP, "can't read %d bits\n", bits);
		}
	} else {
		nbits = 0;
		if (bits&7) {
			nbits = bits&7;
			for(i=0;i<nbits;i++) {
				value |= (Huff_getBit(msg->data, &msg->bit)<<i);
			}
			bits = bits - nbits;
		}
		if (bits) {
//			fp = fopen("c:\\netchan.bin", "a");
			for(i=0;i<bits;i+=8) {
				Huff_offsetReceive (msgHuff.decompressor.tree, &get, msg->data, &msg->bit);
//				fwrite(&get, 1, 1, fp);
				value |= (get<<(i+nbits));
			}
//			fclose(fp);
		}
		msg->readcount = (msg->bit>>3)+1;
	}
	if ( sgn ) {
		if ( value & ( 1 << ( bits - 1 ) ) ) {
			value |= -1 ^ ( ( 1 << bits ) - 1 );
		}
	}

	return value;
}



//================================================================================

//
// writing functions
//

void MSG_WriteChar( msg_t *sb, int c ) {
#ifdef PARANOID
	if (c < -128 || c > 127)
		Com_Error (ERR_FATAL, "MSG_WriteChar: range error");
#endif

	MSG_WriteBits( sb, c, 8 );
}

void MSG_WriteByte( msg_t *sb, int c ) {
#ifdef PARANOID
	if (c < 0 || c > 255)
		Com_Error (ERR_FATAL, "MSG_WriteByte: range error");
#endif

	MSG_WriteBits( sb, c, 8 );
}

void MSG_WriteData( msg_t *buf, const void *data, size_t length ) {
	int i;
	for(i=0;i<length;i++) {
		MSG_WriteByte(buf, ((byte *)data)[i]);
	}
}

void MSG_WriteShort( msg_t *sb, int c ) {
#ifdef PARANOID
	if (c < ((short)0x8000) || c > (short)0x7fff)
		Com_Error (ERR_FATAL, "MSG_WriteShort: range error");
#endif

	MSG_WriteBits( sb, c, 16 );
}

void MSG_WriteSVC( msg_t *sb, int c ) {
	MSG_WriteBits( sb, c, 8 );
}

void MSG_WriteLong( msg_t *sb, int c ) {
	MSG_WriteBits( sb, c, 32 );
}

void MSG_WriteFloat( msg_t *sb, float f ) {
	union {
		float	f;
		int	l;
	} dat;
	
	dat.f = f;
	MSG_WriteBits( sb, dat.l, 32 );
}

void MSG_WriteString( msg_t *sb, const char *s ) {
	if ( !s ) {
		MSG_WriteData (sb, "", 1);
	} else {
		size_t	l;
		int		i;
		char	string[MAX_STRING_CHARS];

		l = strlen( s );
		if ( l >= MAX_STRING_CHARS ) {
			Com_Printf( "MSG_WriteString: MAX_STRING_CHARS" );
			MSG_WriteData (sb, "", 1);
			return;
		}
		Q_strncpyz( string, s, sizeof( string ) );

		// get rid of 0xff chars, because old clients don't like them
		for ( i = 0 ; i < l ; i++ ) {
			if ( ((byte *)string)[i] > 127 ) {
				string[i] = '.';
			}
		}

		MSG_WriteData (sb, string, l+1);
	}
}

void MSG_WriteBigString( msg_t *sb, const char *s ) {
	if ( !s ) {
		MSG_WriteData (sb, "", 1);
	} else {
		size_t	l;
		int		i;
		char	string[BIG_INFO_STRING];

		l = strlen( s );
		if ( l >= BIG_INFO_STRING ) {
			Com_Printf( "MSG_WriteString: BIG_INFO_STRING" );
			MSG_WriteData (sb, "", 1);
			return;
		}
		Q_strncpyz( string, s, sizeof( string ) );

		// get rid of 0xff chars, because old clients don't like them
		for ( i = 0 ; i < l ; i++ ) {
			if ( ((byte *)string)[i] > 127 ) {
				string[i] = '.';
			}
		}

		MSG_WriteData (sb, string, l+1);
	}
}

void MSG_WriteAngle( msg_t *sb, float f ) {
	MSG_WriteByte (sb, (int)(f*256/360) & 255);
}

void MSG_WriteAngle16( msg_t *sb, float f ) {
	MSG_WriteShort (sb, ANGLE2SHORT(f));
}


//============================================================

//
// reading functions
//

// returns -1 if no more characters are available
int MSG_ReadChar (msg_t *msg ) {
	int	c;
	
	c = (signed char)MSG_ReadBits( msg, 8 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}	
	
	return c;
}

int MSG_ReadByte( msg_t *msg ) {
	int	c;
	
	c = (unsigned char)MSG_ReadBits( msg, 8 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}	
	return c;
}

int MSG_ReadSVC( msg_t *msg ) {
	int	c;
	
	c = (unsigned char)MSG_ReadBits( msg, 8 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}	
	return c;
}

int MSG_ReadShort( msg_t *msg ) {
	int	c;
	
	c = (short)MSG_ReadBits( msg, 16 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}	

	return c;
}

int MSG_ReadLong( msg_t *msg ) {
	int	c;
	
	c = MSG_ReadBits( msg, 32 );
	if ( msg->readcount > msg->cursize ) {
		c = -1;
	}	
	
	return c;
}

void MSG_ReadDir( msg_t *msg, vec3_t dir ) {
	int b;

	b = MSG_ReadByte(msg);
	ByteToDir( b, dir );
}

float MSG_ReadCoord( msg_t *msg ) {
	float	sign = 1.0f;
	int		read;
	float	rtn;

	read = MSG_ReadBits( msg, 19 );
	if ( read & 262144 )	// the 19th bit is the sign
		sign = -1.0f;
	read &= ~262144; //  uint=4294705151
	rtn =  sign * read /16.0f;
	
	return rtn;	
}

float MSG_ReadFloat( msg_t *msg ) {
	union {
		byte	b[4];
		float	f;
		int	l;
	} dat;
	
	dat.l = MSG_ReadBits( msg, 32 );
	if ( msg->readcount > msg->cursize ) {
		dat.f = -1;
	}	
	
	return dat.f;	
}

char *MSG_ReadString( msg_t *msg ) {
	static char	string[MAX_STRING_CHARS];
	int		l,c;
	
	l = 0;
	do {
		c = MSG_ReadByte(msg);		// use ReadByte so -1 is out of bounds
		if ( c <= 0 ) {
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if ( c == '%' ) {
			c = '.';
		}
		// don't allow higher ascii values
		// (su44: this check is missing in MoHAA)
		if ( c > 127 ) {
			c = '.';
		}

		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

char *MSG_ReadBigString( msg_t *msg ) {
	static char	string[BIG_INFO_STRING];
	int		l,c;
	
	l = 0;
	do {
		c = MSG_ReadByte(msg);		// use ReadByte so -1 is out of bounds
		if ( c == -1 || c == 0 ) {
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if ( c == '%' ) {
			c = '.';
		}
		// don't allow higher ascii values
		if ( c > 127 ) {
			c = '.';
		}

		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

char *MSG_ReadStringLine( msg_t *msg ) {
	static char	string[MAX_STRING_CHARS];
	int		l,c;

	l = 0;
	do {
		c = MSG_ReadByte(msg);		// use ReadByte so -1 is out of bounds
		if (c <= 0 || c == '\n') {
		//if (c == -1 || c == 0 || c == '\n') { //Q3
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if ( c == '%' ) {
			c = '.';
		}
		// don't allow higher ascii values
		if ( c > 127 ) {
			c = '.';
		}

		string[l] = c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

void MSG_GetNullEntityState(entityState_t *nullState) {

	Com_Memset( nullState, 0, sizeof(entityState_t) );
	nullState->alpha = 1.0f;
	nullState->scale = 1.0f;
	nullState->parent = ENTITYNUM_NONE;
	nullState->tag_num = -1; // su44
	nullState->constantLight = -1;
	nullState->renderfx = 16;
	nullState->bone_tag[4] = -1;
	nullState->bone_tag[3] = -1;
	nullState->bone_tag[2] = -1;
	nullState->bone_tag[1] = -1;
	nullState->bone_tag[0] = -1;
}

float MSG_ReadAngle8( msg_t *msg ) {
	return BYTE2ANGLE(MSG_ReadShort(msg));
}

float MSG_ReadAngle16( msg_t *msg ) {
	return SHORT2ANGLE(MSG_ReadShort(msg));
}

void MSG_ReadData( msg_t *msg, void *data, int len ) {
	int		i;

	for (i=0 ; i<len ; i++) {
		((byte *)data)[i] = MSG_ReadByte (msg);
	}
}


/*
=============================================================================

delta functions
  
=============================================================================
*/

extern cvar_t *cl_shownet;

#define	LOG(x) if( cl_shownet->integer == 4 ) { Com_Printf("%s ", x ); };

void MSG_WriteDelta( msg_t *msg, int oldV, int newV, int bits ) {
	if ( oldV == newV ) {
		MSG_WriteBits( msg, 0, 1 );
		return;
	}
	MSG_WriteBits( msg, 1, 1 );
	MSG_WriteBits( msg, newV, bits );
}

int	MSG_ReadDelta( msg_t *msg, int oldV, int bits ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		return MSG_ReadBits( msg, bits );
	}
	return oldV;
}

void MSG_WriteDeltaFloat( msg_t *msg, float oldV, float newV ) {
	if ( oldV == newV ) {
		MSG_WriteBits( msg, 0, 1 );
		return;
	}
	MSG_WriteBits( msg, 1, 1 );
	MSG_WriteBits( msg, *(int *)&newV, 32 );
}

float MSG_ReadDeltaFloat( msg_t *msg, float oldV ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		float	newV;

		*(int *)&newV = MSG_ReadBits( msg, 32 );
		return newV;
	}
	return oldV;
}

/*
=============================================================================

delta functions with keys
  
=============================================================================
*/

int kbitmask[32] = {
	0x00000001, 0x00000003, 0x00000007, 0x0000000F,
	0x0000001F,	0x0000003F,	0x0000007F,	0x000000FF,
	0x000001FF,	0x000003FF,	0x000007FF,	0x00000FFF,
	0x00001FFF,	0x00003FFF,	0x00007FFF,	0x0000FFFF,
	0x0001FFFF,	0x0003FFFF,	0x0007FFFF,	0x000FFFFF,
	0x001FFFFf,	0x003FFFFF,	0x007FFFFF,	0x00FFFFFF,
	0x01FFFFFF,	0x03FFFFFF,	0x07FFFFFF,	0x0FFFFFFF,
	0x1FFFFFFF,	0x3FFFFFFF,	0x7FFFFFFF,	0xFFFFFFFF,
};

void MSG_WriteDeltaKey( msg_t *msg, int key, int oldV, int newV, int bits ) {
	if ( oldV == newV ) {
		MSG_WriteBits( msg, 0, 1 );
		return;
	}
	MSG_WriteBits( msg, 1, 1 );
	MSG_WriteBits( msg, newV ^ key, bits );
}

int	MSG_ReadDeltaKey( msg_t *msg, int key, int oldV, int bits ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		return MSG_ReadBits( msg, bits ) ^ (key & kbitmask[bits]);
	}
	return oldV;
}

void MSG_WriteDeltaKeyFloat( msg_t *msg, int key, float oldV, float newV ) {
	if ( oldV == newV ) {
		MSG_WriteBits( msg, 0, 1 );
		return;
	}
	MSG_WriteBits( msg, 1, 1 );
	MSG_WriteBits( msg, (*(int *)&newV) ^ key, 32 );
}

float MSG_ReadDeltaKeyFloat( msg_t *msg, int key, float oldV ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		float	newV;

		*(int *)&newV = MSG_ReadBits( msg, 32 ) ^ key;
		return newV;
	}
	return oldV;
}


/*
============================================================================

usercmd_t communication

============================================================================
*/

// ms is allways sent, the others are optional
#define	CM_ANGLE1 	(1<<0)
#define	CM_ANGLE2 	(1<<1)
#define	CM_ANGLE3 	(1<<2)
#define	CM_FORWARD	(1<<3)
#define	CM_SIDE		(1<<4)
#define	CM_UP		(1<<5)
#define	CM_BUTTONS	(1<<6)
#define CM_WEAPON	(1<<7)

/*
=====================
MSG_WriteDeltaUsercmd
=====================
*/
void MSG_WriteDeltaUsercmd( msg_t *msg, usercmd_t *from, usercmd_t *to ) {
	if ( to->serverTime - from->serverTime < 256 ) {
		MSG_WriteBits( msg, 1, 1 );
		MSG_WriteBits( msg, to->serverTime - from->serverTime, 8 );
	} else {
		MSG_WriteBits( msg, 0, 1 );
		MSG_WriteBits( msg, to->serverTime, 32 );
	}
	MSG_WriteDelta( msg, from->angles[0], to->angles[0], 16 );
	MSG_WriteDelta( msg, from->angles[1], to->angles[1], 16 );
	MSG_WriteDelta( msg, from->angles[2], to->angles[2], 16 );
	MSG_WriteDelta( msg, from->forwardmove, to->forwardmove, 8 );
	MSG_WriteDelta( msg, from->rightmove, to->rightmove, 8 );
	MSG_WriteDelta( msg, from->upmove, to->upmove, 8 );
	MSG_WriteDelta( msg, from->buttons, to->buttons, 16 );
//	MSG_WriteDelta( msg, from->weapon, to->weapon, 8 );
}


/*
=====================
MSG_ReadDeltaUsercmd
=====================
*/
void MSG_ReadDeltaUsercmd( msg_t *msg, usercmd_t *from, usercmd_t *to ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		to->serverTime = from->serverTime + MSG_ReadBits( msg, 8 );
	} else {
		to->serverTime = MSG_ReadBits( msg, 32 );
	}
	to->angles[0] = MSG_ReadDelta( msg, from->angles[0], 16);
	to->angles[1] = MSG_ReadDelta( msg, from->angles[1], 16);
	to->angles[2] = MSG_ReadDelta( msg, from->angles[2], 16);
	to->forwardmove = MSG_ReadDelta( msg, from->forwardmove, 8);
	to->rightmove = MSG_ReadDelta( msg, from->rightmove, 8);
	to->upmove = MSG_ReadDelta( msg, from->upmove, 8);
	to->buttons = MSG_ReadDelta( msg, from->buttons, 16);
//	to->weapon = MSG_ReadDelta( msg, from->weapon, 8);
}

/*
=====================
MSG_WriteDeltaUsercmd
=====================
*/
void MSG_WriteDeltaUsercmdKey( msg_t *msg, int key, usercmd_t *from, usercmd_t *to ) {
	if ( to->serverTime - from->serverTime < 256 ) {
		MSG_WriteBits( msg, 1, 1 );
		MSG_WriteBits( msg, to->serverTime - from->serverTime, 8 );
	} else {
		MSG_WriteBits( msg, 0, 1 );
		MSG_WriteBits( msg, to->serverTime, 32 );
	}
	if (from->angles[0] == to->angles[0] &&
		from->angles[1] == to->angles[1] &&
		from->angles[2] == to->angles[2] &&
		from->forwardmove == to->forwardmove &&
		from->rightmove == to->rightmove &&
		from->upmove == to->upmove &&
		from->buttons == to->buttons /*&&
		from->weapon == to->weapon*/) {
			MSG_WriteBits( msg, 0, 1 );				// no change
			oldsize += 7;
			return;
	}
	key ^= to->serverTime;
	MSG_WriteBits( msg, 1, 1 );
	MSG_WriteDeltaKey( msg, key, from->angles[0], to->angles[0], 16 );
	MSG_WriteDeltaKey( msg, key, from->angles[1], to->angles[1], 16 );
	MSG_WriteDeltaKey( msg, key, from->angles[2], to->angles[2], 16 );
	MSG_WriteDeltaKey( msg, key, from->forwardmove, to->forwardmove, 8 );
	MSG_WriteDeltaKey( msg, key, from->rightmove, to->rightmove, 8 );
	MSG_WriteDeltaKey( msg, key, from->upmove, to->upmove, 8 );
	MSG_WriteDeltaKey( msg, key, from->buttons, to->buttons, 16 );
//	MSG_WriteDeltaKey( msg, key, from->weapon, to->weapon, 8 );
}


/*
=====================
MSG_ReadDeltaUsercmd
=====================
*/
void MSG_ReadDeltaUsercmdKey( msg_t *msg, int key, usercmd_t *from, usercmd_t *to ) {
	if ( MSG_ReadBits( msg, 1 ) ) {
		to->serverTime = from->serverTime + MSG_ReadBits( msg, 8 );
	} else {
		to->serverTime = MSG_ReadBits( msg, 32 );
	}
	if ( MSG_ReadBits( msg, 1 ) ) {
		key ^= to->serverTime;
		to->angles[0] = MSG_ReadDeltaKey( msg, key, from->angles[0], 16);
		to->angles[1] = MSG_ReadDeltaKey( msg, key, from->angles[1], 16);
		to->angles[2] = MSG_ReadDeltaKey( msg, key, from->angles[2], 16);
		to->forwardmove = MSG_ReadDeltaKey( msg, key, from->forwardmove, 8);
		to->rightmove = MSG_ReadDeltaKey( msg, key, from->rightmove, 8);
		to->upmove = MSG_ReadDeltaKey( msg, key, from->upmove, 8);
		to->buttons = MSG_ReadDeltaKey( msg, key, from->buttons, 16);
//		to->weapon = MSG_ReadDeltaKey( msg, key, from->weapon, 8);
	} else {
		to->angles[0] = from->angles[0];
		to->angles[1] = from->angles[1];
		to->angles[2] = from->angles[2];
		to->forwardmove = from->forwardmove;
		to->rightmove = from->rightmove;
		to->upmove = from->upmove;
		to->buttons = from->buttons;
//		to->weapon = from->weapon;
	}
}

/*
=============================================================================

entityState_t communication
  
=============================================================================
*/

/*
=================
MSG_ReportChangeVectors_f

Prints out a table from the current statistics for copying to code
=================
*/
void MSG_ReportChangeVectors_f( void ) {
	int i;
	for(i=0;i<256;i++) {
		if (pcount[i]) {
			Com_Printf("%d used %d\n", i, pcount[i]);
		}
	}
}

typedef struct {
	char	*name;
	int		offset;
	int		bits;		// 0 = float
	int		type;
} netField_t;

// using the stringizing operator to save typing...
#define	NETF(x) #x,(size_t)&((entityState_t*)0)->x

netField_t	entityStateFields[] = 
{
{ NETF(netorigin[0]), 0, 6 },
{ NETF(netorigin[1]), 0, 6 },
{ NETF(netangles[1]), 12, 1 },
{ NETF(frameInfo[0].time), 0, 2 },
{ NETF(frameInfo[1].time), 0, 2 },
{ NETF(bone_angles[0][0]), -13, 1 },
{ NETF(bone_angles[3][0]), -13, 1 },
{ NETF(bone_angles[1][0]), -13, 1 },
{ NETF(bone_angles[2][0]), -13, 1 },
{ NETF(netorigin[2]), 0, 6 },
{ NETF(frameInfo[0].weight), 0, 3 },
{ NETF(frameInfo[1].weight), 0, 3},
{ NETF(frameInfo[2].time), 0, 2 },
{ NETF(frameInfo[3].time), 0, 2 },
{ NETF(frameInfo[0].index), 12, 0 },
{ NETF(frameInfo[1].index), 12, 0 },
{ NETF(actionWeight), 0, 3 },
{ NETF(frameInfo[2].weight), 0, 3 },
{ NETF(frameInfo[3].weight), 0, 3 },
{ NETF(frameInfo[2].index), 12, 0 },
{ NETF(frameInfo[3].index), 12, 0 },
{ NETF(eType), 8, 0 },
{ NETF(modelindex), 16, 0 },
{ NETF(parent), 16, 0 },
{ NETF(constantLight), 32, 0 },
{ NETF(renderfx), 32, 0 },
{ NETF(bone_tag[0]), -8, 0 },
{ NETF(bone_tag[1]), -8, 0 },
{ NETF(bone_tag[2]), -8, 0 },
{ NETF(bone_tag[3]), -8, 0 },
{ NETF(bone_tag[4]), -8, 0 },
{ NETF(scale), 0, 4 },
{ NETF(alpha), 0, 5 },
{ NETF(usageIndex), 16, 0 },
{ NETF(eFlags), 16, 0 },
{ NETF(solid), 32, 0 },
{ NETF(netangles[2]), 12, 1 },
{ NETF(netangles[0]), 12, 1 },
{ NETF(tag_num), 10, 0 },
{ NETF(bone_angles[1][2]), -13, 1 },
{ NETF(attach_use_angles), 1, 0 },
{ NETF(origin2[1]), 0, 6 },
{ NETF(origin2[0]), 0, 6 },
{ NETF(origin2[2]), 0, 6 },
{ NETF(bone_angles[0][2]), -13, 1 },
{ NETF(bone_angles[2][2]), -13, 1 },
{ NETF(bone_angles[3][2]), -13, 1 },
{ NETF(surfaces[0]), 8, 0 },
{ NETF(surfaces[1]), 8, 0 },
{ NETF(surfaces[2]), 8, 0 },
{ NETF(surfaces[3]), 8, 0 },
{ NETF(bone_angles[0][1]), -13, 1 },
{ NETF(surfaces[4]), 8, 0 },
{ NETF(surfaces[5]), 8, 0 },
{ NETF(pos.trTime), 32, 0 },
//{ NETF(pos.trBase[0]), 0, 0 },
//{ NETF(pos.trBase[1]), 0, 0 },
{ NETF(pos.trDelta[0]), 0, 7 },
{ NETF(pos.trDelta[1]), 0, 7 },
//{ NETF(pos.trBase[2]), 0, 0 },
//{ NETF(apos.trBase[1]), 0, 0 },
{ NETF(pos.trDelta[2]), 0, 7 },
//{ NETF(apos.trBase[0]), 0, 0 },
{ NETF(loopSound), 16, 0 },
{ NETF(loopSoundVolume), 0, 0 },
{ NETF(loopSoundMinDist), 0, 0 },
{ NETF(loopSoundMaxDist), 0, 0 },
{ NETF(loopSoundPitch), 0, 0 },
{ NETF(loopSoundFlags), 8, 0 },
{ NETF(attach_offset[0]), 0, 0 },
{ NETF(attach_offset[1]), 0, 0 },
{ NETF(attach_offset[2]), 0, 0 },
{ NETF(beam_entnum), 16, 0 },
{ NETF(skinNum), 16, 0 },
{ NETF(wasframe), 10, 0 },
{ NETF(frameInfo[4].index), 12, 0 },
{ NETF(frameInfo[5].index), 12, 0 },
{ NETF(frameInfo[6].index), 12, 0 },
{ NETF(frameInfo[7].index), 12, 0 },
{ NETF(frameInfo[8].index), 12, 0 },
{ NETF(frameInfo[9].index), 12, 0 },
{ NETF(frameInfo[10].index), 12, 0 },
{ NETF(frameInfo[11].index), 12, 0 },
{ NETF(frameInfo[12].index), 12, 0 },
{ NETF(frameInfo[13].index), 12, 0 },
{ NETF(frameInfo[14].index), 12, 0 },
{ NETF(frameInfo[15].index), 12, 0 },
{ NETF(frameInfo[4].time), 0, 2 },
{ NETF(frameInfo[5].time), 0, 2 },
{ NETF(frameInfo[6].time), 0, 2 },
{ NETF(frameInfo[7].time), 0, 2 },
{ NETF(frameInfo[8].time), 0, 2 },
{ NETF(frameInfo[9].time), 0, 2 },
{ NETF(frameInfo[10].time), 0, 2 },
{ NETF(frameInfo[11].time), 0, 2 },
{ NETF(frameInfo[12].time), 0, 2 },
{ NETF(frameInfo[13].time), 0, 2 },
{ NETF(frameInfo[14].time), 0, 2 },
{ NETF(frameInfo[15].time), 0, 2 },
{ NETF(frameInfo[4].weight), 0, 3 },
{ NETF(frameInfo[5].weight), 0, 3 },
{ NETF(frameInfo[6].weight), 0, 3 },
{ NETF(frameInfo[7].weight), 0, 3 },
{ NETF(frameInfo[8].weight), 0, 3 },
{ NETF(frameInfo[9].weight), 0, 3 },
{ NETF(frameInfo[10].weight), 0, 3 },
{ NETF(frameInfo[11].weight), 0, 3 },
{ NETF(frameInfo[12].weight), 0, 3 },
{ NETF(frameInfo[13].weight), 0, 3 },
{ NETF(frameInfo[14].weight), 0, 3 },
{ NETF(frameInfo[15].weight), 0, 3 },
{ NETF(bone_angles[1][1]), -13, 1 },
{ NETF(bone_angles[2][1]), -13, 1 },
{ NETF(bone_angles[3][1]), -13, 1 },
{ NETF(bone_angles[4][0]), -13, 1 },
{ NETF(bone_angles[4][1]), -13, 1 },
{ NETF(bone_angles[4][2]), -13, 1 },
{ NETF(clientNum), 8, 0 },
{ NETF(groundEntityNum), GENTITYNUM_BITS, 0 },
{ NETF(shader_data[0]), 0, 0 },
{ NETF(shader_data[1]), 0, 0 },
{ NETF(shader_time), 0, 0 },
{ NETF(eyeVector[0]), 0, 0 },
{ NETF(eyeVector[1]), 0, 0 },
{ NETF(eyeVector[2]), 0, 0 },
{ NETF(surfaces[6]), 8, 0 },
{ NETF(surfaces[7]), 8, 0 },
{ NETF(surfaces[8]), 8, 0 },
{ NETF(surfaces[9]), 8, 0 },
{ NETF(surfaces[10]), 8, 0 },
{ NETF(surfaces[11]), 8, 0 },
{ NETF(surfaces[12]), 8, 0 },
{ NETF(surfaces[13]), 8, 0 },
{ NETF(surfaces[14]), 8, 0 },
{ NETF(surfaces[15]), 8, 0 },
{ NETF(surfaces[16]), 8, 0 },
{ NETF(surfaces[17]), 8, 0 },
{ NETF(surfaces[18]), 8, 0 },
{ NETF(surfaces[19]), 8, 0 },
{ NETF(surfaces[20]), 8, 0 },
{ NETF(surfaces[21]), 8, 0 },
{ NETF(surfaces[22]), 8, 0 },
{ NETF(surfaces[23]), 8, 0 },
{ NETF(surfaces[24]), 8, 0 },
{ NETF(surfaces[25]), 8, 0 },
{ NETF(surfaces[26]), 8, 0 },
{ NETF(surfaces[27]), 8, 0 },
{ NETF(surfaces[28]), 8, 0 },
{ NETF(surfaces[29]), 8, 0 },
{ NETF(surfaces[30]), 8, 0 },
{ NETF(surfaces[31]), 8, 0 }

/*
{ NETF(event), 10 },
{ NETF(angles2[1]), 0 },

{ NETF(torsoAnim), 8 },
{ NETF(eventParm), 8 },
{ NETF(legsAnim), 8 },

{ NETF(pos.trType), 8 },

{ NETF(otherEntityNum), GENTITYNUM_BITS },
{ NETF(weapon), 8 },

{ NETF(angles[1]), 0 },
{ NETF(pos.trDuration), 32 },
{ NETF(apos.trType), 8 },
{ NETF(origin[0]), 0 },
{ NETF(origin[1]), 0 },
{ NETF(origin[2]), 0 },

{ NETF(powerups), MAX_POWERUPS },

{ NETF(otherEntityNum2), GENTITYNUM_BITS },

{ NETF(generic1), 8 },

{ NETF(modelindex2), 8 },
{ NETF(angles[0]), 0 },
{ NETF(time), 32 },
{ NETF(apos.trTime), 32 },
{ NETF(apos.trDuration), 32 },
{ NETF(apos.trBase[2]), 0 },
{ NETF(apos.trDelta[0]), 0 },
{ NETF(apos.trDelta[1]), 0 },
{ NETF(apos.trDelta[2]), 0 },
{ NETF(time2), 32 },
{ NETF(angles[2]), 0 },
{ NETF(angles2[0]), 0 },
{ NETF(angles2[2]), 0 },

{ NETF(frame), 16 }*/
};


// if (int)f == f and (int)f + ( 1<<(FLOAT_INT_BITS-1) ) < ( 1 << FLOAT_INT_BITS )
// the float will be sent with FLOAT_INT_BITS, otherwise all 32 bits will be sent
#define	FLOAT_INT_BITS	13
#define	FLOAT_INT_BIAS	(1<<(FLOAT_INT_BITS-1))

/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message, including the entity number.
Can delta from either a baseline or a previous packet_entity
If to is NULL, a remove entity update will be sent
If force is not set, then nothing at all will be generated if the entity is
identical, under the assumption that the in-order delta code will catch it.
==================
*/
void MSG_WriteDeltaEntity( msg_t *msg, struct entityState_s *from, struct entityState_s *to, 
						   qboolean force ) {
	int			i, lc;
	int			numFields;
	netField_t	*field;
	int			trunc;
	float		fullFloat;
	int			*fromF, *toF;
	float tmp;
	int bits;

	numFields = sizeof(entityStateFields)/sizeof(entityStateFields[0]);

	// all fields should be 32 bits to avoid any compiler packing issues
	// the "number" field is not part of the field list
	// if this assert fails, someone added a field to the entityState_t
	// struct without updating the message fields

	// wombat: we may do it cause this is sparta!
	//assert( numFields + 1 == sizeof( *from )/4 );

	// a NULL to is a delta remove message
	if ( to == NULL ) {
		if ( from == NULL ) {
			return;
		}
		MSG_WriteBits( msg, from->number, GENTITYNUM_BITS );
		MSG_WriteBits( msg, 1, 1 );
		return;
	}

	if ( to->number < 0 || to->number >= MAX_GENTITIES ) {
		Com_Error (ERR_FATAL, "MSG_WriteDeltaEntity: Bad entity number: %i", to->number );
	}

	lc = 0;
	// build the change vector as bytes so it is endien independent
	for ( i = 0, field = entityStateFields ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		if ( *fromF != *toF ) {
			lc = i+1;
		}
	}

	if ( lc == 0 ) {
		// nothing at all changed
		if ( !force ) {
			return;		// nothing at all
		}
		// write two bits for no change
		MSG_WriteBits( msg, to->number, GENTITYNUM_BITS );
		MSG_WriteBits( msg, 0, 1 );		// not removed
		MSG_WriteBits( msg, 0, 1 );		// no delta
		return;
	}

	MSG_WriteBits( msg, to->number, GENTITYNUM_BITS );
	MSG_WriteBits( msg, 0, 1 );			// not removed
	MSG_WriteBits( msg, 1, 1 );			// we have a delta

	MSG_WriteByte( msg, lc );	// # of changes

	oldsize += numFields;

	for ( i = 0, field = entityStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( *fromF == *toF ) {
			MSG_WriteBits( msg, 0, 1 );	// no change
			continue;
		}

		MSG_WriteBits( msg, 1, 1 );	// changed

		switch ( field->type ) {
			case 0: // normal style
			if ( field->bits == 0 ) {
				// float
				fullFloat = *(float *)toF;
				trunc = (int)fullFloat;

				if (fullFloat == 0.0f) {
						MSG_WriteBits( msg, 0, 1 );
						oldsize += FLOAT_INT_BITS;
				} else {
					MSG_WriteBits( msg, 1, 1 );
					if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 && 
						trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
						// send as small integer
						MSG_WriteBits( msg, 0, 1 );
						MSG_WriteBits( msg, trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
					} else {
						// send as full floating point value
						MSG_WriteBits( msg, 1, 1 );
						MSG_WriteBits( msg, *toF, 32 );
					}
				}
			} else {
				if (*toF == 0) {
					MSG_WriteBits( msg, 0, 1 );
				} else {
					MSG_WriteBits( msg, 1, 1 );
					// integer
					MSG_WriteBits( msg, *toF, field->bits );
				}
			}
			break;
			case 1: // angles, what a mess! it wouldnt surprise me if something goes wrong here ;)
				tmp = *(float *)toF;
				if ( field->bits < 0 ) {
					if ( tmp < 0.0f ) {
						MSG_WriteBits( msg, 1, 1 );
						tmp = -tmp;
					}
					else {
						MSG_WriteBits( msg, 0, 1 );
					}
					bits = ~field->bits;
				}
				else bits = field->bits;

				if ( bits == 12 ) {
					tmp = tmp * 4096.0f / 360.0f;
					MSG_WriteBits( msg, ((int)tmp) & 4095, 12 );
				}
				else if ( bits == 8 ) {
					tmp = tmp * 256.0f / 360.0f;
					MSG_WriteBits( msg, ((int)tmp) & 255, 8 );
				}
				else if ( bits == 16 ) {
					tmp = tmp * 65536.0f / 360.0f;
					MSG_WriteBits( msg, ((int)tmp) & 65535, 16 );
				}
				else {
					tmp = tmp * (1<<(byte)bits) / 360.0f;
					MSG_WriteBits( msg, ((int)tmp) & ((1<<(byte)bits) -1), bits );
				}
				break;
			case 2: // time
				tmp = *(float *)toF;
				bits = tmp * 100.0f;
				if ( bits < 0 )
					bits = 0;
				else if ( bits > 32767 )
					bits = 32767;
				MSG_WriteBits( msg, bits, 15 );
				break;
			case 3: // nasty!
				tmp = *(float *)toF;

				bits = (tmp * 255.0f) + 0.5f;
				if ( bits < 0 )
					bits = 0;
				else if ( bits > 255 )
					bits = 255;
				MSG_WriteBits( msg, bits, 8 );
				break;
			case 4:
				tmp = *(float *)toF;
				bits = tmp * 100.0f;
				if ( bits < 0 )
					bits = 0;
				else if ( bits > 1023 )
					bits = 1023;
				MSG_WriteBits( msg, bits, 10 );
				break;
			case 5:
				tmp = *(float *)toF;

				bits = (tmp * 255.0f) + 0.5f;
				if ( bits < 0 )
					bits = 0;
				else if ( bits > 255 )
					bits = 255;
				MSG_WriteBits( msg, bits, 8 );
				break;
			case 6:
				tmp = *(float *)toF;

				bits = tmp * 16.0f;
				if ( tmp < 0 )
					bits = ((-bits) & 262143) | 262144;
				else
					bits = bits & 262143;
				MSG_WriteBits( msg, bits, 19 );
				break;
			case 7:
				tmp = *(float *)toF;

				bits = tmp * 8.0f;
				if ( tmp < 0 )
					bits = ((-bits) & 65535) | 65536;
				else
					bits = bits & 65535;
				MSG_WriteBits( msg, bits, 17 );
				break;
			default:
				Com_Error( ERR_DROP, "MSG_WriteDeltaEntity: unrecognized entity field type %i for field %i\n", field->bits, i );
				break;
		}


	}
}

/*
==================
MSG_ReadSounds

read the sounds from the snapshot...
1:1 translated from assembly code
==================
*/
void MSG_ReadSounds (msg_t *msg, server_sound_t *sounds, int *snapshot_number_of_sounds) {

	int		fubar;
	int		i;

	if ( MSG_ReadBits(msg, 1) ) {
		fubar = MSG_ReadBits( msg, 7 );

		if ( fubar <= 64 ) {
			*snapshot_number_of_sounds = fubar;
			for (i=0; i<fubar; i++ ) {
				if ( MSG_ReadBits(msg, 1) == 1 ) {
					sounds[i].entity_number = MSG_ReadBits(msg, 10 );
					sounds[i].channel = MSG_ReadBits(msg, 7 );
					sounds[i].stop_flag = qtrue; // su44 was here
				} else {
					sounds[i].stop_flag = qfalse;
					sounds[i].streamed = (qboolean)MSG_ReadBits( msg, 1 );
					if ( MSG_ReadBits(msg, 1) == 1 ) {
						sounds[i].origin[0] = MSG_ReadFloat( msg );
						sounds[i].origin[1] = MSG_ReadFloat( msg );
						sounds[i].origin[2] = MSG_ReadFloat( msg );
					} else {
						sounds[i].origin[0] = 0;
						sounds[i].origin[1] = 0;
						sounds[i].origin[2] = 0;
					}
					sounds[i].entity_number = MSG_ReadBits(msg, 11 );
					sounds[i].channel = MSG_ReadBits(msg, 7 );
					sounds[i].sound_index = MSG_ReadBits(msg, 9 );

					if ( MSG_ReadBits(msg, 1) == 1 ) {
						sounds[i].volume = MSG_ReadFloat( msg );
					} else {
						sounds[i].volume = -1.0f;
					}

					if ( MSG_ReadBits(msg, 1) == 1 ) {
						sounds[i].min_dist = MSG_ReadFloat( msg );
					} else {
						sounds[i].min_dist = -1.0f;
					}

					if ( MSG_ReadBits( msg, 1 ) == 1 ) {
						sounds[i].pitch = MSG_ReadFloat( msg );
					} else {
						sounds[i].pitch = 1.0f; // su44 was here
					}

					sounds[i].maxDist = MSG_ReadFloat( msg );
				}
			}

		}
	}
}

/*
==================
MSG_WriteSounds

write the sounds to the snapshot...
1:1 translated from assembly code
==================
*/
void MSG_WriteSounds (msg_t *msg, server_sound_t *sounds, int snapshot_number_of_sounds) {

	int		i;

	if ( !snapshot_number_of_sounds ) {
		MSG_WriteBits( msg, 0, 1 );
	}
	else {
		MSG_WriteBits( msg, 1, 1 );
		MSG_WriteBits( msg, snapshot_number_of_sounds, 7 );

		for ( i=0; i < snapshot_number_of_sounds; i++ ) {
			if (!sounds[i].stop_flag) {
				MSG_WriteBits( msg, 0, 1 );
				MSG_WriteBits( msg, sounds[i].streamed, 1 );

				if ( sounds[i].origin[0] == 0.0f && sounds[i].origin[1] == 0.0f && sounds[i].origin[2] == 0.0f )
					MSG_WriteBits( msg, 0, 1 );
				else {
					MSG_WriteBits( msg, 1, 1 );
					MSG_WriteFloat( msg, sounds[i].origin[0] );
					MSG_WriteFloat( msg, sounds[i].origin[1] );
					MSG_WriteFloat( msg, sounds[i].origin[2] );
				}
				MSG_WriteBits( msg, sounds[i].entity_number, 11 );
				MSG_WriteBits( msg, sounds[i].channel, 7 );
				MSG_WriteBits( msg, sounds[i].sound_index, 9 );

				if( sounds[ i ].volume != -1.0f ) {
					MSG_WriteBits( msg, 1, 1 );
					MSG_WriteFloat( msg, sounds[i].volume );
				} else {
					MSG_WriteBits( msg, 0, 1 );
				}

				if ( sounds[i].min_dist != -1.0f ){
					MSG_WriteBits( msg, 1, 1 );
					MSG_WriteFloat( msg, sounds[i].min_dist );
				} else {
					MSG_WriteBits( msg, 0, 1 );
				}

				if ( sounds[i].pitch != -1.0f ){
					MSG_WriteBits( msg, 1, 1 );
					MSG_WriteFloat( msg, sounds[i].pitch );
				} else {
					MSG_WriteBits( msg, 0, 1 );
				}

				MSG_WriteFloat( msg, sounds[i].maxDist );
			}
			else {
				MSG_WriteBits( msg, 1, 1 );
				MSG_WriteBits( msg, sounds[i].entity_number, 10 );
				MSG_WriteBits( msg, sounds[i].channel, 7 );
			}
		}
	}
}

void MSG_ReadDeltaEyeInfo(msg_t *msg, usereyes_t *from, usereyes_t *to) {

	if ( MSG_ReadBits( msg, 1 ) ) {
		to->ofs[0] = MSG_ReadDelta( msg, from->ofs[0], 8 );
		to->ofs[1] = MSG_ReadDelta( msg, from->ofs[1], 8 );
		to->ofs[2] = MSG_ReadDelta( msg, from->ofs[2], 8 );

		to->angles[0] = MSG_ReadDeltaFloat( msg, from->angles[0] );
		to->angles[1] = MSG_ReadDeltaFloat( msg, from->angles[1] );
	} else {
		to->angles[0] = from->angles[0];
		to->angles[1] = from->angles[1];

		to->ofs[0] = from->ofs[0];
		to->ofs[1] = from->ofs[1];
		to->ofs[2] = from->ofs[2];
	}

}

/*
==================
MSG_ReadDeltaEntity

The entity number has already been read from the message, which
is how the from state is identified.

If the delta removes the entity, entityState_t->number will be set to MAX_GENTITIES-1

Can go from either a baseline or a previous packet_entity
==================
*/
extern	cvar_t	*cl_shownet;

void MSG_ReadDeltaEntity( msg_t *msg, entityState_t *from, entityState_t *to, 
						 int number) {
	int			i, lc;
	int			numFields;
	netField_t	*field;
	int			*fromF, *toF;
	int			print;
	int			trunc;
	int			startBit, endBit;
	int			bits;
	int			result;
	float		tmp;

	if ( number < 0 || number >= MAX_GENTITIES) {
		Com_Error( ERR_DROP, "Bad delta entity number: %i", number );
	}

	if ( msg->bit == 0 ) {
		startBit = msg->readcount * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
	}

	// check for a remove
	if ( MSG_ReadBits( msg, 1 ) == 1 ) {
		Com_Memset( to, 0, sizeof( *to ) );	
		to->number = MAX_GENTITIES - 1;
		if ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) {
			Com_Printf( "%3i: #%-3i remove\n", msg->readcount, number );
		}
		return;
	}

	// check for no delta
	if ( MSG_ReadBits( msg, 1 ) == 0 ) {
		*to = *from;
		to->number = number;
		return;
	}

	numFields = sizeof(entityStateFields)/sizeof(entityStateFields[0]);
	lc = MSG_ReadByte(msg);

	// su44: trying to find the cause of "unrecognized entity field type" error.
	// Number of changed fields (lc) should never be larger than total field count....
	if(numFields < lc) {
		Com_Error( ERR_DROP, "MSG_ReadDeltaEntity: number of changed fields (%i) is higher than total field count (%i)\n",
			lc, numFields);
	}

	// shownet 2/3 will interleave with other printed info, -1 will
	// just print the delta records`
	if ( cl_shownet->integer >= 2 || cl_shownet->integer == -1 ) {
		print = 1;
		Com_Printf( "%3i: #%-3i ", msg->readcount, to->number );
	} else {
		print = 0;
	}

	to->number = number;

	for ( i = 0, field = entityStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
#if 1
		// su44: trying to find the cause of "unrecognized entity field type" error.
		// It seems that "field" ptr got screwed somewhere
		if(field->offset > sizeof(entityState_t)) {
			Com_Error( ERR_DROP, "MSG_ReadDeltaEntity: field offset %i is larger than size of entityState (i %i)\n", field->offset, i );
		}
#endif
		if ( ! MSG_ReadBits( msg, 1 ) ) {
			// no change
			*toF = *fromF;
		} else {
			switch (field->type) {
				case 0:
					if ( field->bits == 0 ) {
						// float
						if ( MSG_ReadBits( msg, 1 ) == 0 ) {
								*(float *)toF = 0.0f; 
						} else {
							if ( MSG_ReadBits( msg, 1 ) == 0 ) {
								// integral float
								trunc = MSG_ReadBits( msg, FLOAT_INT_BITS );
								// bias to allow equal parts positive and negative
								trunc -= FLOAT_INT_BIAS;
								*(float *)toF = trunc; 
								if ( print ) {
									Com_Printf( "%s:%i ", field->name, trunc );
								}
							} else {
								// full floating point value
								*toF = MSG_ReadBits( msg, 32 );
								if ( print ) {
									Com_Printf( "%s:%f ", field->name, *(float *)toF );
								}
							}
						}
					} else {
						if ( MSG_ReadBits( msg, 1 ) == 0 ) {
							*toF = 0;
						} else {
							// integer
							*toF = MSG_ReadBits( msg, field->bits );
							if ( print ) {
								Com_Printf( "%s:%i ", field->name, *toF );
							}
						}
					}
					break;
				case 1: // angles, what a mess! it wouldnt surprise me if something goes wrong here ;)
					tmp = 1.0f;
					if ( field->bits < 0 ) {
						if ( MSG_ReadBits( msg, 1 ) )
							tmp = -1.0f;
						bits = ~field->bits;
					}
					else bits = field->bits;

					result = MSG_ReadBits( msg, bits );
					if ( bits == 12 )
						*(float *)toF = result * 0.087890625f * tmp;
					else if ( bits == 8 )
						*(float *)toF = result * 1.411764705882353f * tmp;
					else if ( bits == 16 )
						*(float *)toF = result * 0.0054931640625f * tmp;
					else
						*(float *)toF = result * (1 << bits) * tmp / 360.0f;
					break;
				case 2: // time
					*(float *)toF = MSG_ReadBits( msg, 15 ) * 0.0099999998f;
					break;
				case 3: // nasty!
					tmp = MSG_ReadBits( msg, 8 ) / 255.0f;
					if ( tmp < 0.0f )
						*(float *)toF = 0.0f;
					else if ( tmp > 1.0f )
						*(float *)toF = 1.0f;
					else
						*(float *)toF = tmp;
					// FPU instructions yay
					break;
				case 4:
					*(float *)toF = MSG_ReadBits( msg, 10 ) *0.0099999998f;
					break;
				case 5:
					tmp = MSG_ReadBits( msg, 8 ) / 255.0f;
					if ( tmp < 0.0f )
						*(float *)toF = 0.0f;
					else if ( tmp > 1.0f )
						*(float *)toF = 1.0f;
					else
						*(float *)toF = tmp;
					break;
				case 6:
					tmp = 1.0f;
					bits = MSG_ReadBits( msg, 19 );
					if ( bits & 262144 ) // test for 19th bit
						tmp = -1.0f;
					bits &= ~262144;	// remove that bit
					*(float *)toF = tmp * bits / 16.0f;
					break;
				case 7:
					tmp = 1.0f;
					bits = MSG_ReadBits( msg, 17 );
					if ( bits & 65536 ) // test for 17th bit
						tmp = -1.0f;
					bits &= ~65536; // remove that bit
					*(float *)toF = tmp * bits / 8.0f;
					break;
				default:
					Com_Error( ERR_DROP, "MSG_ReadDeltaEntity: unrecognized entity field type %i for field\n", i );
					break;
			}

//			pcount[i]++;
		}
	}
	for ( i = lc, field = &entityStateFields[lc] ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		// no change
		*toF = *fromF;
	}

	if ( print ) {
		if ( msg->bit == 0 ) {
			endBit = msg->readcount * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}

	VectorCopy( to->netorigin, to->origin );
	VectorCopy( to->netangles, to->angles );
	EulerToQuat( to->angles, to->quat );

	// get bone controllers
	for( i = 0; i < NUM_BONE_CONTROLLERS; i++ )
	{
		if( to->bone_tag[ i ] >= 0 ) {
			EulerToQuat( to->bone_angles[ i ], to->bone_quat[ i ] );
		}
	}
}


/*
============================================================================

plyer_state_t communication

============================================================================
*/

// using the stringizing operator to save typing...
#define	PSF(x) #x,(size_t)&((playerState_t*)0)->x

netField_t	playerStateFields[] = 
{
{ PSF(commandTime), 32, 0 },				
{ PSF(origin[0]), 0, 6 },
{ PSF(origin[1]), 0, 6 },
{ PSF(viewangles[1]), 0, 0 },
{ PSF(velocity[1]), 0, 7 },
{ PSF(velocity[0]), 0, 7 },
{ PSF(viewangles[0]), 0, 0 },
{ PSF(pm_time), -16, 0 },
//{ PSF(weaponTime), -16, 0 },
{ PSF(origin[2]), 0, 6 },
{ PSF(velocity[2]), 0, 7 },
{ PSF(iViewModelAnimChanged), 2, 0 },
{ PSF(damage_angles[0]), -13, 1 },
{ PSF(damage_angles[1]), -13, 1 },
{ PSF(damage_angles[2]), -13, 1 },
{ PSF(speed), 16, 0 },
{ PSF(delta_angles[1]), 16, 0 },
{ PSF(viewheight), -8, 0 },
{ PSF(groundEntityNum), GENTITYNUM_BITS, 0 },
{ PSF(delta_angles[0]), 16, 0 },
{ PSF(iViewModelAnim), 4, 0 },
{ PSF(fov), 0, 0 },
{ PSF(current_music_mood), 8, 0 },
{ PSF(gravity), 16, 0 },
{ PSF(fallback_music_mood), 8, 0 },
{ PSF(music_volume), 0, 0 },
{ PSF(pm_flags), 16, 0 },
{ PSF(clientNum), 8, 0 },
{ PSF(fLeanAngle), 0, 0 },
{ PSF(blend[3]), 0, 0 },
{ PSF(blend[0]), 0, 0 },
{ PSF(pm_type), 8, 0 },
{ PSF(feetfalling), 8, 0 },
{ PSF(camera_angles[0]), 16, 1 },
{ PSF(camera_angles[1]), 16, 1 },
{ PSF(camera_angles[2]), 16, 1 },
{ PSF(camera_origin[0]), 0, 6 },
{ PSF(camera_origin[1]), 0, 6 },
{ PSF(camera_origin[2]), 0, 6 },
{ PSF(camera_posofs[0]), 0, 6 },
{ PSF(camera_posofs[2]), 0, 6 },
{ PSF(camera_time), 0, 0 },
{ PSF(bobCycle), 8, 0 },
{ PSF(delta_angles[2]), 16, 0 },
{ PSF(viewangles[2]), 0, 0 },
{ PSF(music_volume_fade_time), 0, 0 },
{ PSF(reverb_type), 6, 0 },
{ PSF(reverb_level), 0, 0 },
{ PSF(blend[1]), 0, 0 },
{ PSF(blend[2]), 0, 0 },
{ PSF(camera_offset[0]), 0, 0 },
{ PSF(camera_offset[1]), 0, 0 },
{ PSF(camera_offset[2]), 0, 0 },
{ PSF(camera_posofs[1]), 0, 6 },
{ PSF(camera_flags), 16, 0 }

/*
{ PSF(eventSequence), 16 },
{ PSF(torsoAnim), 8 },
{ PSF(movementDir), 4 },
{ PSF(events[0]), 8 },
{ PSF(legsAnim), 8 },
{ PSF(events[1]), 8 },


{ PSF(weaponstate), 4 },
{ PSF(eFlags), 16 },
{ PSF(externalEvent), 10 },



{ PSF(externalEventParm), 8 },

{ PSF(damageEvent), 8 },
{ PSF(damageYaw), 8 },
{ PSF(damagePitch), 8 },
{ PSF(damageCount), 8 },
{ PSF(generic1), 8 },



{ PSF(torsoTimer), 12 },
{ PSF(eventParms[0]), 8 },
{ PSF(eventParms[1]), 8 },

{ PSF(weapon), 5 },

{ PSF(grapplePoint[0]), 0 },
{ PSF(grapplePoint[1]), 0 },
{ PSF(grapplePoint[2]), 0 },
{ PSF(jumppad_ent), 10 },
{ PSF(loopSound), 16 }*/
};

void MSG_WriteDeltaEyeInfo(msg_t  *msg, usereyes_t *from, usereyes_t *to) {

	if ( to->angles[0] != from->angles[0] || to->angles[1] != from->angles[1] || to->ofs[0] != from->ofs[0] || to->ofs[1] != from->ofs[1] || to->ofs[2] != from->ofs[2] ) {
		MSG_WriteBits( msg, 1, 1 );
		MSG_WriteDelta( msg, from->ofs[0], to->ofs[0], 8 );
		MSG_WriteDelta( msg, from->ofs[1], to->ofs[1], 8 );
		MSG_WriteDelta( msg, from->ofs[2], to->ofs[2], 8 );

		MSG_WriteDeltaFloat( msg, from->angles[0], to->angles[0] );
		MSG_WriteDeltaFloat( msg, from->angles[1], to->angles[1] );
	} else MSG_WriteBits( msg, 0, 1 );
}

/*
=============
MSG_WriteDeltaPlayerstate

=============
*/
void MSG_WriteDeltaPlayerstate( msg_t *msg, struct playerState_s *from, struct playerState_s *to ) {
	int				i;
	playerState_t	dummy;
	int				statsbits;
	int				activeitemsbits;
	int				ammobits;
	int				ammo_amountbits;
	int				max_ammo_amountbits;
	int				numFields;
	size_t			c;
	netField_t		*field;
	int				*fromF, *toF;
	float			fullFloat;
	int				trunc, lc;
	int bits;

	if (!from) {
		from = &dummy;
		Com_Memset (&dummy, 0, sizeof(dummy));
	}

	c = msg->cursize;

	numFields = sizeof( playerStateFields ) / sizeof( playerStateFields[0] );

	lc = 0;
	for ( i = 0, field = playerStateFields ; i < numFields ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		if ( *fromF != *toF ) {
			lc = i+1;
		}
	}

	MSG_WriteByte( msg, lc );	// # of changes

	oldsize += numFields - lc;

	for ( i = 0, field = playerStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if ( *fromF == *toF ) {
			MSG_WriteBits( msg, 0, 1 );	// no change
			continue;
		}

		MSG_WriteBits( msg, 1, 1 );	// changed
//		pcount[i]++;

		switch ( field->type ) {
			case 0:
				if ( field->bits == 0 ) {
					// float
					fullFloat = *(float *)toF;
					trunc = (int)fullFloat;

					if ( trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 && 
						trunc + FLOAT_INT_BIAS < ( 1 << FLOAT_INT_BITS ) ) {
						// send as small integer
						MSG_WriteBits( msg, 0, 1 );
						MSG_WriteBits( msg, trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS );
					} else {
						// send as full floating point value
						MSG_WriteBits( msg, 1, 1 );
						MSG_WriteBits( msg, *toF, 32 );
					}
				} else {
					// integer
					MSG_WriteBits( msg, *toF, field->bits );
				}
				break;
			case 1:
				fullFloat = *(float *)toF;
				if ( field->bits < 0 ) {
					if ( 0.0f > fullFloat ) {
						MSG_WriteBits( msg, 1, 1 );
						fullFloat = - fullFloat;
					}
					else {
						MSG_WriteBits( msg, 0, 1 );
					}

					bits = ~field->bits;
				} else bits = field->bits;
				if ( bits == 12 ) {
					fullFloat = fullFloat * 4096.0f / 360.0f;
					trunc = (int)fullFloat & 4095;
					MSG_WriteBits( msg, trunc, 12 );
				}
				else if ( bits == 16 ) {
					fullFloat = fullFloat * 65536.0f / 360.0f;
					trunc = (int)fullFloat & 65535;
					MSG_WriteBits( msg, trunc, 16 );
				}
				else if ( bits == 8 ) {
					fullFloat = fullFloat * 256.0f / 360.0f;
					trunc = (int)fullFloat & 255;
					MSG_WriteBits( msg, trunc, 8 );
				}
				else {
					fullFloat = fullFloat * (1<<bits) / 360.0f;
					trunc = (int)fullFloat & ((1<<bits)-1);
					MSG_WriteBits( msg, trunc, bits );
				}
				break;
			case 6:
				fullFloat = *(float *)toF;
				fullFloat = fullFloat * 16.0f;
				trunc = fullFloat;

				if ( 0.0f > fullFloat )
					trunc = ((-trunc)&262143)|262144;
				else
					trunc = trunc & 262143;

				MSG_WriteBits( msg, trunc, 19 );
				break;
			case 7:
				fullFloat = *(float *)toF;
				fullFloat = fullFloat * 8.0f;
				trunc = fullFloat;

				if ( 0.0f > fullFloat )
					trunc = ((-trunc)&65535)|65536;
				else
					trunc = trunc & 65535;

				MSG_WriteBits( msg, trunc, 17 );
				break;
			default:
					break;
		}
	}
	c = msg->cursize - c;


	//
	// send the arrays
	//
	statsbits = 0;
	for (i=0 ; i<MAX_STATS ; i++) {
		if (to->stats[i] != from->stats[i]) {
			statsbits |= 1<<i;
		}
	}
	activeitemsbits = 0;
	for (i=0 ; i<MAX_ACTIVEITEMS ; i++) {
		if (to->activeItems[i] != from->activeItems[i]) {
			activeitemsbits |= 1<<i;
		}
	}
	ammo_amountbits = 0;
	for (i=0 ; i<MAX_AMMO_AMOUNT ; i++) {
		if (to->ammo_amount[i] != from->ammo_amount[i]) {
			ammo_amountbits |= 1<<i;
		}
	}
	ammobits = 0;
	for (i=0 ; i<MAX_WEAPONS ; i++) {
		if (to->ammo_name_index[i] != from->ammo_name_index[i]) {
			ammobits |= 1<<i;
		}
	}
	max_ammo_amountbits = 0;
	for (i=0 ; i<MAX_MAX_AMMO_AMOUNT ; i++) {
		if (to->max_ammo_amount[i] != from->max_ammo_amount[i]) {
			max_ammo_amountbits |= 1<<i;
		}
	}

	if (!statsbits && !activeitemsbits && !ammobits && !ammo_amountbits && !max_ammo_amountbits) {
		MSG_WriteBits( msg, 0, 1 );	// no change
		oldsize += 5;
		return;
	}
	MSG_WriteBits( msg, 1, 1 );	// changed

	if ( statsbits ) {
		MSG_WriteBits( msg, 1, 1 );	// changed
		//MSG_WriteBits( msg, statsbits, MAX_STATS );
		MSG_WriteLong( msg, statsbits );
		for (i=0 ; i<MAX_STATS ; i++)
			if (statsbits & (1<<i) )
				MSG_WriteShort (msg, to->stats[i]);
	} else {
		MSG_WriteBits( msg, 0, 1 );	// no change
	}


	if ( activeitemsbits ) {
		MSG_WriteBits( msg, 1, 1 );	// changed
		MSG_WriteBits( msg, activeitemsbits, MAX_ACTIVEITEMS );
		for (i=0 ; i<MAX_ACTIVEITEMS ; i++)
			if (activeitemsbits & (1<<i) )
				MSG_WriteShort (msg, to->activeItems[i]);
	} else {
		MSG_WriteBits( msg, 0, 1 );	// no change
	}

	if ( ammo_amountbits ) {
		MSG_WriteBits( msg, 1, 1 );	// changed
		MSG_WriteBits( msg, ammo_amountbits, MAX_AMMO_AMOUNT );
		for (i=0 ; i<MAX_AMMO_AMOUNT ; i++)
			if (ammo_amountbits & (1<<i) )
				MSG_WriteShort (msg, to->ammo_amount[i]);
	} else {
		MSG_WriteBits( msg, 0, 1 );	// no change
	}


	if ( ammobits ) {
		MSG_WriteBits( msg, 1, 1 );	// changed
		MSG_WriteBits( msg, ammobits, MAX_AMMO );
		for (i=0 ; i<MAX_AMMO ; i++)
			if (ammobits & (1<<i) )
				MSG_WriteShort (msg, to->ammo_name_index[i]);
	} else {
		MSG_WriteBits( msg, 0, 1 );	// no change
	}


	if ( max_ammo_amountbits ) {
		MSG_WriteBits( msg, 1, 1 );	// changed
		MSG_WriteBits( msg, max_ammo_amountbits, MAX_MAX_AMMO_AMOUNT );
		for (i=0 ; i<MAX_MAX_AMMO_AMOUNT ; i++)
			if (max_ammo_amountbits & (1<<i) )
				MSG_WriteShort( msg, to->max_ammo_amount[i] );
	} else {
		MSG_WriteBits( msg, 0, 1 );	// no change
	}
}


/*
===================
MSG_ReadDeltaPlayerstate
===================
*/
void MSG_ReadDeltaPlayerstate (msg_t *msg, playerState_t *from, playerState_t *to ) {
	int			i, lc;
	int			bits;
	netField_t	*field;
	int			numFields;
	int			startBit, endBit;
	int			print;
	int			*fromF, *toF;
	int			trunc;
	playerState_t	dummy;

//	int			bits;
	int			result;
	float		tmp;

	if ( !from ) {
		from = &dummy;
		Com_Memset( &dummy, 0, sizeof( dummy ) );
	}
	*to = *from;

	if ( msg->bit == 0 ) {
		startBit = msg->readcount * 8 - GENTITYNUM_BITS;
	} else {
		startBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
	}

	// shownet 2/3 will interleave with other printed info, -2 will
	// just print the delta records
	if ( cl_shownet->integer >= 2 || cl_shownet->integer == -2 ) {
		print = 1;
		Com_Printf( "%3i: playerstate ", msg->readcount );
	} else {
		print = 0;
	}

	numFields = sizeof( playerStateFields ) / sizeof( playerStateFields[0] );
	lc = MSG_ReadByte(msg);
//Com_DPrintf( "===\nMSG_ReadDeltaPlayerstate: count %i\n===\n", lc );
	for ( i = 0, field = playerStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
//Com_DPrintf( "field %s ", field->name );
		if ( ! MSG_ReadBits( msg, 1 ) ) {
			// no change
			*toF = *fromF;
//Com_DPrintf( "NO CHANGE " );
//Com_DPrintf( "value int %i, float %f\n", *toF, *(float *)toF );
		} else {
//Com_DPrintf( "type %i, ", field->type );
			switch ( field->type ) {
				case 0:
					if ( field->bits == 0 ) {
						// float
						if ( MSG_ReadBits( msg, 1 ) == 0 ) {
							// integral float
							trunc = MSG_ReadBits( msg, FLOAT_INT_BITS );
							// bias to allow equal parts positive and negative
							trunc -= FLOAT_INT_BIAS;
							*(float *)toF = trunc; 
							if ( print ) {
								Com_Printf( "%s:%i ", field->name, trunc );
							}
						} else {
							// full floating point value
							*toF = MSG_ReadBits( msg, 32 );
							if ( print ) {
								Com_Printf( "%s:%f ", field->name, *(float *)toF );
							}
						}
					} else {
						// integer
						*toF = MSG_ReadBits( msg, field->bits );
						if ( print ) {
							Com_Printf( "%s:%i ", field->name, *toF );
						}
					}
					break;
				case 1:
					tmp = 1.0f;
					if ( field->bits < 0 ) {
						if ( MSG_ReadBits( msg, 1 ) )
							tmp = -1.0f;
						bits = - field->bits -1;
					}
					else bits = field->bits;

					result = MSG_ReadBits( msg, bits );
					if ( field->bits == 12 )
						*(float *)toF = result * 0.087890625f * tmp;
					else if ( field->bits == 8 )
						*(float *)toF = result * 1.411764705882353f * tmp;
					else if ( field->bits == 16 )
						*(float *)toF = result * 0.0054931640625f * tmp;
					else
						*(float *)toF = result * (1 << bits) * tmp / 360.0f;
					break;
				case 6:
					tmp = 1.0f;
					bits = MSG_ReadBits( msg, 19 );
					if ( bits & 262144 ) // test for 19th bit
						tmp = -1.0f;
					bits &= ~262144; // 4294705151;	// remove that bit
					*(float *)toF = tmp * bits / 16.0f;
					break;
				case 7:
					tmp = 1.0f;
					bits = MSG_ReadBits( msg, 17 );
					if ( bits & 65536 ) // test for 17th bit
						tmp = -1.0f;
					bits &= ~65536; // 4294901759;	// remove that bit
					*(float *)toF = tmp * bits / 8.0f;
					break;
				default:
					break;
			}
//Com_DPrintf( "value int %i, float %f\n", *toF, *(float *)toF );
		}
	}
	for ( i=lc,field = &playerStateFields[lc];i<numFields; i++, field++) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		// no change
		*toF = *fromF;
	}


	// read the arrays
	if (MSG_ReadBits( msg, 1 ) ) {
		// parse stats
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG("PS_STATS");
			bits = MSG_ReadLong(msg);
			for (i=0 ; i<MAX_STATS ; i++) {
				if (bits & (1<<i) ) {
					to->stats[i] = MSG_ReadShort(msg);
				}
			}
		}

		// parse activeItems
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG("PS_ITEMS");
			bits = MSG_ReadByte(msg);
			for (i=0 ; i<MAX_ACTIVEITEMS ; i++) {
				if (bits & (1<<i) ) {
					to->activeItems[i] = MSG_ReadShort(msg);
				}
			}
		}

		// parse ammo_amount
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG("PS_AMMO_AMOUNT");
			bits = MSG_ReadShort(msg);
			for (i=0 ; i<MAX_AMMO_AMOUNT ; i++) {
				if (bits & (1<<i) ) {
					to->ammo_amount[i] = MSG_ReadShort(msg);
				}
			}
		}

		// parse ammo_name_index
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG("PS_AMMO");
			bits = MSG_ReadShort(msg);
			for (i=0 ; i<MAX_AMMO ; i++) {
				if (bits & (1<<i) ) {
					to->ammo_name_index[i] = MSG_ReadShort(msg);
				}
			}
		}

		// parse powerups
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG("PS_MAX_AMMO_AMOUNT");
			bits = MSG_ReadShort(msg);
			for (i=0 ; i<MAX_MAX_AMMO_AMOUNT ; i++) {
				if (bits & (1<<i) ) {
					to->max_ammo_amount[i] = MSG_ReadShort(msg);
				}
			}
		}
	}

	if ( print ) {
		if ( msg->bit == 0 ) {
			endBit = msg->readcount * 8 - GENTITYNUM_BITS;
		} else {
			endBit = ( msg->readcount - 1 ) * 8 + msg->bit - GENTITYNUM_BITS;
		}
		Com_Printf( " (%i bits)\n", endBit - startBit  );
	}
}

int msg_hData[256] = {
250315,			// 0
41193,			// 1
6292,			// 2
7106,			// 3
3730,			// 4
3750,			// 5
6110,			// 6
23283,			// 7
33317,			// 8
6950,			// 9
7838,			// 10
9714,			// 11
9257,			// 12
17259,			// 13
3949,			// 14
1778,			// 15
8288,			// 16
1604,			// 17
1590,			// 18
1663,			// 19
1100,			// 20
1213,			// 21
1238,			// 22
1134,			// 23
1749,			// 24
1059,			// 25
1246,			// 26
1149,			// 27
1273,			// 28
4486,			// 29
2805,			// 30
3472,			// 31
21819,			// 32
1159,			// 33
1670,			// 34
1066,			// 35
1043,			// 36
1012,			// 37
1053,			// 38
1070,			// 39
1726,			// 40
888,			// 41
1180,			// 42
850,			// 43
960,			// 44
780,			// 45
1752,			// 46
3296,			// 47
10630,			// 48
4514,			// 49
5881,			// 50
2685,			// 51
4650,			// 52
3837,			// 53
2093,			// 54
1867,			// 55
2584,			// 56
1949,			// 57
1972,			// 58
940,			// 59
1134,			// 60
1788,			// 61
1670,			// 62
1206,			// 63
5719,			// 64
6128,			// 65
7222,			// 66
6654,			// 67
3710,			// 68
3795,			// 69
1492,			// 70
1524,			// 71
2215,			// 72
1140,			// 73
1355,			// 74
971,			// 75
2180,			// 76
1248,			// 77
1328,			// 78
1195,			// 79
1770,			// 80
1078,			// 81
1264,			// 82
1266,			// 83
1168,			// 84
965,			// 85
1155,			// 86
1186,			// 87
1347,			// 88
1228,			// 89
1529,			// 90
1600,			// 91
2617,			// 92
2048,			// 93
2546,			// 94
3275,			// 95
2410,			// 96
3585,			// 97
2504,			// 98
2800,			// 99
2675,			// 100
6146,			// 101
3663,			// 102
2840,			// 103
14253,			// 104
3164,			// 105
2221,			// 106
1687,			// 107
3208,			// 108
2739,			// 109
3512,			// 110
4796,			// 111
4091,			// 112
3515,			// 113
5288,			// 114
4016,			// 115
7937,			// 116
6031,			// 117
5360,			// 118
3924,			// 119
4892,			// 120
3743,			// 121
4566,			// 122
4807,			// 123
5852,			// 124
6400,			// 125
6225,			// 126
8291,			// 127
23243,			// 128
7838,			// 129
7073,			// 130
8935,			// 131
5437,			// 132
4483,			// 133
3641,			// 134
5256,			// 135
5312,			// 136
5328,			// 137
5370,			// 138
3492,			// 139
2458,			// 140
1694,			// 141
1821,			// 142
2121,			// 143
1916,			// 144
1149,			// 145
1516,			// 146
1367,			// 147
1236,			// 148
1029,			// 149
1258,			// 150
1104,			// 151
1245,			// 152
1006,			// 153
1149,			// 154
1025,			// 155
1241,			// 156
952,			// 157
1287,			// 158
997,			// 159
1713,			// 160
1009,			// 161
1187,			// 162
879,			// 163
1099,			// 164
929,			// 165
1078,			// 166
951,			// 167
1656,			// 168
930,			// 169
1153,			// 170
1030,			// 171
1262,			// 172
1062,			// 173
1214,			// 174
1060,			// 175
1621,			// 176
930,			// 177
1106,			// 178
912,			// 179
1034,			// 180
892,			// 181
1158,			// 182
990,			// 183
1175,			// 184
850,			// 185
1121,			// 186
903,			// 187
1087,			// 188
920,			// 189
1144,			// 190
1056,			// 191
3462,			// 192
2240,			// 193
4397,			// 194
12136,			// 195
7758,			// 196
1345,			// 197
1307,			// 198
3278,			// 199
1950,			// 200
886,			// 201
1023,			// 202
1112,			// 203
1077,			// 204
1042,			// 205
1061,			// 206
1071,			// 207
1484,			// 208
1001,			// 209
1096,			// 210
915,			// 211
1052,			// 212
995,			// 213
1070,			// 214
876,			// 215
1111,			// 216
851,			// 217
1059,			// 218
805,			// 219
1112,			// 220
923,			// 221
1103,			// 222
817,			// 223
1899,			// 224
1872,			// 225
976,			// 226
841,			// 227
1127,			// 228
956,			// 229
1159,			// 230
950,			// 231
7791,			// 232
954,			// 233
1289,			// 234
933,			// 235
1127,			// 236
3207,			// 237
1020,			// 238
927,			// 239
1355,			// 240
768,			// 241
1040,			// 242
745,			// 243
952,			// 244
805,			// 245
1073,			// 246
740,			// 247
1013,			// 248
805,			// 249
1008,			// 250
796,			// 251
996,			// 252
1057,			// 253
11457,			// 254
13504,			// 255
};

void MSG_initHuffman( void ) {
	int i,j;

	msgInit = qtrue;
	Huff_Init(&msgHuff);
	for(i=0;i<256;i++) {
		for (j=0;j<msg_hData[i];j++) {
			Huff_addRef(&msgHuff.compressor,	(byte)i);			// Do update
			Huff_addRef(&msgHuff.decompressor,	(byte)i);			// Do update
		}
	}
}

/*
void MSG_NUinitHuffman() {
	byte	*data;
	int		size, i, ch;
	int		array[256];

	msgInit = qtrue;

	Huff_Init(&msgHuff);
	// load it in
	size = FS_ReadFile( "netchan/netchan.bin", (void **)&data );

	for(i=0;i<256;i++) {
		array[i] = 0;
	}
	for(i=0;i<size;i++) {
		ch = data[i];
		Huff_addRef(&msgHuff.compressor,	ch);			// Do update
		Huff_addRef(&msgHuff.decompressor,	ch);			// Do update
		array[ch]++;
	}
	Com_Printf("msg_hData {\n");
	for(i=0;i<256;i++) {
		if (array[i] == 0) {
			Huff_addRef(&msgHuff.compressor,	i);			// Do update
			Huff_addRef(&msgHuff.decompressor,	i);			// Do update
		}
		Com_Printf("%d,			// %d\n", array[i], i);
	}
	Com_Printf("};\n");
	FS_FreeFile( data );
	Cbuf_AddText( "condump dump.txt\n" );
}
*/

//===========================================================================
