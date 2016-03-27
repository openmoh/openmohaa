/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2005 Stuart Dalton (badcdev@gmail.com)

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "client.h"
#include "server.h"
#include "snd_codec.h"
#include "snd_local.h"
#include "snd_public.h"

cvar_t *s_volume;
cvar_t *s_loadas8bit;
cvar_t *s_khz;
cvar_t *s_show;
cvar_t *s_testsound;
cvar_t *s_separation;
cvar_t *s_musicVolume;
cvar_t *s_ambientVolume;
cvar_t *s_doppler;
cvar_t *s_backend;
cvar_t *s_muteWhenMinimized;
cvar_t *s_muted;

static qboolean s_bFading;
static float s_fFadeVolume;
static float s_fFadeStartTime;
static float s_fFadeStopTime;

sfx_info_t sfx_infos[ 1000 ];
sfx_t s_knownSfx[ 1400 ];
int s_numSfx;
s_entity_t s_entity[ MAX_GENTITIES ];
static int s_registrationSequence;
static qboolean s_inRegistration;
cvar_t *s_mixPreStep;
cvar_t *s_dialogscale;
int numLoopSounds;
loopSound_t loopSounds[ 64 ];

sndparm_t soundparm;

static soundInterface_t si;

/*
=================
S_ValidateInterface
=================
*/
static qboolean S_ValidSoundInterface( soundInterface_t *si )
{
	if( !si->Shutdown ) return qfalse;
	if( !si->StartSound ) return qfalse;
	if( !si->StartLocalSound ) return qfalse;
	if( !si->StartBackgroundTrack ) return qfalse;
	if( !si->StopBackgroundTrack ) return qfalse;
	if( !si->RawSamples ) return qfalse;
	if( !si->StopAllSounds ) return qfalse;
	if( !si->ClearLoopingSounds ) return qfalse;
	if( !si->AddLoopingSound ) return qfalse;
	if( !si->AddRealLoopingSound ) return qfalse;
	if( !si->StopLoopingSound ) return qfalse;
	if( !si->Respatialize ) return qfalse;
	if( !si->UpdateEntity ) return qfalse;
	if( !si->Update ) return qfalse;
	if( !si->DisableSounds ) return qfalse;
	if( !si->BeginRegistration ) return qfalse;
	if( !si->RegisterSound ) return qfalse;
	if( !si->ClearSoundBuffer ) return qfalse;
	if( !si->SoundInfo ) return qfalse;
	if( !si->SoundList ) return qfalse;

	return qtrue;
}

/*
=================
MUSIC_Pause
=================
*/
void MUSIC_Pause()
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_Unpause
=================
*/
void MUSIC_Unpause()
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_LoadSoundtrackFile
=================
*/
qboolean MUSIC_LoadSoundtrackFile( const char *filename )
{
	// FIXME: stub
	STUB();
	return qfalse;
}

/*
=================
MUSIC_SongValid
=================
*/
qboolean MUSIC_SongValid( const char *mood )
{
	// FIXME: stub
	STUB();
	return qfalse;
}

/*
=================
MUSIC_Loaded
=================
*/
qboolean MUSIC_Loaded( void )
{
	// FIXME: stub
	STUB();
	return qfalse;
}

/*
=================
Music_Update
=================
*/
void Music_Update( void )
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_SongEnded
=================
*/
void MUSIC_SongEnded( void )
{
	// FIXME: stub
	STUB();
}

/*
=================
S_StartSound
=================
*/
void MUSIC_NewSoundtrack( const char *name )
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_UpdateMood
=================
*/
void MUSIC_UpdateMood( int current, int fallback )
{
	// FIXME: stub
	//STUB();
}

/*
=================
MUSIC_UpdateVolume
=================
*/
void MUSIC_UpdateVolume( float volume, float fade_time )
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_StopAllSongs
=================
*/
void MUSIC_StopAllSongs( void )
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_FreeAllSongs
=================
*/
void MUSIC_FreeAllSongs( void )
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_Playing
=================
*/
qboolean MUSIC_Playing( void )
{
	// FIXME: stub
	STUB();
	return qfalse;
}

/*
=================
MUSIC_FindSong
=================
*/
int MUSIC_FindSong( const char *name )
{
	// FIXME: stub
	STUB();
	return 0;
}

