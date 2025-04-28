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

huffman_t msgHuff;

qboolean msgInit = qfalse;

int oldsize = 0;

//===================
// TA stuff
//===================

static constexpr int MAX_PACKED_COORD = 65536;
static constexpr int MAX_PACKED_COORD_HALF = MAX_PACKED_COORD / 2;
static constexpr int MAX_PACKED_COORD_EXTRA = 262144;
static constexpr int MAX_PACKED_COORD_EXTRA_HALF = MAX_PACKED_COORD_EXTRA / 2;

//===
// Statistics for changes reporting
//===
int strstats[256];
int huffstats[256];
int weightstats[256];
int scalestats[1024];
int coordextrastats[MAX_PACKED_COORD_EXTRA];
int iPlayerFieldChanges[256];
int timestats[32768];
int coordstats[MAX_PACKED_COORD];
int iEntityFieldChanges[256];
int alphastats[257];

// Scrambled string conversion (write)
const uint8_t StrCharToNetByte[256] =
{
	254, 120, 89, 13, 27, 73, 103, 78, 74, 102, 21, 117, 76, 86, 238, 96, 88, 62, 59, 60,
	40, 84, 52, 119, 251, 51, 75, 121, 192, 85, 44, 54, 114, 87, 25, 53, 35, 224, 67, 31,
	82, 41, 45, 99, 233, 112, 255, 11, 46, 115, 8, 32, 19, 100, 110, 95, 116, 48, 58, 107,
	70, 91, 104, 81, 118, 109, 36, 24, 17, 39, 43, 65, 49, 83, 56, 57, 33, 64, 80, 28,
	184, 160, 18, 105, 42, 20, 194, 38, 29, 26, 61, 50, 9, 90, 37, 128, 79, 2, 108, 34,
	4, 0, 47, 12, 101, 10, 92, 15, 5, 7, 22, 55, 23, 14, 3, 1, 66, 16, 63, 30,
	6, 97, 111, 248, 72, 197, 191, 122, 176, 245, 250, 68, 195, 77, 232, 106, 228, 93, 240, 98,
	208, 69, 164, 144, 186, 222, 94, 246, 148, 170, 244, 190, 205, 234, 252, 202, 230, 239, 174, 225,
	226, 209, 236, 216, 237, 151, 149, 231, 129, 188, 200, 172, 204, 154, 168, 71, 133, 217, 196, 223,
	134, 253, 173, 177, 219, 235, 214, 182, 132, 227, 183, 175, 137, 152, 158, 221, 243, 150, 210, 136,
	167, 211, 179, 193, 218, 124, 140, 178, 213, 249, 185, 113, 127, 220, 180, 145, 138, 198, 123, 162,
	189, 203, 166, 126, 159, 156, 212, 207, 146, 181, 247, 139, 142, 169, 242, 241, 171, 187, 153, 135,
	201, 155, 161, 125, 163, 130, 229, 206, 165, 157, 141, 147, 143, 199, 215, 131
};

// Scrambled string conversion (read)
const uint8_t NetByteToStrChar[256] =
{
	101, 115, 97, 114, 100, 108, 120, 109, 50, 92, 105, 47, 103, 3, 113, 107, 117, 68, 82, 52,
	85, 10, 110, 112, 67, 34, 89, 4, 79, 88, 119, 39, 51, 76, 99, 36, 66, 94, 87, 69,
	20, 41, 84, 70, 30, 42, 48, 102, 57, 72, 91, 25, 22, 35, 31, 111, 74, 75, 58, 18,
	19, 90, 17, 118, 77, 71, 116, 38, 131, 141, 60, 175, 124, 5, 8, 26, 12, 133, 7, 96,
	78, 63, 40, 73, 21, 29, 13, 33, 16, 2, 93, 61, 106, 137, 146, 55, 15, 121, 139, 43,
	53, 104, 9, 6, 62, 83, 135, 59, 98, 65, 54, 122, 45, 211, 32, 49, 56, 11, 64, 23,
	1, 27, 127, 218, 205, 243, 223, 212, 95, 168, 245, 255, 188, 176, 180, 239, 199, 192, 216, 231,
	206, 250, 232, 252, 143, 215, 228, 251, 148, 166, 197, 165, 193, 238, 173, 241, 225, 249, 194, 224,
	81, 242, 219, 244, 142, 248, 222, 200, 174, 233, 149, 236, 171, 182, 158, 191, 128, 183, 207, 202,
	214, 229, 187, 190, 80, 210, 144, 237, 169, 220, 151, 126, 28, 203, 86, 132, 178, 125, 217, 253,
	170, 240, 155, 221, 172, 152, 247, 227, 140, 161, 198, 201, 226, 208, 186, 254, 163, 177, 204, 184,
	213, 195, 145, 179, 37, 159, 160, 189, 136, 246, 156, 167, 134, 44, 153, 185, 162, 164, 14, 157,
	138, 235, 234, 196, 150, 129, 147, 230, 123, 209, 130, 24, 154, 181, 0, 46
};

/*
==============================================================================

			MESSAGE IO FUNCTIONS

Handles byte ordering and avoids alignment errors
==============================================================================
*/

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

qboolean MSG_IsProtocolVersion15()
{
	return com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN;
}

/*
=============================================================================

bit functions
  
=============================================================================
*/

int	overflows;

int MSG_WriteNegateValue_ver_15(int value, int bits)
{
	if (value >= 0) {
		value <<= 1;
	}
	else {
		value = (~value << 1) | 1;
	}

	return value;
}

int MSG_ReadNegateValue_ver_15(int value, int bits)
{
	if (value & 1) {
		value = ~(value >> 1);
	}
	else {
		value >>= 1;
	}

	return value;
}

int MSG_WriteNegateValue_ver_6(int value, int bits)
{
	return value;
}

int MSG_ReadNegateValue_ver_6(int value, int bits)
{
	if (value & (1 << (bits - 1))) {
		value |= -1 ^ ((1 << bits) - 1);
	}

	return value;
}

int MSG_WriteNegateValue(int value, int bits)
{
    if (MSG_IsProtocolVersion15())
    {
        return MSG_WriteNegateValue_ver_15(value, bits);
    }
    else
    {
        return MSG_WriteNegateValue_ver_6(value, bits);
    }
}

int MSG_ReadNegateValue(int value, int bits)
{
	if (MSG_IsProtocolVersion15())
	{
		return MSG_ReadNegateValue_ver_15(value, bits);
	}
	else
    {
        return MSG_ReadNegateValue_ver_6(value, bits);
	}
}

// negative bit values include signs
void MSG_WriteBits( msg_t *msg, int value, int bits ) {
	int	i;

	oldsize += bits;

	if ( msg->overflowed ) {
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
		value = MSG_WriteNegateValue(value, bits);
	}

	if (msg->oob) {
		if (msg->cursize + (bits >> 3) > msg->maxsize) {
			msg->overflowed = qtrue;
			return;
		}

		if (bits == 8) {
			msg->data[msg->cursize] = value;
			msg->cursize += 1;
			msg->bit += 8;
		}
		else if (bits == 16) {
			short temp = value;

			CopyLittleShort(&msg->data[msg->cursize], &temp);
			msg->cursize += 2;
			msg->bit += 16;
		}
		else if (bits == 32) {
			CopyLittleLong(&msg->data[msg->cursize], &value);
			msg->cursize += 4;
			msg->bit += 32;
		}
		else {
			Com_Error(ERR_DROP, "can't write %d bits", bits);
		}
	} else {
		value &= (0xffffffff>>(32-bits));
		if (bits&7) {
			int nbits;
			nbits = bits&7;
			if ( msg->bit + nbits >= msg->maxsize << 3 ) {
				msg->overflowed = qtrue;
				return;
			}
			for(i=0;i<nbits;i++) {
				Huff_putBit((value&1), msg->data, &msg->bit);
				value = (value>>1);
			}
			bits = bits - nbits;
		}
		if (bits) {
			for(i=0;i<bits;i+=8) {
				Huff_offsetTransmit( &msgHuff.compressor, (value & 0xff), msg->data, &msg->bit, msg->maxsize << 3 );
				value = (value>>8);

				if ( msg->bit >= msg->maxsize << 3 ) {
					msg->overflowed = qtrue;
					return;
				}
			}
		}
		msg->cursize = (msg->bit>>3)+1;
	}
}

