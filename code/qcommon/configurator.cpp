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

// configurator.cpp: Config class for INI files.

#include "configurator.h"

CLASS_DECLARATION( Class, Configurator, NULL )
{
	{ NULL, NULL }
};

Configurator::Configurator( const char *filename )
{
	Parse( filename );
}

Configurator::Configurator()
{
	m_bNoWrite = false;
	m_bNeedWrite = false;
}

Configurator::~Configurator()
{
	Close();
}

size_t Configurator::GetLine( char *dest, const char *data, size_t size )
{
	const char *p = data;
	size_t i = 0;

	while( *p == '\n' )
		p++;

	if( p >= data + size )
		return 0;

	while( *p && *p != '\n' && i < size )
		dest[ i++ ] = *p++;

	if( i < size )
		dest[ i++ ] = '\n';

	dest[ i ] = 0;

	return p - data;
}

void Configurator::RemoveSection( configSection_t *section )
{
	configKey_t *key;
	int i;

	for( i = 0; i < section->key.NumObjects(); i++ )
	{
		key = section->key[ i ];
		RemoveKey( section, key );
	}

	m_reverseSections.RemoveObject( section );
	m_sections.remove( section->name );
}

void Configurator::RemoveKey( configSection_t *section, configKey_t *key )
{
	section->key.RemoveObject( key );
	delete key;
}

configSection_t *Configurator::CreateSection( const char *section )
{
	configSection_t *s = &m_sections.addKeyValue( section );

	if( s->key.NumObjects() )
	{
		RemoveSection( s );
		s = &m_sections.addKeyValue( section );
	}
	else
	{
		m_reverseSections.AddObject( s );
	}

	s->name = section;
	return s;
}

int Configurator::GetKeyArray( char *key )
{
	int count;
	size_t len;
	int arrayindex = -1;
	char *p1, *p2;

	len = strlen( key ) + 1;
	p1 = new char[ len ];
	p2 = new char[ len ];

	count = sscanf( key, "%[^[] [%[^]]", p1, p2 );
	if( count == 2 )
	{
		// Check for an array [num]
		strcpy( key, p1 );
		arrayindex = atoi( p2 );
		if( arrayindex < 0 || arrayindex > MAX_CONFIGURATOR_ARRAY_SIZE )
		{
			arrayindex = -2;
		}
	}

	delete[] p1;
	delete[] p2;

	return arrayindex;
}

int Configurator::GetKeyArray( str& key )
{
	int count;
	size_t len;
	int arrayindex = -1;
	str arrayval;
	char *p1, *p2;

	len = key.length() + 1;
	p1 = new char[ len ];
	p2 = new char[ len ];

	count = sscanf( key, "%[^[] [%[^]]", p1, p2 );
	if( count == 2 )
	{
		// Check for an array [num]
		arrayval = p2;
		arrayindex = atoi( arrayval.c_str() );

		key = p1;

		if( arrayindex < 0 || arrayindex > MAX_CONFIGURATOR_ARRAY_SIZE )
		{
			arrayindex = -2;
		}
	}

	delete[] p1;
	delete[] p2;

	return arrayindex;
}

configKey_t *Configurator::CreateKey( configSection_t *section, const char *key, unsigned int *index )
{
	configKey_t *k;
	str keyname;
	int arrayindex;

	if( index ) *index = 0;

	keyname = key;

	arrayindex = GetKeyArray( keyname );
	if( arrayindex == -2 )
	{
		return NULL;
	}
	else if( arrayindex >= 0 )
	{
		if( index ) *index = arrayindex;
	}

	k = FindKey( section, keyname );
	if( k )
	{
		if( arrayindex == -1 || !k->value.NumObjects() )
		{
			RemoveKey( section, k );
			k = NULL;
		}
	}

	if( !k )
	{
		k = new configKey_t;
		section->key.AddObject( k );
		k->name = keyname;
		k->m_bArray = arrayindex != -1;
		k->m_bNeedWrite = false;
	}

	if( arrayindex >= k->value.MaxObjects() )
	{
		k->value.Resize( ( arrayindex + 1 ) * 2);
	}

	return k;
}

configSection_t *Configurator::FindSection( const char *section )
{
	configSection_t *s = m_sections.findKeyValue( section );
	return s;
}

configKey_t *Configurator::FindKey( configSection_t *section, const char *key )
{
	configKey_t *k;
	int i;

	for( i = 0; i < section->key.NumObjects(); i++ )
	{
		k = section->key[ i ];
		if( !k->name.icmp( key ) )
		{
			return k;
		}
	}

	return NULL;
}

