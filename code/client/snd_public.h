/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#if defined(NO_MODERN_DMA) && NO_MODERN_DMA

#ifdef __cplusplus
extern "C" {
#endif

void S_Init( void );
void S_Shutdown( void );

// if origin is NULL, the sound will be dynamically sourced from the entity
void S_StartSound( vec3_t origin, int entnum, int entchannel, sfxHandle_t sfx );
void S_StartLocalSound( sfxHandle_t sfx, int channelNum );

void S_StartBackgroundTrack( const char *intro, const char *loop );
void S_StopBackgroundTrack( void );

// cinematics and voice-over-network will send raw samples
// 1.0 volume will be direct output of source samples
void S_RawSamples(int stream, int samples, int rate, int width, int channels,
				   const byte *data, float volume, int entityNum);

// stop all sounds and the background track
void S_StopAllSounds( void );

// all continuous looping sounds must be added before calling S_Update
void S_ClearLoopingSounds( qboolean killall );
void S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void S_StopLoopingSound(int entityNum );

// recompute the relative volumes for all running sounds
// relative to the given entityNum / orientation
void S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );

// let the sound system know where an entity currently is
void S_UpdateEntityPosition( int entityNum, const vec3_t origin );

void S_Update( void );

void S_DisableSounds( void );

void S_BeginRegistration( void );

// RegisterSound will allways return a valid sample, even if it
// has to create a placeholder.  This prevents continuous filesystem
// checks for missing files
sfxHandle_t	S_RegisterSound( const char *sample, qboolean compressed );

void S_DisplayFreeMemory(void);

void S_ClearSoundBuffer( void );

void SNDDMA_Activate( void );

void S_UpdateBackgroundTrack( void );


#ifdef USE_VOIP
void S_StartCapture(void);
int S_AvailableCaptureSamples(void);
void S_Capture(int samples, byte* data);
void S_StopCapture(void);
void S_MasterGain(float gain);
#endif


#ifdef __cplusplus
}
#endif

#include "new/snd_public_new.h"

#else

#ifdef __cplusplus
extern "C" {
#endif

void S_SoundInfo_f();
void S_SoundDump_f();
void S_Init(qboolean full_startup);
void S_Shutdown(qboolean full_shutdown);
qboolean S_NameExists(const char* name);
void S_BeginRegistration();
qboolean S_IsSoundRegistered(const char* name);
sfxHandle_t S_RegisterSound(const char* name, int streamed, qboolean force_load);
float S_GetSoundTime(sfxHandle_t handle);
void S_EndRegistration();
void S_UpdateEntity(int entityNum, const vec3_t origin, const vec3_t vel, qboolean use_listener);
void S_SetGlobalAmbientVolumeLevel(float volume);
void S_StartSound(const vec3_t origin, int entnum, int entchannel, sfxHandle_t sfxHandle, float volume, float min_dist, float pitch, float maxDist, int streamed);
void S_StartLocalSound(const char* sound_name, qboolean force_load);
void S_StartLocalSoundChannel(const char* sound_name, qboolean force_load, soundChannel_t channel);
void S_StopAllSounds(qboolean stop_music);
void S_ClearLoopingSounds();
void S_AddLoopingSound(const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle, float volume, float min_dist, float max_dist, float pitch, int flags);
void S_Respatialize(int entityNum, const vec3_t head, vec3_t axis[3]);
void S_Update();
void S_StopSound(int entnum, int channel);
void S_SetReverb(int reverb_type, float reverb_level);
void S_Play();
void S_SoundList();

qboolean S_IsSoundPlaying(int channel_number, const char* sfxName);
void S_FadeSound(float fTime);

qboolean MUSIC_LoadSoundtrackFile(const char* filename);
qboolean MUSIC_SongValid(const char* mood);
qboolean MUSIC_Loaded();
void Music_Update();
void MUSIC_SongEnded();
void MUSIC_NewSoundtrack(const char* name);
void MUSIC_UpdateMood(int current, int fallback);
void MUSIC_UpdateVolume(float volume, float fade_time);
void MUSIC_StopAllSongs();
void MUSIC_FreeAllSongs();
qboolean MUSIC_Playing();
int MUSIC_FindSong(const char* name);
void S_loadsoundtrack();
const char* S_CurrentSoundtrack();
void S_PlaySong();
int MUSIC_CurrentSongChannel();
void MUSIC_StopChannel(int channel_number);
qboolean MUSIC_PlaySong(const char* alias);
void MUSIC_UpdateMusicVolumes();
void MUSIC_CheckForStoppedSongs();
void S_TriggeredMusic_SetupHandle(const char* pszName, int iLoopCount, int iOffset, qboolean autostart);
void S_TriggeredMusic_Start();
void S_TriggeredMusic_StartLoop();
void S_TriggeredMusic_Stop();
void S_TriggeredMusic_Volume();
void S_TriggeredMusic_Pause();
void S_TriggeredMusic_Unpause();
void S_TriggeredMusic_PlayIntroMusic();
void S_StopMovieAudio();
void S_SetupMovieAudio(const char* pszMovieName);
int S_CurrentMoviePosition();

const char* S_GetMusicFilename();
int S_GetMusicLoopCount();
unsigned int S_GetMusicOffset();

#ifdef __cplusplus
}
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif

#ifdef USE_VOIP
void S_StartCapture( void );
int S_AvailableCaptureSamples( void );
void S_Capture( int samples, byte *data );
void S_StopCapture( void );
void S_MasterGain( float gain );
#endif

#ifdef __cplusplus
}
#endif