int MSG_ReadBits( msg_t *msg, int bits ) {
	int			value;
	int			get;
	qboolean	sgn;
	int			i, nbits;

	if ( msg->readcount > msg->cursize ) {
		return 0;
	}

	value = 0;

	if ( bits < 0 ) {
		bits = -bits;
		sgn = qtrue;
	} else {
		sgn = qfalse;
	}

	if (msg->oob) {
		if (msg->readcount + (bits >> 3) > msg->cursize) {
			msg->readcount = msg->cursize + 1;
			return 0;
		}

		if (bits == 8)
		{
			value = msg->data[msg->readcount];
			msg->readcount += 1;
			msg->bit += 8;
		}
		else if (bits == 16)
		{
			short temp;

			CopyLittleShort(&temp, &msg->data[msg->readcount]);
			value = temp;
			msg->readcount += 2;
			msg->bit += 16;
		}
		else if (bits == 32)
		{
			CopyLittleLong(&value, &msg->data[msg->readcount]);
			msg->readcount += 4;
			msg->bit += 32;
		}
		else
			Com_Error(ERR_DROP, "can't read %d bits", bits);
	} else {
		nbits = 0;
		if (bits&7) {
			nbits = bits&7;
			if (msg->bit + nbits > msg->cursize << 3) {
				msg->readcount = msg->cursize + 1;
				return 0;
			}
			for(i=0;i<nbits;i++) {
				value |= (Huff_getBit(msg->data, &msg->bit)<<i);
			}
			bits = bits - nbits;
		}
		if (bits) {
			for(i=0;i<bits;i+=8) {
				Huff_offsetReceive (msgHuff.decompressor.tree, &get, msg->data, &msg->bit, msg->cursize<<3);
				value |= (get<<(i+nbits));

				if (msg->bit > msg->cursize<<3) {
					msg->readcount = msg->cursize + 1;
					return 0;
				}
			}
		}
		msg->readcount = (msg->bit>>3)+1;
	}

	if (sgn) {
		value = MSG_ReadNegateValue(value, bits);
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
	if (!s) {
		MSG_WriteByte(sb, 0);
	} else {
		size_t	l;
		int		i;
		char	string[MAX_STRING_CHARS];

		l = strlen( s );
		if ( l >= MAX_STRING_CHARS ) {
			Com_Printf("MSG_WriteString: MAX_STRING_CHARS");
			MSG_WriteByte(sb, 0);
			return;
		}
		Q_strncpyz( string, s, sizeof( string ) );

		for ( i = 0 ; i <= l ; i++ ) {
			MSG_WriteByte(sb, string[i]);
		}
	}
}

void MSG_WriteBigString( msg_t *sb, const char *s ) {
	if ( !s ) {
		MSG_WriteByte(sb, 0);
	} else {
		size_t	l;
		int		i;
		char	string[BIG_INFO_STRING];

		l = strlen( s );
		if ( l >= BIG_INFO_STRING ) {
			Com_Printf("MSG_WriteString: BIG_INFO_STRING");
			MSG_WriteByte(sb, 0);
			return;
		}
		Q_strncpyz( string, s, sizeof( string ) );

		for ( i = 0 ; i <= l ; i++ ) {
			MSG_WriteByte(sb, string[i]);
		}
	}
}

void MSG_WriteScrambledString_ver_15(msg_t* sb, const char* s) {
	if (!s) {
		strstats[0]++;
		MSG_WriteByte(sb, StrCharToNetByte[0]);
	}
	else {
		size_t	l;
		int		i;
		char	string[MAX_STRING_CHARS];

		l = strlen(s);
		if (l >= MAX_STRING_CHARS) {
			strstats[0]++;
			Com_Printf("MSG_WriteString: MAX_STRING_CHARS");
			MSG_WriteByte(sb, StrCharToNetByte[0]);
			return;
		}
		Q_strncpyz(string, s, sizeof(string));

		for (i = 0; i <= l; i++) {
			unsigned char c = string[i];
			strstats[c]++;
			MSG_WriteByte(sb, StrCharToNetByte[c]);
		}
	}
}

void MSG_WriteScrambledBigString_ver_15(msg_t* sb, const char* s) {
	if (!s) {
		strstats[0]++;
		MSG_WriteByte(sb, StrCharToNetByte[0]);
	}
	else {
		size_t	l;
		int		i;
		char	string[BIG_INFO_STRING];

		l = strlen(s);
		if (l >= BIG_INFO_STRING) {
			strstats[0]++;
			Com_Printf("MSG_WriteString: BIG_INFO_STRING");
			MSG_WriteByte(sb, StrCharToNetByte[0]);
			return;
		}
		Q_strncpyz(string, s, sizeof(string));

		for (i = 0; i <= l; i++) {
			unsigned char c = string[i];
			strstats[c]++;
			MSG_WriteByte(sb, StrCharToNetByte[c]);
		}
	}
}

void MSG_WriteScrambledString_ver_6(msg_t* sb, const char* s) {
	return MSG_WriteString(sb, s);
}

void MSG_WriteScrambledBigString_ver_6(msg_t* sb, const char* s) {
	return MSG_WriteBigString(sb, s);
}

void MSG_WriteScrambledString(msg_t* sb, const char* s) {
	if (MSG_IsProtocolVersion15()) {
		return MSG_WriteScrambledString_ver_15(sb, s);
	} else {
        return MSG_WriteScrambledString_ver_6(sb, s);
	}
}

void MSG_WriteScrambledBigString(msg_t* sb, const char* s) {
	if (MSG_IsProtocolVersion15()) {
		return MSG_WriteScrambledBigString_ver_15(sb, s);
	} else {
        return MSG_WriteScrambledBigString_ver_6(sb, s);
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

char* MSG_ReadScrambledString_ver_15(msg_t* msg) {
	static char	string[MAX_STRING_CHARS];
	int l;
	int c;

	l = 0;
	do {
		c = MSG_ReadByte(msg);		// use ReadByte so -1 is out of bounds
		if (c == -1) {
			break;
		}

		c = NetByteToStrChar[c];
		if (!c) {
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if (c == '%') {
			c = '.';
		}

		string[l] = (char)c;
		l++;
	} while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}

char* MSG_ReadScrambledBigString_ver_15(msg_t* msg) {
	static char	string[BIG_INFO_STRING];
    int l;
    int c;

	l = 0;
	do {
		c = MSG_ReadByte(msg);		// use ReadByte so -1 is out of bounds
		if (c == -1) {
			break;
		}

		c = NetByteToStrChar[c];
		if (!c) {
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if (c == '%') {
			c = '.';
		}

		string[l] = (char)c;
		l++;
	} while (l < sizeof(string) - 1);

	string[l] = 0;

	return string;
}

char* MSG_ReadScrambledString_ver_6(msg_t* msg) {
	return MSG_ReadString(msg);
}

char* MSG_ReadScrambledBigString_ver_6(msg_t* msg) {
	return MSG_ReadBigString(msg);
}

char* MSG_ReadScrambledString(msg_t* msg) {
	if (MSG_IsProtocolVersion15()) {
		return MSG_ReadScrambledString_ver_15(msg);
    } else {
        return MSG_ReadScrambledString_ver_6(msg);
	}
}

char* MSG_ReadScrambledBigString(msg_t* msg) {
	if (MSG_IsProtocolVersion15()) {
		return MSG_ReadScrambledBigString_ver_15(msg);
    } else {
        return MSG_ReadScrambledBigString_ver_6(msg);
	}
}

char *MSG_ReadString( msg_t *msg ) {
	static char	string[MAX_STRING_CHARS];
	int l;
	int c;
	
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

		string[l] = (char)c;
		l++;
	} while (l < sizeof(string)-1);
	
	string[l] = 0;
	
	return string;
}

char *MSG_ReadBigString( msg_t *msg ) {
	static char	string[BIG_INFO_STRING];
	int l;
	int c;
	
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

		string[l] = (char)c;
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
		if (c == -1 || c == 0 || c == '\n') {
			break;
		}
		// translate all fmt spec to avoid crash bugs
		if ( c == '%' ) {
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
	nullState->constantLight = 0xffffff;
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

// a string hasher which gives the same hash value even if the
// string is later modified via the legacy MSG read/write code
int MSG_HashKey(const char *string, int maxlen) {
	int hash, i;

	hash = 0;
	for (i = 0; i < maxlen && string[i] != '\0'; i++) {
		hash += string[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	return hash;
}

/*
=============================================================================

delta functions
  
=============================================================================
*/

extern "C" cvar_t *cl_shownet;

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

unsigned int kbitmask[32] = {
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
		from->buttons == to->buttons)
	{
		// no change
		MSG_WriteBits(msg, 0, 1);
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
	} else {
		to->angles[0] = from->angles[0];
		to->angles[1] = from->angles[1];
		to->angles[2] = from->angles[2];
		to->forwardmove = from->forwardmove;
		to->rightmove = from->rightmove;
		to->upmove = from->upmove;
		to->buttons = from->buttons;
	}
}

void MSG_WriteDeltaEyeInfo(msg_t* msg, usereyes_t* from, usereyes_t* to) {

	if (to->angles[0] != from->angles[0] || to->angles[1] != from->angles[1] || to->ofs[0] != from->ofs[0] || to->ofs[1] != from->ofs[1] || to->ofs[2] != from->ofs[2]) {
		MSG_WriteBits(msg, 1, 1);
		MSG_WriteDelta(msg, from->ofs[0], to->ofs[0], 8);
		MSG_WriteDelta(msg, from->ofs[1], to->ofs[1], 8);
		MSG_WriteDelta(msg, from->ofs[2], to->ofs[2], 8);

		MSG_WriteDeltaFloat(msg, from->angles[0], to->angles[0]);
		MSG_WriteDeltaFloat(msg, from->angles[1], to->angles[1]);
	}
	else {
		MSG_WriteBits(msg, 0, 1);
		oldsize += 7;
	}
}

void MSG_ReadDeltaEyeInfo(msg_t* msg, usereyes_t* from, usereyes_t* to) {

	if (MSG_ReadBits(msg, 1)) {
		to->ofs[0] = MSG_ReadDelta(msg, from->ofs[0], 8);
		to->ofs[1] = MSG_ReadDelta(msg, from->ofs[1], 8);
		to->ofs[2] = MSG_ReadDelta(msg, from->ofs[2], 8);

		to->angles[0] = MSG_ReadDeltaFloat(msg, from->angles[0]);
		to->angles[1] = MSG_ReadDeltaFloat(msg, from->angles[1]);
	}
	else {
		to->angles[0] = from->angles[0];
		to->angles[1] = from->angles[1];

		to->ofs[0] = from->ofs[0];
		to->ofs[1] = from->ofs[1];
		to->ofs[2] = from->ofs[2];
	}

}

int compare_huffstats(const int* e1, const int* e2)
{
	return huffstats[*e2] - huffstats[*e1];
}

int compare_strstats(const int* e1, const int* e2)
{
	return strstats[*e2] - strstats[*e1];
}

void MSG_WriteDeltaCoord(msg_t* msg, int from, int to)
{
	int delta = to - from;
	int deltaAbs = abs(delta);

	if (deltaAbs <= 0 || deltaAbs > 128)
	{
		// high delta, direct value
		MSG_WriteBits(msg, 0, 1);
		MSG_WriteBits(msg, to, 16);
	}
	else
	{
		MSG_WriteBits(msg, 1, 1);

		if (delta < 0) {
			MSG_WriteBits(msg, 1 + ((deltaAbs - 1) << 1), 8);
		}
		else {
			MSG_WriteBits(msg, (deltaAbs - 1) << 1, 8);
		}
	}
}

int MSG_ReadDeltaCoord(msg_t* msg, int from)
{
	int value;
	int delta;

	if (!MSG_ReadBits(msg, 1))
	{
		// no delta
		return MSG_ReadBits(msg, 16);
	}

	value = MSG_ReadBits(msg, 8);
	delta = (value >> 1) + 1;

	if ((value & 1) != 0) {
		delta = -delta;
	}

	return delta + from;
}

void MSG_WriteDeltaCoordExtra(msg_t* msg, int from, int to)
{
	int delta = to - from;
	int deltaAbs = abs(delta);

	if (deltaAbs <= 0 || deltaAbs > 512)
	{
		// high delta, direct value
		MSG_WriteBits(msg, 0, 1);
		MSG_WriteBits(msg, to, 18);
	}
	else
	{
		MSG_WriteBits(msg, 1, 1);

		if (delta < 0) {
			MSG_WriteBits(msg, 1 + ((deltaAbs - 1) << 1), 10);
		}
		else {
			MSG_WriteBits(msg, (deltaAbs - 1) << 1, 10);
		}
	}
}

int MSG_ReadDeltaCoordExtra(msg_t* msg, int from)
{
	int value;
	int delta;

	if (!MSG_ReadBits(msg, 1))
	{
		// no delta
		return MSG_ReadBits(msg, 18);
	}

	value = MSG_ReadBits(msg, 10);
	delta = (value >> 1) + 1;

	if ((value & 1) != 0) {
		delta = -delta;
	}

	return delta + from;
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
		if (iEntityFieldChanges[i]) {
			Com_Printf("%d used %d\n", i, iEntityFieldChanges[i]);
		}
	}
}

typedef enum netFieldType_e {
	regular,
	angle,
	animTime,
	animWeight,
	scale,
	alpha,
	coord,
	// This field was introduced in TA.
	coordExtra,
	velocity,
	// not sure what is this, but it's only present in the Mac build (since AA)
	simple
} netFieldType_t;

typedef struct {
	const char *name;
	size_t offset;
	size_t size;
	// bits: 0 = float
	int bits;
	int type;
} netField_t;

byte* LittleOffset(void* value, size_t size, size_t targetSize) {
#ifdef Q3_BIG_ENDIAN
	return (byte*)value + size - targetSize;
#else
	return (byte*)value;
#endif
}

void CopyToLittleField(void* toF, const void* value, size_t size, size_t targetSize) {
	Com_Memcpy(toF, LittleOffset((byte*)value, size, targetSize), targetSize);
}

void CopyFromLittleField(void* value, const void* fromF, size_t size, size_t targetSize) {
	Com_Memcpy(LittleOffset((byte*)value, size, targetSize), fromF, targetSize);
}

// using the stringizing operator to save typing...
#define	NETF(x) #x,(size_t)&((entityState_t*)0)->x,sizeof(entityState_t::x)

netField_t	entityStateFields_ver_15[] =
{
{ NETF(netorigin[0]), 0, netFieldType_t::coord },
{ NETF(netorigin[1]), 0, netFieldType_t::coord },
{ NETF(netangles[1]), 12, netFieldType_t::angle },
{ NETF(frameInfo[0].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[1].time), 15, netFieldType_t::animTime },
{ NETF(bone_angles[0][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[1][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][0]), -13, netFieldType_t::angle },
{ NETF(netorigin[2]), 0, netFieldType_t::coord },
{ NETF(frameInfo[0].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[1].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[2].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[3].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[0].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[1].index), 12, netFieldType_t::regular },
{ NETF(actionWeight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[2].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[3].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[2].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[3].index), 12, netFieldType_t::regular },
{ NETF(eType), 8, netFieldType_t::regular },
{ NETF(modelindex), 16, netFieldType_t::regular },
{ NETF(parent), 16, netFieldType_t::regular },
{ NETF(constantLight), 32, netFieldType_t::regular },
{ NETF(renderfx), 32, netFieldType_t::regular },
{ NETF(bone_tag[0]), -8, netFieldType_t::regular },
{ NETF(bone_tag[1]), -8, netFieldType_t::regular },
{ NETF(bone_tag[2]), -8, netFieldType_t::regular },
{ NETF(bone_tag[3]), -8, netFieldType_t::regular },
{ NETF(bone_tag[4]), -8, netFieldType_t::regular },
{ NETF(scale), 10, netFieldType_t::scale },
{ NETF(alpha), 8, netFieldType_t::alpha },
{ NETF(usageIndex), 16, netFieldType_t::regular },
{ NETF(eFlags), 16, netFieldType_t::regular },
{ NETF(solid), 32, netFieldType_t::regular },
{ NETF(netangles[2]), 12, netFieldType_t::angle },
{ NETF(netangles[0]), 12, netFieldType_t::angle },
{ NETF(tag_num), 10, netFieldType_t::regular },
{ NETF(bone_angles[1][2]), -13, netFieldType_t::angle },
{ NETF(attach_use_angles), 1, netFieldType_t::regular },
{ NETF(origin2[1]), 0, netFieldType_t::coord },
{ NETF(origin2[0]), 0, netFieldType_t::coord },
{ NETF(origin2[2]), 0, netFieldType_t::coord },
{ NETF(bone_angles[0][2]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][2]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][2]), -13, netFieldType_t::angle },
{ NETF(surfaces[0]), 8, netFieldType_t::regular },
{ NETF(surfaces[1]), 8, netFieldType_t::regular },
{ NETF(surfaces[2]), 8, netFieldType_t::regular },
{ NETF(surfaces[3]), 8, netFieldType_t::regular },
{ NETF(bone_angles[0][1]), -13, netFieldType_t::angle },
{ NETF(surfaces[4]), 8, netFieldType_t::regular },
{ NETF(surfaces[5]), 8, netFieldType_t::regular },
{ NETF(pos.trTime), 32, netFieldType_t::regular },
{ NETF(pos.trDelta[0]), 0, netFieldType_t::velocity },
{ NETF(pos.trDelta[1]), 0, netFieldType_t::velocity },
{ NETF(pos.trDelta[2]), 0, netFieldType_t::velocity },
{ NETF(loopSound), 16, netFieldType_t::regular },
{ NETF(loopSoundVolume), 0, netFieldType_t::regular },
{ NETF(loopSoundMinDist), 0, netFieldType_t::regular },
{ NETF(loopSoundMaxDist), 0, netFieldType_t::regular },
{ NETF(loopSoundPitch), 0, netFieldType_t::regular },
{ NETF(loopSoundFlags), 8, netFieldType_t::regular },
{ NETF(attach_offset[0]), 0, netFieldType_t::regular },
{ NETF(attach_offset[1]), 0, netFieldType_t::regular },
{ NETF(attach_offset[2]), 0, netFieldType_t::regular },
{ NETF(beam_entnum), 16, netFieldType_t::regular },
{ NETF(skinNum), 16, netFieldType_t::regular },
{ NETF(wasframe), 10, netFieldType_t::regular },
{ NETF(frameInfo[4].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[5].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[6].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[7].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[8].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[9].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[10].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[11].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[12].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[13].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[14].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[15].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[4].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[5].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[6].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[7].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[8].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[9].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[10].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[11].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[12].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[13].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[14].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[15].time), 15, netFieldType_t::animTime },
{ NETF(frameInfo[4].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[5].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[6].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[7].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[8].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[9].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[10].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[11].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[12].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[13].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[14].weight), 8, netFieldType_t::animWeight },
{ NETF(frameInfo[15].weight), 8, netFieldType_t::animWeight },
{ NETF(bone_angles[1][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][2]), -13, netFieldType_t::angle },
{ NETF(clientNum), 8, netFieldType_t::regular },
{ NETF(groundEntityNum), GENTITYNUM_BITS, netFieldType_t::regular },
{ NETF(shader_data[0]), 0, netFieldType_t::regular },
{ NETF(shader_data[1]), 0, netFieldType_t::regular },
{ NETF(shader_time), 0, netFieldType_t::regular },
{ NETF(eyeVector[0]), 0, netFieldType_t::regular },
{ NETF(eyeVector[1]), 0, netFieldType_t::regular },
{ NETF(eyeVector[2]), 0, netFieldType_t::regular },
{ NETF(surfaces[6]), 8, netFieldType_t::regular },
{ NETF(surfaces[7]), 8, netFieldType_t::regular },
{ NETF(surfaces[8]), 8, netFieldType_t::regular },
{ NETF(surfaces[9]), 8, netFieldType_t::regular },
{ NETF(surfaces[10]), 8, netFieldType_t::regular },
{ NETF(surfaces[11]), 8, netFieldType_t::regular },
{ NETF(surfaces[12]), 8, netFieldType_t::regular },
{ NETF(surfaces[13]), 8, netFieldType_t::regular },
{ NETF(surfaces[14]), 8, netFieldType_t::regular },
{ NETF(surfaces[15]), 8, netFieldType_t::regular },
{ NETF(surfaces[16]), 8, netFieldType_t::regular },
{ NETF(surfaces[17]), 8, netFieldType_t::regular },
{ NETF(surfaces[18]), 8, netFieldType_t::regular },
{ NETF(surfaces[19]), 8, netFieldType_t::regular },
{ NETF(surfaces[20]), 8, netFieldType_t::regular },
{ NETF(surfaces[21]), 8, netFieldType_t::regular },
{ NETF(surfaces[22]), 8, netFieldType_t::regular },
{ NETF(surfaces[23]), 8, netFieldType_t::regular },
{ NETF(surfaces[24]), 8, netFieldType_t::regular },
{ NETF(surfaces[25]), 8, netFieldType_t::regular },
{ NETF(surfaces[26]), 8, netFieldType_t::regular },
{ NETF(surfaces[27]), 8, netFieldType_t::regular },
{ NETF(surfaces[28]), 8, netFieldType_t::regular },
{ NETF(surfaces[29]), 8, netFieldType_t::regular },
{ NETF(surfaces[30]), 8, netFieldType_t::regular },
{ NETF(surfaces[31]), 8, netFieldType_t::regular }
};
static constexpr unsigned long numEntityStateFields_ver_15 = sizeof(entityStateFields_ver_15) / sizeof(entityStateFields_ver_15[0]);

//
// Protocol version 6
//
netField_t	entityStateFields_ver_6[] = 
{
{ NETF(netorigin[0]), 0, netFieldType_t::coord },
{ NETF(netorigin[1]), 0, netFieldType_t::coord },
{ NETF(netangles[1]), 12, netFieldType_t::angle },
{ NETF(frameInfo[0].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[1].time), 0, netFieldType_t::animTime },
{ NETF(bone_angles[0][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[1][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][0]), -13, netFieldType_t::angle },
{ NETF(netorigin[2]), 0, netFieldType_t::coord },
{ NETF(frameInfo[0].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[1].weight), 0, netFieldType_t::animWeight},
{ NETF(frameInfo[2].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[3].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[0].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[1].index), 12, netFieldType_t::regular },
{ NETF(actionWeight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[2].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[3].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[2].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[3].index), 12, netFieldType_t::regular },
{ NETF(eType), 8, netFieldType_t::regular },
{ NETF(modelindex), 16, netFieldType_t::regular },
{ NETF(parent), 16, netFieldType_t::regular },
{ NETF(constantLight), 32, netFieldType_t::regular },
{ NETF(renderfx), 32, netFieldType_t::regular },
{ NETF(bone_tag[0]), -8, netFieldType_t::regular },
{ NETF(bone_tag[1]), -8, netFieldType_t::regular },
{ NETF(bone_tag[2]), -8, netFieldType_t::regular },
{ NETF(bone_tag[3]), -8, netFieldType_t::regular },
{ NETF(bone_tag[4]), -8, netFieldType_t::regular },
{ NETF(scale), 0, netFieldType_t::scale },
{ NETF(alpha), 0, netFieldType_t::alpha },
{ NETF(usageIndex), 16, netFieldType_t::regular },
{ NETF(eFlags), 16, netFieldType_t::regular },
{ NETF(solid), 32, netFieldType_t::regular },
{ NETF(netangles[2]), 12, netFieldType_t::angle },
{ NETF(netangles[0]), 12, netFieldType_t::angle },
{ NETF(tag_num), 10, netFieldType_t::regular },
{ NETF(bone_angles[1][2]), -13, netFieldType_t::angle },
{ NETF(attach_use_angles), 1, netFieldType_t::regular },
{ NETF(origin2[1]), 0, netFieldType_t::coord },
{ NETF(origin2[0]), 0, netFieldType_t::coord },
{ NETF(origin2[2]), 0, netFieldType_t::coord },
{ NETF(bone_angles[0][2]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][2]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][2]), -13, netFieldType_t::angle },
{ NETF(surfaces[0]), 8, netFieldType_t::regular },
{ NETF(surfaces[1]), 8, netFieldType_t::regular },
{ NETF(surfaces[2]), 8, netFieldType_t::regular },
{ NETF(surfaces[3]), 8, netFieldType_t::regular },
{ NETF(bone_angles[0][1]), -13, netFieldType_t::angle },
{ NETF(surfaces[4]), 8, netFieldType_t::regular },
{ NETF(surfaces[5]), 8, netFieldType_t::regular },
{ NETF(loopSound), 16, netFieldType_t::regular },
{ NETF(loopSoundVolume), 0, netFieldType_t::regular },
{ NETF(loopSoundMinDist), 0, netFieldType_t::regular },
{ NETF(loopSoundMaxDist), 0, netFieldType_t::regular },
{ NETF(loopSoundPitch), 0, netFieldType_t::regular },
{ NETF(loopSoundFlags), 8, netFieldType_t::regular },
{ NETF(attach_offset[0]), 0, netFieldType_t::regular },
{ NETF(attach_offset[1]), 0, netFieldType_t::regular },
{ NETF(attach_offset[2]), 0, netFieldType_t::regular },
{ NETF(beam_entnum), 16, netFieldType_t::regular },
{ NETF(skinNum), 16, netFieldType_t::regular },
{ NETF(wasframe), 10, netFieldType_t::regular },
{ NETF(frameInfo[4].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[5].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[6].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[7].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[8].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[9].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[10].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[11].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[12].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[13].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[14].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[15].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[4].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[5].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[6].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[7].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[8].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[9].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[10].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[11].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[12].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[13].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[14].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[15].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[4].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[5].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[6].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[7].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[8].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[9].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[10].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[11].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[12].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[13].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[14].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[15].weight), 0, netFieldType_t::animWeight },
{ NETF(bone_angles[1][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][2]), -13, netFieldType_t::angle },
{ NETF(clientNum), 8, netFieldType_t::regular },
{ NETF(groundEntityNum), GENTITYNUM_BITS, netFieldType_t::regular },
{ NETF(shader_data[0]), 0, netFieldType_t::regular },
{ NETF(shader_data[1]), 0, netFieldType_t::regular },
{ NETF(shader_time), 0, netFieldType_t::regular },
{ NETF(eyeVector[0]), 0, netFieldType_t::regular },
{ NETF(eyeVector[1]), 0, netFieldType_t::regular },
{ NETF(eyeVector[2]), 0, netFieldType_t::regular },
{ NETF(surfaces[6]), 8, netFieldType_t::regular },
{ NETF(surfaces[7]), 8, netFieldType_t::regular },
{ NETF(surfaces[8]), 8, netFieldType_t::regular },
{ NETF(surfaces[9]), 8, netFieldType_t::regular },
{ NETF(surfaces[10]), 8, netFieldType_t::regular },
{ NETF(surfaces[11]), 8, netFieldType_t::regular },
{ NETF(surfaces[12]), 8, netFieldType_t::regular },
{ NETF(surfaces[13]), 8, netFieldType_t::regular },
{ NETF(surfaces[14]), 8, netFieldType_t::regular },
{ NETF(surfaces[15]), 8, netFieldType_t::regular },
{ NETF(surfaces[16]), 8, netFieldType_t::regular },
{ NETF(surfaces[17]), 8, netFieldType_t::regular },
{ NETF(surfaces[18]), 8, netFieldType_t::regular },
{ NETF(surfaces[19]), 8, netFieldType_t::regular },
{ NETF(surfaces[20]), 8, netFieldType_t::regular },
{ NETF(surfaces[21]), 8, netFieldType_t::regular },
{ NETF(surfaces[22]), 8, netFieldType_t::regular },
{ NETF(surfaces[23]), 8, netFieldType_t::regular },
{ NETF(surfaces[24]), 8, netFieldType_t::regular },
{ NETF(surfaces[25]), 8, netFieldType_t::regular },
{ NETF(surfaces[26]), 8, netFieldType_t::regular },
{ NETF(surfaces[27]), 8, netFieldType_t::regular },
{ NETF(surfaces[28]), 8, netFieldType_t::regular },
{ NETF(surfaces[29]), 8, netFieldType_t::regular },
{ NETF(surfaces[30]), 8, netFieldType_t::regular },
{ NETF(surfaces[31]), 8, netFieldType_t::regular }
};

//
// Protocol version 8
// It adds the following fields:
// - pos.trTime
// - pos.trDelta[0]
// - pos.trDelta[1]
// - pos.trDelta[2]
//
netField_t	entityStateFields_ver_8[] = 
{
{ NETF(netorigin[0]), 0, netFieldType_t::coord },
{ NETF(netorigin[1]), 0, netFieldType_t::coord },
{ NETF(netangles[1]), 12, netFieldType_t::angle },
{ NETF(frameInfo[0].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[1].time), 0, netFieldType_t::animTime },
{ NETF(bone_angles[0][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[1][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][0]), -13, netFieldType_t::angle },
{ NETF(netorigin[2]), 0, netFieldType_t::coord },
{ NETF(frameInfo[0].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[1].weight), 0, netFieldType_t::animWeight},
{ NETF(frameInfo[2].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[3].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[0].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[1].index), 12, netFieldType_t::regular },
{ NETF(actionWeight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[2].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[3].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[2].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[3].index), 12, netFieldType_t::regular },
{ NETF(eType), 8, netFieldType_t::regular },
{ NETF(modelindex), 16, netFieldType_t::regular },
{ NETF(parent), 16, netFieldType_t::regular },
{ NETF(constantLight), 32, netFieldType_t::regular },
{ NETF(renderfx), 32, netFieldType_t::regular },
{ NETF(bone_tag[0]), -8, netFieldType_t::regular },
{ NETF(bone_tag[1]), -8, netFieldType_t::regular },
{ NETF(bone_tag[2]), -8, netFieldType_t::regular },
{ NETF(bone_tag[3]), -8, netFieldType_t::regular },
{ NETF(bone_tag[4]), -8, netFieldType_t::regular },
{ NETF(scale), 0, netFieldType_t::scale },
{ NETF(alpha), 0, netFieldType_t::alpha },
{ NETF(usageIndex), 16, netFieldType_t::regular },
{ NETF(eFlags), 16, netFieldType_t::regular },
{ NETF(solid), 32, netFieldType_t::regular },
{ NETF(netangles[2]), 12, netFieldType_t::angle },
{ NETF(netangles[0]), 12, netFieldType_t::angle },
{ NETF(tag_num), 10, netFieldType_t::regular },
{ NETF(bone_angles[1][2]), -13, netFieldType_t::angle },
{ NETF(attach_use_angles), 1, netFieldType_t::regular },
{ NETF(origin2[1]), 0, netFieldType_t::coord },
{ NETF(origin2[0]), 0, netFieldType_t::coord },
{ NETF(origin2[2]), 0, netFieldType_t::coord },
{ NETF(bone_angles[0][2]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][2]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][2]), -13, netFieldType_t::angle },
{ NETF(surfaces[0]), 8, netFieldType_t::regular },
{ NETF(surfaces[1]), 8, netFieldType_t::regular },
{ NETF(surfaces[2]), 8, netFieldType_t::regular },
{ NETF(surfaces[3]), 8, netFieldType_t::regular },
{ NETF(bone_angles[0][1]), -13, netFieldType_t::angle },
{ NETF(surfaces[4]), 8, netFieldType_t::regular },
{ NETF(surfaces[5]), 8, netFieldType_t::regular },
{ NETF(pos.trTime), 32, netFieldType_t::regular },
{ NETF(pos.trDelta[0]), 0, netFieldType_t::velocity },
{ NETF(pos.trDelta[1]), 0, netFieldType_t::velocity },
{ NETF(pos.trDelta[2]), 0, netFieldType_t::velocity },
{ NETF(loopSound), 16, netFieldType_t::regular },
{ NETF(loopSoundVolume), 0, netFieldType_t::regular },
{ NETF(loopSoundMinDist), 0, netFieldType_t::regular },
{ NETF(loopSoundMaxDist), 0, netFieldType_t::regular },
{ NETF(loopSoundPitch), 0, netFieldType_t::regular },
{ NETF(loopSoundFlags), 8, netFieldType_t::regular },
{ NETF(attach_offset[0]), 0, netFieldType_t::regular },
{ NETF(attach_offset[1]), 0, netFieldType_t::regular },
{ NETF(attach_offset[2]), 0, netFieldType_t::regular },
{ NETF(beam_entnum), 16, netFieldType_t::regular },
{ NETF(skinNum), 16, netFieldType_t::regular },
{ NETF(wasframe), 10, netFieldType_t::regular },
{ NETF(frameInfo[4].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[5].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[6].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[7].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[8].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[9].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[10].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[11].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[12].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[13].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[14].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[15].index), 12, netFieldType_t::regular },
{ NETF(frameInfo[4].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[5].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[6].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[7].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[8].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[9].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[10].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[11].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[12].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[13].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[14].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[15].time), 0, netFieldType_t::animTime },
{ NETF(frameInfo[4].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[5].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[6].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[7].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[8].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[9].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[10].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[11].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[12].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[13].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[14].weight), 0, netFieldType_t::animWeight },
{ NETF(frameInfo[15].weight), 0, netFieldType_t::animWeight },
{ NETF(bone_angles[1][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[2][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[3][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][0]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][1]), -13, netFieldType_t::angle },
{ NETF(bone_angles[4][2]), -13, netFieldType_t::angle },
{ NETF(clientNum), 8, netFieldType_t::regular },
{ NETF(groundEntityNum), GENTITYNUM_BITS, netFieldType_t::regular },
{ NETF(shader_data[0]), 0, netFieldType_t::regular },
{ NETF(shader_data[1]), 0, netFieldType_t::regular },
{ NETF(shader_time), 0, netFieldType_t::regular },
{ NETF(eyeVector[0]), 0, netFieldType_t::regular },
{ NETF(eyeVector[1]), 0, netFieldType_t::regular },
{ NETF(eyeVector[2]), 0, netFieldType_t::regular },
{ NETF(surfaces[6]), 8, netFieldType_t::regular },
{ NETF(surfaces[7]), 8, netFieldType_t::regular },
{ NETF(surfaces[8]), 8, netFieldType_t::regular },
{ NETF(surfaces[9]), 8, netFieldType_t::regular },
{ NETF(surfaces[10]), 8, netFieldType_t::regular },
{ NETF(surfaces[11]), 8, netFieldType_t::regular },
{ NETF(surfaces[12]), 8, netFieldType_t::regular },
{ NETF(surfaces[13]), 8, netFieldType_t::regular },
{ NETF(surfaces[14]), 8, netFieldType_t::regular },
{ NETF(surfaces[15]), 8, netFieldType_t::regular },
{ NETF(surfaces[16]), 8, netFieldType_t::regular },
{ NETF(surfaces[17]), 8, netFieldType_t::regular },
{ NETF(surfaces[18]), 8, netFieldType_t::regular },
{ NETF(surfaces[19]), 8, netFieldType_t::regular },
{ NETF(surfaces[20]), 8, netFieldType_t::regular },
{ NETF(surfaces[21]), 8, netFieldType_t::regular },
{ NETF(surfaces[22]), 8, netFieldType_t::regular },
{ NETF(surfaces[23]), 8, netFieldType_t::regular },
{ NETF(surfaces[24]), 8, netFieldType_t::regular },
{ NETF(surfaces[25]), 8, netFieldType_t::regular },
{ NETF(surfaces[26]), 8, netFieldType_t::regular },
{ NETF(surfaces[27]), 8, netFieldType_t::regular },
{ NETF(surfaces[28]), 8, netFieldType_t::regular },
{ NETF(surfaces[29]), 8, netFieldType_t::regular },
{ NETF(surfaces[30]), 8, netFieldType_t::regular },
{ NETF(surfaces[31]), 8, netFieldType_t::regular }
};
static constexpr unsigned long numEntityStateFields_ver_6 = sizeof(entityStateFields_ver_6) / sizeof(entityStateFields_ver_6[0]);
static constexpr unsigned long numEntityStateFields_ver_8 = sizeof(entityStateFields_ver_8) / sizeof(entityStateFields_ver_8[0]);
static constexpr unsigned long numBiggestEntityStateFields = numEntityStateFields_ver_15 >= numEntityStateFields_ver_8 ? numEntityStateFields_ver_15 : numEntityStateFields_ver_8;

netField_t* MSG_GetEntityStateFields(size_t& outNumFields)
{
    if (com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN)
    {
        outNumFields = sizeof(entityStateFields_ver_15) / sizeof(entityStateFields_ver_15[0]);
        return entityStateFields_ver_15;
    }
    else if (com_protocol->integer >= protocol_e::PROTOCOL_MOH)
    {
        outNumFields = sizeof(entityStateFields_ver_8) / sizeof(entityStateFields_ver_8[0]);
        return entityStateFields_ver_8;
    }
	else
    {
        outNumFields = sizeof(entityStateFields_ver_6) / sizeof(entityStateFields_ver_6[0]);
        return entityStateFields_ver_6;
	}
}

// if (int)f == f and (int)f + ( 1<<(FLOAT_INT_BITS-1) ) < ( 1 << FLOAT_INT_BITS )
// the float will be sent with FLOAT_INT_BITS, otherwise all 32 bits will be sent
#define	FLOAT_INT_BITS	13
#define	FLOAT_INT_BIAS	(1<<(FLOAT_INT_BITS-1))

void MSG_ReadRegular_ver_15(msg_t* sb, int bits, int size, void* toF)
{
	if (bits == 0)
	{
		// float
		if (!MSG_ReadBits(sb, 1)) {
			// float
			*(float*)toF = 0.0f;
		}
		else
		{
			if (!MSG_ReadBits(sb, 1))
			{
				// integral float
				*(float*)toF = MSG_ReadBits(sb, -FLOAT_INT_BITS);
			}
			else
			{
				// full floating point value
				unsigned int v = MSG_ReadBits(sb, 32);
				if (v & 1) {
					*(int*)toF = ((v + 0x7A000000) >> 1) | 0x80000000;
				}
				else {
					*(int*)toF = (v + 0x7A000000) >> 1;
				}
			}
		}
	}
	else
	{
		if (MSG_ReadBits(sb, 1)) {
			int tmp = MSG_ReadBits(sb, bits);
			CopyToLittleField(toF, &tmp, sizeof(int), size);
		}
		else {
			Com_Memset(toF, 0, size);
		}
	}
}

void MSG_ReadRegularSimple_ver_15(msg_t* sb, int bits, int size, void* toF)
{
	MSG_ReadRegular_ver_15(sb, bits, size, toF);
}

void MSG_WriteRegular_ver_15(msg_t* sb, int bits, int size, const void* toF)
{
	float fullFloat;
	int trunc;

	if (bits == 0) {
		// float
		fullFloat = *(float*)toF;
		trunc = (int)fullFloat;

		if (fullFloat == 0.0f) {
			MSG_WriteBits(sb, 0, 1);
			oldsize += FLOAT_INT_BITS;
		}
		else {
			MSG_WriteBits(sb, 1, 1);
			if (trunc == fullFloat && trunc >= -4096 && trunc < 4096) {
				// send as small integer
				MSG_WriteBits(sb, 0, 1);
				MSG_WriteBits(sb, trunc, -FLOAT_INT_BITS);
			}
			else {
				int newvalue = *(int*)toF * 2 - 0x7A000000;
				if (*(int*)toF < 0) {
					newvalue |= 1;
				}
				MSG_WriteBits(sb, 1, 1);
				// send as full floating point value
				MSG_WriteBits(sb, newvalue, 32);
			}
		}
	}
	else {
		qboolean hasValue = qfalse;
		for (size_t i = 0; i < size; ++i)
		{
			if (((byte*)toF)[i] != 0)
			{
				hasValue = qtrue;
				break;
			}
		}

		if (!hasValue) {
			MSG_WriteBits(sb, 0, 1);
		}
		else {
			//
			// Added in OPM
			//  Properly swap on little-endian architectures
			int tmp = 0;
			CopyFromLittleField(&tmp, toF, sizeof(int), size);

			MSG_WriteBits(sb, 1, 1);
			// integer
			MSG_WriteBits(sb, tmp, bits);
		}
	}
}

void MSG_WriteRegularSimple_ver_15(msg_t* sb, int bits, int size, const void* toF)
{
	MSG_WriteRegular_ver_15(sb, bits, size, toF);
}

void MSG_WriteEntityNum_ver_15(msg_t* sb, short number)
{
	// protocols version 15 and above adds 1 to the entity number
	MSG_WriteBits(sb, (number + 1) % MAX_GENTITIES, GENTITYNUM_BITS);
}

unsigned short MSG_ReadEntityNum_ver_15(msg_t* sb)
{
	return (unsigned short)(MSG_ReadBits(sb, GENTITYNUM_BITS) - 1) % MAX_GENTITIES;
}

void MSG_ReadRegular_ver_6(msg_t* sb, int bits, int size, void* toF)
{
	if (bits == 0)
	{
		if (!MSG_ReadBits(sb, 1)) {
			// float
			*(float*)toF = 0.0f;
		}
		else
		{
			if (!MSG_ReadBits(sb, 1)) {
				*(float*)toF = (int)MSG_ReadBits(sb, FLOAT_INT_BITS) - FLOAT_INT_BIAS;
			}
			else
			{
				// full floating point value
				*(float*)toF = MSG_ReadFloat(sb);
			}
		}
	}
	else
	{
		if (MSG_ReadBits(sb, 1)) {
			int tmp = MSG_ReadBits(sb, bits);
			CopyToLittleField(toF, &tmp, sizeof(int), size);
		}
		else {
			Com_Memset(toF, 0, size);
		}
	}
}

void MSG_ReadRegularSimple_ver_6(msg_t* sb, int bits, int size, void* toF)
{
	if (bits == 0) {
		// float
		if (MSG_ReadBits(sb, 1) == 0) {
			// integral float
			int trunc = MSG_ReadBits(sb, FLOAT_INT_BITS);
			// bias to allow equal parts positive and negative
			trunc -= FLOAT_INT_BIAS;
			*(float*)toF = trunc;
		}
		else {
			// full floating point value
			*(int*)toF = MSG_ReadBits(sb, 32);
		}
	}
	else {
		// integer
		int tmp = MSG_ReadBits(sb, bits);
		CopyToLittleField(toF, &tmp, sizeof(int), size);
	}
}

void MSG_WriteRegular_ver_6(msg_t* sb, int bits, int size, const void* toF)
{
	float fullFloat;
	int trunc;

	if (bits == 0) {
		// float
		fullFloat = *(float*)toF;
		trunc = (int)fullFloat;

		if (fullFloat == 0.0f) {
			MSG_WriteBits(sb, 0, 1);
			oldsize += FLOAT_INT_BITS;
		}
		else {
			MSG_WriteBits(sb, 1, 1);
			if (trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 &&
				trunc + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS)) {
				// send as small integer
				MSG_WriteBits(sb, 0, 1);
				MSG_WriteBits(sb, trunc + FLOAT_INT_BIAS, -FLOAT_INT_BITS);
			}
			else {
				// send as full floating point value
				MSG_WriteBits(sb, 1, 1);
				MSG_WriteBits(sb, *(int*)toF, 32);
			}
		}
	}
	else {
		if (!*(int*)toF) {
			MSG_WriteBits(sb, 0, 1);
		}
		else {
			//
			// Added in OPM
			//  Properly swap on little-endian architectures
			int tmp = 0;
			CopyFromLittleField(&tmp, toF, sizeof(int), size);

			MSG_WriteBits(sb, 1, 1);
			// integer
			MSG_WriteBits(sb, tmp, bits);
		}
	}
}

void MSG_WriteRegularSimple_ver_6(msg_t* sb, int bits, int size, const void* toF)
{
	float fullFloat;
	int trunc;

	if (bits == 0) {
		// float
		fullFloat = *(float*)toF;
		trunc = (int)fullFloat;

		if (trunc == fullFloat && trunc + FLOAT_INT_BIAS >= 0 &&
			trunc + FLOAT_INT_BIAS < (1 << FLOAT_INT_BITS)) {
			// send as small integer
			MSG_WriteBits(sb, 0, 1);
			MSG_WriteBits(sb, trunc + FLOAT_INT_BIAS, FLOAT_INT_BITS);
		}
		else {
			// send as full floating point value
			MSG_WriteBits(sb, 1, 1);
			MSG_WriteBits(sb, *(int*)toF, 32);
		}
	}
	else {
		// integer
		//
		// Added in OPM
		//  Properly swap on little-endian architectures
		int tmp = 0;
		CopyFromLittleField(&tmp, toF, sizeof(int), size);

		// integer
		MSG_WriteBits(sb, tmp, bits);
	}
}

void MSG_WriteEntityNum_ver_6(msg_t* sb, short number)
{
	MSG_WriteBits(sb, number % MAX_GENTITIES, GENTITYNUM_BITS);
}

unsigned short MSG_ReadEntityNum_ver_6(msg_t* sb)
{
	return MSG_ReadBits(sb, GENTITYNUM_BITS) % MAX_GENTITIES;
}

void MSG_ReadRegular(msg_t* sb, int bits, int size, void* toF)
{
	if (MSG_IsProtocolVersion15()) {
		return MSG_ReadRegular_ver_15(sb, bits, size, toF);
	} else {
		return MSG_ReadRegular_ver_6(sb, bits, size, toF);
	}
}

void MSG_ReadRegularSimple(msg_t* sb, int bits, int size, void* toF)
{
	if (MSG_IsProtocolVersion15()) {
		return MSG_ReadRegularSimple_ver_15(sb, bits, size, toF);
	} else {
		return MSG_ReadRegularSimple_ver_6(sb, bits, size, toF);
	}
}

void MSG_WriteRegular(msg_t* sb, int bits, int size, const void* toF)
{
	if (MSG_IsProtocolVersion15()) {
		return MSG_WriteRegular_ver_15(sb, bits, size, toF);
	} else {
		return MSG_WriteRegular_ver_6(sb, bits, size, toF);
	}
}

void MSG_WriteRegularSimple(msg_t* sb, int bits, int size, const void* toF)
{
	if (MSG_IsProtocolVersion15()) {
		return MSG_WriteRegularSimple_ver_15(sb, bits, size, toF);
	} else {
		return MSG_WriteRegularSimple_ver_6(sb, bits, size, toF);
	}
}

void MSG_WriteEntityNum(msg_t* sb, short number)
{
	if (MSG_IsProtocolVersion15()) {
		return MSG_WriteEntityNum_ver_15(sb, number);
	} else {
		return MSG_WriteEntityNum_ver_6(sb, number);
	}
}

unsigned short MSG_ReadEntityNum(msg_t* sb)
{
	if (MSG_IsProtocolVersion15()) {
		return MSG_ReadEntityNum_ver_15(sb);
	} else {
		return MSG_ReadEntityNum_ver_6(sb);
	}
}

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
						   qboolean force, float frameTime ) {
	int i, lc;
	netField_t* entityStateFields;
	netField_t *field;
	size_t numFields;
	int *fromF, *toF;
	qboolean deltasNeeded[numBiggestEntityStateFields];

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
		MSG_WriteEntityNum(msg, from->number);
		MSG_WriteBits( msg, 1, 1 );
		return;
	}

	if ( to->number < 0 || to->number >= MAX_GENTITIES ) {
		Com_Error (ERR_FATAL, "MSG_WriteDeltaEntity: Bad entity number: %i", to->number );
	}

    entityStateFields = MSG_GetEntityStateFields(numFields);

	lc = 0;
	// build the change vector as bytes so it is endien independent
	for ( i = 0, field = entityStateFields ; i < numFields; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		deltasNeeded[i] = MSG_DeltaNeeded(fromF, toF, field->type, field->bits, field->size);
		if (deltasNeeded[i]) {
			lc = i+1;
		}
	}

	if ( lc == 0 ) {
		// nothing at all changed
		if ( !force ) {
			return;		// nothing at all
		}
		// write two bits for no change
		MSG_WriteEntityNum(msg, to->number);
		MSG_WriteBits( msg, 0, 1 );		// not removed
		MSG_WriteBits( msg, 0, 1 );		// no delta
		return;
	}

	MSG_WriteEntityNum(msg, to->number);
	MSG_WriteBits( msg, 0, 1 );			// not removed
	MSG_WriteBits( msg, 1, 1 );			// we have a delta

	MSG_WriteByte( msg, lc );	// # of changes

	oldsize += numFields;

	for ( i = 0, field = entityStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );

		if (!deltasNeeded[i]) {
			// no change
			MSG_WriteBits( msg, 0, 1 );
			continue;
		}

		// changed
		MSG_WriteBits( msg, 1, 1 );

		switch ( field->type ) {
			// normal style
			case netFieldType_e::regular:
				MSG_WriteRegular(msg, field->bits, field->size, toF);
				break;
			case netFieldType_e::angle:
				MSG_WritePackedAngle(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::animTime:
				MSG_WritePackedAnimTime(msg, *(float*)fromF, *(float*)toF, frameTime, field->bits);
				break;
			case netFieldType_e::animWeight:
				MSG_WritePackedAnimWeight(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::scale:
				MSG_WritePackedScale(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::alpha:
				MSG_WritePackedAlpha(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::coord:
				MSG_WritePackedCoord(msg, *(float*)fromF, *(float*)toF, field->bits);
				break;
			case netFieldType_e::coordExtra:
				// Team Assault
				MSG_WritePackedCoordExtra(msg, *(float*)fromF, *(float*)toF, field->bits);
				break;
			case netFieldType_e::velocity:
				MSG_WritePackedVelocity(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::simple:
				MSG_WritePackedSimple(msg, *(int*)toF, field->bits);
				break;
			default:
				Com_Error( ERR_DROP, "MSG_WriteDeltaEntity: unrecognized entity field type %i for field %i\n", field->bits, i );
				break;
		}
	}
}

int MSG_PackAngle(float angle, int bits)
{
	int bit;
	float calc;

	bit = 0;
	if (bits < 0)
	{
		bits = ~bits;
		if (angle < 0.0)
		{
			angle = -angle;
			bit = 1 << bits;
		}
	}

	switch (bits)
	{
	case 8:
		calc = angle * 256.f / 360.f;
		return bit | ((int)calc & 0xFF);
	case 12:
		calc = angle * 4096.f / 360.f;
		return bit | ((int)calc & 0xFFF);
	case 16:
		calc = angle * 65536.f / 360.f;
		return bit | ((int)calc & 0xFFFF);
	default:
		calc = (1 << bits) * angle / 360.f;
		return bit | ((int)calc & ((1 << bits) - 1));
	}
}

int MSG_PackAnimTime(float time, int bits)
{
	int maxValue;
	int packed;

	maxValue = (1 << bits) - 1;
	packed = time * 100.f;
	if (packed >= 0)
	{
		if (packed > maxValue) {
			packed = maxValue;
		}
	}
	else
	{
		packed = 0;
	}

	timestats[packed]++;

	return packed;
}

int MSG_PackAnimWeight(float weight, int bits)
{
	int maxValue;
	int packed;

	maxValue = (1 << bits) - 1;
	packed = maxValue * weight;
	if (packed >= 0)
	{
		if (packed > maxValue) {
			packed = maxValue;
		}
	}
	else
	{
		packed = 0;
	}

	weightstats[packed]++;

	return packed;
}

int MSG_PackScale(float scale, int bits)
{
	int maxValue;
	int packed;

	maxValue = (1 << bits) - 1;
	packed = scale * 100.f;
	if (packed >= 0)
	{
		if (packed > maxValue) {
			packed = maxValue;
		}
	}
	else
	{
		packed = 0;
	}

	scalestats[packed]++;

	return packed;
}

int MSG_PackAlpha(float alpha, int bits)
{
	int maxValue;
	int packed;

	maxValue = (1 << bits) - 1;
	packed = maxValue * alpha;
	if (packed >= 0)
	{
		if (packed > maxValue) {
			packed = maxValue;
		}
	}
	else
	{
		packed = 0;
	}

	alphastats[packed]++;

	return packed;
}

int MSG_PackCoord(float coord)
{
	unsigned int packed;

	packed = (unsigned int)round(coord * 4.0 + MAX_PACKED_COORD_HALF);

	if (packed < MAX_PACKED_COORD) {
		coordstats[packed]++;
	}
// 	else {
// 		Com_DPrintf("Illegal XYZ coordinates for an entity, small information lost in transmission\n");
// 	}

	return packed;
}

int MSG_PackCoordExtra(float coord)
{
	unsigned int packed;

	packed = (unsigned int)round(coord * 16.0 + MAX_PACKED_COORD_EXTRA_HALF);

	// Fixed in OPM
	//  This check wasn't added in >= 2.0
	//  which means a player could crash a server when out of bounds
	if (packed < MAX_PACKED_COORD_EXTRA) {
		++coordextrastats[packed];
	}
//	else {
//		Com_DPrintf("Illegal XYZ coordinates for an entity, information lost in transmission\n");
//	}

	return packed;
}

float MSG_ReadPackedAngle_ver_15(msg_t* msg, int bits)
{
	int packed = MSG_ReadBits(msg, abs(bits));
	return MSG_UnpackAngle(packed, bits);
}

float MSG_ReadPackedAnimTime_ver_15(msg_t* msg, int bits, float fromValue, float frameTime)
{
	int packed;
	if (!MSG_ReadBits(msg, 1)) {
		return ceil((fromValue + frameTime) * 10000) / 10000;
	}

	packed = MSG_ReadBits(msg, bits);
	return MSG_UnpackAnimTime(packed);
}

float MSG_ReadPackedAnimWeight_ver_15(msg_t* msg, int bits)
{
	int packed = MSG_ReadBits(msg, bits);
	return MSG_UnpackAnimWeight(packed, bits);
}

float MSG_ReadPackedScale_ver_15(msg_t* msg, int bits)
{
	int packed = MSG_ReadBits(msg, bits);
	return MSG_UnpackScale(packed);
}

float MSG_ReadPackedAlpha_ver_15(msg_t* msg, int bits)
{
	int packed = MSG_ReadBits(msg, bits);
	return MSG_UnpackAlpha(packed, bits);
}

float MSG_ReadPackedCoord_ver_15(msg_t* msg, float fromValue, int bits)
{
	int packedFrom = MSG_PackCoord(fromValue);
	int packedTo = MSG_ReadDeltaCoord(msg, packedFrom);
	return MSG_UnpackCoord(packedTo, bits);
}

float MSG_ReadPackedCoordExtra_ver_15(msg_t* msg, float fromValue, int bits)
{
	int packedFrom = MSG_PackCoordExtra(fromValue);
	int packedTo = MSG_ReadDeltaCoordExtra(msg, packedFrom);
	return MSG_UnpackCoordExtra(packedTo, bits);
}

void MSG_WritePackedAngle_ver_15(msg_t* msg, float value, int bits)
{
	int packed = MSG_PackAngle(value, bits);
	MSG_WriteBits(msg, packed, abs(bits));
}

void MSG_WritePackedAnimTime_ver_15(msg_t* msg, float fromValue, float toValue, float frameTime, int bits)
{
	int packed;

	if (fabs(toValue - fromValue - frameTime) < 0.001f) {
		// below the frame time, don't send
		MSG_WriteBits(msg, 0, 1);
		return;
	}

	MSG_WriteBits(msg, 1, 1);
	packed = MSG_PackAnimTime(toValue, bits);
	MSG_WriteBits(msg, packed, bits);
}

void MSG_WritePackedAnimWeight_ver_15(msg_t* msg, float value, int bits)
{
	int packed = MSG_PackAnimWeight(value, bits);
	MSG_WriteBits(msg, packed, bits);
}

void MSG_WritePackedScale_ver_15(msg_t* msg, float value, int bits)
{
	int packed = MSG_PackScale(value, bits);
	MSG_WriteBits(msg, packed, bits);
}

void MSG_WritePackedAlpha_ver_15(msg_t* msg, float value, int bits)
{
	int packed = MSG_PackAlpha(value, bits);
	MSG_WriteBits(msg, packed, bits);
}

void MSG_WritePackedCoord_ver_15(msg_t* msg, float fromValue, float toValue, int bits)
{
	int packedFrom = MSG_PackCoord(fromValue);
	int packedTo = MSG_PackCoord(toValue);
	MSG_WriteDeltaCoord(msg, packedFrom, packedTo);
}

void MSG_WritePackedCoordExtra_ver_15(msg_t* msg, float fromValue, float toValue, int bits)
{
	int packedFrom = MSG_PackCoordExtra(fromValue);
	int packedTo = MSG_PackCoordExtra(toValue);
	MSG_WriteDeltaCoordExtra(msg, packedFrom, packedTo);
}

qboolean MSG_DeltaNeeded_ver_15(const void* fromField, const void* toField, int fieldType, int bits, int size)
{
	int packedFrom;
	int packedTo;
	int maxValue;
	int xoredValue;
	int i;

	if (!memcmp(fromField, toField, size)) {
		// same values, not needed
		return qfalse;
	}

	switch (fieldType)
	{
	case netFieldType_e::regular:
		if (!bits || bits == 32) {
			return qtrue;
		}

		maxValue = (1 << abs(bits)) - 1;
		xoredValue = 0;

		for (i = 0; i < size; i++) {
			byte fromVal, toVal;

			fromVal = ((byte*)fromField)[i];
			toVal = ((byte*)toField)[i];
			xoredValue += fromVal ^ toVal;
		}
		return (xoredValue & maxValue) != 0;
	case netFieldType_e::angle:
		packedFrom = MSG_PackAngle(*(float*)fromField, bits);
		packedTo = MSG_PackAngle(*(float*)toField, bits);
		return packedFrom != packedTo;
	case netFieldType_e::animTime:
		packedFrom = MSG_PackAnimTime(*(float*)fromField, bits);
		packedTo = MSG_PackAnimTime(*(float*)toField, bits);
		return packedFrom != packedTo;
	case netFieldType_e::animWeight:
		packedFrom = MSG_PackAnimWeight(*(float*)fromField, bits);
		packedTo = MSG_PackAnimWeight(*(float*)toField, bits);
		return packedFrom != packedTo;
	case netFieldType_e::scale:
		packedFrom = MSG_PackScale(*(float*)fromField, bits);
		packedTo = MSG_PackScale(*(float*)toField, bits);
		return packedFrom != packedTo;
	case netFieldType_e::alpha:
		packedFrom = MSG_PackAlpha(*(float*)fromField, bits);
		packedTo = MSG_PackAlpha(*(float*)toField, bits);
		return packedFrom != packedTo;
	case netFieldType_e::coord:
		packedFrom = MSG_PackCoord(*(float*)fromField);
		packedTo = MSG_PackCoord(*(float*)toField);
		return packedFrom != packedTo;
	case netFieldType_e::coordExtra:
		packedFrom = MSG_PackCoordExtra(*(float*)fromField);
		packedTo = MSG_PackCoordExtra(*(float*)toField);
		return packedFrom != packedTo;
	case netFieldType_e::velocity:
		return true;
	case netFieldType_e::simple:
		return true;
	default:
		return qtrue;
	}
}

float MSG_ReadPackedAngle_ver_6(msg_t* msg, int bits)
{
	int result;
	float tmp = 1.0f;
	if (bits < 0) {
		if (MSG_ReadBits(msg, 1))
			tmp = -1.0f;
		bits = ~bits;
	}

	result = MSG_ReadBits(msg, bits);
	switch (bits)
	{
	case 8:
		return tmp * 360.f / 256.f;
	case 12:
		return tmp * result * 360.f / 4096.f;
	case 16:
		return tmp * result * 360.f / 65536.f;
	default:
		return tmp * 360.f / (1 << bits) * result;
	}
}

float MSG_ReadPackedAnimTime_ver_6(msg_t* msg, int bits, float fromValue, float frameTime)
{
	return MSG_ReadBits(msg, 15) / 100.0f;
}

float MSG_ReadPackedAnimWeight_ver_6(msg_t* msg, int bits)
{
	float tmp = MSG_ReadBits(msg, 8) / 255.0f;
	if (tmp < 0.0f)
		return 0.0f;
	else if (tmp > 1.0f)
		return 1.0f;
	else
		return tmp;
}

float MSG_ReadPackedScale_ver_6(msg_t* msg, int bits)
{
	return MSG_ReadBits(msg, 10) / 100.0f;
}

float MSG_ReadPackedAlpha_ver_6(msg_t* msg, int bits)
{
	float tmp = MSG_ReadBits(msg, 8) / 255.0f;
	if (tmp < 0.0f)
		return 0.0f;
	else if (tmp > 1.0f)
		return 1.0f;
	else
		return tmp;
}

float MSG_ReadPackedCoord_ver_6(msg_t* msg, float fromValue, int bits)
{
	float tmp = 1.0f;
	int value = MSG_ReadBits(msg, 19);
	if (value & 262144) // test for 19th bit
		tmp = -1.0f;
	value &= ~262144;	// remove that bit
	return tmp * value / 16.0f;
}

float MSG_ReadPackedCoordExtra_ver_6(msg_t* msg, float fromValue, int bits)
{
	int packedFrom = MSG_PackCoordExtra(fromValue);
	int packedTo = MSG_ReadDeltaCoordExtra(msg, packedFrom);
	return MSG_UnpackCoordExtra(packedTo, bits);
}


void MSG_WritePackedAngle_ver_6(msg_t* msg, float value, int bits)
{
	// angles, what a mess! it wouldnt surprise me if something goes wrong here ;)

	float tmp = value;

	if (bits < 0) {
		if (tmp < 0.0f) {
			MSG_WriteBits(msg, 1, 1);
			tmp = -tmp;
		}
		else {
			MSG_WriteBits(msg, 0, 1);
		}

		bits = ~bits;
	}
	else {
		bits = bits;
	}

	if (bits == 12) {
		tmp = tmp * 4096.0f / 360.0f;
		MSG_WriteBits(msg, ((int)tmp) & 4095, 12);
	}
	else if (bits == 8) {
		tmp = tmp * 256.0f / 360.0f;
		MSG_WriteBits(msg, ((int)tmp) & 255, 8);
	}
	else if (bits == 16) {
		tmp = tmp * 65536.0f / 360.0f;
		MSG_WriteBits(msg, ((int)tmp) & 65535, 16);
	}
	else {
		tmp = tmp * (1 << (byte)bits) / 360.0f;
		MSG_WriteBits(msg, ((int)tmp) & ((1 << (byte)bits) - 1), bits);
	}
}

void MSG_WritePackedAnimTime_ver_6(msg_t* msg, float fromValue, float toValue, float frameTime, int bits)
{
	int packed = toValue * 100.0f;
	if (packed < 0) {
		packed = 0;
	}
	else if (packed >= (1 << 15)) {
		packed = (1 << 15);
	}

	MSG_WriteBits(msg, packed, 15);
}

void MSG_WritePackedAnimWeight_ver_6(msg_t* msg, float value, int bits)
{
	int packed = (value * 255.0f) + 0.5f;

	if (packed < 0) {
		packed = 0;
	}
	else if (packed > 255) {
		packed = 255;
	}

	MSG_WriteBits(msg, packed, 8);
}

void MSG_WritePackedScale_ver_6(msg_t* msg, float value, int bits)
{
	int packed = value * 100.0f;
	if (packed < 0) {
		packed = 0;
	}
	else if (packed > 1023) {
		packed = 1023;
	}

	MSG_WriteBits(msg, packed, 10);
}

void MSG_WritePackedAlpha_ver_6(msg_t* msg, float value, int bits)
{
	int packed = (value * 255.0f) + 0.5f;

	if (packed < 0) {
		packed = 0;
	}
	else if (packed > 255) {
		packed = 255;
	}

	MSG_WriteBits(msg, packed, 8);
}

void MSG_WritePackedCoord_ver_6(msg_t* msg, float fromValue, float toValue, int bits)
{
	int packed = toValue * 16.0f;

	if (toValue < 0) {
		packed = ((-packed) & 262143) | 262144;
	}
	else {
		packed = packed & 262143;
	}

	MSG_WriteBits(msg, packed, 19);
}

void MSG_WritePackedCoordExtra_ver_6(msg_t* msg, float fromValue, float toValue, int bits) {
	// Don't implement
}

qboolean MSG_DeltaNeeded_ver_6(const void* fromField, const void* toField, int fieldType, int bits, int size)
{
	// Unoptimized in base game
	// Doesn't compare packed values
	return memcmp(fromField, toField, size);
}

float MSG_ReadPackedAngle(msg_t* msg, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_ReadPackedAngle_ver_15(msg, bits);
    } else {
		return MSG_ReadPackedAngle_ver_6(msg, bits);
    }
}

float MSG_ReadPackedAnimTime(msg_t* msg, int bits, float fromValue, float frameTime) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_ReadPackedAnimTime_ver_15(msg, bits, fromValue, frameTime);
    } else {
		return MSG_ReadPackedAnimTime_ver_6(msg, bits, fromValue, frameTime);
    }
}

float MSG_ReadPackedAnimWeight(msg_t* msg, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_ReadPackedAnimWeight_ver_15(msg, bits);
    } else {
		return MSG_ReadPackedAnimWeight_ver_6(msg, bits);
    }
}

float MSG_ReadPackedScale(msg_t* msg, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_ReadPackedScale_ver_15(msg, bits);
    } else {
		return MSG_ReadPackedScale_ver_6(msg, bits);
    }
}

