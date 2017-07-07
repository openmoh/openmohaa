%{
/*
* ===========================================================================
* Copyright (C) 2015 the OpenMoHAA team
* 
* This file is part of OpenMoHAA source code.
* 
* OpenMoHAA source code is free software; you can redistribute it
* and/or modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of the License,
* or (at your option) any later version.
* 
* OpenMoHAA source code is distributed in the hope that it will be
* useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with OpenMoHAA source code; if not, write to the Free Software
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
* ===========================================================================
*
*
* yyLexer.*: FLEX Lexical grammar for MoHScript.
*/

#include "compiler.h"
#include "yyParser.h"

#include <stdio.h>

void fprintf2( FILE *f, const char *format, ... )
{
	va_list va;
	static char buffer[ 4200 ];

	va_start( va, format );
	vsprintf( buffer, format, va );
	va_end( va );

	glbs.Printf( buffer );
}

#define fprintf fprintf2

int prev_yylex = 0;

extern yyparsedata parsedata;

#define YYLLOCSET { yylval.s.sourcePos = parsedata.pos - yyleng; }
#define YYLEX(n) { prev_yylex = n; return n; }

#define YY_USER_ACTION parsedata.pos; parsedata.pos += yyleng;
#define YY_FATAL_ERROR( n ) yylexerror( n )

void yylexerror( const char *msg )
{
	glbs.Printf( "%s\n%s", msg, yytext );
	assert( 0 );
}

static void TextEscapeValue( char *str, size_t len )
{
	char *to = parsetree_malloc( len + 1 );

	yylval.s.val.stringValue = to;

	while( len )
	{
		if( *str == '\\' )
		{
			len--;

			if( !len )
				break;

			str++;
			if( *str == 'n' )
			{
				*to = '\n';
			}
			else if( *str == 't' )
			{
				*to = '\t';
			}
			else if( *str == '"' )
			{
				*to = '\"';
			}
			else
			{
				*to = *str;
			}
		}
		else
		{
			*to = *str;
		}

		len--;
		str++;
		to++;
	}

	*to = 0;
}


static void TextValue( char *str, size_t len )
{
	yylval.s.val.stringValue = parsetree_malloc( len + 1 );
	strncpy( yylval.s.val.stringValue, str, len );
	yylval.s.val.stringValue[ len ] = 0;
}

static bool UseField( void )
{
	//return prev_yylex == TOKEN_PERIOD || prev_yylex == TOKEN_DOLLAR;
	return ( !strncmp( yytext, "game.", 5 ) ||
	!strncmp( yytext, "group.", 6 ) ||
	!strncmp( yytext, "level.", 6 ) ||
	!strncmp( yytext, "local.", 6 ) ||
	!strncmp( yytext, "parm.", 5 ) ||
	!strncmp( yytext, "owner.", 6 ) ||
	!strncmp( yytext, "self.", 5 ) ||
	*yytext == '$' || *yytext == '-' || *yytext == '/' );
}

%}

/*%option debug*/
 
%option outfile="../../../code/globalcpp/parser/yyLexer.cpp" header-file="../../../code/globalcpp/parser/yyLexer.h"
%option warn nodefault

%option noyywrap never-interactive
%option yylineno

%x C_COMMENT
%x C_LINE_COMMENT
%x VARIABLES
%x IDENTIFIER

%%

"/*"							{ BEGIN( C_COMMENT ); }
<C_COMMENT>"*/"					{ BEGIN( INITIAL ); }
<C_COMMENT>\n					{ ; }
<C_COMMENT>.					{ ; }
"*/"							{ Compiler.CompileError( parsedata.pos - yyleng, "'*/' found outside of comment" ); }


\\[\r\n]+						{ ; }
"//"[^\r\n]*					{ if( prev_yylex != TOKEN_EOL ) YYLEX( TOKEN_EOL ); }

