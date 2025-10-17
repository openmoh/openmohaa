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
//
// q_shared.c -- stateless support routines that are included in each code dll
#include "q_shared.h"

#ifdef WIN32
#include <intrin.h>
#endif

static char musicmoods[ mood_totalnumber ][ 16 ] =
{
	"none",
	"normal",
	"action",
	"suspense",
	"mystery",
	"success",
	"failure",
	"surprise",
	"special",
	"aux1",
	"aux2",
	"aux3",
	"aux4",
	"aux5",
	"aux6",
	"aux7"
};

static char eaxmodes[ eax_totalnumber ][ 16 ] =
{
	"generic",
	"paddedcell",
	"room",
	"bathroom",
	"livingroom",
	"stoneroom",
	"auditorium",
	"concerthall",
	"cave",
	"arena",
	"hangar",
	"carpetedhallway",
	"hallway",
	"stonecorridor",
	"alley",
	"forest",
	"city",
	"mountains",
	"quarry",
	"plain",
	"parkinglot",
	"sewerpipe",
	"underwater",
	"drugged",
	"dizzy",
	"psychotic"
};

float vectoyaw(const vec3_t vec)
{
	float yaw;

	if (vec[YAW] == 0 && vec[PITCH] == 0) {
		yaw = 0;
	}
	else {
		//if (vec[PITCH]) {
		yaw = (int)(atan2(vec[YAW], vec[PITCH]) * 180 / M_PI);
		//} else if (vec[YAW] > 0) {
		//	yaw = 90;
		//} else {
		//	yaw = 270;
		//}
		if (yaw < 0) {
			yaw += 360;
		}
	}

	return yaw;
}

/*
=================
MusicMood_NameToNum
=================
*/
int MusicMood_NameToNum( const char * name )
{
	int i;

	if( !name )
		return -1;

	for( i = 0; i < mood_totalnumber; i++ )
	{
		if( !Q_stricmp( name, musicmoods[ i ] ) )
		{
			return i;
		}
	}
	return -1;
}

/*
=================
MusicMood_NumToName
=================
*/
const char * MusicMood_NumToName( int num )
{
	if( ( num < 0 ) || ( num >= mood_totalnumber ) )
		return "";
	else
		return musicmoods[ num ];
}

/*
=================
EAXMode_NameToNum
=================
*/
int EAXMode_NameToNum( const char * name )
{
	int i;

	if( !name )
		return -1;

	for( i = 0; i < eax_totalnumber; i++ )
	{
		if( !Q_stricmp( name, eaxmodes[ i ] ) )
		{
			return i;
		}
	}
	return -1;
}

/*
=================
EAXMode_NumToName
=================
*/
const char * EAXMode_NumToName( int num )
{
	if( ( num < 0 ) || ( num >= eax_totalnumber ) )
		return "";
	else
		return eaxmodes[ num ];
}

float Com_Clamp( float min, float max, float value ) {
	if ( value < min ) {
		return min;
	}
	if ( value > max ) {
		return max;
	}
	return value;
}


/*
============
COM_SkipPath
============
*/
const char *COM_SkipPath (const char *pathname)
{
	const char	*last;
	
	last = pathname;
	while (*pathname)
	{
		if (*pathname=='/' || *pathname=='\\')
			last = pathname+1;
		pathname++;
	}
	return last;
}

/*
============
COM_GetExtension
============
*/
const char *COM_GetExtension( const char *name ) {
	size_t length, i;

	length = strlen(name)-1;
	i = length;

	while (name[i] != '.')
	{
		i--;
		if (name[i] == '/' || i == 0)
			return ""; // no extension
	}

	return &name[i+1];
}


/*
============
COM_StripExtension
============
*/
void COM_StripExtension( const char *in, char *out, int destsize ) {
	size_t length;

	Q_strncpyz(out, in, destsize);

	length = strlen(out)-1;
	while (length > 0 && out[length] != '.')
	{
		length--;
		if (out[length] == '/')
			return;		// no extension
	}
	if (length)
		out[length] = 0;
}

/*
============
COM_CompareExtension

string compare the end of the strings and return qtrue if strings match
============
*/
qboolean COM_CompareExtension(const char* in, const char* ext)
{
	size_t inlen, extlen;

	inlen = strlen(in);
	extlen = strlen(ext);

	if (extlen <= inlen)
	{
		in += inlen - extlen;

		if (!Q_stricmp(in, ext))
			return qtrue;
	}

	return qfalse;
}

