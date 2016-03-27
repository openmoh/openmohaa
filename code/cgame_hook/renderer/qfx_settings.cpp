/***************************************************************************
* Copyright (C) 2012, Chain Studios.
* 
* This file is part of QeffectsGL source code.
* 
* QeffectsGL source code is free software; you can redistribute it 
* and/or modify it under the terms of the GNU General Public License 
* as published by the Free Software Foundation; either version 2 of 
* the License, or (at your option) any later version.
* 
* QeffectsGL source code is distributed in the hope that it will be 
* useful, but WITHOUT ANY WARRANTY; without even the implied 
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
* See the GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
***************************************************************************/
#include "qfx_library.h"
#include "qfx_log.h"
#include "qfx_settings.h"

//=========================================
// QFXSettings class implementation
//-----------------------------------------
// Load settings from INI file
//=========================================
QFXSettings :: QFXSettings()
{
	// get current executable name
	/*if (!GetModuleFileName( NULL, m_szExeName, sizeof(m_szExeName)-1 )) {
		m_szExeName[0] = 0;
	}

	if ( m_szExeName[1] != ':' ) {
		// not a full path, assume current directory for INI
		GetCurrentDirectory( sizeof(m_szIniFile)-1, m_szIniFile );
		strcat_s( m_szIniFile, "\\" QFX_LIBRARY_TITLE ".ini" );

	} else {
		// full path, assume its directory for INI
		strcpy_s( m_szIniFile, m_szExeName );
		size_t l = strlen( m_szIniFile );
		for ( size_t i = l - 1; i > 0; i-- ) {
			if ( m_szIniFile[i] == '\\' || m_szIniFile[i] == '/' ) {
				m_szIniFile[i] = 0;
				break;
			}
		}
		strcat_s( m_szIniFile, "\\" QFX_LIBRARY_TITLE ".ini" );
	}

	char *p1 = strrchr( m_szExeName, '\\' );
	char *p2 = strrchr( m_szExeName, '/' );
	char *p = p1 > p2 ? p1 : p2;
	if ( p )
		strncpy_s( m_szExeName, p + 1, strlen( p + 1 ) );

	QFXLog::Instance().Printf("Settings: %s\n", m_szIniFile);
	QFXLog::Instance().Printf("Module: %s\n", m_szExeName);*/
}

QFXSettings :: ~QFXSettings()
{
}

int QFXSettings :: GetInteger( const char *setting, int defaultValue )
{
	int value = GetPrivateProfileInt( m_szExeName, setting, -99999, m_szIniFile );
	if ( value == -99999 )
		return GetPrivateProfileInt( "Generic", setting, defaultValue, m_szIniFile );
	else
		return value;
}

float QFXSettings :: GetFloat( const char *setting, float defaultValue )
{
	char buffer[32];
	GetPrivateProfileString( m_szExeName, setting, NULL, buffer, sizeof(buffer)-1, m_szIniFile );
	if ( !strlen( buffer ) ) {
		GetPrivateProfileString( "Generic", setting, NULL, buffer, sizeof(buffer)-1, m_szIniFile );
		if ( !strlen( buffer ) )
			return defaultValue;
	}
	return (float)atof( buffer );
}

bool QFXSettings :: CheckEXE( const char *s )
{
	if ( !_stricmp( m_szExeName, s ) )
		return true;
	return false;
}
