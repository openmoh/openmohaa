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

// main.cpp : TestUtils Entry point.

#include <glb_local.h>
#include <scriptmaster.h>
#include <level.h>
#include <console.h>
#include <baseimp.h>
#include <dummy_base.h>
#include <world.h>
#include <g_spawn.h>
#include <stdio.h>
#include "conevent.h"
#include "ubersdk.h"
#include "mem_blockalloc.h"
#include "configurator.h"

extern size_t totalmemallocated;
extern unsigned int numclassesallocated;

baseImport_t bi;
char **commandLine;
int numCmdLine;

ConsoleInput *g_console;

cvar_t *developer;
cvar_t *g_scriptcheck;
cvar_t *g_showopcodes;
cvar_t *precache;
cvar_t *sv_scriptfiles;
cvar_t c_dummy;

MEM_BlockAlloc< int, char[ 256 ] > g_memtest;

#ifndef WIN32

static struct termios old, n;

void initTermios( int echo )
{
	tcgetattr( 0, &old );
	n = old;
	n.c_lflag &= ~ICANON;
	n.c_lflag &= echo ? ECHO : ~ECHO;
	tcsetattr( 0, TCSANOW, &n );
}

void resetTermios( void )
{
	tcsetattr( 0, TCSANOW, &old );
}

char getch_( int echo )
{
	char ch;
	initTermios( echo );
	ch = getchar();
	resetTermios();
	return ch;
}

char getch( void )
{
	return getch_( 0 );
}

char getche( void )
{
	return getch_( 1 );
}

#endif

static const char *scriptData = "main:\n"
								"local.k = 0\n"
								"loop:\n"
								/*"local.test = 1\n"
								"for ( local.i = 0; local.i <= 100; local.i++ )"
								"{\n"
								"print \"hello world\\n\"\n"
								"}\n"*/
								//"print \"fuck\\n\"\n"
								"local.k++\n"
								"print( \"value: \" + local.k + \"\\n\" )\n"
								//"wait 0.005\n"
								//"goto \"loop\"\n"
								/*"dead_code:\n"
								"local.test = 2\n"
								"end\n"
								"alive_code:\n"
								"local.test = 3\n"
								"end\n"*/
								;

void CompilerTest( const char *filename )
{
	// Preload the script (to show the "Execute VM" message properly)
	Director.GetScript( filename );

	printf( "\n--------------------------\n"
			"Execute VM"
			"\n--------------------------\n" );

	Event fast;

	try
	{
		fast.AddFloat( 1 );
		fast.AddString( "test" );
		fast.AddInteger( 2 );

		try
		{
			Director.ExecuteThread( filename, "", fast );
		}
		catch( ScriptException& exc )
		{
			printf( "%s\n", exc.string.c_str() );
			return;
		}
	}
	catch( ScriptException& exc )
	{
		printf( "^~^~^ Script Error : %s\n", exc.string.c_str() );
	}

	printf( "\n--------------------------\n"
			"Done. VM returned '" );

	ScriptVariable *ret = &fast.GetValue( -1 );

	ret->PrintValue();

	if( ret->GetType() == VARIABLE_POINTER )
	{
		printf( " (waiting ?)\n" );
	}

	printf( "'\n--------------------------\n" );
}

void Error( int errortype, const char *format, ... )
{
	char buffer[ 4000 ];
	va_list va;

	va_start( va, format );
	vsprintf( buffer, format, va );
	va_end( va );

	throw( buffer );
}

void U_Init()
{
	c_dummy.integer = 1;

	g_showopcodes = &c_dummy;
	developer = &c_dummy;

	level.m_LoopProtection = false;
	world = new World;

	Director.Init();
	Director.Reset( false );

	Director.maxTime = 50;
}

void U_RunFrame( double frametime )
{
	level.setFrametime( frametime );
	level.setTime( clock() );
	Director.SetTime( level.inttime );

	Director.m_bAllowContextSwitch = true;

	L_ProcessPendingEvents();
	Director.ExecuteRunning();
}

