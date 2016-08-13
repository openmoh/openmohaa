#include <glb_local.h>
#include <scriptmaster.h>
#include <baseimp.h>
#include <dummy_base.h>
#include <world.h>
#include <g_spawn.h>
#include <Windows.h>

void BaseInit( void )
{
	developer = new cvar_t;
	g_scriptcheck = new cvar_t;
	g_showopcodes = new cvar_t;
	precache = new cvar_t;
	sv_scriptfiles = new cvar_t;

	memset( developer, 0, sizeof( cvar_t ) );
	memset( g_scriptcheck, 0, sizeof( cvar_t ) );
	memset( g_showopcodes, 0, sizeof( cvar_t ) );
	memset( precache, 0, sizeof( cvar_t ) );
	memset( sv_scriptfiles, 0, sizeof( cvar_t ) );

	Com_FillBaseImports();

	Swap_Init();
	Z_InitMemory();

	Cmd_Init();
	Cvar_Init();

	FS_InitFilesystem2();

	L_InitEvents();

#ifndef NO_SCRIPTENGINE
	level.m_LoopProtection = false;
	world = new World;

	Director.Init();
	Director.Reset( false );

	Director.maxTime = 50;
#endif
}

void BaseRunFrame( double frametime )
{
	level.setFrametime( frametime );
	level.setTime( clock() );

#ifndef NO_SCRIPTENGINE
	Director.SetTime( level.inttime );
	Director.m_bAllowContextSwitch = true;
#endif

	L_ProcessPendingEvents();
#ifndef NO_SCRIPTENGINE
	Director.ExecuteRunning();
#endif
}

void BaseIdle( void )
{
	double frametime = 0;
	double lastframetime;
	double tick = 0;

	bi.Printf( "BaseIdle()\n" );

	tick = ( double )clock();// / 1000.0;
	lastframetime = tick;

	while( 1 )
	{
		tick = ( double )clock();// / 1000.0;

		frametime = ( tick - lastframetime );
		lastframetime = tick;

		BaseRunFrame( frametime );

		Sleep( 50 );
	}
}

int MainEvent( const Container< Event * >& conev );

int main( int argc, char **argv )
{
	Container< Event * > conev;
	Event *ev;
	int i;
	char *arg;

	BaseInit();

	// copy the argument list
	for( i = 0; i < argc; i++ )
	{
		arg = argv[ i ];

		if( strlen( arg ) <= 1 ) {
			continue;
		}

		if( *arg != '/' && *arg != '-' && *arg != '+' ) {
			continue;
		}

		ev = new Event( arg + 1 );

		for( i++; i < argc; i++ )
		{
			arg = argv[ i ];
			if( *arg == '/' || *arg == '-' || *arg == '+' ) {
				i--;
				break;
			}

			ev->AddString( argv[ i ] );
		}

		conev.AddObject( ev );
	}

	MainEvent( conev );

	conev.FreeObjectList();

	L_ShutdownEvents();

	Com_Shutdown();
	FS_Shutdown( qtrue );
}
