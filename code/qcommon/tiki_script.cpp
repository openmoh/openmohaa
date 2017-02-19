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

// tiki_parse.cpp : TIKI Script

#include "q_shared.h"
#include "qcommon.h"
#include <tiki.h>
#include "dbgheap.h"

TikiScript *TikiScript::currentScript;

TikiScript::~TikiScript()
{
	Close();
}

TikiScript::TikiScript()
{
	error			= false;
	buffer			= NULL;
	script_p		= NULL;
	end_p			= NULL;
	line			= 0;
	releaseBuffer	= false;
	tokenready		= false;
	memset( token, 0, sizeof( token ) );
	include			= NULL;
	parent			= NULL;
	nummacros		= 0;
	mark_pos		= 0;
	path[ 0 ]		= 0;
}

/*
==============
=
= Close
=
==============
*/
void TikiScript::Close( void )
{
	Uninclude();
	if( this->releaseBuffer )
	{
		if( this->buffer )
			TIKI_Free( this->buffer );
	}

	buffer			= NULL;
	script_p		= NULL;
	end_p			= NULL;
	line			= 0;
	error			= false;
	releaseBuffer	= false;
	tokenready		= false;
	nummacros		= 0;
	memset( token, 0, sizeof( token ) );
	mark_pos		= 0;
}

/*
==============
=
= Filename
=
==============
*/
const char *TikiScript::Filename()
{
	return filename;
}

/*
==============
=
= GetLineNumber
=
==============
*/
int TikiScript::GetLineNumber()
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
void TikiScript::Reset( void )
{
	Uninclude();

	nummacros		= 0;
	error			= 0;
	line			= 1;
	tokenready		= 0;
	mark_pos		= 0;
	script_p		= buffer;
}