int Configurator::CutLine( char *data )
{
	char *p = data;
	while( *p == '\n' )
		*p++ = 0;
	while( *p && *p != '\n' )
		*p++ = 0;
	return p - data;
}

bool Configurator::SetupLine( char *line, int& lineno, size_t& len, size_t& last )
{
	lineno++;
	len = ( int )strlen( line ) - 1;
	if( len <= 0 )
	{
		return false;
	}

	if( line[ len ] != '\n' )
	{
		last = 0;
		glbs.DPrintf( "Configurator::SetupLine: input line too long in %s (line %d)\n", m_filename.c_str(), lineno );
		return false;
	}

	while( ( len >= 0 ) && ( ( line[ len ] == '\n' ) || ( isspace( line[ len ] ) ) ) )
	{
		line[ len ] = 0;
		len--;
	}
	if( line[ len ] == '\\' )
	{
		last = len;
		return false;
	}
	else
	{
		last = 0;
	}

	return true;
}

void Configurator::WriteData( char **data, size_t *size )
{
	configSection_t *section;
	configKey_t *key;
	configValue_t *val;
	str value;
	char *oldData = *data;
	char *currentData = *data;
	size_t currentSize = *size;
	size_t len, tlen = 0;
	int i, j, k;
	size_t offset, offset2;
	bool bFound;

	for( i = 1; i <= m_reverseSections.NumObjects(); i++ )
	{
		section = m_reverseSections.ObjectAt( i );

		if( !FindData( LINE_SECTION, section->name, NULL, &offset, currentData, currentSize ) )
		{
			oldData = currentData;

			len = ( offset ? 2 : 0 ) + 1 + section->name.length() + 1; // \n\n + [ + section + ]
			currentSize += len;
			currentData = new char[ currentSize + 1 ];
			strncpy( currentData, oldData, offset );
			sprintf( currentData + offset, offset ? "\n\n[%s]" : "[%s]", section->name.c_str() );
			strcpy( currentData + offset + len, oldData + offset );

			if( currentData != oldData ) delete[] oldData;
		}

		for( j = 0; j < section->key.NumObjects(); j++ )
		{
			key = section->key[ j ];
			if( !key->m_bNeedWrite )
				continue;

			if( key->m_bArray )
			{
				for( k = 0; k < key->value.NumObjects(); k++ )
				{
					val = &key->value[ k ];
					if( !val->m_bNeedWrite )
						continue;
					value = val->value;

					bFound = FindData( LINE_VALUE, section->name, key->name + "[" + k + "]", &offset, currentData, currentSize );
					{
						int digits, l;

						oldData = currentData;

						l = k;
						digits = 0;
						if( !l )
							l = 1;
						while( l )
						{
							l /= 10;
							digits++;
						}

						if( bFound )
						{
							len = CutLine( currentData + offset );
							currentSize -= len;
							offset2 = offset + len;
						}
						else
						{
							offset2 = offset;
						}

						len = 1 + key->name.length() + 1 + digits + 1 + 1 + value.length(); // \n + name + [ + digits + ] + = + value
						currentSize += len;
						currentData = new char[ currentSize + 1 ];
						strncpy( currentData, oldData, offset );
						sprintf( currentData + offset, "\n%s[%d]=%s", key->name.c_str(), k, value.c_str() );
						strcpy( currentData + offset + len, oldData + offset2 );

						if( currentData != oldData ) delete[] oldData;
					}
				}
			}
			else
			{
				val = &key->value[ 0 ];
				if( !val->m_bNeedWrite )
					continue;
				value = val->value;

				bFound = FindData( LINE_VALUE, section->name, key->name, &offset, currentData, currentSize );
				{
					oldData = currentData;

					if( bFound )
					{
						len = CutLine( currentData + offset );
						currentSize -= len;
						offset2 = offset + len;
					}
					else
					{
						offset2 = offset;
					}

					len = 1 + key->name.length() + 1 + value.length(); // \n + name + = + value
					currentSize += len;
					currentData = new char[ currentSize + 1 ];
					strncpy( currentData, oldData, offset );
					sprintf( currentData + offset, "\n%s=%s", key->name.c_str(), value.c_str() );
					strcpy( currentData + offset + len, oldData + offset2 );

					if( currentData != oldData ) delete[] oldData;
				}
			}
		}
	}

	if( currentData[ currentSize - 1 ] != '\n' )
	{
		currentData[ currentSize ] = '\n';
		currentSize++;
	}

	*data = currentData;
	*size = currentSize;
}