float MSG_ReadPackedAlpha(msg_t* msg, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_ReadPackedAlpha_ver_15(msg, bits);
    } else {
		return MSG_ReadPackedAlpha_ver_6(msg, bits);
    }
}

float MSG_ReadPackedCoord(msg_t* msg, float fromValue, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_ReadPackedCoord_ver_15(msg, fromValue, bits);
    } else {
		return MSG_ReadPackedCoord_ver_6(msg, fromValue, bits);
    }
}

float MSG_ReadPackedCoordExtra(msg_t* msg, float fromValue, int bits)
{
    if (MSG_IsProtocolVersion15()) {
		return MSG_ReadPackedCoordExtra_ver_15(msg, fromValue, bits);
    } else {
		return MSG_ReadPackedCoordExtra_ver_6(msg, fromValue, bits);
    }
}


void MSG_WritePackedAngle(msg_t* msg, float value, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_WritePackedAngle_ver_15(msg, value, bits);
    } else {
		return MSG_WritePackedAngle_ver_6(msg, value, bits);
    }
}

void MSG_WritePackedAnimTime(msg_t* msg, float fromValue, float toValue, float frameTime, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_WritePackedAnimTime_ver_15(msg, fromValue, toValue, frameTime, bits);
    } else {
		return MSG_WritePackedAnimTime_ver_6(msg, fromValue, toValue, frameTime, bits);
    }
}