/*
==================
COM_DefaultExtension
==================
*/
void COM_DefaultExtension (char *path, int maxSize, const char *extension ) {
	char	oldPath[MAX_QPATH];
	char    *src;

//
// if path doesn't have a .EXT, append extension
// (extension should include the .)
//
	src = path + strlen(path) - 1;

	while (*src != '/' && src != path) {
		if ( *src == '.' ) {
			return;                 // it has an extension
		}
		src--;
	}

	Q_strncpyz( oldPath, path, sizeof( oldPath ) );
	Com_sprintf( path, maxSize, "%s%s", oldPath, extension );
}

/*
============
Com_HashKey
============
*/
int Com_HashKey(char *string, int maxlen) {
	int register hash, i;

	hash = 0;
	for (i = 0; i < maxlen && string[i] != '\0'; i++) {
		hash += (signed char)string[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20));
	return hash;
}


/*
============================================================================

PARSING

============================================================================
*/

static	char	com_token[MAX_TOKEN_CHARS];
static	char	com_parsename[MAX_TOKEN_CHARS];
static	int		com_lines;

void COM_BeginParseSession( const char *name )
{
	com_lines = 0;
	Com_sprintf(com_parsename, sizeof(com_parsename), "%s", name);
}

int COM_GetCurrentParseLine( void )
{
	return com_lines;
}

const char *COM_Parse( char **data_p )
{
	return COM_GetToken( data_p, 1 );
}

void COM_ParseError( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("ERROR: %s, line %d: %s\n", com_parsename, com_lines, string);
}

void COM_ParseWarning( char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("WARNING: %s, line %d: %s\n", com_parsename, com_lines, string);
}

/*
==============
COM_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
static char *SkipWhitespace( char *data, qboolean *hasNewLines ) {
	unsigned int c;

	while( (c = *data) <= ' ') {
		if( !c ) {
			return NULL;
		}
		if( c == '\n' ) {
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}

int COM_Compress( char *data_p ) {
	char *in, *out;
	int c;
	qboolean newline = qfalse, whitespace = qfalse;

	in = out = data_p;
	if (in) {
		while ((c = *in) != 0) {
			// skip double slash comments
			if ( c == '/' && in[1] == '/' ) {
				while (*in && *in != '\n') {
					in++;
				}
			// skip /* */ comments
			} else if ( c == '/' && in[1] == '*' ) {
				while ( *in && ( *in != '*' || in[1] != '/' ) ) 
					in++;
				if ( *in ) 
					in += 2;
                        // record when we hit a newline
                        } else if ( c == '\n' || c == '\r' ) {
                            newline = qtrue;
                            in++;
                        // record when we hit whitespace
                        } else if ( c == ' ' || c == '\t') {
                            whitespace = qtrue;
                            in++;
                        // an actual token
			} else {
                            // if we have a pending newline, emit it (and it counts as whitespace)
                            if (newline) {
                                *out++ = '\n';
                                newline = qfalse;
                                whitespace = qfalse;
                            } if (whitespace) {
                                *out++ = ' ';
                                whitespace = qfalse;
                            }
                            
                            // copy quoted strings unmolested
                            if (c == '"') {
                                    *out++ = c;
                                    in++;
                                    while (1) {
                                        c = *in;
                                        if (c && c != '"') {
                                            *out++ = c;
                                            in++;
                                        } else {
                                            break;
                                        }
                                    }
                                    if (c == '"') {
                                        *out++ = c;
                                        in++;
                                    }
                            } else {
                                *out = c;
                                out++;
                                in++;
                            }
			}
		}
	}
	*out = 0;
	return out - data_p;
}