void Configurator::WriteData2( char **data, size_t *size )
{
	configSection_t *section;
	configKey_t *key;
	configValue_t *val;
	str value;
	char *oldData = *data;
	char *currentData = *data;
	size_t currentSize = *size;
	char *p = currentData;
	size_t tlen = 0;
	int i, j, k;

	for( i = 1; i <= m_reverseSections.NumObjects(); i++ )
	{
		section = m_reverseSections.ObjectAt( i );

		if( i == 1 )
		{
			tlen += 1 + section->name.length() + 1 + 1; // [ + name + ] + \n
		}
		else
		{
			tlen += 1 + 1 + section->name.length() + 1 + 1; // \n + [ name + ] + \n
		}

		if( tlen > currentSize )
		{
			oldData = currentData;

			currentSize = tlen + 1024;
			currentData = new char[ currentSize ];
			p = currentData + ( p - oldData );
			strcpy( currentData, oldData );

			delete[] oldData;
		}

		if( i == 1 )
		{
			p += sprintf( p, "[%s]\n", section->name.c_str() );
		}
		else
		{
			p += sprintf( p, "\n[%s]\n", section->name.c_str() );
		}

		for( j = 0; j < section->key.NumObjects(); j++ )
		{
			key = section->key[ j ];
			if( !key->m_bNeedWrite )
				continue;

			if( key->m_bArray )
			{
				for( k = 0; k < key->value.NumObjects(); k++ )
				{
					val = &key->value[ k ];
					if( !val->m_bNeedWrite )
						continue;
					value = val->value;

					{
						int digits, l;

						l = k;
						digits = 0;
						if( !l )
							l = 1;
						while( l )
						{
							l /= 10;
							digits++;
						}

						tlen += key->name.length() + 1 + digits + 1 + 1 + value.length() + 1; // name + [ + digits + ] + = + value + \n
						if( tlen > currentSize )
						{
							oldData = currentData;

							currentSize = tlen + 1024;
							currentData = new char[ currentSize ];
							p = currentData + ( p - oldData );
							strcpy( currentData, oldData );

							delete[] oldData;
						}

						p += sprintf( p, "%s[%d]=%s\n", key->name.c_str(), k, value.c_str() );
					}
				}
			}
			else
			{
				val = &key->value[ 0 ];
				if( !val->m_bNeedWrite )
					continue;
				value = val->value;

				tlen += key->name.length() + 1 + value.length() + 1; // name + = + value + \n
				if( tlen > currentSize )
				{
					oldData = currentData;

					currentSize = tlen + 1024;
					currentData = new char[ currentSize ];
					p = currentData + ( p - oldData );
					strcpy( currentData, oldData );

					delete[] oldData;
				}

				p += sprintf( p, "%s=%s\n", key->name.c_str(), value.c_str() );
			}
		}
	}

	*data = currentData;
	*size = tlen;
}

bool Configurator::FindData( int type, const char *s, const char *k, size_t *offset, const char *data, size_t size )
{
	static char	line[ MAX_STRING_TOKENS + 1 ];
	static char	section[ MAX_STRING_TOKENS + 1 ];
	static char	key[ MAX_STRING_TOKENS + 1 ];
	static char	val[ MAX_STRING_TOKENS + 1 ];

	size_t	last = 0;
	size_t	len;
	size_t	olen = 0;
	size_t	tlen = 0;
	int		lineno = 0;
	int		arrayindex;

	bool	bFoundSection = false;

	*offset = 0;

	while( ( olen = GetLine( line + last, data + tlen, MAX_STRING_TOKENS - last ) ) != 0 )
	{
		tlen += olen;
		if( !SetupLine( line + last, lineno, len, last ) )
		{
			continue;
		}

		switch( ParseLine( line, section, key, val ) )
		{
		case LINE_SECTION:
			if( bFoundSection )
			{
				// Return the last line of the section to append
				*offset = tlen - olen;
				return false;
			}

			if( !Q_stricmp( s, section ) )
			{
				bFoundSection = true;

				if( type == LINE_SECTION )
				{
					*offset = tlen;
					return true;
				}
			}
			break;

		case LINE_VALUE:
			arrayindex = GetKeyArray( key );
			if( arrayindex >= 0 )
			{
				sprintf( key, "%s[%d]", key, arrayindex );
			}

			if( type == LINE_VALUE && bFoundSection && !Q_stricmp( k, key ) )
			{
				*offset = tlen - olen;
				return true;
			}
			break;

		default:
			break;
		}
	}

	*offset = tlen;
	return false;
}

