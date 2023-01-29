/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

// script.cpp :
// C++ implementaion of tokenizing text interpretation.  Class accepts filename
// to load or pointer to preloaded text data.  Standard tokenizing operations
// such as skip white-space, get string, get integer, get float, get token,
// and skip line are implemented.
//
// Note: all '//', '#', and ';' are treated as comments.  Probably should
// make this behaviour toggleable.
//

#include "script.h"

#if defined(GAME_DLL)

#include "g_local.h"

#define FILE_FS_FreeFile gi.FS_FreeFile
#define FILE_FS_ReadFile(a,b) gi.FS_ReadFile(a,b,true)
#define FILE_Malloc gi.Malloc
#define FILE_Free gi.Free
#define FILE_Error gi.Error

#elif defined(CGAME_DLL)

#define FILE_FS_FreeFile cgi.FS_FreeFile
#define FILE_FS_ReadFile(a,b) cgi.FS_ReadFile(a,b,true)
#define FILE_Malloc cgi.Malloc
#define FILE_Free cgi.Free
#define FILE_Error cgi.Error

#else

#include "qcommon.h"

#define FILE_FS_FreeFile FS_FreeFile
#define FILE_FS_ReadFile(a,b) FS_ReadFile(a,b)
#define FILE_Malloc Z_Malloc
#define FILE_Free Z_Free
#define FILE_Error Com_Error

#endif

typedef unsigned char byte;

CLASS_DECLARATION( Class, Script, NULL )
{
	{ NULL, NULL }
};

Script::~Script()
{
	Close();
}

Script::Script(const char* filename /*= 0*/)
{
	buffer			= NULL;
	script_p		= NULL;
	end_p			= NULL;
	line			= 0;
	length			= 0;
	releaseBuffer	= false;
	tokenready		= false;
	token[ 0 ]		= 0;
	
	if ( filename != 0 )
		LoadFile( filename );
}

Script::Script()
{
	buffer			= NULL;
	script_p		= NULL;
	end_p			= NULL;
	line			= 0;
	length			= 0;
	releaseBuffer	= false;
	tokenready		= false;
	token[ 0 ]		= 0;
}

void Script::Close( void )
{
	if ( releaseBuffer && buffer )
	{
		FILE_Free( ( void * )buffer );
	}
	
	buffer			= NULL;
	script_p		= NULL;
	end_p			= NULL;
	line			= 0;
	releaseBuffer	= false;
	tokenready		= false;
	token[ 0 ]		= 0;
	
	//Loop Through the macro container and delete (del33t -hehe) them all
	for( int i = 1; i <= macrolist.NumObjects(); i++)
	{
		if (macrolist.ObjectAt( i ) )
		{
			delete macrolist.ObjectAt( i );
			macrolist.ObjectAt( i ) = 0;
		}	
	}
}

/*
==============
=
= Filename
=
==============
*/

const char *Script::Filename( void )
{
	return filename.c_str();
}

/*
==============
=
= GetLineNumber
=
==============
*/

int Script::GetLineNumber( void )
{
	return line;
}

/*
==============
=
= Reset
=
==============
*/

void Script::Reset( void )
{
	script_p = buffer;
	line = 1;
	tokenready = false;
	hasError = false;
}

/*
==============
=
= MarkPosition
=
==============
*/

void Script::MarkPosition( scriptmarker_t *mark )
{
	assert( mark );
	
	mark->tokenready = tokenready;
	mark->offset     = script_p - buffer;
	mark->line		  = line;
	strcpy( mark->token, token );
}

/*
==============
=
= RestorePosition
=
==============
*/

void Script::RestorePosition( const scriptmarker_t *mark )
{
	assert( mark );
	
	tokenready	= mark->tokenready;
	script_p		= buffer + mark->offset;
	line			= mark->line;
	strcpy( token, mark->token );
	
	assert( script_p <= end_p );
	if ( script_p > end_p )
	{
		script_p = end_p;
	}
}

/*
==============
=
= SkipToEOL
=
==============
*/

qboolean Script::SkipToEOL( void )
{
	if ( script_p >= end_p )
	{
		return true;
	}
	
	while( *script_p != TOKENEOL )
	{
		if ( script_p >= end_p )
		{
			return true;
		}
		script_p++;
	}
	return false;
}

/*
==============
=
= CheckOverflow
=
==============
*/

void Script::CheckOverflow(	void )
{
	if ( script_p >= end_p )
	{
		FILE_Error( ERR_DROP, "End of token file reached prematurely reading %s\n", filename.c_str() );
	}
}