char *COM_ParseExt( char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	char *data;

	data = *data_p;
	len = 0;
	com_token[ 0 ] = 0;

	// make sure incoming data is valid
	if( !data )
	{
		*data_p = NULL;
		return com_token;
	}

	while( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if( !data )
		{
			*data_p = NULL;
			return com_token;
		}
		if( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if( c == '/' && data[ 1 ] == '/' )
		{
			while( *data && *data != '\n' )
				data++;
		}
		// skip /* */ comments
		else if( c == '/' && data[ 1 ] == '*' )
		{
			while( *data && ( *data != '*' || data[ 1 ] != '/' ) )
			{
				data++;
			}
			if( *data )
			{
				data += 2;
			}
		}
		else
		{
			break;
		}
	}

	// handle quoted strings
	if( c == '\"' )
	{
		data++;
		while( 1 )
		{
			c = *data++;
			if( c == '\"' || !c )
			{
				com_token[ len ] = 0;
				*data_p = ( char * )data;
				return com_token;
			}
			if( len < MAX_TOKEN_CHARS )
			{
				com_token[ len ] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if( ( len > 0 ) && ( c == '{' || c == '}' ) )
		{
			break;
		}

		data++;
		if( len < MAX_TOKEN_CHARS )
		{
			// handle '\n' correctly
			if( c == '\\' && *data == 'n' )
			{
				com_token[ len ] = '\n';
				data++;
			}
			else
			{
				com_token[ len ] = c;
			}
			len++;

			if( ( len == 1 ) && ( c == '{' || c == '}' ) )
			{
				break;
			}

		}
		c = *data;
		if( c == '\n' )
			com_lines++;
	} while( c>32 );

	if( len == MAX_TOKEN_CHARS )
	{
		//		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_TOKEN_CHARS);
		len = 0;
	}
	com_token[ len ] = 0;

	*data_p = ( char * )data;
	return com_token;
}

char *COM_ParseExtIgnoreQuotes( char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	char *data;

	data = *data_p;
	len = 0;
	com_token[0] = 0;

	// make sure incoming data is valid
	if ( !data )
	{
		*data_p = NULL;
		return com_token;
	}

	while ( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data )
		{
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' )
		{
			data += 2;
			while (*data && *data != '\n') {
				data++;
			}
		}
		// skip /* */ comments
		else if ( c=='/' && data[1] == '*' ) 
		{
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) ) 
			{
				data++;
			}
			if ( *data ) 
			{
				data += 2;
			}
		}
		else
		{
			break;
		}
	}

	// parse a regular word
	do
	{
		if (len < MAX_TOKEN_CHARS - 1)
		{
			com_token[len] = c;
			len++;
		}
		data++;
		c = *data;
		if ( c == '\n' )
			com_lines++;
	} while (c>32);

	com_token[len] = 0;

	*data_p = ( char * ) data;
	return com_token;
}

/*
==============
COM_GetToken

Parse a token out of a string
==============
*/
const char *COM_GetToken( char **data_p, qboolean crossline )
{
	int		c;
	int		len;
	char *data;

	data = *data_p;
	len = 0;
	com_token[ 0 ] = 0;

	if( !data )
	{
		*data_p = NULL;
		return "";
	}

	// skip whitespace
skipwhite:
	while( ( c = *data ) <= ' ' )
	{
		if( c == '\n' && !crossline )
		{
			*data_p = data;
			return "";
		}
		if( !c )
		{
			*data_p = NULL;
			return "";
		}
		data++;
	}

	// skip // comments
	if( c == '/' && data[ 1 ] == '/' )
	{
		while( *data && *data != '\n' )
			data++;
		goto skipwhite;
	}

	// skip /* comments
	if( c == '/' && data[ 1 ] == '*' )
	{
		data++;
		while( *data )
		{
			if( ( *( data - 1 ) == '*' ) && ( *data == '/' ) )
				break;
			data++;
		}
		while( *data && *data != '\n' )
			data++;
		goto skipwhite;
	}


	// handle quoted strings specially
	if( c == '\"' )
	{
		data++;
		while( 1 )
		{
			c = *data++;
			if( c == '\\' && *data == '\"' )
			{
				if( len < MAX_STRING_CHARS )
				{
					com_token[ len ] = '\"';
					len++;
				}
				data++;
			}
			else if( c == '\"' || !c )
			{
				com_token[ len ] = 0;
				*data_p = data;
				return com_token;
			}
			else if( len < MAX_STRING_CHARS )
			{
				if( c == '\\' && *data == 'n' )
				{
					com_token[ len ] = '\n';
					data++;
				}
				else
				{
					com_token[ len ] = c;
				}
				len++;
				//            com_token[len] = c;
				//				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if( len < MAX_STRING_CHARS )
		{
			com_token[ len ] = c;
			len++;
		}
		data++;
		c = *data;
	} while( c>32 );

	if( len == MAX_STRING_CHARS )
	{
		//		Com_Printf ("Token exceeded %i chars, discarded.\n", MAX_STRING_CHARS);
		len = 0;
	}
	com_token[ len ] = 0;

	*data_p = data;
	return com_token;
}

/*
=================
Com_SkipBracedSection

The next token should be an open brace.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
void Com_SkipBracedSection(char **program)
{
	char           *token;
	int             depth;

	depth = 0;
	do
	{
		token = COM_ParseExt(program, qtrue);
		if(token[1] == 0)
		{
			if(token[0] == '{')
			{
				depth++;
			}
			else if(token[0] == '}')
			{
				depth--;
			}
		}
	} while(depth && *program);
}

/*
=================
Com_SkipRestOfLine
=================
*/
void Com_SkipRestOfLine(char **data)
{
	char           *p;
	int             c;

	p = *data;
	while((c = *p++) != 0)
	{
		if(c == '\n')
		{
			com_lines++;
			break;
		}
	}

	*data = p;
}

void Com_Parse1DMatrix(char **buf_p, int x, float *m, qboolean checkBrackets)
{
	const char	*token;
	int			i;

	if( checkBrackets )
	{
		COM_MatchToken( buf_p, "(" );
	}

	for( i = 0; i < x; i++ )
	{
		token = COM_Parse( buf_p );
		m[ i ] = atof( token );
	}

	if( checkBrackets )
	{
		COM_MatchToken( buf_p, ")" );
	}
}


#if 0
// no longer used
/*
===============
COM_ParseInfos
===============
*/
int COM_ParseInfos( char *buf, int max, char infos[][MAX_INFO_STRING] ) {
	char	*token;
	int		count;
	char	key[MAX_TOKEN_CHARS];

	count = 0;

	while ( 1 ) {
		token = COM_Parse( &buf );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) {
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		infos[count][0] = 0;
		while ( 1 ) {
			token = COM_ParseExt( &buf, qtrue );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, qfalse );
			if ( !token[0] ) {
				strcpy( token, "<NULL>" );
			}
			Info_SetValueForKey( infos[count], key, token );
		}
		count++;
	}

	return count;
}
#endif


/*
==================
COM_MatchToken
==================
*/
void COM_MatchToken( char **buf_p, char *match ) {
	const char *token;

	token = COM_Parse( buf_p );
	if( strcmp( token, match ) ) {
		Com_Error( ERR_DROP, "MatchToken: %s != %s", token, match );
	}
}


/*
=================
SkipBracedSection

The next token should be an open brace or set depth to 1 if already parsed it.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
qboolean SkipBracedSection (char **program, int depth) {
	char			*token;

	do {
		token = COM_ParseExt( program, qtrue );
		if( token[1] == 0 ) {
			if( token[0] == '{' ) {
				depth++;
			}
			else if( token[0] == '}' ) {
				depth--;
			}
		}
	} while( depth && *program );

	return ( depth == 0 );
}

/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine ( char **data ) {
	char	*p;
	int		c;

	p = *data;
	while ( (c = *p++) != 0 ) {
		if ( c == '\n' ) {
			com_lines++;
			break;
		}
	}

	*data = p;
}


void Parse1DMatrix (char **buf_p, int x, float *m) {
	const char	*token;
	int			i;

	COM_MatchToken( buf_p, "(" );

	for( i = 0; i < x; i++ ) {
		token = COM_Parse( buf_p );
		m[ i ] = atof( token );
	}

	COM_MatchToken( buf_p, ")" );
}

void Parse2DMatrix (char **buf_p, int y, int x, float *m) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < y ; i++) {
		Parse1DMatrix (buf_p, x, m + i * x);
	}

	COM_MatchToken( buf_p, ")" );
}

void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < z ; i++) {
		Parse2DMatrix (buf_p, y, x, m + i * x*y);
	}

	COM_MatchToken( buf_p, ")" );
}

/*
===================
Com_HexStrToInt
===================
*/
int Com_HexStrToInt( const char *str )
{
	if ( !str || !str[ 0 ] )
		return -1;

	// check for hex code
	if( str[ 0 ] == '0' && str[ 1 ] == 'x' )
	{
		int i, n = 0;

		for( i = 2; i < strlen( str ); i++ )
		{
			char digit;

			n *= 16;

			digit = tolower( str[ i ] );

			if( digit >= '0' && digit <= '9' )
				digit -= '0';
			else if( digit >= 'a' && digit <= 'f' )
				digit = digit - 'a' + 10;
			else
				return -1;

			n += digit;
		}

		return n;
	}

	return -1;
}


/*
============================================================================

					LIBRARY REPLACEMENT FUNCTIONS

============================================================================
*/

int Q_isprint(int c)
{
	if (c >= 0x20 && c <= 0x7E)
		return (1);
	return (0);
}

int Q_islower(int c)
{
	if (c >= 'a' && c <= 'z')
		return (1);
	return (0);
}

int Q_isupper(int c)
{
	if (c >= 'A' && c <= 'Z')
		return (1);
	return (0);
}

int Q_isalpha(int c)
{
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
		return (1);
	return (0);
}

qboolean Q_isanumber(const char* s)
{
	char *p;
	double Q_UNUSED_VAR d;

	if (*s == '\0')
		return qfalse;

	d = strtod(s, &p);

	return *p == '\0';
}

qboolean Q_isintegral(float f)
{
	return (int)f == f;
}

#ifdef _WIN32
/*
=============
Q_vsnprintf

Special wrapper function for Microsoft's broken _vsnprintf() function.
MinGW comes with its own vsnprintf() which is not broken. mingw-w64
however, uses Microsoft's broken _vsnprintf() function.
=============
*/

size_t Q_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
	int retval;

	retval = _vsnprintf(str, size, format, ap);

	if (retval < 0 || retval == size)
	{
		// Microsoft doesn't adhere to the C99 standard of vsnprintf,
		// which states that the return value must be the number of
		// bytes written if the output string had sufficient length.
		//
		// Obviously we cannot determine that value from Microsoft's
		// implementation, so we have no choice but to return size.

		str[size - 1] = '\0';
		return size;
	}

	return retval;
}
#endif

char* Q_strrchr( const char* string, int c )
{
	char cc = c;
	char *s;
	char *sp=(char *)0;

	s = (char*)string;

	while (*s)
	{
		if (*s == cc)
			sp = s;
		s++;
	}
	if (cc == 0)
		sp = s;

	return sp;
}

/*
=============
Q_strncpyz
 
Safe strncpy that ensures a trailing zero
=============
*/
void Q_strncpyz( char *dest, const char *src, size_t destsize ) {
    if ( !dest ) {
        Com_Error( ERR_FATAL, "Q_strncpyz: NULL dest" );
    }

    if ( !src ) {
        Com_Error( ERR_FATAL, "Q_strncpyz: NULL src" );
    }

    if ( destsize < 1 ) {
        Com_Error( ERR_FATAL, "Q_strncpyz: destsize < 1" );
    }

    if ( dest == src ) {
        return;
    }

    strncpy( dest, src, destsize - 1 );
    dest[ destsize - 1 ] = 0;
}
                 
int Q_stricmpn( const char *s1, const char *s2, size_t n ) {
	int		c1, c2;

        if ( s1 == NULL ) {
           if ( s2 == NULL )
             return 0;
           else
             return -1;
        }
        else if ( s2==NULL )
          return 1;


	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			if (c1 >= 'a' && c1 <= 'z') {
				c1 -= ('a' - 'A');
			}
			if (c2 >= 'a' && c2 <= 'z') {
				c2 -= ('a' - 'A');
			}
			if (c1 != c2) {
				return c1 < c2 ? -1 : 1;
			}
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int Q_strncmp( const char *s1, const char *s2, size_t n ) {
	int		c1, c2;
	
	do {
		c1 = *s1++;
		c2 = *s2++;

		if (!n--) {
			return 0;		// strings are equal until end point
		}
		
		if (c1 != c2) {
			return c1 < c2 ? -1 : 1;
		}
	} while (c1);
	
	return 0;		// strings are equal
}

int Q_stricmp (const char *s1, const char *s2) {
	return (s1 && s2) ? Q_stricmpn (s1, s2, 99999) : -1;
}


char *Q_strlwr( char *s1 ) {
    char	*s;

    s = s1;
	while ( *s ) {
		*s = tolower(*s);
		s++;
	}
    return s1;
}

char *Q_strupr( char *s1 ) {
    char	*s;

    s = s1;
	while ( *s ) {
		*s = toupper(*s);
		s++;
	}
    return s1;
}


// never goes past bounds or leaves without a terminating 0
void Q_strcat( char *dest, int size, const char *src ) {
	size_t l1;

	l1 = strlen( dest );
	if ( l1 >= size ) {
		Com_Error( ERR_FATAL, "Q_strcat: already overflowed" );
	}
	Q_strncpyz( dest + l1, src, size - l1 );
}

/*
* Find the first occurrence of find in s.
*/
const char *Q_stristr( const char *s, const char *find)
{
  char c, sc;
  size_t len;

  if ((c = *find++) != 0)
  {
    if (c >= 'a' && c <= 'z')
    {
      c -= ('a' - 'A');
    }
    len = strlen(find);
    do
    {
      do
      {
        if ((sc = *s++) == 0)
          return NULL;
        if (sc >= 'a' && sc <= 'z')
        {
          sc -= ('a' - 'A');
        }
      } while (sc != c);
    } while (Q_stricmpn(s, find, len) != 0);
    s--;
  }
  return s;
}



/*
=============
Q_strreplace

replaces content of find by replace in dest
=============
*/
qboolean Q_strreplace(char *dest, int destsize, const char *find, const char *replace)
{
	size_t		lstart, lfind, lreplace, lend;
	char		*s;
	char		backup[32000];	// big, but small enough to fit in PPC stack

	lend = strlen(dest);
	if(lend >= destsize)
	{
		Com_Error(ERR_FATAL, "Q_strreplace: already overflowed");
	}

	s = strstr(dest, find);
	if(!s)
	{
		return qfalse;
	}
	else
	{
		Q_strncpyz(backup, dest, lend + 1);
		lstart = s - dest;
		lfind = strlen(find);
		lreplace = strlen(replace);

		strncpy(s, replace, destsize - lstart - 1);
		strncpy(s + lreplace, backup + lstart + lfind, destsize - lstart - lreplace - 1);

		return qtrue;
	}
}



int Q_PrintStrlen( const char *string ) {
	int			len;
	const char	*p;

	if( !string ) {
		return 0;
	}

	len = 0;
	p = string;
	while( *p ) {
		if( Q_IsColorString( p ) ) {
			p += 2;
			continue;
		}
		p++;
		len++;
	}

	return len;
}


char *Q_CleanStr( char *string ) {
	char*	d;
	char*	s;
	int		c;

	s = string;
	d = string;
	while ((c = *s) != 0 ) {
		if ( Q_IsColorString( s ) ) {
			s++;
		}		
		else if ( c >= 0x20 && c <= 0x7E ) {
			*d++ = c;
		}
		s++;
	}
	*d = '\0';

	return string;
}

int Q_CountChar(const char* string, char tocount)
{
    int count;

    for (count = 0; *string; string++)
    {
        if (*string == tocount)
            count++;
    }

    return count;
}

size_t QDECL Com_sprintf( char *dest, size_t size, const char *fmt, ...) {
	size_t		len;
	va_list		argptr;

	va_start(argptr, fmt);
	len = Q_vsnprintf(dest, size, fmt, argptr);
	va_end(argptr);

	if (len >= size)
		Com_Printf("Com_sprintf: Output length %zd too short, require %zd bytes.\n", size, len + 1);

	return len;
}

void Com_BackslashToSlash( char *str )
{
	int i;
	size_t len;

	len = strlen( str );

	for( i = 0; i < len; i++ )
	{
		if( str[ i ] == '\\' ) {
			str[ i ] = '/';
		}
	}
}


/*
============================================================================
					BYTE ORDER FUNCTIONS
============================================================================
*/

void CopyShortSwap(void* dest, const void* src)
{
	const byte* from = src;
	byte* to = dest;
	byte tmp[2];

	Com_Memcpy(tmp, from, sizeof(tmp));

	to[0] = tmp[1];
	to[1] = tmp[0];
}

void CopyLongSwap(void* dest, const void* src)
{
	const byte* from = src;
	byte* to = dest;
	byte tmp[4];

	Com_Memcpy(tmp, from, sizeof(tmp));

	to[0] = tmp[3];
	to[1] = tmp[2];
	to[2] = tmp[1];
	to[3] = tmp[0];
}

void SwapValue(void* dest, size_t size) {
	byte* data = (byte*)dest;

	// swap values
	for (size_t i = size / 2; i > 0; --i)
	{
		uint8_t* in = data + i - 1;
		uint8_t* out = data + size - i;

		const uint8_t tmp = *in;
		*in = *out;
		*out = tmp;
	}
}

short ShortSwap(short l)
{
	byte b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

unsigned short UnsignedShortSwap(unsigned short l)
{
	byte b1, b2;

	b1 = l & 255;
	b2 = (l >> 8) & 255;

	return (b1 << 8) + b2;
}

short ShortNoSwap(short l)
{
	return l;
}

unsigned short UnsignedShortNoSwap(unsigned short l)
{
	return l;
}

int LongSwap(int l)
{
	byte b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((int)b1 << 24) + ((int)b2 << 16) + ((int)b3 << 8) + b4;
}

unsigned int UnsignedLongSwap(unsigned int l)
{
	byte b1, b2, b3, b4;

	b1 = l & 255;
	b2 = (l >> 8) & 255;
	b3 = (l >> 16) & 255;
	b4 = (l >> 24) & 255;

	return ((unsigned int)b1 << 24) + ((unsigned int)b2 << 16) + ((unsigned int)b3 << 8) + b4;
}

int LongNoSwap(int l)
{
	return l;
}

unsigned int UnsignedLongNoSwap(unsigned int l)
{
	return l;
}

qint64 Long64Swap(qint64 ll)
{
	qint64 result;

	result.b0 = ll.b7;
	result.b1 = ll.b6;
	result.b2 = ll.b5;
	result.b3 = ll.b4;
	result.b4 = ll.b3;
	result.b5 = ll.b2;
	result.b6 = ll.b1;
	result.b7 = ll.b0;

	return result;
}

qint64 Long64NoSwap(qint64 ll)
{
	return ll;
}

float FloatSwap(const float* f) {
	floatint_t out;

	out.f = *f;
	out.ui = LongSwap(out.ui);

	return out.f;
}

float FloatNoSwap(const float* f)
{
	return *f;
}

short ShortSwapPtr(const void* l)
{
	short out;
	CopyLittleShort(&out, l);
	return out;
}

int LongSwapPtr(const void* l)
{
	int out;
	CopyLittleLong(&out, l);
	return out;
}

short ShortNoSwapPtr(const void* l)
{
	short out;
	Com_Memcpy(&out, l, sizeof(short));
	return out;
}

int LongNoSwapPtr(const void* l)
{
	int out;
	Com_Memcpy(&out, l, sizeof(int));
	return out;
}

float FloatSwapPtr(const void* l)
{
	float out;
	CopyLittleLong((int*)&out, l);
	return out;
}

float FloatNoSwapPtr(const void* l)
{
	float out;
	Com_Memcpy(&out, l, sizeof(float));
	return out;
}

void Swap_Init(void)
{
	// Endianness is now computed at compile time
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
============
*/
const char *va( const char *format, ... )
{
	va_list		argptr;
	static char		string[ 2 ][ 16384 ];	// in case va is called by nested functions
	static int		index = 0;
	char	*buf;

	buf = string[ index & 1 ];
	index++;

	va_start( argptr, format );
	Q_vsnprintf( buf, sizeof(*string), format, argptr );
	va_end( argptr );

	return buf;
}

/*
============
Com_TruncateLongString

Assumes buffer is atleast TRUNCATE_LENGTH big
============
*/
void Com_TruncateLongString( char *buffer, const char *s )
{
	size_t length = strlen( s );

	if( length <= TRUNCATE_LENGTH )
		Q_strncpyz( buffer, s, TRUNCATE_LENGTH );
	else
	{
		Q_strncpyz( buffer, s, ( TRUNCATE_LENGTH / 2 ) - 3 );
		Q_strcat( buffer, TRUNCATE_LENGTH, " ... " );
		Q_strcat( buffer, TRUNCATE_LENGTH, s + length - ( TRUNCATE_LENGTH / 2 ) + 3 );
	}
}

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
FIXME: overflow check?
===============
*/
char *Info_ValueForKey( const char *s, const char *key ) {
	char	pkey[BIG_INFO_KEY];
	static	char value[2][BIG_INFO_VALUE];	// use two buffers so compares
											// work without stomping on each other
	static	int	valueindex = 0;
	char	*o;
	
	if ( !s || !key ) {
		return "";
	}

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring" );
	}

	valueindex ^= 1;
	if (*s == '\\')
		s++;
	while (1)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value[valueindex];

		while (*s != '\\' && *s)
		{
			if (!*s)
				return "";
			*o++ = *s++;
		}
		*o = 0;

		if (!Q_stricmp (key, pkey) )
			return value[valueindex];

		if (!*s)
			break;
		s++;
	}

	return "";
}


/*
===================
Info_NextPair

Used to itterate through all the key/value pairs in an info string
===================
*/
void Info_NextPair( const char **head, char *key, char *value ) {
	char	*o;
	const char	*s;

	s = *head;

	if ( *s == '\\' ) {
		s++;
	}
	key[0] = 0;
	value[0] = 0;

	o = key;
	while ( *s != '\\' ) {
		if ( !*s ) {
			*o = 0;
			*head = s;
			return;
		}
		*o++ = *s++;
	}
	*o = 0;
	s++;

	o = value;
	while ( *s != '\\' && *s ) {
		*o++ = *s++;
	}
	*o = 0;

	*head = s;
}


/*
===================
Info_RemoveKey
===================
*/
void Info_RemoveKey( char *s, const char *key ) {
	char	*start;
	char	pkey[MAX_INFO_KEY];
	char	value[MAX_INFO_VALUE];
	char	*o;

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_RemoveKey: oversize infostring" );
	}

	if (strchr (key, '\\')) {
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			memmove(start, s, strlen(s) + 1); // remove this part
			
			return;
		}

		if (!*s)
			return;
	}

}

