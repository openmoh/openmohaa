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
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifdef __cplusplus
extern "C" {
#endif

void S_Init( void );
void S_Shutdown( void );

// if origin is NULL, the sound will be dynamically sourced from the entity
void S_StartSound(const vec3_t origin, int entNum, int entChannel, sfxHandle_t sfxHandle, float volume, float minDist, float pitch, float maxDist, int streamed);
void S_StopSound(int entnum, int channel);
void S_StartLocalSound(sfxHandle_t sfx, int channelNum);
void S_StartLocalSoundByName(const char* sound_name, qboolean force_load);

void S_StartBackgroundTrack( const char *intro, const char *loop );
void S_StopBackgroundTrack( void );

// cinematics and voice-over-network will send raw samples
// 1.0 volume will be direct output of source samples
void S_RawSamples (int samples, int rate, int width, int channels, 
				   const byte *data, float volume);

// stop all sounds and the background track
void S_StopAllSounds(qboolean stop_music);

// all continuous looping sounds must be added before calling S_Update
void S_ClearLoopingSoundsNoParam(void);
void S_ClearLoopingSounds( qboolean killall );
void S_AddLoopingSound(const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle, float volume, float minDist, float maxDist, float pitch, int flags);
void S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx );
void S_StopLoopingSound(int entityNum );

// recompute the reletive volumes for all running sounds
// reletive to the given entityNum / orientation
void S_Respatialize( int entityNum, const vec3_t origin, vec3_t axis[3], int inwater );

// let the sound system know where an entity currently is
void S_UpdateEntityPosition( int entityNum, const vec3_t origin );

void S_Update( void );

void S_DisableSounds( void );

void S_BeginRegistration( void );
void S_EndRegistration(void);
qboolean S_IsSoundRegistered(const char* name);
// RegisterSound will allways return a valid sample, even if it
// has to create a placeholder.  This prevents continuous filesystem
// checks for missing files
sfxHandle_t	S_RegisterSound(const char *sample, qboolean compressed, qboolean streamed);

void S_DisplayFreeMemory(void);

void S_ClearSoundBuffer( void );

void SNDDMA_Activate( void );

void S_UpdateBackgroundTrack( void );

// Music soundtrack

void MUSIC_Pause();
void MUSIC_Unpause();
qboolean MUSIC_LoadSoundtrackFile(const char* filename);
qboolean MUSIC_SongValid(const char* mood);
qboolean MUSIC_Loaded(void);
void Music_Update(void);
void MUSIC_SongEnded(void);
void MUSIC_NewSoundtrack(const char* name);
void MUSIC_UpdateMood(int current, int fallback);
void MUSIC_UpdateVolume(float volume, float fade_time);
void MUSIC_StopAllSongs(void);
void MUSIC_FreeAllSongs(void);
qboolean MUSIC_Playing(void);
int MUSIC_FindSong(const char* name);
int MUSIC_CurrentSongChannel(void);
void MUSIC_StopChannel(int channel_number);
qboolean MUSIC_PlaySong(const char* alias);
void MUSIC_UpdateMusicVolumes(void);
void MUSIC_CheckForStoppedSongs(void);

float S_GetSoundTime(sfxHandle_t handle);
void S_SetGlobalAmbientVolumeLevel(float volume);
void S_SetReverb(int reverb_type, float reverb_level);
int S_IsSoundPlaying(int channelNumber, const char* name);

void S_RespatializeOld(int entityNum, const vec3_t origin, vec3_t axis[3]);
void S_UpdateEntity(int entityNum, const vec3_t origin, const vec3_t velocity, qboolean use_listener);
void S_FadeSound(float fTime);

#ifdef __cplusplus
}
#endif