/*
=================
MUSIC_CurrentSongChannel
=================
*/
int MUSIC_CurrentSongChannel( void )
{
	// FIXME: stub
	STUB();
	return 0;
}

/*
=================
MUSIC_StopChannel
=================
*/
void MUSIC_StopChannel( int channel_number )
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_PlaySong
=================
*/
qboolean MUSIC_PlaySong( const char *alias )
{
	// FIXME: stub
	STUB();
	return qfalse;
}

/*
=================
MUSIC_UpdateMusicVolumes
=================
*/
void MUSIC_UpdateMusicVolumes( void )
{
	// FIXME: stub
	STUB();
}

/*
=================
MUSIC_CheckForStoppedSongs
=================
*/
void MUSIC_CheckForStoppedSongs( void )
{
	// FIXME: stub
	STUB();
}

/*
=================
S_StartSound
=================
*/
void S_StartSound( const vec3_t origin, int entNum, int entChannel, sfxHandle_t sfxHandle, float volume, float minDist, float pitch, float maxDist, qboolean streamed )
{
	if( !si.StartSound || sfxHandle == -1 ) {
		return;
	}

	soundparm.volume = volume;
	soundparm.minDist = minDist;
	soundparm.pitch = pitch;
	soundparm.maxDist = maxDist;
	soundparm.streamed = streamed;

	si.StartSound( origin, entNum, entChannel, sfxHandle );
}

/*
=================
S_StartLocalSound
=================
*/
void S_StartLocalSound( const char *sound_name, qboolean force_load )
{
	sfxHandle_t sfxHandle;
	const char *name;
	AliasListNode_t *pSoundAlias = NULL;

	if( !si.StartLocalSound ) {
		return;
	}

	if( !sound_name ) {
		return;
	}

	// obtain the sound alias
	name = Alias_FindRandom( sound_name, &pSoundAlias );

	if( !name ) {
		name = sound_name;
	}

	if( pSoundAlias ) {
		sfxHandle = S_RegisterSound( name, pSoundAlias->streamed, qfalse );
	} else {
		sfxHandle = S_RegisterSound( name, qfalse, qfalse );
	}

	if( sfxHandle < 0 || sfxHandle >= s_numSfx ) {
		Com_Printf( "S_StartLocalSound: handle %i out of range\n", sfxHandle );
		return;
	}

	if( name )
	{
		soundparm.volume = pSoundAlias->volumeMod * randweight() + pSoundAlias->volume;
		soundparm.minDist = -1.0;
		soundparm.pitch = pSoundAlias->pitchMod * randweight() + pSoundAlias->pitch;
		soundparm.maxDist = -1.0;
		soundparm.streamed = pSoundAlias->streamed;
	}
	else
	{
		soundparm.volume = -1.0;
		soundparm.minDist = -1.0;
		soundparm.pitch = 1.0;
		soundparm.maxDist = -1.0;
		soundparm.streamed = qfalse;
	}

	si.StartLocalSound( sfxHandle, CHAN_MENU );
}

/*
=================
S_StartBackgroundTrack
=================
*/
void S_StartBackgroundTrack( const char *intro, const char *loop )
{
	if( si.StartBackgroundTrack ) {
		si.StartBackgroundTrack( intro, loop );
	}
}

/*
=================
S_StopBackgroundTrack
=================
*/
void S_StopBackgroundTrack( void )
{
	if( si.StopBackgroundTrack ) {
		si.StopBackgroundTrack( );
	}
}

/*
=================
S_RawSamples
=================
*/
void S_RawSamples (int stream, int samples, int rate, int width, int channels,
		   const byte *data, float volume, int entityNum)
{
	if(si.RawSamples)
		si.RawSamples(stream, samples, rate, width, channels, data, volume, entityNum);
}

/*
=================
S_StopSound
=================
*/
void S_StopSound( int entNum, int channel )
{
	// FIXME: stub
}

/*
=================
S_StopAllSounds
=================
*/
void S_StopAllSounds( qboolean stop_music )
{
	if( si.StopAllSounds ) {
		si.StopAllSounds( );
	}

	// FIXME: stop music
}

/*
=================
S_StopAllSounds_f
=================
*/
void S_StopAllSounds_f( void )
{
	S_StopAllSounds( qtrue );
}