/*
===================
Info_RemoveKey_Big
===================
*/
void Info_RemoveKey_Big( char *s, const char *key ) {
	char	*start;
	char	pkey[BIG_INFO_KEY];
	char	value[BIG_INFO_VALUE];
	char	*o;

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_RemoveKey_Big: oversize infostring" );
	}

	if (strchr (key, '\\')) {
		return;
	}

	while (1)
	{
		start = s;
		if (*s == '\\')
			s++;
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;
		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		if (!strcmp (key, pkey) )
		{
			strcpy (start, s);	// remove this part
			return;
		}

		if (!*s)
			return;
	}

}




/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate( const char *s ) {
	const char* ch = s;

	while ( *ch != '\0' )
	{
		if( !Q_isprint( *ch ) )
			return qfalse;

		if( *ch == '\"' )
			return qfalse;

		if( *ch == ';' )
			return qfalse;

		++ch;
	}

	return qtrue;
}

/*
==================
Info_SetValueForKey

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey( char *s, const char *key, const char *value ) {
	char	newi[MAX_INFO_STRING];
	const char* blacklist = "\\;\"";

	if ( strlen( s ) >= MAX_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
	}

	for(; *blacklist; ++blacklist)
	{
		if (strchr (key, *blacklist) || strchr (value, *blacklist))
		{
			Com_Printf (S_COLOR_YELLOW "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
			return;
		}
	}
	
	Info_RemoveKey (s, key);
	if (!value || !strlen(value))
		return;

	Com_sprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) >= MAX_INFO_STRING)
	{
		Com_Printf ("Info string length exceeded\n");
		return;
	}

	strcat (newi, s);
	strcpy (s, newi);
}

/*
==================
Info_SetValueForKey_Big

Changes or adds a key/value pair
==================
*/
void Info_SetValueForKey_Big( char *s, const char *key, const char *value ) {
	char	newi[BIG_INFO_STRING];
	const char* blacklist = "\\;\"";

	if ( strlen( s ) >= BIG_INFO_STRING ) {
		Com_Error( ERR_DROP, "Info_SetValueForKey: oversize infostring" );
	}

	for(; *blacklist; ++blacklist)
	{
		if (strchr (key, *blacklist) || strchr (value, *blacklist))
		{
			assert( 0 );
			Com_Printf (S_COLOR_YELLOW "Can't use keys or values with a '%c': %s = %s\n", *blacklist, key, value);
			return;
		}
	}

	Info_RemoveKey_Big (s, key);
	if (!value || !strlen(value))
		return;

	Com_sprintf (newi, sizeof(newi), "\\%s\\%s", key, value);

	if (strlen(newi) + strlen(s) >= BIG_INFO_STRING)
	{
		Com_Printf ("BIG Info string length exceeded\n");
		return;
	}

	strcat (s, newi);
}