void MSG_WritePackedAnimWeight(msg_t* msg, float value, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_WritePackedAnimWeight_ver_15(msg, value, bits);
    } else {
		return MSG_WritePackedAnimWeight_ver_6(msg, value, bits);
    }
}

void MSG_WritePackedScale(msg_t* msg, float value, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_WritePackedScale_ver_15(msg, value, bits);
    } else {
		return MSG_WritePackedScale_ver_6(msg, value, bits);
    }
}

void MSG_WritePackedAlpha(msg_t* msg, float value, int bits) {
    if (MSG_IsProtocolVersion15()) {
		return MSG_WritePackedAlpha_ver_15(msg, value, bits);
    } else {
		return MSG_WritePackedAlpha_ver_6(msg, value, bits);
    }
}

void MSG_WritePackedCoord(msg_t* msg, float fromValue, float toValue, int bits) {
	if (MSG_IsProtocolVersion15()) {
		return MSG_WritePackedCoord_ver_15(msg, fromValue, toValue, bits);
    } else {
		return MSG_WritePackedCoord_ver_6(msg, fromValue, toValue, bits);
	}
}

void MSG_WritePackedCoordExtra(msg_t* msg, float fromValue, float toValue, int bits) {
	if (MSG_IsProtocolVersion15()) {
		return MSG_WritePackedCoordExtra_ver_15(msg, fromValue, toValue, bits);
    } else {
		return MSG_WritePackedCoordExtra_ver_6(msg, fromValue, toValue, bits);
	}
}