/*
=================
S_ClearLoopingSounds
=================
*/
void S_ClearLoopingSounds( void )
{
	if( si.ClearLoopingSounds ) {
		si.ClearLoopingSounds( qtrue );
	}
}

/*
=================
S_AddLoopingSound
=================
*/
void S_AddLoopingSound( const vec3_t origin, const vec3_t velocity,
		sfxHandle_t sfx, float volume, float minDist, float maxDist, float pitch, int flags )
{
	if( !si.AddLoopingSound ) {
		return;
	}

	soundparm.volume = volume;
	soundparm.minDist = minDist;
	soundparm.maxDist = maxDist;
	soundparm.pitch = pitch;
	soundparm.flags = flags;

	si.AddLoopingSound( ENTITYNUM_WORLD, origin, velocity, sfx );
}

/*
=================
S_AddRealLoopingSound
=================
*/
void S_AddRealLoopingSound( int entityNum, const vec3_t origin,
		const vec3_t velocity, sfxHandle_t sfx )
{
	if( si.AddRealLoopingSound ) {
		si.AddRealLoopingSound( entityNum, origin, velocity, sfx );
	}
}

/*
=================
S_StopLoopingSound
=================
*/
void S_StopLoopingSound( int entityNum )
{
	if( si.StopLoopingSound ) {
		si.StopLoopingSound( entityNum );
	}
}

/*
=================
S_Respatialize
=================
*/
void S_Respatialize( int entityNum, const vec3_t origin,
		vec3_t axis[3] )
{
	S_RespatializeEx( entityNum, origin, axis, qfalse );
}

/*
=================
S_RespatializeEx
=================
*/
void S_RespatializeEx( int entityNum, const vec3_t origin,
	vec3_t axis[ 3 ], int inwater )
{
	if( si.Respatialize ) {
		si.Respatialize( entityNum, origin, axis, inwater );
	}
}

/*
=================
S_UpdateEntity
=================
*/
void S_UpdateEntity( int entityNum, const vec3_t origin, const vec3_t velocity, qboolean use_listener )
{
	if( si.UpdateEntity ) {
		si.UpdateEntity( entityNum, origin, velocity );
	}
}

/*
=================
S_Update
=================
*/
void S_Update( void )
{
	if( s_muteWhenMinimized->integer && com_minimized->integer ) {
		S_StopAllSounds( qtrue );
		return;
	}

	if( si.Update ) {
		si.Update( );
	}
}

/*
=================
S_DisableSounds
=================
*/
void S_DisableSounds( void )
{
	if( si.DisableSounds ) {
		si.DisableSounds( );
	}
}

/*
=================
S_BeginRegistration
=================
*/
void S_BeginRegistration( void )
{
	if( si.BeginRegistration ) {
		si.BeginRegistration( );
	}
}

/*
=================
S_EndRegistration
=================
*/
void S_EndRegistration( void )
{
	// FIXME: stub
}

/*
=================
S_IsSoundRegistered
=================
*/
qboolean S_IsSoundRegistered( const char *name )
{
	// FIXME: stub
	return qfalse;
}

/*
=================
S_RegisterSound
=================
*/
sfxHandle_t	S_RegisterSound( const char *sample, qboolean compressed, qboolean force_load )
{
	if( si.RegisterSound ) {
		if ( !strstr(sample, "sound/null.wav") )
			return si.RegisterSound( sample, compressed );
		else return -1;
	} else {
		return 0;
	}
}

/*
=================
S_GetSoundTime
=================
*/
float S_GetSoundTime( sfxHandle_t handle )
{
	// FIXME: stub
	STUB();
	return 0.0;
}

/*
=================
S_SetGlobalAmbientVolumeLevel
=================
*/
void S_SetGlobalAmbientVolumeLevel( float volume )
{
	// FIXME: stub
	STUB();
}

/*
=================
S_SetReverb
=================
*/
void S_SetReverb( int reverb_type, float reverb_level )
{
	// FIXME: stub
	STUB();
}

/*
=================
S_IsSoundPlaying
=================
*/
int S_IsSoundPlaying( int channelNumber, const char *name )
{
	if( si.IsSoundPlaying ) {
		return si.IsSoundPlaying( channelNumber, name );
	}

	return qfalse;
}