//====================================================================

/*
==================
Com_CharIsOneOfCharset
==================
*/
static qboolean Com_CharIsOneOfCharset( char c, const char *set )
{
	int i;

	for( i = 0; i < strlen( set ); i++ )
	{
		if( set[ i ] == c )
			return qtrue;
	}

	return qfalse;
}

/*
==================
Com_SkipCharset
==================
*/
char *Com_SkipCharset( char *s, const char *sep )
{
	char	*p = s;

	while( p )
	{
		if( Com_CharIsOneOfCharset( *p, sep ) )
			p++;
		else
			break;
	}

	return p;
}

/*
==================
Com_SkipTokens
==================
*/
char *Com_SkipTokens( char *s, int numTokens, const char *sep )
{
	int		sepCount = 0;
	char	*p = s;

	while( sepCount < numTokens )
	{
		if( Com_CharIsOneOfCharset( *p++, sep ) )
		{
			sepCount++;
			while( Com_CharIsOneOfCharset( *p, sep ) )
				p++;
		}
		else if( *p == '\0' )
			break;
	}

	if( sepCount == numTokens )
		return p;
	else
		return s;
}


/*
============================================================================

GROWLISTS

============================================================================
*/

// malloc / free all in one place for debugging
//extern          "C" void *Com_Allocate(int bytes);
//extern          "C" void Com_Dealloc(void *ptr);