void Configurator::ParseData( const char *data, size_t size )
{
	static char	line[ MAX_STRING_TOKENS + 1 ];
	static char	section[ MAX_STRING_TOKENS + 1 ];
	static char	key[ MAX_STRING_TOKENS + 1 ];
	static char	val[ MAX_STRING_TOKENS + 1 ];

	size_t	last = 0;
	size_t	len;
	size_t	tlen = 0;
	int		lineno = 0;
	unsigned int index = 0;

	configSection_t *current;
	configKey_t *k;
	configValue_t v;

	while( ( len = GetLine( line + last, data + tlen, MAX_STRING_TOKENS - last ) ) != 0 )
	{
		tlen += len;
		if( !SetupLine( line + last, lineno, len, last ) )
		{
			continue;
		}

		switch( ParseLine( line, section, key, val ) )
		{
		case LINE_EMPTY:
		case LINE_COMMENT:
			break;

		case LINE_SECTION:
			current = CreateSection( section );
			break;

		case LINE_VALUE:
			k = CreateKey( current, key, &index );
			if( !k )
			{
				glbs.DPrintf( "Configurator::ParseData: Could not create key:\n" );
				goto __error;
			}
			v.m_bNeedWrite = false;
			v.value = val;
			k->value.AddObjectAt( index + 1, v );
			break;

		case LINE_ERROR:
			glbs.DPrintf( "Configurator::ParseData: syntax error:\n" );

__error:
			glbs.DPrintf( "(%s, %d):", m_filename.c_str(), lineno );
			glbs.DPrintf( "%s\n", line );
			break;

		default:
			break;
		}
	}
}


int Configurator::ParseLine( char *line, char *section, char *key, char *value )
{
	size_t len;
	int result;

	len = strlen( line );

	if( len < 1 )
	{
		result = LINE_EMPTY;
	}
	else if( line[ 0 ] == ';' )
	{
		result = LINE_COMMENT;
	}
	else if( line[ 0 ] == '[' && line[ len - 1 ] == ']' )
	{
		sscanf( line, "[%[^]]", section );
		strcpy( section, strstrip( section ) );
		strlwc( section );

		result = LINE_SECTION;
	}
	else if( ( sscanf( line, "%[^=] = \"%[^\"]\"", key, value ) ) == 2
		  || ( sscanf( line, "%[^=] = %[^;]", key, value ) ) == 2 )
	{
		strcpy( key, strstrip( key ) );
		strlwc( key );
		strcpy( value, strstrip( value ) );

		// sscanf cannot handle empty "" values
		if( !strcmp( value, "\"\"" ) )
		{
			value = "";
		}

		result = LINE_VALUE;
	}
	else if( ( sscanf( line, "%[^=] = %[;]", key, value ) ) == 2
		  || ( sscanf( line, "%[^=] %[=]", key, value ) ) == 2 )
	{
		strcpy( key, strstrip( key ) );
		strlwc( key );
		value[ 0 ] = 0;

		result = LINE_VALUE;
	}
	else
	{
		result = LINE_ERROR;
	}

	return result;
}

str Configurator::GetValue( const char *section, const char *key, str defaultValue, int index )
{
	configKey_t *k = GetKey( section, key, index );
	configValue_t val;

	if( !k )
	{
		return defaultValue;
	}

	if( index < 0 ) index = 0;

	if( k->m_bNeedWrite )
	{
		val.m_bNeedWrite = true;
		val.value = defaultValue;
		k->value.AddObjectAt( index + 1, val );
	}

	return k->value[ index ].value;
}

configKey_t *Configurator::GetKey( const char *section, const char *key, int index )
{
	configSection_t *s;
	configKey_t *k;

	bool bArray = index >= 0;
	if( index < 0 ) index = 0;

	s = FindSection( section );
	if( !s )
	{
		if( m_bNoWrite )
		{
			return NULL;
		}

		s = CreateSection( section );
		s->m_bNeedWrite = true;
		m_bNeedWrite = true;
	}

	k = FindKey( s, key );
	if( !k )
	{
		if( m_bNoWrite )
		{
			return NULL;
		}

		str keyname;
		if( bArray )
		{
			keyname = str( key ) + "[" + str( index ) + "]";
		}
		else
		{
			keyname = key;
		}
		k = CreateKey( s, keyname, NULL );
		k->m_bNeedWrite = true;
		m_bNeedWrite = true;
	}
	else if( index >= k->value.NumObjects() )
	{
		k->m_bNeedWrite = true;
		m_bNeedWrite = true;
	}

	return k;
}

//======================================
// Configurator::GetString
//======================================
str Configurator::GetString( const char *section, const char *key, str defaultValue, int index )
{
	str value = GetValue( section, key, defaultValue, index );
	return value;
}

