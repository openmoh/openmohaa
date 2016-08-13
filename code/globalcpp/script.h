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

// script.h: C++ implementation of tokenization/interpretation.

#ifndef __SCRIPT_H__
#define __SCRIPT_H__

#include "class.h"
#include "vector.h"
#include "str.h"
#include "archive.h"

#define TOKENCOMMENT		(';')
#define TOKENCOMMENT2		('#')
#define TOKENEOL			('\n')
//#define TOKENNULL			('\0')
#define TOKENSPACE			(' ')
#define TOKENSPECIAL		('$')

#define	MAXTOKEN	512

typedef struct
	{
	qboolean		tokenready;
   int         offset;
	int			line;
	char			token[ MAXTOKEN ];
	} scriptmarker_t;

typedef struct
	{
	//const char		*macroName;
	//const char		*macroText;
	str				macroName;
	str				macroText;
	} macro;

class Script : public Class
	{
	protected:
		qboolean		tokenready;

		str			filename;
		const char	*script_p;
		const char	*end_p;
		Container<macro *> macrolist;

		int				line;
		char			token[ MAXTOKEN ];

		qboolean		releaseBuffer;
		qboolean		hasError;

		qboolean		AtComment( void );
		void			CheckOverflow( void );

	public:
		const char	*buffer;
		size_t		length;

      CLASS_PROTOTYPE( Script );

	  virtual void		Archive( Archiver &arc );

		~Script();
		Script( const char* filename );
		Script();

		void			Close( void );
		const char *Filename( void );
		int			GetLineNumber( void );
		void			Reset( void );
		void			MarkPosition( scriptmarker_t *mark );
		void			RestorePosition( const scriptmarker_t *mark );
		qboolean		SkipToEOL( void );
		void			SkipWhiteSpace( qboolean crossline );
		void			SkipNonToken( qboolean crossline );
		qboolean		TokenAvailable( qboolean crossline );
		qboolean		CommentAvailable( qboolean crossline );
		void			UnGetToken( void );
		qboolean		AtString( qboolean crossline );
		qboolean		AtOpenParen( qboolean crossline );
		qboolean		AtCloseParen( qboolean crossline );
		qboolean		AtComma( qboolean crossline );
		qboolean		AtDot( qboolean crossline );
		qboolean		AtAssignment( qboolean crossline );
		const char		*GetToken( qboolean crossline );
		const char		*GetLine( qboolean crossline );
		const char		*GetRaw( void );
		const char		*GetString( qboolean crossline );
		qboolean		GetSpecific( const char *string );
		qboolean		GetBoolean( qboolean crossline );
		int				GetInteger( qboolean crossline );
		double			GetDouble( qboolean crossline );
		float			GetFloat( qboolean crossline );
		Vector			GetVector( qboolean crossline );
		int				LinesInFile( void );
		void			Parse( const char *data, size_t length, const char *name );
		void			LoadFile( const char *name );
		void			LoadFile( const char *name, int length, const char *buf );
		const char		*Token( void );
		void			AddMacroDefinition( qboolean crossline );
		const char		*GetMacroString( const char *theMacroName );
		char			*EvaluateMacroString( const char *theMacroString );
		float			EvaluateMacroMath(float value, float newval, char oper);
		const char		*GetExprToken(const char *ptr, char *token);
		const char		*GrabNextToken( qboolean crossline );
		qboolean		isMacro( void );

		qboolean		EndOfFile();
		qboolean		isValid( void );

		Container<macro *>	*GetMacroList() { return &macrolist; }
		void				AddMacro(const char *name, const char *value);
	};

inline void Script::Archive
	(
	Archiver &arc
	)
   {
	int pos;

   arc.ArchiveBoolean( &tokenready );

   arc.ArchiveString( &filename );
   if ( arc.Loading() )
      {
      //
      // load the file in
      //
      LoadFile( filename.c_str() );
      }

   if ( !arc.Loading() )
      {
      //
      // save out current pointer as an offset
      //
      pos = script_p - buffer;
      }
   arc.ArchiveInteger( &pos );
   if ( arc.Loading() )
      {
      //
      // restore the script pointer
      //
      script_p = buffer + pos;
      }

   	//const char	*end_p;
	//Container<macro *> macrolist;

   arc.ArchiveInteger( &line );
   arc.ArchiveRaw( &token, sizeof( token ) );

   //qboolean		releaseBuffer;
   }

#endif