/*
==============
=
= SkipWhiteSpace
=
==============
*/

void Script::SkipWhiteSpace( qboolean crossline )
{
	//
	// skip space
	//
	CheckOverflow();
	
	while( *script_p <= TOKENSPACE )
	{
		if ( *script_p++ == TOKENEOL )
		{
			if ( !crossline )
			{
				FILE_Error( ERR_DROP, "Line %i is incomplete in file %s\n", line, filename.c_str() );
			}
			
			line++;
		}
		CheckOverflow();
	}
}

qboolean Script::AtComment( void )
{
	if ( script_p >= end_p )
	{
		return false;
	}
	
	if ( *script_p == TOKENCOMMENT )
	{
		return true;
	}
	
	if ( *script_p == TOKENCOMMENT2 )
	{
		return true;
	}
	
	// Two or more character comment specifiers
	if ( ( script_p + 1 ) >= end_p )
	{
		return false;
	}
	
	if ( ( *script_p == '/' ) && ( *( script_p + 1 ) == '/' ) )
	{
		return true;
	}
	
	return false;
}

/*
==============
=
= SkipNonToken
=
==============
*/

void Script::SkipNonToken( qboolean crossline )
{
	//
	// skip space and comments
	//
	SkipWhiteSpace( crossline );
	while( AtComment() )
	{
		SkipToEOL();
		SkipWhiteSpace( crossline );
	}
}

/*
=============================================================================
=
= Token section
=
=============================================================================
*/

/*
==============
=
= TokenAvailable
=
==============
*/

qboolean Script::TokenAvailable( qboolean crossline )
{
	if ( script_p >= end_p )
	{
		return false;
	}
	
	while ( 1 )
	{
		while ( *script_p <= TOKENSPACE )
		{
			if ( *script_p == TOKENEOL )
			{
				if ( !crossline )
				{
					return( false );
				}
				line++;
			}
			
			script_p++;
			if ( script_p >= end_p )
			{
				return false;
			}
		}
		
		if ( AtComment() )
		{
			qboolean done;
			
			done = SkipToEOL();
			if ( done )
			{
				return false;
			}
		}
		else
		{
			break;
		}
	}
	
	return true;
}

/*
==============
=
= CommentAvailable
=
==============
*/

qboolean Script::CommentAvailable( qboolean crossline )
{
	const char *searchptr;
	
	searchptr = script_p;
	
	if ( searchptr >= end_p )
	{
		return false;
	}
	
	while ( *searchptr <= TOKENSPACE )
	{
		if ( ( *searchptr == TOKENEOL ) && ( !crossline ) )
		{
			return false;
		}
		searchptr++;
		if ( searchptr >= end_p )
		{
			return false;
		}
	}
	
	return true;
}

/*
==============
=
= UnGet
=
= Signals that the current token was not used, and should be reported
= for the next GetToken.  Note that

GetToken (true);
UnGetToken ();
GetToken (false);

= could cross a line boundary.
=
==============
*/

void Script::UnGetToken( void )
{
	tokenready = true;
}

/*
==============
=
= Get
=
==============
*/
qboolean Script::AtString( qboolean crossline )
{
	//
	// skip space
	//
	SkipNonToken( crossline );
	
	return ( *script_p == '"' );
}

qboolean Script::AtOpenParen( qboolean crossline )
{
	//
	// skip space
	//
	SkipNonToken( crossline );
	
	return ( *script_p == '(' );	
}

qboolean Script::AtCloseParen( qboolean crossline )
{
	//
	// skip space
	//
	SkipNonToken( crossline );
	
	return ( *script_p == ')' );	
}

qboolean Script::AtComma( qboolean crossline )
{
	//
	// skip space
	//
	SkipNonToken( crossline );

	return ( *script_p == ',' );
}

qboolean Script::AtDot( qboolean crossline )
{
	//
	// skip space
	//
	SkipNonToken( crossline );

	return ( *script_p == '.' );
}

qboolean Script::AtAssignment( qboolean crossline )
{
	//
	// skip space
	//
	SkipNonToken( crossline );

	return	( *script_p == '=' ) ||
			( ( *script_p == '+' ) && ( *( script_p + 1 ) == '=' ) ) ||
			( ( *script_p == '-' ) && ( *( script_p + 1 ) == '=' ) ) ||
			( ( *script_p == '*' ) && ( *( script_p + 1 ) == '=' ) ) ||
			( ( *script_p == '/' ) && ( *( script_p + 1 ) == '=' ) );
}

/*
==============
=
= Get
=
==============
*/

