/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

#pragma once

#ifdef __cplusplus

void S_EndRegistration(void);

void S_Respatialize(int entityNum, const vec3_t origin, vec3_t axis[3]);
sfxHandle_t	S_RegisterSound(const char* sample, qboolean compressed, qboolean streamed);
void S_AddLoopingSound(const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle, float volume, float minDist, float maxDist, float pitch, int flags);
void S_StartLocalSound(const char* name, qboolean force_load);
void S_StartSound(const vec3_t origin, int entNum, int entChannel, sfxHandle_t sfxHandle, float volume, float minDist, float pitch, float maxDist, qboolean streamed);
void S_StopAllSounds(qboolean stop_music);
void S_ClearLoopingSounds(void);
qboolean S_IsSoundRegistered(const char* name);
void S_Init2();

#endif

#ifdef __cplusplus
extern "C" {
#endif

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

void S_StopSound(int entnum, int channel);
float S_GetSoundTime(sfxHandle_t handle);
void S_SetGlobalAmbientVolumeLevel(float volume);
void S_SetReverb(int reverb_type, float reverb_level);
qboolean S_IsSoundPlaying(int channelNumber, const char* name);

void S_UpdateEntity(int entityNum, const vec3_t origin, const vec3_t velocity, qboolean use_listener);
void S_FadeSound(float fTime);
void S_TriggeredMusic_PlayIntroMusic();

#ifdef __cplusplus
}
#endif