<VARIABLES>[\r\n]*				{ BEGIN( INITIAL ); YYLEX( TOKEN_EOL ); }
<VARIABLES>"size"				{ YYLEX( TOKEN_SIZE ); }
<VARIABLES>"."					{ YYLEX( TOKEN_PERIOD ); }
<VARIABLES>\"([^\\\"]|\\.)*\"	{ YYLLOCSET; TextEscapeValue( yytext + 1, strlen( yytext ) - 2 ); YYLEX( TOKEN_STRING ); }
<VARIABLES>[a-zA-Z0-9_\"]+		{ YYLLOCSET; TextValue( yytext, strlen( yytext ) ); YYLEX( TOKEN_IDENTIFIER ); }
<VARIABLES>.					{
									for ( int i = yyleng - 1; i >= 0; --i )
										unput( yytext[ i ] );

									parsedata.pos -= yyleng;

									BEGIN( INITIAL );
								}

[\r\n]*							{ if( prev_yylex != TOKEN_EOL ) YYLEX( TOKEN_EOL ); }
[ \t]*							{ ; }

\"([^\\\"]|\\.)*\"				{ YYLLOCSET; TextEscapeValue( yytext + 1, strlen( yytext ) - 2 ); YYLEX( TOKEN_STRING ); }


"?"								{ YYLEX( TOKEN_TERNARY ); }
"if"							{ YYLEX( TOKEN_IF ); }
"else"							{ YYLEX( TOKEN_ELSE ); }
"while"							{ YYLEX( TOKEN_WHILE ); }
"for"							{ YYLEX( TOKEN_FOR ); }
"do"							{ YYLEX( TOKEN_DO ); }

"game"?							{ BEGIN( VARIABLES ); yylval.s.val = node1_( method_game ); YYLEX( TOKEN_LISTENER ); }
"group"							{ BEGIN( VARIABLES ); yylval.s.val = node1_( method_group ); YYLEX( TOKEN_LISTENER ); }
"level"							{ BEGIN( VARIABLES ); yylval.s.val = node1_( method_level ); YYLEX( TOKEN_LISTENER ); }
"local"							{ BEGIN( VARIABLES ); yylval.s.val = node1_( method_local ); YYLEX( TOKEN_LISTENER ); }
"parm"							{ BEGIN( VARIABLES ); yylval.s.val = node1_( method_parm ); YYLEX( TOKEN_LISTENER ); }
"owner"							{ BEGIN( VARIABLES ); yylval.s.val = node1_( method_owner ); YYLEX( TOKEN_LISTENER ); }
"self"							{ BEGIN( VARIABLES ); yylval.s.val = node1_( method_self ); YYLEX( TOKEN_LISTENER ); }

"{"								{ parsedata.braces_count++; YYLEX( TOKEN_LBRACKET ); }
"}"								{ parsedata.braces_count--; YYLEX( TOKEN_RBRACKET ); }
"("								{ YYLLOCSET; YYLEX( TOKEN_LPAREN ); }
")"								{ BEGIN( VARIABLES ); YYLLOCSET; YYLEX( TOKEN_RPAREN ); }
"["								{ YYLEX( TOKEN_LSQUARE ); }
"]"								{ BEGIN( VARIABLES ); YYLEX( TOKEN_RSQUARE ); }

"="								{ YYLEX( TOKEN_ASSIGNMENT ); }
":"								{ YYLEX( TOKEN_COLON ); }
"::"							{ YYLEX( TOKEN_DOUBLE_COLON ); }
";"								{ YYLEX( TOKEN_SEMICOLON ); }

"=="							{ YYLEX( TOKEN_EQUALITY ); }
"||"							{ YYLEX( TOKEN_LOGICAL_OR ); }
"&&"							{ YYLEX( TOKEN_LOGICAL_AND ); }

"|"								{ YYLEX( TOKEN_BITWISE_OR ); }
"^"								{ YYLEX( TOKEN_BITWISE_EXCL_OR ); }
"&"								{ YYLEX( TOKEN_BITWISE_AND ); }
"!="							{ YYLEX( TOKEN_INEQUALITY ); }
"<"								{ YYLEX( TOKEN_LESS_THAN ); }
">"								{ YYLEX( TOKEN_GREATER_THAN ); }
"<="							{ YYLEX( TOKEN_LESS_THAN_OR_EQUAL ); }
">="							{ YYLEX( TOKEN_GREATER_THAN_OR_EQUAL ); }
[ ]"-"							{ YYLEX( TOKEN_NEG ); }

"+"								{ YYLEX( TOKEN_PLUS ); }
"+="							{ YYLEX( TOKEN_PLUS_EQUALS ); }
"++"|[ ]"++"					{ YYLEX( TOKEN_INCREMENT ); }
"-"|"-"[ ]|[ ]"-"[ ]			{ YYLEX( TOKEN_MINUS ); }
"-="							{ YYLEX( TOKEN_MINUS_EQUALS ); }
[ ]"-="							{ YYLEX( TOKEN_MINUS_EQUALS ); }
"--"|[ ]"--"					{ YYLEX( TOKEN_DECREMENT ); }
"*"								{ YYLEX( TOKEN_MULTIPLY ); }
"*="							{ YYLEX( TOKEN_MULTIPLY_EQUALS ); }
"/"								{ YYLEX( TOKEN_DIVIDE ); }
"/="							{ YYLEX( TOKEN_DIVIDE_EQUALS ); }
"%"								{ YYLEX( TOKEN_MODULUS ); }
"%="							{ YYLEX( TOKEN_MODULUS_EQUALS ); }
"<<"							{ YYLEX( TOKEN_SHIFT_LEFT ); }
"<<="							{ YYLEX( TOKEN_SHIFT_LEFT_EQUALS ); }
">>"							{ YYLEX( TOKEN_SHIFT_RIGHT ); }
">>="							{ YYLEX( TOKEN_SHIFT_RIGHT_EQUALS ); }
"&="							{ YYLEX( TOKEN_AND_EQUALS ); }
"^="							{ YYLEX( TOKEN_EXCL_OR_EQUALS ); }
"|="							{ YYLEX( TOKEN_OR_EQUALS ); }
"$"								{ BEGIN( VARIABLES ); YYLEX( TOKEN_DOLLAR ); }
"!"								{ YYLEX( TOKEN_NOT ); }
"~"								{ YYLEX( TOKEN_COMPLEMENT ); }

"."								{ YYLEX( TOKEN_PERIOD ); }

","								{ YYLEX( TOKEN_COMMA ); }
"#"								{ YYLEX( TOKEN_NUMBER ); }

"NULL"							{ YYLEX( TOKEN_NULL ); }
"NIL"							{ YYLEX( TOKEN_NIL ); }

[0-9]+									{ YYLLOCSET; sscanf( yytext, "%d", &yylval.s.val.intValue ); YYLEX( TOKEN_INTEGER ); }
[0-9\.]+|[0-9\.]+("e+"|"e-")+[0-9\.]	{ YYLLOCSET; sscanf( yytext, "%f", &yylval.s.val.floatValue ); YYLEX( TOKEN_FLOAT ); }

"try"							{ YYLEX( TOKEN_TRY ); }
"catch"							{ YYLEX( TOKEN_CATCH ); }
"switch"						{ YYLEX( TOKEN_SWITCH ); }

"case"							{ YYLEX( TOKEN_CASE ); }
"break"							{ YYLEX( TOKEN_BREAK ); }
"continue"						{ YYLEX( TOKEN_CONTINUE ); }

"makearray"|"makeArray"			{ YYLEX( TOKEN_MAKEARRAY ); }
"endarray"|"endArray"			{ YYLEX( TOKEN_ENDARRAY ); }

[a-zA-Z0-9_\./\\-]+				{
									if( UseField() )
									{
										parsedata.pos -= yyleng;
										REJECT;
									}
									else
									{
										YYLLOCSET;
										TextEscapeValue( yytext, yyleng );
										YYLEX( TOKEN_IDENTIFIER );
									}
								}

.								{ yylexerror( "bad token:\n" ); }

%{

#undef fprintf

%}

%%
