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

#include "glb_local.h"

//=========================================
// QFXLog class implementation
//-----------------------------------------
// Print messages to log file
//=========================================

#define QFX_LOG_FILENAME	QFX_LIBRARY_TITLE ".log"
#define QFX_LOG_MAXSTRING	8192

QFXLog :: QFXLog() : m_pszLogString( NULL ), m_pFile( NULL )
{
	m_pszLogString = new char[ QFX_LOG_MAXSTRING ];
	assert( m_pszLogString != NULL );

	/*char timeBuf[64];
	time_t t;

	if ( fopen_s( &m_pFile, QFX_LOG_FILENAME, "w" ) )
		return;

	m_pszLogString = new char[QFX_LOG_MAXSTRING];
	assert( m_pszLogString != NULL );
	
	memset( &t, 0, sizeof(t) );
	time( &t );
	memset( timeBuf, 0, sizeof(timeBuf) );
	ctime_s( timeBuf, sizeof(timeBuf), &t );

	fprintf( m_pFile, "==================================================================\n" );
	fprintf( m_pFile, " " QFX_LIBRARY_TITLE " initialized at %s", timeBuf );
	fprintf( m_pFile, "==================================================================\n" );

	fprintf( m_pFile, "\n" );
	fflush( m_pFile );*/
}

QFXLog :: ~QFXLog()
{
	/*if ( m_pszLogString ) {
		delete [] m_pszLogString;
		m_pszLogString = NULL;
	}

	if ( m_pFile ) {
		char timeBuf[64];
		time_t t;

		memset( &t, 0, sizeof(t) );
		time( &t );
		memset( timeBuf, 0, sizeof(timeBuf) );
		ctime_s( timeBuf, sizeof(timeBuf), &t );

		fprintf( m_pFile, "\n==================================================================\n" );
		fprintf( m_pFile, " " QFX_LIBRARY_TITLE " shutdown at %s", timeBuf );
		fprintf( m_pFile, "==================================================================\n" );

		fclose( m_pFile );
		m_pFile = NULL;
	}*/
}

void QFXLog :: Printf( const char *fmt, ... )
{
	va_list argptr;

	//if ( !m_pFile || !m_pszLogString )
	//	return;

	if( !m_pszLogString )
		return;

	va_start( argptr, fmt );
	_vsnprintf_s( m_pszLogString, QFX_LOG_MAXSTRING, QFX_LOG_MAXSTRING-1, fmt, argptr );
	va_end( argptr );

	cgi.Printf( m_pszLogString );

	//fputs( m_pszLogString, m_pFile );
	//fflush( m_pFile );
}

void QFXLog :: Error( const char *fmt, ... )
{
	va_list argptr;

	if ( !m_pFile || !m_pszLogString )
		return;

	va_start( argptr, fmt );
	_vsnprintf_s( m_pszLogString, QFX_LOG_MAXSTRING, QFX_LOG_MAXSTRING-1, fmt, argptr );
	va_end( argptr );

	cgi.Printf( m_pszLogString );

	//fprintf( m_pFile, "ERROR: %s", m_pszLogString );
	//fflush( m_pFile );
}

void QFXLog :: Warning( const char *fmt, ... )
{
	va_list argptr;

	if ( !m_pFile || !m_pszLogString )
		return;

	va_start( argptr, fmt );
	_vsnprintf_s( m_pszLogString, QFX_LOG_MAXSTRING, QFX_LOG_MAXSTRING-1, fmt, argptr );
	va_end( argptr );

	cgi.Printf( m_pszLogString );

	//fprintf( m_pFile, "WARNING: %s", m_pszLogString );
	//fflush( m_pFile );
}
