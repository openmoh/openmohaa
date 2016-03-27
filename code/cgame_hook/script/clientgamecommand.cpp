/*
 * ClientGameCommandManager.cpp
 *
 * DESCRIPTION : Client Game Command Manager
 */

#include "clientgamecommand.h"
#include "cgame/cl_sound.h"
#include "archive.h"

uintptr_t *sndDriver = ( uintptr_t * )0x7F5D74;

typedef void( __cdecl *AIL_CLOSE_STREAM )( void *stream );
typedef void* ( __cdecl *AIL_OPEN_STREAM )( uintptr_t soundDriver, const char *pFilename, int channel );
typedef void ( __cdecl *AIL_SET_STREAM_LOOP_COUNT )( void *stream, int count );
typedef void( __cdecl *AIL_SET_STREAM_PLAYBACK_RATE )( void *stream, int rate );
typedef void( __cdecl *AIL_SET_STREAM_POSITION )( void *stream, int position );
typedef void( __cdecl *AIL_SET_STREAM_VOLUME )( void *stream, int volume );
typedef void ( __cdecl *AIL_START_STREAM )( void *stream );
typedef int( __cdecl *AIL_STREAM_PLAYBACK_RATE )( void *stream );
typedef int( __cdecl *AIL_STREAM_POSITION )( void *stream );
typedef int ( __cdecl *AIL_STREAM_STATUS )( void *stream );

typedef char * ( __cdecl *GETFILEPATH )( const char *game_path, const char *path, const char *filename );

AIL_CLOSE_STREAM AIL_close_stream = ( AIL_CLOSE_STREAM )0x00486C80;
AIL_OPEN_STREAM	AIL_open_stream = ( AIL_OPEN_STREAM )0x00486C20;
AIL_SET_STREAM_LOOP_COUNT AIL_set_stream_loop_count = ( AIL_SET_STREAM_LOOP_COUNT )0x00487030;
AIL_SET_STREAM_PLAYBACK_RATE AIL_set_stream_playback_rate = ( AIL_SET_STREAM_PLAYBACK_RATE )0x00486E70;
AIL_SET_STREAM_POSITION AIL_set_stream_position = ( AIL_SET_STREAM_POSITION )0x00487100;
AIL_SET_STREAM_VOLUME AIL_set_stream_volume = ( AIL_SET_STREAM_VOLUME )0x00486EE0;
AIL_START_STREAM AIL_start_stream = ( AIL_START_STREAM )0x00486DA0;
AIL_STREAM_PLAYBACK_RATE AIL_stream_playback_rate = ( AIL_STREAM_PLAYBACK_RATE )0x00487230;
AIL_STREAM_POSITION AIL_stream_position = ( AIL_STREAM_POSITION )0x004873E0;
AIL_STREAM_STATUS AIL_stream_status = ( AIL_STREAM_STATUS )0x004871E0;

GETFILEPATH GetFilePath = ( GETFILEPATH )0x00450840;
char *gameDir = ( char * )0x00EC4EE0;

Container< LocalSound * > localSounds;

ClientGameCommandManager *commandManager = ( ClientGameCommandManager * )0x30117B90;
ClientSpecialEffectsManager *sfxManager = ( ClientSpecialEffectsManager * )0x3026AB78;

Event EV_LocalSound_Think
(
	"localsound_think",
	EV_CODEONLY,
	NULL,
	NULL,
	"Internal event",
	EV_NORMAL
);

LocalSound::LocalSound( str n, qboolean l )
{
	path = GetFilePath( fs_basepath->string, gameDir, n );

	stream = AIL_open_stream( *sndDriver, path, 0 );

	name = n;

	stopping = false;
	loop = l;

	if( loop ) {
		AIL_set_stream_loop_count( stream, 0 );
	} else {
		AIL_set_stream_loop_count( stream, 1 );
	}

	volume = 0.0f;
	rate = 1.0f;

	localSounds.AddObject( this );
}

LocalSound::LocalSound()
{
	stream = NULL;

	stopping = false;
	loop = false;

	volume = 0.0f;
	rate = 1.0f;

	localSounds.AddObject( this );
}

LocalSound::~LocalSound()
{
	CancelEventsOfType( &EV_LocalSound_Think );

	if( stream ) {
		AIL_close_stream( stream );
	}

	localSounds.RemoveObject( this );
}

void LocalSound::Archive( Archiver &arc )
{
	Listener::Archive( arc );

	arc.ArchiveString( &name );

	arc.ArchiveBoolean( &stopping );
	arc.ArchiveBoolean( &loop );

	if( arc.Saving() ) {
		position = AIL_stream_position( stream );
	}

	arc.ArchiveInteger( &position );

	arc.ArchiveFloat( &volume );
	arc.ArchiveFloat( &start_volume );
	arc.ArchiveFloat( &target_volume );

	arc.ArchiveFloat( &rate );

	arc.ArchiveObject( &timer );

	if( arc.Loading() )
	{
		path = GetFilePath( fs_basepath->string, gameDir, name );

		stream = AIL_open_stream( *sndDriver, path, 0 );

		if( stream )
		{
			AIL_start_stream( stream );

			if( loop ) {
				AIL_set_stream_loop_count( stream, 0 );
			}
			else {
				AIL_set_stream_loop_count( stream, 1 );
			}

			AIL_set_stream_position( stream, position );

			Event *ev = new Event( &EV_LocalSound_Think );
			ProcessEvent( ev );
		}
	}
}