void Com_InitGrowList(growList_t * list, int maxElements)
{
	list->maxElements = maxElements;
	list->currentElements = 0;
	list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));
}

void Com_DestroyGrowList(growList_t * list)
{
	Com_Dealloc(list->elements);
	memset(list, 0, sizeof(*list));
}

int Com_AddToGrowList(growList_t * list, void *data)
{
	void          **old;

	if(list->currentElements != list->maxElements)
	{
		list->elements[list->currentElements] = data;
		return list->currentElements++;
	}

	// grow, reallocate and move
	old = list->elements;

	if(list->maxElements < 0)
	{
		Com_Error(ERR_FATAL, "Com_AddToGrowList: maxElements = %i", list->maxElements);
	}

	if(list->maxElements == 0)
	{
		// initialize the list to hold 100 elements
		Com_InitGrowList(list, 100);
		return Com_AddToGrowList(list, data);
	}

	list->maxElements *= 2;

//  Com_DPrintf("Resizing growlist to %i maxElements\n", list->maxElements);

	list->elements = (void **)Com_Allocate(list->maxElements * sizeof(void *));

	if(!list->elements)
	{
		Com_Error(ERR_DROP, "Growlist alloc failed");
	}

	Com_Memcpy(list->elements, old, list->currentElements * sizeof(void *));

	Com_Dealloc(old);

	return Com_AddToGrowList(list, data);
}

void           *Com_GrowListElement(const growList_t * list, int index)
{
	if(index < 0 || index >= list->currentElements)
	{
		Com_Error(ERR_DROP, "Com_GrowListElement: %i out of range of %i", index, list->currentElements);
	}
	return list->elements[index];
}

int Com_IndexForGrowListElement(const growList_t * list, const void *element)
{
	int             i;

	for(i = 0; i < list->currentElements; i++)
	{
		if(list->elements[i] == element)
		{
			return i;
		}
	}
	return -1;
}