/*
==============
=
= SkipToEOL
=
==============
*/
qboolean TikiScript::SkipToEOL( void )
{
	if( script_p >= end_p )
	{
		return true;
	}

	while( *script_p != TOKENEOL )
	{
		if( script_p >= end_p )
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
= SafeCheckOverflow
=
==============
*/
qboolean TikiScript::SafeCheckOverflow( void )
{
	return script_p >= end_p;
}

/*
==============
=
= CheckOverflow
=
==============
*/
void TikiScript::CheckOverflow( void )
{
	if( script_p >= end_p )
	{
		this->error = 1;
		TIKI_DPrintf( "End of tiki file reached prematurely reading %s\n", Filename() );
		TIKI_DPrintf( "   This may be caused by having a tiki file command at the end of the file\n" );
		TIKI_DPrintf( "   without an 'end' at the end of the tiki file.\n" );
	}
}

/*
==============
=
= SkipWhiteSpace
=
==============
*/
void TikiScript::SkipWhiteSpace( qboolean crossline )
{
	//
	// skip space
	//
	while( !SafeCheckOverflow() )
	{
		if( *script_p <= TOKENSPACE )
		{
			if( *script_p == TOKENEOL )
			{
				if( !crossline )
				{
					TIKI_DPrintf( "Line %i is incomplete in file %s\n", line, Filename() );
					return;
				}

				line++;
			}
			script_p++;
		}
		else
		{
			if( !AtComment() )
			{
				return;
			}

			if( AtExtendedComment() )
			{
				SkipExtendedComment();
			}
			else
			{
				if( !crossline )
				{
					TIKI_DPrintf( "Line %i is incomplete in file %s\n", line, Filename() );
					return;
				}
				SkipToEOL();
			}
		}
	}
}

/*
==============
=
= AtComment
=
==============
*/
qboolean TikiScript::AtComment( void )
{
	if( script_p >= end_p )
	{
		return false;
	}

	if( *script_p == TOKENCOMMENT )
	{
		return true;
	}

	if( *script_p == TOKENCOMMENT2 )
	{
		return true;
	}

	// Two or more character comment specifiers
	if( ( script_p + 1 ) >= end_p )
	{
		return false;
	}

	if( ( *script_p == '/' ) && ( ( *( script_p + 1 ) == '/' ) || ( *( script_p + 1 ) == '*' ) ) )
	{
		return true;
	}

	return false;
}

/*
==============
=
= AtExtendedComment
=
==============
*/
qboolean TikiScript::AtExtendedComment( void )
{
	if( script_p >= end_p )
	{
		return false;
	}

	// Two or more character comment specifiers
	if( ( script_p + 1 ) >= end_p )
	{
		return false;
	}

	if( ( *( script_p ) == '/' ) && ( *( script_p + 1 ) == '*' ) )
	{
		return true;
	}

	return false;
}

/*
==============
=
= SkipExtendedComment
=
==============
*/
void TikiScript::SkipExtendedComment( void )
{
	CheckOverflow();

	if( error )
	{
		return;
	}

	while( ( script_p + 1 ) < end_p )
	{
		if( ( *( script_p ) == '*' ) && ( *( script_p + 1 ) == '/' ) )
		{
			script_p = script_p + 2;
			return;
		}

		if( *script_p == '\n' )
			line++;

		script_p++;
	}
}

/*
==============
=
= AtCommand
=
==============
*/
qboolean TikiScript::AtCommand( void )
{
	if( script_p >= end_p )
	{
		return false;
	}

	// Two or more character comment specifiers
	if( ( script_p + 1 ) >= end_p )
	{
		return false;
	}

	if( ( *script_p == '$' ) && ( *( script_p + 1 ) != '$' ) )
	{
		return true;
	}

	return false;
}

/*
==============
=
= Uninclude
=
==============
*/
void TikiScript::Uninclude( void )
{
	if( include )
	{
		currentScript = this;
		include->Close();
		delete include;
		include = NULL;
	}
}

/*
==============
=
= AddMacro
=
==============
*/
void TikiScript::AddMacro( const char *name, const char *expansion )
{
	int i;

	for( i = 0; i < nummacros; i++ )
	{
		if( !stricmp( name, macros[ i ].name ) )
			return;
	}

	if( nummacros >= MAXMACROS )
	{
		TIKI_DPrintf( "Too many %cdefine in file %s\n", TOKENSPECIAL, Filename() );
	}
	else
	{
		strcpy( macros[ nummacros ].name, name );
		strcpy( macros[ nummacros ].macro, expansion );
		nummacros++;
	}

	// Add the macro to parent
	if( parent )
		parent->AddMacro( name, expansion );
}

/*
==============
=
= ProcessCommand
=
==============
*/
qboolean TikiScript::ProcessCommand( qboolean crossline )
{
	char dummy;
	int i;
	size_t len;
	char command[ 256 ];
	char argument1[ 256 ];
	char argument2[ 256 ];

	sscanf( script_p, "%c%s %s %s\n", &dummy, &command, &argument1, &argument2 );

	if( !stricmp( command, "define" ) )
	{
		AddMacro( argument1, argument2 );
		SkipToEOL();
		SkipWhiteSpace( crossline );
	}
	else if( !stricmp( command, "include" ) )
	{
		SkipToEOL();
		SkipWhiteSpace( crossline );
		include = new TikiScript;
		if( include->LoadFile( argument1, qfalse ) )
		{
			include->SkipNonToken( crossline );
			for( i = 0; i < nummacros; i++ )
			{
				include->AddMacro( macros[ i ].name, macros[ i ].macro );
			}
			include->parent = this;
		}
		else
		{
			TIKI_Error( "^~^~^ Cannot find include file '%s' in %s on line %d\n", &argument1, Filename(), GetLineNumber() );
			Uninclude();
		}
	}
	else if( !stricmp( command, "path" ) )
	{
		strcpy( path, argument1 );
		len = strlen( path );

		if( path[ len - 1 ] != '/' &&
			path[ len - 1 ] != '\\' )
		{
			strcat( path, "/" );
		}

		SkipToEOL();
		SkipWhiteSpace( crossline );
	}
	else
	{
		return false;
	}

	return true;
}

/*
==============
=
= Completed
=
==============
*/
qboolean TikiScript::Completed( void )
{
	return script_p >= end_p;
}

/*
==============
=
= FindMacro
=
==============
*/
const char *TikiScript::FindMacro( const char *macro )
{
	int i;

	for( i = 0; i < nummacros; i++ )
	{
		if( !stricmp( macro, macros[ i ].name ) )
			return macros[ i ].macro;
	}

	return NULL;
}

/*
==============
=
= SkipNonToken
=
==============
*/
void TikiScript::SkipNonToken( qboolean crossline )
{
	//
	// skip space and comments
	//
	SkipWhiteSpace( crossline );
	while( !error )
	{
		if( !AtCommand() )
			break;
		if( !ProcessCommand( crossline ) )
			break;
		if( include )
			break;
	}
}

/*
==============
=
= TokenAvailable
=
==============
*/
qboolean TikiScript::TokenAvailable( qboolean crossline )
{
	if( include )
	{
		qboolean ret = include->TokenAvailable( crossline );

		if( ret || !crossline || !include->Completed() )
			return ret;

		Uninclude();
	}
	if( tokenready )
	{
		return true;
	}

	while( !SafeCheckOverflow() )
	{
		if( *script_p <= TOKENSPACE )
		{
			if( *script_p == TOKENEOL )
			{
				if( !crossline )
				{
					return false;
				}
				line++;
			}
			script_p++;
		}
		else
		{
			if( !AtComment() )
				return true;

			if( AtExtendedComment() )
			{
				SkipExtendedComment();
			}
			else
			{
				if( !crossline )
					return false;
				SkipToEOL();
			}
		}
	}

	return false;
}

/*
==============
=
= CommentAvailable
=
==============
*/
qboolean TikiScript::CommentAvailable( qboolean crossline )
{
	const char *searchptr;

	searchptr = script_p;

	if( searchptr >= end_p )
	{
		return false;
	}

	while( *searchptr <= TOKENSPACE )
	{
		if( ( *searchptr == TOKENEOL ) && ( !crossline ) )
		{
			return false;
		}
		searchptr++;
		if( searchptr >= end_p )
		{
			return false;
		}
	}

	return true;
}

/*
==============
=
= UnGetToken
=
==============
*/
void TikiScript::UnGetToken( void )
{
	if( include )
	{
		include->UnGetToken();
	}
	else
	{
		tokenready = true;
	}
}

/*
==============
=
= AtString
=
==============
*/
qboolean TikiScript::AtString( qboolean crossline )
{
	TikiScript *i;
	//
	// skip space
	//

	i = this;
	while( 1 )
	{
		i->SkipNonToken( crossline );

		if( !i->include )
			break;

		i = i->include;
	}

	return ( !i->error ) && ( *i->script_p == '"' );
}

/*
==============
=
= GetToken
=
==============
*/
const char *TikiScript::GetToken( qboolean crossline )
{
	TikiScript *i;
	const char *macro_start;
	const char *macro_end;
	char *token_p;
	int len;
	const char *subst;
	char macro[ 256 ];
	char temptoken[ 256 ];

	i = this;
	while( 1 )
	{
		if( i->include )
		{
			const char *ret;

			ret = i->include->GetToken( crossline );

			if( *ret || !crossline || !i->include->Completed() )
			{
				return ret;
			}
			i->Uninclude();
		}

		if( i->tokenready )
		{
			i->tokenready = false;
			currentScript = i;
			return i->token;
		}

		i->SkipNonToken( crossline );

		if( i->error )
		{
			return "";
		}

		if( !i->include )
		{
			break;
		}

		i = i->include;
	}

	token_p = ( char * )i->script_p;

	if( *token_p == '"' )
	{
		currentScript = i;
		return i->GetString( crossline );
	}

	macro_start = NULL;
	macro_end = NULL;
	token_p = i->token;
	while( ( *i->script_p > TOKENSPACE ) && !i->AtComment() )
	{
		if( ( *i->script_p == '\\' ) && ( i->script_p < ( end_p - 1 ) ) )
		{
			i->script_p++;
			switch( *i->script_p )
			{
			case 'n':	*token_p++ = '\n';			break;
			case 'r':	*token_p++ = '\n';			break;
			case '\'':	*token_p++ = '\'';			break;
			case '\"':	*token_p++ = '\"';			break;
			case '\\':	*token_p++ = '\\';			break;
			default:	*token_p++ = *i->script_p;	break;
			}
			i->script_p++;
		}
		else
		{
			if( *i->script_p == '$' )
			{
				if( macro_start )
					macro_end = token_p;
				else
					macro_start = token_p;
			}
			*token_p++ = *i->script_p++;
		}

		if( token_p == &i->token[ MAXTOKEN ] )
		{
			TIKI_DPrintf( "Token too large on line %i in file %s\n", i->line, i->Filename() );
			break;
		}

		if( i->script_p == i->end_p )
		{
			break;
		}
	}

	*token_p = 0;

	if( macro_start )
	{
		if( !macro_end ) macro_end = token_p;

		len = ( macro_end - 1 ) - ( macro_start + 1 );
		if( len == -1 )
		{
			if( macro_start - i->token != 0 )
				memcpy( temptoken, i->token, macro_start - i->token );

			temptoken[ macro_start - i->token ] = '$';
			temptoken[ macro_start - i->token + 1 ] = 0;
			if( *( macro_end + 1 ) )
			{
				strcat( temptoken, macro_end + 1 );
			}
			strcpy( i->token, temptoken );
		}
		else
		{
			memcpy( macro, ( macro_start + 1 ), len + 1 );
			macro[ len + 1 ] = 0;
			subst = i->FindMacro( macro );
			if( subst )
			{
				if( macro_start - i->token != 0 )
					memcpy( temptoken, i->token, macro_start - i->token );

				temptoken[ macro_start - i->token + 1 ] = 0;
				strcat( temptoken, subst );
				if( *( macro_end + 1 ) )
				{
					strcat( temptoken, macro_end + 1 );
				}
				strcpy( i->token, temptoken );
			}
		}
	}

	i->currentScript = i;
	return i->token;
}

/*
==============
=
= GetLine
=
==============
*/
const char *TikiScript::GetLine( qboolean crossline )
{
	TikiScript *i;
	const char *start;
	int size;

	i = this;
	while( 1 )
	{
		if( i->include )
		{
			const char *ret;

			ret = i->include->GetToken( crossline );

			if( *ret || !crossline || !i->include->Completed() )
			{
				return ret;
			}
			i->Uninclude();
		}

		if( i->tokenready )
		{
			i->tokenready = false;
			return i->token;
		}

		if( !i->include )
			break;

		i = i->include;
	}

	if( i->error )
	{
		return "";
	}

	//
	// copy token
	//
	start = i->script_p;
	SkipToEOL();
	size = i->script_p - start;
	if( size < ( MAXTOKEN - 1 ) )
	{
		memcpy( i->token, start, size );
		i->token[ size ] = '\0';
	}
	else
	{
		TIKI_DPrintf( "Token too large on line %i in file %s\n", i->line, i->filename );
	}

	return i->token;
}

/*
==============
=
= GetAndIgnoreLine
=
==============
*/
const char *TikiScript::GetAndIgnoreLine( qboolean crossline )
{
	const char *start;
	int size;

	if( include )
	{
		const char *ret;

		ret = include->GetAndIgnoreLine( crossline );

		if( *ret || !crossline || !include->Completed() )
			return ret;

		Uninclude();
	}

	if( tokenready )
	{
		tokenready = false;
		return token;
	}

	//
	// skip space
	//
	SkipWhiteSpace( crossline );

	if( error )
	{
		return "";
	}

	//
	// copy token
	//
	start = script_p;
	SkipToEOL();
	size = script_p - start;
	if( size < ( MAXTOKEN - 1 ) )
	{
		memcpy( token, start, size );
		token[ size ] = '\0';
	}
	else
	{
		TIKI_DPrintf( "Token too large on line %i in file %s\n", line, filename );
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
const char *TikiScript::GetRaw( void )
{
	const char *start;
	int size;

	if( include )
	{
		const char *ret;

		ret = include->GetRaw();

		if( *ret || !include->Completed() )
			return ret;

		Uninclude();
	}

	//
	// skip white space
	//
	SkipWhiteSpace( true );

	if( error )
	{
		return "";
	}

	//
	// copy token
	//
	start = script_p;
	SkipToEOL();
	size = script_p - start;
	if( size < ( MAXTOKEN - 1 ) )
	{
		memset( token, 0, sizeof( token ) );
		memcpy( token, start, size );
	}
	else
	{
		TIKI_DPrintf( "Token too large on line %i in file %s\n", line, filename );
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
const char *TikiScript::GetString( qboolean crossline )
{
	TikiScript *i;
	int startline;
	char *token_p;

	i = this;
	while( 1 )
	{
		if( i->include )
		{
			const char *ret;

			ret = i->include->GetString( crossline );

			if( *ret || !crossline || !i->include->Completed() )
			{
				return ret;
			}
			i->Uninclude();
		}

		// is a token already waiting?
		if( i->tokenready )
		{
			i->tokenready = false;
			return i->token;
		}

		i->SkipNonToken( crossline );

		if( !i->include )
			break;

		i = i->include;
	}

	if( i->error )
	{
		return "";
	}

	if( *i->script_p != '"' )
	{
		TIKI_DPrintf( "Expecting string on line %i in file %s\n", i->line, i->filename );
		return i->token;
	}

	i->script_p++;

	startline = i->line;
	token_p = i->token;
	while( *i->script_p != '"' )
	{
		if( *i->script_p == TOKENEOL )
		{
			TIKI_DPrintf( "Line %i is incomplete while reading string in file %s\n", i->line, i->filename );
			return i->token;
		}

		if( ( *i->script_p == '\\' ) && ( i->script_p < ( end_p - 1 ) ) )
		{
			i->script_p++;
			switch( *i->script_p )
			{
			case 'n':	*token_p++ = '\n';			break;
			case 'r':	*token_p++ = '\n';			break;
			case '\'':	*token_p++ = '\'';			break;
			case '\"':	*token_p++ = '\"';			break;
			case '\\':	*token_p++ = '\\';			break;
			default:	*token_p++ = *i->script_p;	break;
			}
			i->script_p++;
		}
		else
		{
			*token_p++ = *i->script_p++;
		}

		if( i->script_p >= i->end_p )
		{
			TIKI_DPrintf( "End of token file reached prematurely while reading string on\n"
				"line %d in file %s\n", startline, i->filename );
		}

		if( token_p == &i->token[ MAXTOKEN ] )
		{
			TIKI_DPrintf( "String too large on line %i in file %s\n", i->line, i->filename );
		}
	}

	*token_p = 0;

	// skip last quote
	i->script_p++;

	return i->token;
}

/*
==============
=
= GetSpecific
=
==============
*/
qboolean TikiScript::GetSpecific( const char *string )
{
	if( include )
	{
		qboolean ret = include->GetSpecific( string );
		if( ret || !include->Completed() )
		{
			return ret;
		}
	}

	do
	{
		if( !TokenAvailable( true ) )
		{
			return false;
		}
		GetToken( true );
	} while( strcmp( token, string ) );

	return true;
}

/*
==============
=
= GetInteger
=
==============
*/
int TikiScript::GetInteger( qboolean crossline )
{
	return atoi( GetToken( crossline ) );
}

/*
==============
=
= GetDouble
=
==============
*/
double TikiScript::GetDouble( qboolean crossline )
{
	return atof( GetToken( crossline ) );
}

/*
==============
=
= GetFloat
=
==============
*/
float TikiScript::GetFloat( qboolean crossline )
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
void TikiScript::GetVector( qboolean crossline, float *vec )
{
	vec[ 0 ] = GetFloat( crossline );
	vec[ 1 ] = GetFloat( crossline );
	vec[ 2 ] = GetFloat( crossline );
}

/*
==============
=
= LinesInFile
=
==============
*/
int TikiScript::LinesInFile( void )
{
	qboolean		temp_tokenready;
	const char		*temp_script_p;
	int				temp_line;
	char			temp_token[ MAXTOKEN ];
	int				numentries;

	temp_tokenready		= tokenready;
	temp_script_p		= script_p;
	temp_line			= line;
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
void TikiScript::Parse( char *data, int length, const char *name )
{
	Close();

	buffer = data;
	Reset();
	this->length = length;
	end_p = script_p + length;
	strncpy( filename, name, MAXTOKEN );
}

/*
==============
=
= LoadFile
=
==============
*/
qboolean TikiScript::LoadFile( const char *name, qboolean quiet )
{
	int			length;
	char		*buf;

	Close();

	length = TIKI_ReadFileEx( name, ( void ** )&buf, quiet );

	if( length < 0 )
	{
		if( !quiet )
			TIKI_DPrintf( "Tiki:LoadFile Couldn't load %s\n", name );
		return false;
	}

	// create our own space
	buffer = ( char * )TIKI_Alloc( length + 1 );
	// copy the file over to our space
	memcpy( buffer, buf, length );
	buffer[ length ] = 0;
	// free the file
	TIKI_FreeFile( buf );

	Parse( buffer, length, name );
	releaseBuffer = true;

	if( error )
	{
		return false;
	}

	return true;
}

/*
==============
=
= Token
=
==============
*/
const char *TikiScript::Token( void )
{
	if( include )
	{
		return include->token;
	}
	else
	{
		return token;
	}
}

/*
==============
=
= MarkPos
=
==============
*/
void TikiScript::MarkPos( void )
{
	mark[ mark_pos ].mark_script_p = script_p;
	mark[ mark_pos ].mark_tokenready = tokenready;
	strcpy( mark[ mark_pos ].mark_token, token );
	mark_pos++;
}

/*
==============
=
= ReturnToMark
=
==============
*/
void TikiScript::ReturnToMark( void )
{
	if( mark_pos <= 0 )
	{
		return;
	}

	mark_pos--;
	script_p	= mark[ mark_pos ].mark_script_p;
	tokenready	= mark[ mark_pos ].mark_tokenready;
	memcpy( token, mark[ mark_pos ].mark_token, sizeof( token ) );
}

/*
==============
=
= ReplaceLineWithWhitespace
=
==============
*/
void TikiScript::ReplaceLineWithWhitespace( bool deleteFromStartOfLine )
{
	char *change_p = ( char * )script_p;

	if( deleteFromStartOfLine && *( change_p - 1 ) != '\n' )
	{
		while( *( change_p - 1 ) != TOKENEOL && change_p > buffer )
		{
			change_p--;
		}
	}

	if( *change_p != TOKENEOL )
	{
		if( change_p >= end_p )
		{
			script_p = change_p;
			return;
		}

		do
		{
			*change_p++ = TOKENSPACE;
		} while( *change_p != TOKENEOL && change_p < end_p );
	}

	if( change_p < end_p )
	{
		script_p = change_p + 1;
		line++;
	}
	else
	{
		script_p = change_p;
	}
}

/*
==============
=
= GetParentToken
=
==============
*/
const char *TikiScript::GetParentToken( void )
{
	if( parent )
		return parent->token;
	else
		return "";
}