const char *Script::GetToken( qboolean crossline )
{
	
	str		 token_p = token;
	qboolean	 is_Macro = false;
	
	// is a token already waiting?
	if ( tokenready )
	{
		tokenready = false;
		return token;
	}
	
	is_Macro = isMacro();
	
	token_p = GrabNextToken(crossline);
	
	if ( is_Macro && ( token_p != "$include" ) )
	{
		
		//Check to see if we need to add any definitions
		while ( ( token_p == "$define" ) || ( token_p == "$Define" ) )
		{
			AddMacroDefinition(crossline);
			is_Macro = isMacro();
			//if ( !is_Macro )
			//	return "";
			token_p = GrabNextToken(crossline);			
		}
		
		//Check to see if we need return any defines strings
		if( is_Macro && ( token_p != "$include" ) && ( token_p[token_p.length() - 1] == '$' ) )
		{
			return GetMacroString(token_p);
		}
		
	}
		
	return token;
}

/*
==============
=
= GrabNextToken
=
==============
*/
const char *Script::GrabNextToken( qboolean crossline )
{
	char *token_p;
	
	//
	// skip space
	//
	SkipNonToken( crossline );
	
	//
	// copy token
	//
	if ( *script_p == '"' )
	{
		return GetString( crossline );
	}
	
	token_p = token;
	while( ( *script_p > TOKENSPACE ) && !AtComment() )
	{
		if ( ( *script_p == '\\' ) && ( script_p < ( end_p - 1 ) ) )
		{
			script_p++;
			switch( *script_p )
			{
			case 'n' :	*token_p++ = '\n'; break;
			case 'r' :	*token_p++ = '\n'; break;
			case '\'' : *token_p++ = '\''; break;
			case '\"' : *token_p++ = '\"'; break;
			case '\\' : *token_p++ = '\\'; break;
			default:		*token_p++ = *script_p; break;
			}
			script_p++;
		}
		else
		{
			*token_p++ = *script_p++;
		}
		
		if ( token_p == &token[ MAXTOKEN ] )
		{
			FILE_Error( ERR_DROP, "Token too large on line %i in file %s\n", line, filename.c_str() );
		}
		
		if ( script_p == end_p )
		{
			break;
		}
	}
	
	
	*token_p = 0;
	
	return token;
}

/*
==============
=
= AddMacroDefinition
=
==============
*/
void Script::AddMacroDefinition( qboolean crossline )
{
	macro      *theMacro;
	
	//Create a new macro structure.  This new macro will be deleted in the script close()
	theMacro = new macro;
	
	//Grab the macro name
	theMacro->macroName = "$";
	theMacro->macroName.append(GrabNextToken(crossline));
	theMacro->macroName.append( "$" ); //<-- Adding closing ($) to keep formatting consistant
	
	//Grab the macro string
	str tmpstr;
	tmpstr = GrabNextToken(crossline);
	//Check to see if we need return any defines strings
	if( ( tmpstr != "$include" ) && ( tmpstr[tmpstr.length() - 1] == '$' ) )
		theMacro->macroText = GetMacroString(tmpstr);
	else
		theMacro->macroText = tmpstr;
	
	macrolist.AddObject( theMacro );
	
}

/*
==============
=
= GetMacroString
=
==============
*/
const char *Script::GetMacroString( const char *theMacroName )
{
	
	macro *theMacro =0; //Initialize this puppy
	
	for( int i = 1; i <= macrolist.NumObjects(); i++)
	{
		theMacro = macrolist.ObjectAt( i );
		
		if(!theMacro->macroName.cmp(theMacro->macroName.c_str(), theMacroName))
		{
			const char *text = theMacro->macroText.c_str();
			
			// If our define value is another define...
			if( text[0] == '$' )
				return EvaluateMacroString(text);
			else
				return text;
		}
		
	}
	
	char tmpstr[255], *sptr = tmpstr;
	strcpy(tmpstr, theMacroName);
	tmpstr[strlen(tmpstr)-1] = 0;
	sptr++;
	
	// We didn't find what we were looking for
	FILE_Error( ERR_DROP, "No Macro Text found for %s in file %s\n", theMacroName, filename.c_str() );
	return 0;
	
}

//================================================================
// Name:			AddMacro
// Class:			Script
//
// Description:		Adds a macro to the definitions list.
//
// Parameters:		const char *name -- Name of the macro
//					const char *value -- Value
//
// Returns:			None
//
//================================================================
void Script::AddMacro(const char *name, const char *value)
{

}

