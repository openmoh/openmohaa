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

// skeletor.cpp : Skeletor

#include "q_shared.h"
#include "skeletor.h"
#include "dbgheap.h"

void SKEL_Message( const char *fmt, ... )
{
	static char msg[ 32000 ];
	va_list va;

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );
	Skel_DPrintf( msg );
}

void SKEL_Warning( const char *fmt, ... )
{
	char msg[ 1024 ];
	va_list va;

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );
	Skel_DPrintf( msg );
}

void SKEL_Error( const char *fmt, ... )
{
	char msg[ 1024 ];
	va_list va;

	va_start( va, fmt );
	vsprintf( msg, fmt, va );
	va_end( va );
	Skel_DPrintf( msg );
}

int FileLength( FILE *pFile )
{
	int iPos;
	int iEnd;

	iPos = ftell( pFile );
	fseek( pFile, 0, SEEK_END );
	iEnd = ftell( pFile );
	fseek( pFile, iPos, SEEK_SET );

	return iEnd;
}

int skelChannelList_s::GetLocalFromGlobal( int globalChannel ) const
{
	if( m_chanLocalFromGlobal && globalChannel < m_numLocalFromGlobal )
	{
		return m_chanLocalFromGlobal[ globalChannel ];
	}
	else
	{
		return -1;
	}
}

int skelChannelList_s::NumChannels( void ) const
{
	return m_numChannels;
}

void skelChannelList_s::ZeroChannels()
{
	int i;

	m_numChannels = 0;
	m_chanLocalFromGlobal = ( short * )Skel_Alloc( MAX_CHANNELS * sizeof( short ) );
	m_numLocalFromGlobal = MAX_CHANNELS;

	for( i = 0; i < MAX_CHANNELS; i++ )
	{
		m_chanLocalFromGlobal[ i ] = -1;
	}
}

void skelChannelList_s::PackChannels()
{
	m_numLocalFromGlobal = MAX_CHANNELS - 1;

	if( m_chanLocalFromGlobal[ m_numLocalFromGlobal ] == -1 )
	{
		do
		{
			m_numLocalFromGlobal--;
		} while( m_chanLocalFromGlobal[ m_numLocalFromGlobal ] == -1 && m_numLocalFromGlobal >= 0 );
	}

	if( m_numLocalFromGlobal < MAX_CHANNELS )
	{
		m_numLocalFromGlobal++;
	}

	if( m_numLocalFromGlobal <= 0 )
	{
		Skel_Free( m_chanLocalFromGlobal );
		m_chanLocalFromGlobal = NULL;
		m_numLocalFromGlobal = -1;
	}
	else
	{
		short *old_array = m_chanLocalFromGlobal;
		m_chanLocalFromGlobal = ( short * )Skel_Alloc( m_numLocalFromGlobal * sizeof( m_chanLocalFromGlobal[ 0 ] ) );
		memcpy( m_chanLocalFromGlobal, old_array, m_numLocalFromGlobal * sizeof( m_chanLocalFromGlobal[ 0 ] ) );
		Skel_Free( old_array );
	}
}

void skelChannelList_s::CleanUpChannels()
{
	if( m_chanLocalFromGlobal )
	{
		Skel_Free( m_chanLocalFromGlobal );
		m_chanLocalFromGlobal = NULL;
	}

	m_numLocalFromGlobal = 0;
}

void skelChannelList_s::InitChannels()
{
	m_chanLocalFromGlobal = NULL;
	m_numLocalFromGlobal = 0;
}

int skelChannelList_s::AddChannel( int newGlobalChannelNum )
{
	short int iLocalChannel;

	if( newGlobalChannelNum == -1 )
	{
		return m_numChannels++;
	}

	iLocalChannel = GetLocalFromGlobal( newGlobalChannelNum );

	if( iLocalChannel < 0 )
	{
		iLocalChannel = m_numChannels++;
		m_chanGlobalFromLocal[ iLocalChannel ] = newGlobalChannelNum;
		m_chanLocalFromGlobal[ newGlobalChannelNum ] = iLocalChannel;
	}

	return iLocalChannel;
}

qboolean skelChannelList_s::HasChannel( ChannelNameTable *nameTable, const char *channelName ) const
{
	short int iGlobalChannel = nameTable->FindNameLookup( channelName );

	if( iGlobalChannel >= 0 )
	{
		if( m_chanLocalFromGlobal && iGlobalChannel < m_numLocalFromGlobal )
			return ( unsigned int )~m_chanLocalFromGlobal[ iGlobalChannel ] >> 31;
		else
			return qtrue;
	}
	else
	{
		return qfalse;
	}
}


const char *skelChannelList_s::ChannelName( ChannelNameTable *nameTable, int localChannelNum ) const
{
	if( localChannelNum >= 0 && localChannelNum < m_numChannels )
	{
		return nameTable->FindName( m_chanGlobalFromLocal[ localChannelNum ] );
	}
	else
	{
		return NULL;
	}
}

void Skel_ExtractFilePath( const char *path, char *dest )
{
	const char *src = path + strlen( path );

	while( src != path )
	{
		if( *( src - 1 ) == '\\' )
		{
			break;
		}

		if( *( src - 1 ) == '/' )
		{
			break;
		}

		src--;
	}

	memcpy( dest, src, src - path );
	dest[ src - path ] = 0;
}

void Skel_ExtractFileBase( const char *path, char *dest )
{
	const char *src = path + strlen( path );
	const char *dot = src;

	while( src != path )
	{
		if( *( src - 1 ) == '\\' )
		{
			break;
		}

		if( *( src - 1 ) == '/' )
		{
			break;
		}

		src--;
	}

	while( dot != src )
	{
		if( *( dot - 1 ) != '.' )
		{
			break;
		}

		dot--;
	}

	if( dot == src )
	{
		*dest = 0;
		return;
	}

	memcpy( dest, src, dot - src );
	dest[ dot - src ] = 0;
}

void Skel_ExtractFileExtension( const char *path, char *dest )
{
	const char *src = path + strlen( path );

	while( src != path )
	{
		if( *( src - 1 ) == '.' )
		{
			break;
		}

		src--;
	}

	if( src == path )
	{
		*dest = 0;
		return;
	}

	strcpy( dest, src );
}

const char *Skel_ExtractFileExtension( const char *in )
{
	static char exten[ 8 ];
	int i;

	for( i = 0; in[ i ] != 0; i++ )
	{
		if( in[ i ] == '.' )
		{
			i++;
			break;
		}
	}

	if( !in[ i ] )
	{
		return "";
	}

	strncpy( exten, &in[ i ], sizeof( exten ) );
	return exten;
}

void Skel_ExtractFileName( const char *path, char *dest )
{
	const char *src = path + strlen( path );

	while( src != path )
	{
		if( *( src - 1 ) == '\\' )
		{
			break;
		}

		if( *( src - 1 ) == '/' )
		{
			break;
		}

		src--;
	}

	strcpy( dest, src );
}