/*
=================
S_ClearSoundBuffer
=================
*/
void S_ClearSoundBuffer( void )
{
	if( si.ClearSoundBuffer ) {
		si.ClearSoundBuffer( );
	}
}

/*
=================
S_SoundInfo
=================
*/
void S_SoundInfo( void )
{
	if( si.SoundInfo ) {
		si.SoundInfo( );
	}
}

/*
=================
S_SoundList
=================
*/
void S_SoundList( void )
{
	if( si.SoundList ) {
		si.SoundList( );
	}
}

//=============================================================================

/*
=================
S_Play_f
=================
*/
void S_Play_f( void ) {
	int 		i;
	sfxHandle_t	h;
	char		name[256];

	if( !si.RegisterSound || !si.StartLocalSound ) {
		return;
	}

	i = 1;
	while ( i<Cmd_Argc() ) {
		if ( !Q_strrchr(Cmd_Argv(i), '.') ) {
			Com_sprintf( name, sizeof(name), "%s.wav", Cmd_Argv(1) );
		} else {
			Q_strncpyz( name, Cmd_Argv(i), sizeof(name) );
		}
		h = si.RegisterSound( name, qfalse );
		if( h ) {
			si.StartLocalSound( h, CHAN_LOCAL_SOUND );
		}
		i++;
	}
}

/*
=================
S_Music_f
=================
*/
void S_Music_f( void ) {
	int		c;

	if( !si.StartBackgroundTrack ) {
		return;
	}

	c = Cmd_Argc();

	if ( c == 2 ) {
		si.StartBackgroundTrack( Cmd_Argv(1), NULL );
	} else if ( c == 3 ) {
		si.StartBackgroundTrack( Cmd_Argv(1), Cmd_Argv(2) );
	} else {
		Com_Printf ("music <musicfile> [loopfile]\n");
		return;
	}

}

//=============================================================================

/*
=================
S_SaveData
=================
*/
void S_SaveData( soundsystemsavegame_t *pSave )
{
	/*
	int i;
	qboolean bSoundWasUnpaused;

	for( i = 0; i < MAX_SOUNDCHANNELS; i++ ) {
		S_StoreBase( &pSave->Channels[ i ] );
	}
	*/

	// FIXME: stub
}

/*
=================
S_LoadData
=================
*/
void S_LoadData( soundsystemsavegame_t *pSave )
{
	int i;

	for( i = 0; i < MAX_SOUNDCHANNELS; i++ ) {
		S_InitBase( &pSave->Channels[ i ] );
	}
}

/*
=================
S_InitBase
=================
*/
void S_InitBase( channelbasesavegame_t *pBase )
{
	if( !pBase->bPlaying ) {
		return;
	}

	if( strstr( pBase->sfx.szName, "null.wav" ) ) {
		return;
	}

	SV_AddSvsTimeFixup( &pBase->iStartTime );
	SV_AddSvsTimeFixup( &pBase->iEndTime );
	SV_AddSvsTimeFixup( &pBase->iTime );
}

/*
=================
S_InitBase
=================
*/
void S_StoreBase( channelbasesavegame_t *pBase, channel_t *channel )
{
	if( !channel) {
		return;
	}

	if( strstr( pBase->sfx.szName, "null.wav" ) ) {
		return;
	}

	// FIXME: set proper fields

	pBase->bPlaying = qtrue;
	pBase->iOffset = 0;
	pBase->iLoopCount = 0;
	memcpy( pBase->sfx.szName, channel->thesfx->soundName, sizeof( pBase->sfx.szName ) );
	pBase->sfx.iFlags = 0;
	pBase->iBaseRate = 0;
	pBase->iStatus = 0;
	pBase->fNewPitchMult = 0;

	pBase->iStartTime = 0;
	pBase->iEndTime = 0;
	pBase->iEntChannel = channel->entchannel;
	pBase->iEntNum = channel->entnum;
	pBase->iFlags = 0;
	pBase->fMaxDist = 0;
	pBase->fMinDist = 0;
	pBase->iNextCheckObstructionTime = 0;
	VectorCopy( channel->origin, pBase->vOrigin );
	pBase->iTime = 0;
	pBase->fVolume = channel->master_vol / 127.0;
}