/*
==============
=
= EvaluateMacroString
=
==============
*/
char *Script::EvaluateMacroString( const char *theMacroString )
{
	static char evalText[255];
	char buffer[255], *bufferptr = buffer, oper = '+', newoper = '+';
	bool haveoper = false;
	int i;
	float value = 0.0f, val = 0.0f;
	memset(buffer, 0, 255);
	
	for ( i=0;i<=strlen(theMacroString);i++ )
	{
		if ( theMacroString[i] == '+' ) { haveoper = true; newoper = '+'; }
		if ( theMacroString[i] == '-' ) { haveoper = true; newoper = '-'; }
		if ( theMacroString[i] == '*' ) { haveoper = true; newoper = '*'; }
		if ( theMacroString[i] == '/' ) { haveoper = true; newoper = '/'; }
		if ( theMacroString[i] == 0 ) haveoper = true;
		
		if ( haveoper )
		{ 
			if ( buffer[0] == '$' )
				val = atof(GetMacroString(buffer));
			else
				val = atof(buffer);
			
			value = EvaluateMacroMath(value, val, oper);
			oper = newoper;
			
			// Reset everything
			haveoper = false;
			memset(buffer, 0, 255);
			bufferptr = buffer;
			continue; 
		}
		
		*bufferptr = theMacroString[i];
		bufferptr++;
	}
	
	sprintf(evalText,"%f",value);
	return evalText;
}

/*
==============
=
= EvaluateMacroMath
=
==============
*/
float Script::EvaluateMacroMath(float value, float newval, char oper)
{
	switch ( oper )
		{
		case '+' : value += newval; break;
		case '-' : value -= newval; break;
		case '*' : value *= newval; break;
		case '/' : value /= newval; break;
		}

	return value;
}

/*
==============
=
= isMacro
=
==============
*/
qboolean Script::isMacro( void )
{
	if ( !TokenAvailable( true ) )
		return false;
	
	SkipNonToken( true );
	if ( *script_p == TOKENSPECIAL )
	{
		return true;
	}
	
	return false;
}

/*
==============
=
= GetLine
=
==============
*/

const char *Script::GetLine( qboolean crossline )
{
	const char	*start;
	int			size;
	
	// is a token already waiting?
	if ( tokenready )
	{
		tokenready = false;
		return token;
	}
	
	//
	// skip space
	//
	SkipNonToken( crossline );
	
	//
	// copy token
	//
	start = script_p;
	SkipToEOL();
	size = script_p - start;
	if ( size < ( MAXTOKEN - 1 ) )
	{
		memcpy( token, start, size );
		token[ size ] = '\0';
	}
	else
	{
		FILE_Error( ERR_DROP, "Token too large on line %i in file %s\n", line, filename.c_str() );
	}
	
	return token;
}

/*
==============
=
= GetRaw
=
==============
*/

const char *Script::GetRaw( void )
{
	const char	*start;
	int			size;
	
	//
	// skip white space
	//
	SkipWhiteSpace( true );
	
	//
	// copy token
	//
	start = script_p;
	SkipToEOL();
	size = script_p - start;
	if ( size < ( MAXTOKEN - 1 ) )
	{
		memset( token, 0, sizeof( token ) );
		memcpy( token, start, size );
	}
	else
	{
		FILE_Error( ERR_DROP, "Token too large on line %i in file %s\n", line, filename.c_str() );
	}
	
	return token;
}

/*
==============
=
= GetString
=
==============
*/

const char *Script::GetString( qboolean crossline )
{
	int startline;
	char *token_p;
	
	// is a token already waiting?
	if ( tokenready )
	{
		tokenready = false;
		return token;
	}
	
	//
	// skip space
	//
	SkipNonToken( crossline );
	
	if ( *script_p != '"' )
	{
		FILE_Error( ERR_DROP, "Expecting string on line %i in file %s\n", line, filename.c_str() );
	}
	
	script_p++;
	
	startline = line;
	token_p = token;
	while( *script_p != '"' )
	{
		if ( *script_p == TOKENEOL )
		{
			FILE_Error( ERR_DROP, "Line %i is incomplete while reading string in file %s\n", line, filename.c_str() );
		}
		
		if ( ( *script_p == '\\' ) && ( script_p < ( end_p - 1 ) ) )
		{
			script_p++;
			switch( *script_p )
			{
			case 'n' :	*token_p++ = '\n'; break;
			case 'r' :	*token_p++ = '\n'; break;
			case '\'' : *token_p++ = '\''; break;
			case '\"' : *token_p++ = '\"'; break;
			case '\\' : *token_p++ = '\\'; break;
			default:		*token_p++ = *script_p; break;
			}
			script_p++;
		}
		else
		{
			*token_p++ = *script_p++;
		}
		
		if ( script_p >= end_p )
		{
			FILE_Error( ERR_DROP, "End of token file reached prematurely while reading string on\n"
				"line %d in file %s\n", startline, filename.c_str() );
		}
		
		if ( token_p == &token[ MAXTOKEN ] )
		{
			FILE_Error( ERR_DROP, "String too large on line %i in file %s\n", line, filename.c_str() );
		}
	}
	
	*token_p = 0;
	
	// skip last quote
	script_p++;
	
	return token;
}