//======================================
// Configurator::GetInteger
//======================================
int Configurator::GetInteger( const char *section, const char *key, int defaultValue, int index )
{
	str value = GetValue( section, key, defaultValue, index );
	return atoi( value.c_str() );
}

//======================================
// Configurator::GetFloat
//======================================
float Configurator::GetFloat( const char *section, const char *key, float defaultValue, int index )
{
	str value = GetValue( section, key, defaultValue, index );
	return atof( value.c_str() );
}

//======================================
// Configurator::SetString
//======================================
void Configurator::SetString( const char *section, const char *key, str value, int index )
{
	configKey_t *k = GetKey( section, key, index );
	configValue_t v;

	if( !k )
		return;

	if( index < 0 ) index = 0;

	if( index >= k->value.NumObjects() || k->value[ index ].value != value )
	{
		m_bNeedWrite = true;
		v.m_bNeedWrite = true;
		v.value = value;
		k->m_bNeedWrite = true;
		k->value.AddObjectAt( index + 1, v );
	}
}

//======================================
// Configurator::SetInteger
//======================================
void Configurator::SetInteger( const char *section, const char *key, int value, int index )
{
	return SetString( section, key, value, index );
}

//======================================
// Configurator::SetFloat
//======================================
void Configurator::SetFloat( const char *section, const char *key, float value, int index )
{
	return SetString( section, key, value, index );
}

//======================================
// Configurator::SetWrite
//======================================
void Configurator::SetWrite( bool bWrite )
{
	m_bNoWrite = !bWrite;
}

//======================================
// Configurator::Parse
//======================================
void Configurator::Parse( const char *filename )
{
	FILE *file;
	size_t size;
	char *buffer;

	m_bNoWrite = false;
	m_bNeedWrite = false;

	this->m_filename = filename;

	file = fopen( filename, "r" );
	if( !file )
	{
		return;
	}

	fseek( file, 0, SEEK_END );
	size = ftell( file );
	rewind( file );

	buffer = new char[ size + 1 ];
	size = fread( buffer, 1, size, file );
	buffer[ size ] = 0;

	fclose( file );

	ParseData( buffer, size );
}

//======================================
// Configurator::Close
//======================================
void Configurator::Close( void )
{
	FILE *file;
	char *buffer;
	int i;
	size_t size;

	if( !m_filename.length() || !m_bNeedWrite )
	{
		goto _cleanup;
	}

	file = fopen( m_filename.c_str(), "r+" );
	if( !file )
	{
		file = fopen( m_filename.c_str(), "w+" );
		if( !file )
		{
			return;
		}
		file = freopen( m_filename.c_str(), "r+", file );
		if( !file )
		{
			return;
		}
	}

	fseek( file, 0, SEEK_END );
	size = ftell( file );
	rewind( file );

	buffer = new char[ size + 1 ];
	size = fread( buffer, 1, size, file );
	buffer[ size ] = 0;

	glbs.DPrintf( "Configurator::Close: Writing data to %s...\n", m_filename.c_str() );
	WriteData( &buffer, &size );
	glbs.DPrintf( "Configurator::Close: Done.\n" );
	file = freopen( m_filename.c_str(), "w+", file );
	fwrite( buffer, size, 1, file );
	fclose( file );

	m_filename = "";

	delete[] buffer;

_cleanup:
	for( i = m_reverseSections.NumObjects(); i > 0; i-- )
	{
		RemoveSection( m_reverseSections.ObjectAt( i ) );
	}
}

void test_config( void )
{
	Configurator config( "test.ini" );
	str value;

	value = config.GetString( "section1", "testvar1", "100" );
	glbs.DPrintf( "value = %s\n", value.c_str() );
	value = config.GetString( "section1", "testarray1", "100", 0 );
	glbs.DPrintf( "array1 = %s\n", value.c_str() );
	value = config.GetString( "section1", "testarray1", "100", 1 );
	glbs.DPrintf( "array2 = %s\n", value.c_str() );
	value = config.GetString( "section1", "testarray1", "100", 2 );
	glbs.DPrintf( "array3 = %s\n", value.c_str() );

	value = config.GetString( "section1", "testforcewrite", "500" );
	glbs.DPrintf( "testforcewrite = %s\n", value.c_str() );

	value = config.GetString( "section2", "testforcewrite2", "500" );
	glbs.DPrintf( "testforcewrite2 = %s\n", value.c_str() );

	//for( int i = 0; i < 2048; i++ )
	//{
	//	value = config.GetString( "section1", "testforcewrite5", "2048", i );
	//}

	config.SetString( "section1", "testforcewrite5", "newstring", 2048 );

	config.Close();
}
