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

// console.h: TU Console.

#include "console.h"
#include <Windows.h>

CLASS_DECLARATION( Listener, ConsoleInput, NULL )
{
	{ NULL, NULL }
};


#ifdef WIN32

HANDLE WINAPI WIN_CreateThread
	(
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	SIZE_T dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress,
	LPVOID lpParameter,
	DWORD dwCreationFlags,
	LPDWORD lpThreadId
	)

{
	return CreateThread( lpThreadAttributes, dwStackSize, lpStartAddress, lpParameter, dwCreationFlags, lpThreadId );
}


DWORD WINAPI InputThread
	(
	LPVOID lpParameter
	)

{
	ConsoleInput *console = ( ConsoleInput * )lpParameter;
	console->Input_Idle();
	return 0;
}

#else

void *InputThread
	(
	void *arg
	)

{
	ConsoleInput *console = ( ConsoleInput * )lpParameter;
	console->Input_Idle();
	return NULL;
}

#endif

ConsoleInput::ConsoleInput()
{
}

ConsoleInput::ConsoleInput( Listener *consoleEvent )
{
	m_pConsoleEvent = consoleEvent;
}

void ConsoleInput::InitInput( void )
{
#ifdef WIN32
	WIN_CreateThread( NULL, 0, InputThread, ( void * )this, 0, NULL );
#else
	pthread_t thread;

	pthread_create( &thread, NULL, ( void * ( *)( void * ) )&InputThread, ( void * )this );
#endif
}

bool ConsoleInput::CheckEvent
	(
	Event *ev
	)

{
	EventDef *def = m_pConsoleEvent->classinfo()->GetDef( ev->eventnum );

	if( !def )
	{
		return false;
	}

	if( !( def->flags & EV_CONSOLE ) )
	{
		return false;
	}

	return true;
}

void ConsoleInput::PrintHelp
	(
	void
	)

{
	ResponseDef< Class > *r;

	bi.Printf( "Command list :\n\n" );

	r = m_pConsoleEvent->classinfo()->responses;
	while( r->event )
	{
		r->def->PrintEventDocumentation( stdout, false );
		r++;
	}
}

bool ConsoleInput::Execute
	(
	const char *data
	)

{
	char *buffer;
	const char *com_token;
	str sCommand;

	buffer = ( char * )malloc( strlen( data ) + 1 );
	strcpy( buffer, data );

	com_token = COM_Parse( &buffer );

	if( !com_token )
	{
		Com_Printf( "Enter a valid command.\n" );
		free( buffer );
		return false;
	}

	sCommand = com_token;

	if( Event::FindEventNum( sCommand ) )
	{
		Event *ev = new Event( sCommand );

		while( 1 )
		{
			com_token = COM_Parse( &buffer );

			if( !com_token[ 0 ] )
				break;

			ev->AddString( com_token );
		}

		if( CheckEvent( ev ) )
		{
			m_pConsoleEvent->ProcessEvent( ev );
		}
		else
		{
			Com_Printf( "Command '%s' not available from console.\n", sCommand.c_str() );
			free( buffer );
			return false;
		}
	}
	else
	{
		Com_Printf( "Command '%s' is not valid. Type help for more info.\n", sCommand.c_str() );
		free( buffer );
		return false;
	}

	free( buffer );
	return true;
}

bool ConsoleInput::Execute
	(
	int argc,
	char *argv[]
	)

{
	size_t iLength = 0;
	int i;
	char *data;

	for( i = 1; i < argc; i++ )
	{
		iLength += strlen( argv[ i ] ) + 1;
	}

	if( iLength <= 1 )
	{
		return false;
	}

	data = ( char * )malloc( iLength + 1 );

	memset( data, 0, iLength + 1 );

	for( i = 1; i < argc; i++ )
	{
		strcat( data, argv[ i ] );
		strcat( data, " " );
	}

	bool bResult = Execute( data );
	free( data );

	return bResult;
}

void ConsoleInput::Input_Idle
	(
	void
	)

{
	char *szBuffer = ( char * )malloc( 255 );
	int i;

	while( 1 )
	{
		i = 0;

		/*while( !feof( stdin ) )
		{
			szBuffer[ i ] = getchar();

			if( szBuffer[ i ] == '\r' || szBuffer[ i ] == '\n' )
			{
				szBuffer[ i ] = 0;
				break;
			}

			if( szBuffer[ i ] == '\b' )
			{
				szBuffer[ i ] = 0;
				i--;
			}
			else
			{
				i++;
			}
		}
		*/

		fgets( szBuffer, 255, stdin );
		OemToCharBuff( szBuffer, szBuffer, 255 );

		Execute( szBuffer );
	}
}
