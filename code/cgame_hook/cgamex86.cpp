#include "glb_local.h"
#include "cgame/cg_hook.h"
#include "cgame/cg_hud.h"
#include "cgame/cg_servercmds.h"
#include "cgame/cg_viewmodelanim.h"
#include "cgame/cl_sound.h"
#include <iostream>
#include <fstream>
#include "archive.h"
#include "script/cplayer.h"
#include "level.h"
#include "script/clientgamecommand.h"
#include "ogl/opengl_api.h"
#include "cgamex86.h"

#include "renderer/qfx_library.h"
#include "renderer/qfx_log.h"
#include "renderer/qfx_settings.h"
#include "renderer/qfx_opengl.h"
#include "renderer/qfx_renderer.h"
#include "renderer/qfx_shader.h"
#include "renderer/qfx_glprogs.h"

#include <world.h>
#include <scriptmaster.h>

HMODULE hmod;
uintptr_t dwCGameBase = 0;

qboolean inited = false;

CG_CONFIGSTRING CG_ConfigString = (CG_CONFIGSTRING)0x30023FA7;
SPRINTF2 sprintf2 = (SPRINTF2)0x3005D214;
Q_STRCMPI Q_strcmpi = (Q_STRCMPI)0x3005DC34;

typedef clientGameExport_t* (*pGetCGameAPI_spec)( void );
pGetCGameAPI_spec pGetCGameAPI;

pfnCG_CastFootShadow CG_CastFootShadow = ( pfnCG_CastFootShadow )0x30029AA4;

CG_PlaySound_f CG_PlaySound = (CG_PlaySound_f)0x30015E0C;
CG_Trace_t CG_Trace = (CG_Trace_t)0x30031BCB;

HINSTANCE hSystem86 = NULL;

typedef void *( *pSystemMalloc_spec )( int size );
extern pSystemMalloc_spec pSystemMalloc;

typedef void( *pSystemFree_spec )( void *ptr );
extern pSystemFree_spec pSystemFree;

pSystemMalloc_spec pSystemMalloc = NULL;
pSystemFree_spec pSystemFree = NULL;

clientGameExport_t	cge;
clientGameImport_t	cgi;
refImport_t ri;
refExport_t re;

cg_t   *cg;
cgs_t *cgs;

centity_t *cg_entities;

centity_t *cent = NULL;

cvar_t *developer;
cvar_t *timescale;

cvar_t *cg_debugview;
cvar_t *cg_hud;
cvar_t *cl_run;
cvar_t *fs_basepath;
cvar_t *r_anaglyph;
cvar_t *ui_hud;
cvar_t *vm_offset_air_up;
cvar_t *vm_offset_air_side;
cvar_t *vm_offset_air_front;
cvar_t *vm_offset_crouch_side;
cvar_t *vm_offset_vel_up;
cvar_t *vm_offset_vel_side;
cvar_t *vm_offset_vel_front;
cvar_t *archive_save;

cvar_t *s_volume;

cvar_t *cg_showopcodes;
cvar_t *cg_scriptcheck;
cvar_t *cl_scriptfiles;

qboolean reborn;

float *viewAngles = (float*)0x01100B14;

int RenderedScene[5]; 

int cg_frametime;

qboolean fUnloading = false;

extern qboolean render_inited;
extern void R_Init();