str LocalSound::GetName()
{
	return name;
}

void LocalSound::Play( float time, float v )
{
	stopping = false;

	start_volume = volume;

	if( time <= 0.0f ) {
		volume = v;
	} else {
		volume = 0.0f;
		AIL_set_stream_volume( stream, 0 );
	}

	target_volume = v;

	timer.SetTime( time );
	timer.Enable();

	start_rate = 1.0f;
	target_rate = 1.0f;
	rate = 1.0f;

	if( stream )
	{
		AIL_start_stream( stream );

		baserate = AIL_stream_playback_rate( stream );

		Event *ev = new Event( &EV_LocalSound_Think );
		ProcessEvent( ev );
	}
}

void LocalSound::Stop( float time )
{
	if( time <= 0.0f )
	{
		delete this;
		return;
	}

	start_volume = volume;

	timer.SetTime( time );
	timer.Enable();

	stopping = true;
}

void LocalSound::SetRate( float r, float time )
{
	target_rate = r;
	start_rate = rate;

	timer_rate.SetTime( time );
	timer_rate.Enable();

	if( time <= 0.0f )
	{
		rate = r;

		if( stream )
		{
			int ra = ( int )( ( float )baserate * rate );

			AIL_set_stream_playback_rate( stream, ra );
		}
	}
}

void LocalSound::Think( Event *ev )
{
	if( stopping ) {
		volume = timer.LerpValue( start_volume, 0.0f );
	}

	AIL_set_stream_volume( stream, ( int )( volume * s_volume->value * *sound_global_volume * 100.0f ) );

	if( !timer_rate.Done() ) {
		rate = timer_rate.LerpValue( start_rate, target_rate );
	}

	int r = ( int )( ( float )baserate * rate );
	AIL_set_stream_playback_rate( stream, r );

	if( ( stopping && timer.Done() ) || AIL_stream_status( stream ) != STREAM_PLAYING )
	{
		CancelEventsOfType( &EV_LocalSound_Think );

		delete this;
		return;
	}

	if( !stopping ) {
		volume = timer.LerpValue( start_volume, target_volume );
	}

	CancelEventsOfType( &EV_LocalSound_Think );
	PostEvent( EV_LocalSound_Think, level.frametime, 0 );
}

CLASS_DECLARATION( Listener, LocalSound, NULL )
{
	{ &EV_LocalSound_Think,		&LocalSound::Think },
	{ NULL, NULL }
};

void ClientGameCommandManager::CleanUp()
{
	Container< LocalSound * > local = localSounds;

	for( int i = 0; i < local.NumObjects(); i++ )
	{
		LocalSound *localSound = local[ i ];

		delete localSound;
	}

	localSounds.FreeObjectList();
}

void ClientGameCommandManager::ArchiveFunction( Archiver &arc )
{
	int num = localSounds.NumObjects();

	arc.ArchiveInteger( &num );

	if( arc.Saving() )
	{
		for( int i = 0; i < num; i++ ) {
			arc.ArchiveObject( localSounds[ i ] );
		}
	}
	else
	{
		for( int i = 0; i < num; i++ ) {
			arc.ReadObject();
		}
	}
}

void ClientGameCommandManager::Initialize()
{

}

void ClientGameCommandManager::PlaySound( str sound_name, float *origin, int channel, float volume, float min_distance, float pitch, int argstype )
{
	JMPTO( 0x30015E0C );
}

void ClientGameCommandManager::PlayLocalSound( str sound_name, qboolean loop, float volume, float time )
{
	// Don't play the same sound again
	StopLocalSound( sound_name );

	LocalSound *localSound = new LocalSound( sound_name, loop );

	localSound->Play( time, volume );
}

void ClientGameCommandManager::SetLocalSoundRate( str sound_name, float rate, float time )
{
	LocalSound *localSound = GetLocalSound( sound_name );

	if( !localSound ) {
		return;
	}

	localSound->SetRate( rate, time );
}

void ClientGameCommandManager::StopLocalSound( str sound_name, float time )
{
	LocalSound *localSound = GetLocalSound( sound_name );

	if( localSound ) {
		localSound->Stop( time );
	}
}

LocalSound *ClientGameCommandManager::GetLocalSound( str sound_name )
{
	for( int i = 1; i <= localSounds.NumObjects(); i++ )
	{
		LocalSound *localSound = localSounds.ObjectAt( i );

		if( localSound->GetName() == sound_name ) {
			return localSound;
		}
	}

	return NULL;
}

void ClientSpecialEffectsManager::MakeEffect_Normal( int iEffect, Vector position, Vector normal )
{
	JMPTO( 0x3003524D );
}
