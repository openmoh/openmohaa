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

// configurator.h: Config class for INI files.

#ifndef __CONFIGURATOR_H__
#define __CONFIGURATOR_H__

#include <listener.h>

#define MAX_CONFIGURATOR_ARRAY_SIZE				2048

typedef struct configValue_s {
	bool	m_bNeedWrite;
	str		value;
} configValue_t;

typedef struct configKey_s {
	bool			m_bNeedWrite;
	str				name;
	Container< configValue_t > value;
	bool			m_bArray;
} configKey_t;

typedef struct configSection_s {
	bool			m_bNeedWrite;
	str				name;
	Container< configKey_t * > key;
} configSection_t;

enum {
	LINE_EMPTY,
	LINE_COMMENT,
	LINE_SECTION,
	LINE_VALUE,
	LINE_ERROR
};

class Configurator : public Class {
private:
	str		m_filename;
	bool	m_bNoWrite;
	bool	m_bNeedWrite;

	con_set< str, configSection_t > m_sections;
	Container< configSection_t * > m_reverseSections;

private:
	size_t		GetLine( char *dest, const char *data, size_t size );
	str			GetValue( const char *section, const char *key, str defaultValue, int index = -1 );
	configKey_t	*GetKey( const char *section, const char *key, int index = -1 );

	int		CutLine( char *data );
	bool	SetupLine( char *line, int& lineno, size_t& len, size_t& last );
	bool	FindData( int type, const char *section, const char *key, size_t *offset, const char *data, size_t size );
	void	ParseData( const char *data, size_t size );
	void	WriteData( char **data, size_t *size );
	void	WriteData2( char **data, size_t *size );
	int		ParseLine( char *line, char *section, char *key, char *value );

	configSection_t		*CreateSection( const char *section );
	configSection_t		*FindSection( const char *section );
	int					GetKeyArray( char *key );
	int					GetKeyArray( str& key );
	configKey_t			*CreateKey( configSection_t *section, const char *key, unsigned int *index );
	configKey_t			*FindKey( configSection_t *section, const char *key );
	void				RemoveSection( configSection_t *section );
	void				RemoveKey( configSection_t *section, configKey_t *key );

public:
	CLASS_PROTOTYPE( Configurator );

	Configurator( const char *filename );
	Configurator();
	~Configurator();

	void		Parse( const char *filename );
	void		Close();
	void		SetWrite( bool bWrite );

	str			GetString( const char *section, const char *key, str defaultValue, int index = -1 );
	int			GetInteger( const char *section, const char *key, int defaultValue, int index = -1 );
	float		GetFloat( const char *section, const char *key, float defaultValue, int index = -1 );
	void		SetString( const char *section, const char *key, str value, int index = -1 );
	void		SetInteger( const char *section, const char *key, int value, int index = -1 );
	void		SetFloat( const char *section, const char *key, float value, int index = -1 );
};

void test_config( void );

#endif /* __CONFIGURATOR_H__ */