qboolean MSG_DeltaNeeded(const void* fromField, const void* toField, int fieldType, int bits, int size) {
	if (MSG_IsProtocolVersion15()) {
		return MSG_DeltaNeeded_ver_15(fromField, toField, fieldType, bits, size);
    } else {
		return MSG_DeltaNeeded_ver_6(fromField, toField, fieldType, bits, size);
	}
}

float MSG_ReadPackedVelocity(msg_t* msg, int bits)
{
	float tmp = 1.0f;
	int value = MSG_ReadBits(msg, 17);
	if (value & 65536) // test for 17th bit
		tmp = -1.0f;
	value &= ~65536; // remove that bit
	return tmp * value / 8.0f;
}

int MSG_ReadPackedSimple(msg_t* msg, int fromValue, int bits)
{
	if (!MSG_ReadBits(msg, 1)) {
		return fromValue;
	}

	return MSG_ReadBits(msg, bits);
}

void MSG_WritePackedVelocity(msg_t* msg, float value, int bits)
{
	int32_t packed = (int32_t)(value * 8.0f);
	if (value < 0) {
		packed = ((-packed) & 65535) | 65536;
	}
	else {
		packed = packed & 65535;
	}

	MSG_WriteBits(msg, packed, 17);
}

void MSG_WritePackedSimple(msg_t* msg, int value, int bits)
{
	byte packed = (byte)value;
	if (!packed) {
		MSG_WriteBits(msg, 0, 1);
	}

	MSG_WriteBits(msg, 1, 1);
	MSG_WriteBits(msg, packed, bits);
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
						 int number, float frameTime) {
	int			i, lc;
	size_t		numFields;
	netField_t* entityStateFields;
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

	entityStateFields = MSG_GetEntityStateFields(numFields);
	lc = MSG_ReadByte(msg);

	// su44: trying to find the cause of "unrecognized entity field type" error.
	// Number of changed fields (lc) should never be larger than total field count....
	if(numFields < lc) {
		Com_Error( ERR_DROP, "MSG_ReadDeltaEntity: number of changed fields (%i) is higher than total field count (%zu)\n",
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
			Com_Error( ERR_DROP, "MSG_ReadDeltaEntity: field offset %zu is larger than size of entityState (i %i)\n", field->offset, i );
		}
#endif
		if ( ! MSG_ReadBits( msg, 1 ) ) {
			// no change
			*toF = *fromF;
		} else {
			switch (field->type) {
				case netFieldType_e::regular:
					MSG_ReadRegular(msg, field->bits, field->size, toF);
					break;
				case netFieldType_e::angle: // angles, what a mess! it wouldnt surprise me if something goes wrong here ;)
					*(float*)toF = MSG_ReadPackedAngle(msg, field->bits);
					break;
				case netFieldType_e::animTime: // time
					*(float*)toF = MSG_ReadPackedAnimTime(msg, field->bits, *(float*)fromF, frameTime);
					break;
				case netFieldType_e::animWeight: // nasty!
					*(float*)toF = MSG_ReadPackedAnimWeight(msg, field->bits);
					break;
				case netFieldType_e::scale:
					*(float*)toF = MSG_ReadPackedScale(msg, field->bits);
					break;
				case netFieldType_e::alpha:
					*(float*)toF = MSG_ReadPackedAlpha(msg, field->bits);
					break;
				case netFieldType_e::coord:
					*(float*)toF = MSG_ReadPackedCoord(msg, *(float*)fromF, field->bits);
					break;
				case netFieldType_e::coordExtra:
					*(float*)toF = MSG_ReadPackedCoordExtra(msg, *(float*)fromF, field->bits);
					break;
				case netFieldType_e::velocity:
					*(float*)toF = MSG_ReadPackedVelocity(msg, field->bits);
					break;
				case netFieldType_e::simple:
					*(int*)toF = MSG_ReadPackedSimple(msg, *(int*)fromF, field->bits);
					break;
				default:
					Com_Error( ERR_DROP, "MSG_ReadDeltaEntity: unrecognized entity field type %i for field\n", i );
					break;
			}

			iEntityFieldChanges[i]++;
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

float MSG_UnpackAngle(int value, int bits)
{
	int maxValue;
	float neg;
	float calc;

	neg = 1.f;
	if (bits < 0)
	{
		bits = -1 - bits;
		maxValue = 1 << bits;
		if ((maxValue & value) != 0)
		{
			neg = -1.f;
			value &= ~maxValue;
		}
	}

	switch (bits)
	{
	case 8:
		return neg * 360.f / 256.f;
	case 12:
		return neg * value * 360.f / 4096.f;
	case 16:
		calc = value * 360.f / 65536.f;
		break;
	default:
		calc = 360.f / (1 << bits) * value;
		break;
	}
	return neg * calc;
}

float MSG_UnpackAnimTime(int packed)
{
	return packed / 100.f;
}

float MSG_UnpackAnimWeight(int result, int bits)
{
	const int32_t max = (1 << bits) - 1;
	const float tmp = (float)result / (float)max;

	if (tmp < 0.0f) return 0.f;
	else if (tmp > 1.0f) return 1.f;
	else return tmp;
}

float MSG_UnpackScale(int packed)
{
	return packed / 100.f;
}

float MSG_UnpackAlpha(int packed, int bits)
{
	return (float)packed / (float)((1 << bits) - 1);
}

float MSG_UnpackCoord(int packed, int bits)
{
	return (float)(packed - MAX_PACKED_COORD_HALF) / 4.f;
}

float MSG_UnpackCoordExtra(int packed, int bits)
{
	return (float)(packed - MAX_PACKED_COORD_EXTRA_HALF) / 16.f;
}

/*
==================
MSG_ReadSounds

read the sounds from the snapshot...
1:1 translated from assembly code
==================
*/
void MSG_ReadSounds(msg_t* msg, server_sound_t* sounds, int* snapshot_number_of_sounds) {

	int		fubar;
	int		i;

	if (MSG_ReadBits(msg, 1)) {
		fubar = MSG_ReadBits(msg, 7);

		if (fubar <= 64) {
			*snapshot_number_of_sounds = fubar;
			for (i = 0; i < fubar; i++) {
				if (MSG_ReadBits(msg, 1) == 1) {
					sounds[i].entity_number = MSG_ReadBits(msg, 10);
					sounds[i].channel = MSG_ReadBits(msg, 7);
					sounds[i].stop_flag = qtrue; // su44 was here
				}
				else {
					sounds[i].stop_flag = qfalse;
					sounds[i].streamed = (qboolean)MSG_ReadBits(msg, 1);
					if (MSG_ReadBits(msg, 1) == 1) {
						sounds[i].origin[0] = MSG_ReadFloat(msg);
						sounds[i].origin[1] = MSG_ReadFloat(msg);
						sounds[i].origin[2] = MSG_ReadFloat(msg);
					}
					else {
						sounds[i].origin[0] = 0;
						sounds[i].origin[1] = 0;
						sounds[i].origin[2] = 0;
					}
					sounds[i].entity_number = MSG_ReadBits(msg, 11);
					sounds[i].channel = MSG_ReadBits(msg, 7);
					sounds[i].sound_index = MSG_ReadBits(msg, 9);

					if (MSG_ReadBits(msg, 1) == 1) {
						sounds[i].volume = MSG_ReadFloat(msg);
					}
					else {
						sounds[i].volume = -1.0f;
					}

					if (MSG_ReadBits(msg, 1) == 1) {
						sounds[i].min_dist = MSG_ReadFloat(msg);
					}
					else {
						sounds[i].min_dist = -1.0f;
					}

					if (MSG_ReadBits(msg, 1) == 1) {
						sounds[i].pitch = MSG_ReadFloat(msg);
					}
					else {
						sounds[i].pitch = 1.0f; // su44 was here
					}

					sounds[i].maxDist = MSG_ReadFloat(msg);
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
void MSG_WriteSounds(msg_t* msg, server_sound_t* sounds, int snapshot_number_of_sounds) {

	int		i;

	if (!snapshot_number_of_sounds) {
		MSG_WriteBits(msg, 0, 1);
	}
	else {
		MSG_WriteBits(msg, 1, 1);
		MSG_WriteBits(msg, snapshot_number_of_sounds, 7);

		for (i = 0; i < snapshot_number_of_sounds; i++) {
			if (!sounds[i].stop_flag) {
				MSG_WriteBits(msg, 0, 1);
				MSG_WriteBits(msg, sounds[i].streamed, 1);

				if (sounds[i].origin[0] == 0.0f && sounds[i].origin[1] == 0.0f && sounds[i].origin[2] == 0.0f)
					MSG_WriteBits(msg, 0, 1);
				else {
					MSG_WriteBits(msg, 1, 1);
					MSG_WriteFloat(msg, sounds[i].origin[0]);
					MSG_WriteFloat(msg, sounds[i].origin[1]);
					MSG_WriteFloat(msg, sounds[i].origin[2]);
				}
				MSG_WriteBits(msg, sounds[i].entity_number, 11);
				MSG_WriteBits(msg, sounds[i].channel, 7);
				MSG_WriteBits(msg, sounds[i].sound_index, 9);

				if (sounds[i].volume != -1.0f) {
					MSG_WriteBits(msg, 1, 1);
					MSG_WriteFloat(msg, sounds[i].volume);
				}
				else {
					MSG_WriteBits(msg, 0, 1);
				}

				if (sounds[i].min_dist != -1.0f) {
					MSG_WriteBits(msg, 1, 1);
					MSG_WriteFloat(msg, sounds[i].min_dist);
				}
				else {
					MSG_WriteBits(msg, 0, 1);
				}

				if (sounds[i].pitch != -1.0f) {
					MSG_WriteBits(msg, 1, 1);
					MSG_WriteFloat(msg, sounds[i].pitch);
				}
				else {
					MSG_WriteBits(msg, 0, 1);
				}

				MSG_WriteFloat(msg, sounds[i].maxDist);
			}
			else {
				MSG_WriteBits(msg, 1, 1);
				MSG_WriteBits(msg, sounds[i].entity_number, 10);
				MSG_WriteBits(msg, sounds[i].channel, 7);
			}
		}
	}
}

/*
============================================================================

plyer_state_t communication

============================================================================
*/

// using the stringizing operator to save typing...
#define	PSF(x) #x,(size_t)&((playerState_t*)0)->x,sizeof(playerState_t::x)

netField_t	playerStateFields_ver_15[] =
{
{ PSF(commandTime), 32, netFieldType_t::regular },
{ PSF(origin[0]), 0, netFieldType_t::coordExtra },
{ PSF(origin[1]), 0, netFieldType_t::coordExtra },
{ PSF(viewangles[1]), 0, netFieldType_t::regular },
{ PSF(velocity[1]), 0, netFieldType_t::velocity },
{ PSF(velocity[0]), 0, netFieldType_t::velocity },
{ PSF(viewangles[0]), 0, netFieldType_t::regular },
{ PSF(origin[2]), 0, netFieldType_t::coordExtra },
{ PSF(velocity[2]), 0, netFieldType_t::velocity },
{ PSF(iViewModelAnimChanged), 2, netFieldType_t::regular },
{ PSF(damage_angles[0]), -13, netFieldType_t::angle },
{ PSF(damage_angles[1]), -13, netFieldType_t::angle },
{ PSF(damage_angles[2]), -13, netFieldType_t::angle },
{ PSF(speed), 16, netFieldType_t::regular },
{ PSF(delta_angles[1]), 16, netFieldType_t::regular },
{ PSF(viewheight), -8, netFieldType_t::regular },
{ PSF(groundEntityNum), GENTITYNUM_BITS, netFieldType_t::regular },
{ PSF(delta_angles[0]), 16, netFieldType_t::regular },
{ PSF(iNetViewModelAnim), 4, netFieldType_t::regular },
{ PSF(fov), 0, netFieldType_t::regular },
{ PSF(current_music_mood), 8, netFieldType_t::regular },
{ PSF(gravity), 16, netFieldType_t::regular },
{ PSF(fallback_music_mood), 8, netFieldType_t::regular },
{ PSF(music_volume), 0, netFieldType_t::regular },
{ PSF(net_pm_flags), 16, netFieldType_t::regular },
{ PSF(clientNum), 8, netFieldType_t::regular },
{ PSF(fLeanAngle), 0, netFieldType_t::regular },
{ PSF(blend[3]), 0, netFieldType_t::regular },
{ PSF(blend[0]), 0, netFieldType_t::regular },
{ PSF(pm_type), 8, netFieldType_t::regular },
{ PSF(feetfalling), 8, netFieldType_t::regular },
{ PSF(radarInfo), 26, netFieldType_t::regular },
{ PSF(camera_angles[0]), 16, netFieldType_t::angle },
{ PSF(camera_angles[1]), 16, netFieldType_t::angle },
{ PSF(camera_angles[2]), 16, netFieldType_t::angle },
{ PSF(camera_origin[0]), 0, netFieldType_t::coordExtra },
{ PSF(camera_origin[1]), 0, netFieldType_t::coordExtra },
{ PSF(camera_origin[2]), 0, netFieldType_t::coordExtra },
{ PSF(camera_posofs[0]), 0, netFieldType_t::coordExtra },
{ PSF(camera_posofs[2]), 0, netFieldType_t::coordExtra },
{ PSF(camera_time), 0, netFieldType_t::regular },
{ PSF(voted), 1, netFieldType_t::regular },
{ PSF(bobCycle), 8, netFieldType_t::regular },
{ PSF(delta_angles[2]), 16, netFieldType_t::regular },
{ PSF(viewangles[2]), 0, netFieldType_t::regular },
{ PSF(music_volume_fade_time), 0, netFieldType_t::regular },
{ PSF(reverb_type), 6, netFieldType_t::regular },
{ PSF(reverb_level), 0, netFieldType_t::regular },
{ PSF(blend[1]), 0, netFieldType_t::regular },
{ PSF(blend[2]), 0, netFieldType_t::regular },
{ PSF(camera_offset[0]), 0, netFieldType_t::regular },
{ PSF(camera_offset[1]), 0, netFieldType_t::regular },
{ PSF(camera_offset[2]), 0, netFieldType_t::regular },
{ PSF(camera_posofs[1]), 0, netFieldType_t::coordExtra },
{ PSF(camera_flags), 16, netFieldType_t::regular }
};
static constexpr unsigned long numPlayerStateFields_ver_15 = sizeof(playerStateFields_ver_15) / sizeof(playerStateFields_ver_15[0]);

netField_t	playerStateFields_ver_6[] = 
{
{ PSF(commandTime), 32, netFieldType_t::regular },				
{ PSF(origin[0]), 0, netFieldType_t::coord },
{ PSF(origin[1]), 0, netFieldType_t::coord },
{ PSF(viewangles[1]), 0, netFieldType_t::regular },
{ PSF(velocity[1]), 0, netFieldType_t::velocity },
{ PSF(velocity[0]), 0, netFieldType_t::velocity },
{ PSF(viewangles[0]), 0, netFieldType_t::regular },
{ PSF(pm_time), -16, netFieldType_t::regular },
//{ PSF(weaponTime), -16, netFieldType_t::regular },
{ PSF(origin[2]), 0, netFieldType_t::coord },
{ PSF(velocity[2]), 0, netFieldType_t::velocity },
{ PSF(iViewModelAnimChanged), 2, netFieldType_t::regular },
{ PSF(damage_angles[0]), -13, netFieldType_t::angle },
{ PSF(damage_angles[1]), -13, netFieldType_t::angle },
{ PSF(damage_angles[2]), -13, netFieldType_t::angle },
{ PSF(speed), 16, netFieldType_t::regular },
{ PSF(delta_angles[1]), 16, netFieldType_t::regular },
{ PSF(viewheight), -8, netFieldType_t::regular },
{ PSF(groundEntityNum), GENTITYNUM_BITS, netFieldType_t::regular },
{ PSF(delta_angles[0]), 16, netFieldType_t::regular },
{ PSF(iNetViewModelAnim), 4, netFieldType_t::regular },
{ PSF(fov), 0, netFieldType_t::regular },
{ PSF(current_music_mood), 8, netFieldType_t::regular },
{ PSF(gravity), 16, netFieldType_t::regular },
{ PSF(fallback_music_mood), 8, netFieldType_t::regular },
{ PSF(music_volume), 0, netFieldType_t::regular },
{ PSF(net_pm_flags), 16, netFieldType_t::regular },
{ PSF(clientNum), 8, netFieldType_t::regular },
{ PSF(fLeanAngle), 0, netFieldType_t::regular },
{ PSF(blend[3]), 0, netFieldType_t::regular },
{ PSF(blend[0]), 0, netFieldType_t::regular },
{ PSF(pm_type), 8, netFieldType_t::regular },
{ PSF(feetfalling), 8, netFieldType_t::regular },
{ PSF(camera_angles[0]), 16, netFieldType_t::angle },
{ PSF(camera_angles[1]), 16, netFieldType_t::angle },
{ PSF(camera_angles[2]), 16, netFieldType_t::angle },
{ PSF(camera_origin[0]), 0, netFieldType_t::coord },
{ PSF(camera_origin[1]), 0, netFieldType_t::coord },
{ PSF(camera_origin[2]), 0, netFieldType_t::coord },
{ PSF(camera_posofs[0]), 0, netFieldType_t::coord },
{ PSF(camera_posofs[2]), 0, netFieldType_t::coord },
{ PSF(camera_time), 0, netFieldType_t::regular },
{ PSF(bobCycle), 8, netFieldType_t::regular },
{ PSF(delta_angles[2]), 16, netFieldType_t::regular },
{ PSF(viewangles[2]), 0, netFieldType_t::regular },
{ PSF(music_volume_fade_time), 0, netFieldType_t::regular },
{ PSF(reverb_type), 6, netFieldType_t::regular },
{ PSF(reverb_level), 0, netFieldType_t::regular },
{ PSF(blend[1]), 0, netFieldType_t::regular },
{ PSF(blend[2]), 0, netFieldType_t::regular },
{ PSF(camera_offset[0]), 0, netFieldType_t::regular },
{ PSF(camera_offset[1]), 0, netFieldType_t::regular },
{ PSF(camera_offset[2]), 0, netFieldType_t::regular },
{ PSF(camera_posofs[1]), 0, netFieldType_t::coord },
{ PSF(camera_flags), 16, netFieldType_t::regular }
};
static constexpr unsigned long numPlayerStateFields_ver_6 = sizeof(playerStateFields_ver_6) / sizeof(playerStateFields_ver_6[0]);
static constexpr unsigned long numBiggestPlayerStateFields = numPlayerStateFields_ver_15 >= numPlayerStateFields_ver_6 ? numPlayerStateFields_ver_15 : numPlayerStateFields_ver_6;

netField_t* MSG_GetPlayerStateFields(size_t& outNumFields)
{
    if (com_protocol->integer >= protocol_e::PROTOCOL_MOHTA_MIN)
    {
		outNumFields = sizeof(playerStateFields_ver_15) / sizeof(playerStateFields_ver_15[0]);
		return playerStateFields_ver_15;
    }
    else
    {
        outNumFields = sizeof(playerStateFields_ver_6) / sizeof(playerStateFields_ver_6[0]);
        return playerStateFields_ver_6;
    }
}

/*
=============
MSG_WriteDeltaPlayerstate

=============
*/
void MSG_WriteDeltaPlayerstate(msg_t *msg, struct playerState_s *from, struct playerState_s *to, float frameTime) {
	int				i;
	playerState_t	dummy;
	int				statsbits;
	int				activeitemsbits;
	int				ammobits;
	int				ammo_amountbits;
	int				max_ammo_amountbits;
	size_t			numFields;
	size_t			c;
	netField_t		*playerStateFields;
	netField_t		*field;
	int				*fromF, *toF;
	int				lc;

	if (!from) {
		from = &dummy;
		Com_Memset (&dummy, 0, sizeof(dummy));
	}

	c = msg->cursize;

	playerStateFields = MSG_GetPlayerStateFields(numFields);

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
		iPlayerFieldChanges[i]++;

		switch ( field->type ) {
			case netFieldType_e::regular:
				MSG_WriteRegularSimple(msg, field->bits, field->size, toF);
				break;
			case netFieldType_e::angle:
				MSG_WritePackedAngle(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::animTime:
				MSG_WritePackedAnimTime(msg, *(float*)fromF, *(float*)toF, frameTime, field->bits);
				break;
			case netFieldType_e::animWeight:
				MSG_WritePackedAnimWeight(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::scale:
				MSG_WritePackedScale(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::alpha:
				MSG_WritePackedAlpha(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::coord:
				MSG_WritePackedCoord(msg, *(float*)fromF, *(float*)toF, field->bits);
				break;
			case netFieldType_e::coordExtra:
				// Team Assault
				MSG_WritePackedCoordExtra(msg, *(float*)fromF, *(float*)toF, field->bits);
				break;
			case netFieldType_e::velocity:
				MSG_WritePackedVelocity(msg, *(float*)toF, field->bits);
				break;
			case netFieldType_e::simple:
				MSG_WritePackedSimple(msg, *(int*)toF, field->bits);
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
	for (i=0 ; i<MAX_ACTIVE_ITEMS ; i++) {
		if (to->activeItems[i] != from->activeItems[i]) {
			activeitemsbits |= 1<<i;
		}
	}
	ammo_amountbits = 0;
	for (i=0 ; i<MAX_AMMOCOUNT ; i++) {
		if (to->ammo_amount[i] != from->ammo_amount[i]) {
			ammo_amountbits |= 1<<i;
		}
	}
	ammobits = 0;
	for (i=0 ; i<MAX_AMMO ; i++) {
		if (to->ammo_name_index[i] != from->ammo_name_index[i]) {
			ammobits |= 1<<i;
		}
	}
	max_ammo_amountbits = 0;
	for (i=0 ; i<MAX_AMMOCOUNT ; i++) {
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
		MSG_WriteBits( msg, activeitemsbits, MAX_ACTIVE_ITEMS );
		for (i=0 ; i<MAX_ACTIVE_ITEMS ; i++)
			if (activeitemsbits & (1<<i) )
				MSG_WriteShort (msg, to->activeItems[i]);
	} else {
		MSG_WriteBits( msg, 0, 1 );	// no change
	}

	if ( ammo_amountbits ) {
		MSG_WriteBits( msg, 1, 1 );	// changed
		MSG_WriteBits( msg, ammo_amountbits, MAX_AMMOCOUNT );
		for (i=0 ; i<MAX_AMMOCOUNT ; i++)
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
		MSG_WriteBits( msg, max_ammo_amountbits, MAX_AMMOCOUNT );
		for (i=0 ; i<MAX_AMMOCOUNT ; i++)
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
void MSG_ReadDeltaPlayerstate(msg_t *msg, playerState_t *from, playerState_t *to, float frameTime) {
	int			i, lc;
	int			bits;
	netField_t* playerStateFields;
	netField_t	*field;
	size_t		numFields;
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

    playerStateFields = MSG_GetPlayerStateFields(numFields);

	lc = MSG_ReadByte(msg);
	assert(lc <= numFields);
	for ( i = 0, field = playerStateFields ; i < lc ; i++, field++ ) {
		fromF = (int *)( (byte *)from + field->offset );
		toF = (int *)( (byte *)to + field->offset );
		if ( ! MSG_ReadBits( msg, 1 ) ) {
			// no change
			*toF = *fromF;
		} else {
			switch ( field->type ) {
				case netFieldType_e::regular:
					MSG_ReadRegularSimple(msg, field->bits, field->size, toF);
					break;
				case netFieldType_e::angle:
					*(float*)toF = MSG_ReadPackedAngle(msg, field->bits);
					break;
				case netFieldType_e::coord:
					*(float*)toF = MSG_ReadPackedCoord(msg, *(float*)fromF, field->bits);
					break;
				case netFieldType_e::coordExtra:
					*(float*)toF = MSG_ReadPackedCoordExtra(msg, *(float*)fromF, field->bits);
					break;
				case netFieldType_e::velocity:
					*(float*)toF = MSG_ReadPackedVelocity(msg, field->bits);
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
			for (i=0 ; i<MAX_ACTIVE_ITEMS ; i++) {
				if (bits & (1<<i) ) {
					to->activeItems[i] = MSG_ReadShort(msg);
				}
			}
		}

		// parse ammo_amount
		if ( MSG_ReadBits( msg, 1 ) ) {
			LOG("PS_AMMO_AMOUNT");
			bits = MSG_ReadShort(msg);
			for (i=0 ; i<MAX_AMMOCOUNT ; i++) {
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
			LOG("PS_MAX_AMMOCOUNT");
			bits = MSG_ReadShort(msg);
			for (i=0 ; i<MAX_AMMOCOUNT ; i++) {
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

float MSG_ReadServerFrameTime_ver_15(msg_t* msg) {
    return MSG_ReadFloat(msg);
}

float MSG_ReadServerFrameTime_ver_6(msg_t* msg, gameState_t* gameState) {
    return 1.f / atof(Info_ValueForKey(gameState->stringData + gameState->stringOffsets[CS_SERVERINFO], "sv_fps"));
}

float MSG_ReadServerFrameTime(msg_t* msg, gameState_t* gameState) {
    if (MSG_IsProtocolVersion15()) {
        return MSG_ReadServerFrameTime_ver_15(msg);
    } else {
        // smaller below version 15
        return MSG_ReadServerFrameTime_ver_6(msg, gameState);
    }
}

void MSG_WriteServerFrameTime_ver_15(msg_t* msg, float value) {
    MSG_WriteFloat(msg, value);
}

void MSG_WriteServerFrameTime_ver_6(msg_t* msg, float value) {
    // Nothing to write
}

void MSG_WriteServerFrameTime(msg_t* msg, float value) {
    if (MSG_IsProtocolVersion15()) {
        MSG_WriteServerFrameTime_ver_15(msg, value);
    }
    else {
        MSG_WriteServerFrameTime_ver_6(msg, value);
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