regCvar_t	cvars[] = {
	{ &developer,				"developer",				"0",					CVAR_ARCHIVE		},
	{ &cg_debugview,			"cg_debugview",				"0",					CVAR_ARCHIVE		},
	{ &cg_hud,					"cg_hud",					"1",					0					},
	{ &cl_run,					"cl_run",					"1",					CVAR_ARCHIVE		},
	{ &fs_basepath,				"fs_basepath",				"",						CVAR_ARCHIVE		},
	{ &r_anaglyph,				"r_anaglyph",				"0",					CVAR_ARCHIVE		},
	{ &ui_hud,					"ui_hud",					"1",					0					},
	{ &vm_offset_air_up,		"vm_offset_air_up",			"-2.0",					CVAR_ARCHIVE		},
	{ &vm_offset_air_side,		"vm_offset_air_side",		"0",					CVAR_ARCHIVE		},
	{ &vm_offset_air_front,		"vm_offset_air_front",		"0",					CVAR_ARCHIVE		},
	{ &vm_offset_crouch_side,	"vm_offset_crouch_side",	"1.5",					CVAR_ARCHIVE		},
	{ &vm_offset_vel_up,		"vm_offset_vel_up",			"0",					CVAR_ARCHIVE		},
	{ &vm_offset_vel_side,		"vm_offset_vel_side",		"0",					CVAR_ARCHIVE		},
	{ &vm_offset_vel_front,		"vm_offset_vel_front",		"0",					CVAR_ARCHIVE		},
	{ &archive_save,			"archive_save",				"0",					CVAR_ARCHIVE		},
	{ &s_volume,				"s_volume",					"0.9",					CVAR_ARCHIVE		},
	{ &cg_showopcodes,			"cg_showopcodes",			"0",					CVAR_ARCHIVE		},
	{ &cg_scriptcheck,			"cg_scriptcheck",			"0",					CVAR_ARCHIVE		},
	{ &cl_scriptfiles,			"cl_scriptfiles",			"0",					CVAR_ARCHIVE		},
	{ NULL,						NULL,						NULL,					NULL	},
};

//===================================================================

void QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[ 1024 ];

	va_start( argptr, error );
	vsprintf( text, error, argptr );
	va_end( argptr );

	cgi.Error( ( errorParm_t )level, "%s", text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[ 1024 ];

	va_start( argptr, msg );
	vsprintf( text, msg, argptr );
	va_end( argptr );

	cgi.Printf( "%s", text );
}

void *( *CL_Malloc )( int size );
void ( *CL_Free )( void *ptr );

void* CG_Malloc( int size )
{
	return CL_Malloc( size );
}

void CG_Free( void *ptr )
{
	int zoneId = *( int * )( ( unsigned char * )ptr - 4 );
	int size = ( int )( ( unsigned char * )ptr - 16 );

	assert( ptr );

	if( zoneId == 0xC057 )
		return;

	assert( zoneId == 0x2015 );
	if( zoneId != 0x2015 )
		return;

	assert( *( unsigned int * )( *( unsigned int * )size + size - 4 ) == 0x2015 );
	if( *( unsigned int * )( *( unsigned int * )size + size - 4 ) != 0x2015 )
		return;

	CL_Free( ptr );
}


void dummy_Printf( char * format, ... )
{

}

centity_t * CG_GetEntity( int ent_num )
{
	entityState_t * ce = NULL;
	int i;

	for( i = 0; i < cg->activeSnapshots[ 0 ].numEntities; i++ )
	{
		if( cg->activeSnapshots[ 0 ].entities[ i ].number == ent_num ) {
			ce = &cg->activeSnapshots[ 0 ].entities[ i ];
		}
	}

	return ( centity_t * )ce;
}

void debug_save()
{
	Archiver arc;

	if( !arc.Create( "test.reborn" ) ) {
		cgi.Printf( "error saving to file\n" );
		return;
	}

	//Class::PrepareArchive( arc );

	arc.Close();

	cgi.Printf( "successfully saved\n" );
}

void debug_restore()
{
	Archiver arc;

	if( !arc.Read( "test.reborn" ) )
	{
		cgi.Printf( "error restoring from file\n" );
		return;
	}

	//Class::PrepareArchive( arc );

	arc.Close();

	cgi.Printf( "successfully restored\n" );
}

const char *CG_GetServerInfo()
{
	return cgs->gameState.stringData
		+ cgs->gameState.stringOffsets[ CS_SERVERINFO ];
}

const char *CG_GetServerVariable( const char *variable )
{
	return Info_ValueForKey( CG_GetServerInfo(), variable );
}

qboolean script_loaded = false;

