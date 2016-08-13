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

// tiki_script.h : TIKI script

#ifndef TIKI_SCRIPT_H
#define TIKI_SCRIPT_H

#define TOKENCOMMENT		(';')
#define TOKENCOMMENT2		('#')
#define TOKENEOL			('\n')
//#define TOKENNULL			('\0')
#define TOKENSPACE			(' ')
#define TOKENSPECIAL		('$')

#ifndef MAXTOKEN
#define	MAXTOKEN	256
#endif

#define MAXMACROS	48

typedef struct {
	char name[ MAXTOKEN ];
	char macro[ MAXTOKEN ];
} tiki_macro_t;

typedef struct {
	const char *mark_script_p;
	qboolean mark_tokenready;
	char mark_token[ MAXTOKEN ];
} tiki_mark_t;

#ifdef __cplusplus

class TikiScript {
protected:
	qboolean error;
	qboolean tokenready;
	class TikiScript *include;
	class TikiScript *parent;
	char filename[ MAXTOKEN ];
	const char *script_p;
	const char *end_p;
	tiki_macro_t macros[ MAXMACROS ];
	int nummacros;
	int line;
	char token[ MAXTOKEN ];
	qboolean releaseBuffer;
	tiki_mark_t mark[ 64 ];
	int mark_pos;

public:
	char *buffer;
	int length;
	char path[ MAX_QPATH ];
	static class TikiScript *currentScript;

protected:
	qboolean			AtComment();
	qboolean			AtExtendedComment();
	qboolean			AtCommand();
	qboolean			AtString( qboolean crossline );
	qboolean			ProcessCommand( qboolean crossline );
	qboolean			Completed();
	qboolean			SafeCheckOverflow();
	void				CheckOverflow();
	void				Uninclude();
	const char			*FindMacro( const char *macro );
	void				AddMacro( const char *macro, const char *expansion );
	qboolean			SkipToEOL();
	void				SkipWhiteSpace( qboolean crossline );
	void				SkipNonToken( qboolean crossline );
	qboolean			CommentAvailable( qboolean crossline );
	void				SkipExtendedComment();

public:
	TikiScript();
	~TikiScript();

	void		Close();
	const char	*Filename();
	int			GetLineNumber();
	void		Reset();
	qboolean	TokenAvailable( qboolean crossline );
	void		UnGetToken();
	const char	*GetToken( qboolean crossline );
	const char	*GetLine( qboolean crossline );
	const char	*GetAndIgnoreLine( qboolean crossline );
	const char	*GetRaw();
	const char	*GetString( qboolean crossline );
	qboolean	GetSpecific( const char *string );
	int			GetInteger( qboolean crossline );
	double		GetDouble( qboolean crossline );
	float		GetFloat( qboolean crossline );
	void		GetVector( qboolean crossline, float *vec );
	int			LinesInFile();
	void		Parse( char *data, int length, const char *name );
	qboolean	LoadFile( const char *name, qboolean quiet );
	const char	*Token();
	void		MarkPos();
	void		ReturnToMark();
	void		ReplaceLineWithWhitespace( bool deleteFromStartOfLine );
	void		Exclude();
	const char	*GetParentToken();
};

class TikiSwitchKey {
public:
	str sKeyName;
	str sKeyValue;
	TikiSwitchKey *pNextKey;

	TikiSwitchKey();
};

class TikiSwitcher {
	TikiSwitchKey *m_pSwitchKeys;

private:
	TikiSwitchKey *GetSwitchKey( const char * );
	void RemoveUnwantedSwitchOptions( TikiScript * );

public:
	TikiSwitcher();

	void		AddSwitchKey( const char *, const char * );
	void		ChangeSwitchKey( const char *, const char * );
	void		AddOrChangeSwitchKey( const char *, const char * );
	str&		GetSwitchKeyValue( const char * );
	void		PrecompileTikiScript( TikiScript * );
};

#endif

#endif // TIKI_SCRIPT_H