/*
==============
=
= GetSpecific
=
==============
*/

qboolean Script::GetSpecific( const char *string )
{
	do
	{
		if ( !TokenAvailable( true ) )
		{
			return false;
		}
		GetToken( true );
	}
	while( strcmp( token, string ) );
	
	return true;
}

//===============================================================
// Name:		GetBoolean
// Class:		Script
//
// Description: Retrieves the next boolean value in the token
//				stream.  If the next token is either "true"
//				or "1", then it returns true.  Otherwise, it
//				returns false.
// 
// Parameters:	qboolean -- determines if token parsing can cross newlines
//
// Returns:		qboolean -- true if next token was "true" (or "1")
// 
//===============================================================
qboolean Script::GetBoolean( qboolean crossline )
{
	GetToken( crossline );
	if ( Q_stricmp( token, "true" ) == 0 )
		return true;
	else if( Q_stricmp( token, "1" ) == 0 )
		return true;

	return false;
}

/*
==============
=
= GetInteger
=
==============
*/

int Script::GetInteger( qboolean crossline )
{
	GetToken( crossline );
	return atoi( token );
}

/*
==============
=
= GetDouble
=
==============
*/

double Script::GetDouble( qboolean crossline )
{
	GetToken( crossline );
	return atof( token );
}

/*
==============
=
= GetFloat
=
==============
*/

float Script::GetFloat( qboolean crossline )
{
	return ( float )GetDouble( crossline );
}

/*
==============
=
= GetVector
=
==============
*/

Vector Script::GetVector( qboolean crossline )
{
	float	x = GetFloat( crossline );
	float	y = GetFloat( crossline );
	float	z = GetFloat( crossline );
	return Vector( x, y, z );
}

/*
===================
=
= LinesInFile
=
===================
*/
int Script::LinesInFile( void )
{
	qboolean		temp_tokenready;
	const char	*temp_script_p;
	int			temp_line;
	char			temp_token[ MAXTOKEN ];
	int			numentries;
	
	temp_tokenready = tokenready;
	temp_script_p	= script_p;
	temp_line		= line;
	strcpy( temp_token, token );
	
	numentries = 0;
	
	Reset();
	while( TokenAvailable( true ) )
	{
		GetLine( true );
		numentries++;
	}
	
	tokenready	= temp_tokenready;
	script_p	= temp_script_p;
	line		= temp_line;
	strcpy( token, temp_token );
	
	return numentries;
}

/*
==============
=
= Parse
=
==============
*/

void Script::Parse( const char *data, size_t length, const char *name )
{
	Close();
	
	buffer = data;
	Reset();
	this->length = length;
	end_p = script_p + length;
	filename = name;
}

/*
==============
=
= Load
=
==============
*/

void Script::LoadFile( const char *name )
{
	int			length;
	byte        *buffer;
	byte        *tempbuf;
	const char  *const_buffer;
	
	Close();
	
	length = FILE_FS_ReadFile( name, ( void ** )&tempbuf );

	hasError = false;

	if ( length < 0 )
	{
		hasError = true;
		return;
	}

	// create our own space
	buffer = ( byte * )FILE_Malloc( length + 1 );
	// copy the file over to our space
	memcpy( buffer, tempbuf, length );
	buffer[ length ] = 0;
	// free the file
	FILE_FS_FreeFile( tempbuf );
	
	const_buffer = ( char * )buffer;
	
	Parse( const_buffer, length, name );
	releaseBuffer = true;
}

/*
==============
=
= LoadFile
=
==============
*/

void Script::LoadFile( const char *name, int length, const char *buf )
{
	Close();

	// create our own space
	this->buffer = ( const char * )FILE_Malloc( length );
	this->length = length;
	// copy the file over to our space
	memcpy( ( void * )this->buffer, buf, length );

	Parse( buffer, this->length, name );
	releaseBuffer = true;
}

qboolean Script::isValid()
{
	return !hasError;
}

qboolean Script::EndOfFile( void )
{
	return script_p >= end_p;
}

const char *Script::Token( void )
{
	return token;
}