void CG_DrawActiveFrame( int serverTime, int frametime, stereoFrame_t stereoView, qboolean demoPlayback )
{
	if( !render_inited ) {
		R_Init();
	}

	if( frametime < 1 )
	{
		frametime = 1;
	}

	level.setFrametime( frametime );
	level.setTime( serverTime );

	//ScriptTimer::HandleTimer( ( float )frametime );

	L_ProcessPendingEvents();

	if( !script_loaded )
	{
		script_loaded = true;
		ClientGameCommandManager::Initialize();
		//ClientScript::AutoLoadScripts();
	}

	if( archive_save->integer == 1 )
	{
		cgi.Cvar_Set( "archive_save", "0" );
		debug_save();
	}
	else if( archive_save->integer == 2 )
	{
		cgi.Cvar_Set( "archive_save", "0" );
		debug_restore();
	}

	cg_frametime = frametime;

	R_ProcessEffectsFade( frametime );

	CL_SoundProcessing( frametime );
	CL_ProcessCEntities();

	if( cg_debugview->integer )
	{
		if( cg->snap != NULL ) {
			cg->snap->ps.pm_flags = 0;
		}

		if( cg->nextSnap != NULL ) {
			cg->nextSnap->ps.pm_flags = 0;
		}

		cg->activeSnapshots[0].ps.pm_flags = 0;
		cg->activeSnapshots[1].ps.pm_flags = 0;

		cg->predictedPlayerState.pm_flags = 0;
	}

	cge.CG_DrawActiveFrame( serverTime, frametime, stereoView, demoPlayback );
}

void CG_Shutdown(void)
{
	//CG_ViewModelUninitialize();

	ClientGameCommandManager::CleanUp();

	cge.CG_Shutdown();

	/* Unload the original cgamex86mohaa to prevent crashes because of the
	 * replaced instructions
	 */
	//FreeLibrary( hmod );
}

void R_RenderScene( refDef_t *fd );

int R_CvarSize( void );

void CG_InitCvars( void )
{
	int		i;

	for ( i = 0 ; i < sizeof( cvars ) / sizeof( cvars[0] ) ; i++ )
	{
    	if( cvars[i].cvar == NULL ) {
    		continue;
    	}

		*cvars[i].cvar = cgi.Cvar_Get( cvars[i].name, cvars[i].value, cvars[i].flags );
	}

	for ( i = 0 ; i < R_CvarSize() ; i++ )
	{
    	if( r_cvars[i].cvar == NULL ) {
    		continue;
    	}

		*r_cvars[i].cvar = cgi.Cvar_Get( r_cvars[i].name, r_cvars[i].value, r_cvars[i].flags );
	}
}

float Anim_FrameTime( dtiki_t *tiki, int animNum )
{
	float time = cgi.Anim_Frametime( tiki, animNum );

	time /= 2.f;

	return time;
}

int GetSnapshot( int snapshotNumber, snapshot_t *snapshot )
{
	int result = cgi.GetSnapshot( snapshotNumber, snapshot );

	return result;
}

void CG_Init( clientGameImport_t *imported, int serverMessageNum, int serverCommandSequence, int clientNum ) 
{
	CL_Malloc = imported->Malloc;
	imported->Malloc = CG_Malloc;

	CL_Free = imported->Free;
	imported->Free = CG_Free;

	memcpy( &cgi, imported, sizeof( cgi ) );

	imported->R_RenderScene = R_RenderScene;
	imported->R_AddRefEntityToScene = R_AddRefEntityToScene;
	imported->R_AddRefSpriteToScene = R_AddRefSpriteToScene;
	imported->GetSnapshot = GetSnapshot;

	//cvars
	menu = cgi.Cvar_Get( "menu", "0", CVAR_ARCHIVE );
	stats = cgi.Cvar_Get( "stats", "0", CVAR_ARCHIVE );

	timescale = cgi.Cvar_Get( "timescale", "1.0", CVAR_CHEAT );

	CG_ViewInit();

	huddrawExtends = (hdExtend_t*)malloc(sizeof(hdExtend_t) * 256);
	memset(huddrawExtends, 0, sizeof(huddrawExtends) * 256);

	CG_InitCvars();

	Director.Reset();
	L_InitEvents();

	world = new World;
	player.SetTargetName( "player" );

	cge.CG_Init( imported, serverMessageNum, serverCommandSequence, clientNum );

	char buffer[ MAX_QPATH ];

	strcpy( buffer, cgs->mapName + 5 );

	COM_StripExtension( buffer, buffer, sizeof( buffer ) );

	try
	{
		Director.ExecuteThread( "clientscripts/maps/" + str( buffer ) + ".ccr", "" );
	}
	catch( ScriptException& exc )
	{
		glbs.Printf( "%s\n", exc.string.c_str() );
	}

	level.AddWaitTill( "prespawn" );

	level.Unregister( "prespawn" );
	//level.prespawned = true;

	const char *value = CG_GetServerVariable( "sv_reborn" );

	if( value != NULL && isdigit( *value ) ) {
		reborn = !!atoi( value );
	} else {
		reborn = false;
	}

	R_ProcessVision( buffer, 0.f );

	level.AddWaitTill( "spawn" );

	level.Unregister( "spawn" );

	Director.Reset( false );

	//level.spawned = true;

/*	char tmp[ MAX_STRINGCHARS ];
	int intValue;

	Archiver arc;

	if( !arc.FileExists( "main/test.reborn" ) )
	{
		arc.Save( "main/test.reborn" );
	} else {
		arc.Load( "main/test.reborn" );
	}

	if( arc.Loading() ) {
		arc.RemoveSave();
	}

	ClientObject::PrepareArchive( arc );

	arc.Dump();
	arc.Close();*/
}