/*
=================
S_FadeSound
=================
*/
void S_FadeSound( float fTime ) {
	Com_Printf( "Called FadeSound with: %f\n", fTime );

	if( fTime > 0.0 )
	{
		s_bFading = qtrue;
		s_fFadeStartTime = cls.realtime;
		s_fFadeVolume = 1.0;
		s_fFadeStopTime = cls.realtime + fTime;
	}
	else
	{
		s_fFadeVolume = 1.0;
		s_bFading = qfalse;
	}
}

/*
=================
callbackServer
=================
*/
void callbackServer( int entnum, int channel_number, const char *name ) {
	if( com_sv_running->integer ) {
		SV_SoundCallback( entnum, channel_number, name );
	}
}

/*
=================
S_Init
=================
*/
void S_Init( void )
{
	cvar_t		*cv;
	qboolean	started = qfalse;

	Com_Printf( "------ Initializing Sound ------\n" );

	s_volume = Cvar_Get( "s_volume", "0.8", CVAR_ARCHIVE );
	s_musicVolume = Cvar_Get( "s_musicvolume", "0.25", CVAR_ARCHIVE );
	s_ambientVolume = Cvar_Get( "s_ambientvolume", "1.00", CVAR_ARCHIVE );
	s_separation = Cvar_Get( "s_separation", "0.5", CVAR_ARCHIVE );
	s_khz = Cvar_Get( "s_khz", "11", CVAR_SOUND_LATCH | CVAR_ARCHIVE );
	s_loadas8bit = Cvar_Get( "s_loadas8bit", "0", CVAR_LATCH | CVAR_ARCHIVE );
	s_mixPreStep = Cvar_Get( "s_mixPreStep", "0.05", CVAR_ARCHIVE );
	s_show = Cvar_Get( "s_show", "0", CVAR_CHEAT );
	s_testsound = Cvar_Get( "s_testsound", "", CVAR_CHEAT );
	s_dialogscale = Cvar_Get( "s_dialogscale", "1", CVAR_ARCHIVE );
	s_doppler = Cvar_Get( "s_doppler", "1", CVAR_ARCHIVE );
	s_backend = Cvar_Get( "s_backend", "", CVAR_ROM );
	s_muteWhenMinimized = Cvar_Get( "s_muteWhenMinimized", "0", CVAR_ARCHIVE );
	s_muted = Cvar_Get( "s_muted", "0", CVAR_ARCHIVE );

	cv = Cvar_Get( "s_initsound", "1", 0 );
	if( !cv->integer ) {
		Com_Printf( "Sound disabled.\n" );
	} else {

		S_CodecInit( );

		Cmd_AddCommand( "play", S_Play_f );
		Cmd_AddCommand( "music", S_Music_f );
		Cmd_AddCommand( "s_list", S_SoundList );
		Cmd_AddCommand( "s_stop", S_StopAllSounds_f );
		Cmd_AddCommand( "s_info", S_SoundInfo );

		cv = Cvar_Get( "s_useOpenAL", "1", CVAR_ARCHIVE );
		if( cv->integer ) {
			//OpenAL
			started = S_AL_Init( &si );
			Cvar_Set( "s_backend", "OpenAL" );
		}

		if( !started ) {
			started = S_Base_Init( &si );
			Cvar_Set( "s_backend", "base" );
		}

		if( started ) {
			if( !S_ValidSoundInterface( &si ) ) {
				Com_Error( ERR_FATAL, "Sound interface invalid." );
			}

			S_SoundInfo( );
			Com_Printf( "Sound initialization successful.\n" );
		} else {
			Com_Printf( "Sound initialization failed.\n" );
		}
	}

	Com_Printf( "--------------------------------\n");
}

/*
=================
S_Shutdown
=================
*/
void S_Shutdown( void )
{
	if( si.Shutdown ) {
		si.Shutdown( );
	}

	Com_Memset( &si, 0, sizeof( soundInterface_t ) );

	Cmd_RemoveCommand( "play" );
	Cmd_RemoveCommand( "music");
	Cmd_RemoveCommand( "s_list" );
	Cmd_RemoveCommand( "s_stop" );
	Cmd_RemoveCommand( "s_info" );

	S_CodecShutdown( );
}