void U_Shutdown()
{
	bi.Printf( "==== SDK Shutdown ====\n" );
	bi.Printf( "Removing %d byte(s)\n", totalmemallocated );

	Director.Reset( false );

	L_ClearEventList();

	while( g_spawnlist.NumObjects() )
	{
		Container< SafePtr< Listener > > m_container = g_spawnlist;

		g_spawnlist.FreeObjectList();

		for( int i = m_container.NumObjects(); i > 0; i-- )
		{
			Listener *listener = m_container.ObjectAt( i );

			if( listener ) {
				delete listener;
			}
		}
	}

	world->FreeTargetList();

	bi.Printf( "%d byte(s) remaining\n", totalmemallocated );
	bi.Printf( "Press any key to close the program.\n" );

	_getch();

	exit( 0 );
}

void ProcessPrintf( void );

bool udk_shutdown = false;

void Sys_Idle()
{
	double frametime = 0;
	//ULONGLONG longvalue;
	double lastframetime;
	double tick = 0;

	/*GetSystemTimeAsFileTime( ( LPFILETIME )&longvalue );

	tick = ( double )longvalue / 10000.0;
	lastframetime = tick;*/

	bi.Printf( "Sys_Idle()\n" );

	tick = ( double )clock() / 1000.0;
	lastframetime = tick;

	while( 1 )
	{
		//GetSystemTimeAsFileTime( ( LPFILETIME )&longvalue );

		//tick = ( double )longvalue / 10000.0;
		tick = ( double )clock() / 1000.0;

		frametime = ( tick - lastframetime );
		lastframetime = tick;

		//Input_Idle();

		if( udk_shutdown )
		{
			U_Shutdown();
			break;
		}

		U_RunFrame( frametime );
		ProcessPrintf();

#ifdef WIN32
		Sleep( 50 );
#else
		timespec tm;
		timespec rem;
		tm.tv_nsec = 500000;
		nanosleep( &tm, &rem );
#endif
	}
}

str binPath;

static char printf_buffer[ 400000 ];
static int printf_buffer_pos = 0;
static int printf_instances = 0;

void ProcessPrintf( void )
{
	if( printf_instances )
	{
		printf( printf_buffer );
	}

	printf_buffer_pos = 0;
	printf_instances = 0;
}

int Printf( const char *format, ... )
{
	va_list va;

	if( printf_instances > 100 )
	{
		ProcessPrintf();
	}

	va_start( va, format );
	printf_buffer_pos += vsprintf( &printf_buffer[ printf_buffer_pos ], format, va );
	va_end( va );

	printf_instances++;

	return printf_buffer_pos;
}

int Milliseconds()
{
	return clock();
}

double MillisecondsDbl()
{
	/*ULONGLONG longvalue;

	GetSystemTimeAsFileTime( ( LPFILETIME )&longvalue );

	return ( double )longvalue / 10000.0;*/
	return ( double )clock();
}

int main( int argc, char **argv )
{
	/*char ownPath[ MAX_PATH ];
	char *p;

	GetModuleFileName( GetModuleHandle( NULL ), ownPath, ( sizeof( ownPath ) ) );

	p = &ownPath[ strlen( ownPath ) - 1 ];

	while( *p != '\0' && *p != '\\' )
	{
		p--;
	}

	*p = '\0';*/

	binPath = argv[ 0 ];

	bi.Printf		= BI_Printf;
	bi.DPrintf		= BI_DPrintf;
	bi.Error		= BI_Error;
	bi.Free			= BI_Free;
	bi.Malloc		= BI_Malloc;

	bi.Milliseconds		= BI_Milliseconds;
	bi.MillisecondsDbl	= MillisecondsDbl;

	bi.FS_FreeFile				= FS_FreeFile;
	bi.FS_ReadFile				= FS_ReadFile2;
	bi.FS_CanonicalFilename		= FS_CanonicalFilename;

	g_console = new ConsoleInput( &g_consoleEvent );
	g_console->InitInput();

	bi.Printf( "--- OpenMOHAA Test utility ---\n" );
	bi.Printf( "Small utility with MorpheusScript and a basic console.\n" );

	numCmdLine = argc;
	commandLine = argv;

	unsigned int start = clock();

	L_InitEvents();

	U_Init();

	bi.Printf( "%f seconds elasped\n", ( float )( clock() - start ) / 1000.0f );

	Sys_Idle();
}