qboolean CG_ConsoleCommand( void )
{
	char *cmd = cgi.Argv( 0 );
	ScriptThreadLabel *label = m_scriptCmds.findKeyValue( cmd );

	if( label != NULL )
	{
		Event event;

		for( int i = 1; i < cgi.Argc(); i++ )
		{
			event.AddString( cgi.Argv( i ) );
		}

		label->Execute( NULL, event );
	}

	return cge.CG_ConsoleCommand();
}

/*
================
GetCGameAPI

The only exported function from this module
================
*/
clientGameExport_t*	GetCGameAPI( void ) 
{

	clientGameExport_t	*exp = pGetCGameAPI();

	memcpy( &cge, exp, sizeof(cge) );

	exp->CG_Init = CG_Init;

	//exp->CG_ParseCGMessage = CG_ParseCGMessage;
	exp->CG_Draw2D = CG_Draw2D;
	exp->CG_DrawActiveFrame = CG_DrawActiveFrame;
	exp->CG_Shutdown = CG_Shutdown;
	exp->CG_ParseCGMessage = CG_ParseCGMessage2;

	exp->CG_ConsoleCommand = CG_ConsoleCommand;

	//exp->CG_Command_ProcessFile = CG_Command_ProcessFile;

	return exp;

}

void fuck_cgi_printf_crash_noob_shit( const char *fmt, ... )
{

}

BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	char path[MAX_PATH];
	int i;

	switch ( ul_reason_for_call )
	{
		case DLL_PROCESS_ATTACH:
			if( inited ) {
				return TRUE;
			}

			inited = true;

			/*GetModuleFileName( NULL, path, sizeof(path) );

			for ( i=strlen(path); i>0; i-- )
			{
				if (path[i] == '\\')
				{
					path[i+1] = 0;
					break;
				}
			}*/

			hmod = LoadLibrary( "main\\cgamex86mohaa.dll" );

			if ( !hmod )
			{
				printf( "LoadLibrary failed: \"0x%lX\"\n", GetLastError() );
				exit( GetLastError() );
			}

			pGetCGameAPI = (pGetCGameAPI_spec)GetProcAddress( hmod, "GetCGameAPI" );

			dwCGameBase = PtrToUlong( hmod );

			uid = ( uidef_t * )0xBF3B78; 
			keys = ( qkey_t * )0x12F4B20;
			cg = ( cg_t * )( dwCGameBase + 0x2B7DC0 );
			cgs = ( cgs_t * )( dwCGameBase + 0x2A5460 );
			cg_entities = ( centity_t * )( dwCGameBase + 0x3FDA00 );

			/*RenderedScene[0] = (int *)0x01313938;
			RenderedScene[1] = (int *)0x01313940;
			RenderedScene[2] = (int *)0x01313934;
			RenderedScene[3] = (int *)0x01313950;
			RenderedScene[4] = (int *)0x0131394C;*/

			hSystem86 = LoadLibraryA( "system86.dll" );

			pSystemMalloc = (pSystemMalloc_spec)GetProcAddress( hSystem86, "MemoryMalloc" );
			pSystemFree = ( pSystemFree_spec )GetProcAddress( hSystem86, "MemoryFree" );

			CG_HookClientGame();

		break;

		case DLL_PROCESS_DETACH:
			fUnloading = true;
			// Remove the OpenGL hooks when disconnecting/changing maps
			R_Shutdown();
			FreeLibrary( hmod );

		break;
	}

	return TRUE;
}

void loadDll()
{
	if ( !inited ) {
		DllMain( GetModuleHandle( 0 ), DLL_PROCESS_ATTACH, NULL );
	}
}
