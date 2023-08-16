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

#include "../snd_local.h"

void S_Init2()
{
    S_Init();
    SND_setup();
}

/*
=================
S_StartSound
=================
*/
void S_StartSound(const vec3_t origin, int entNum, int entChannel, sfxHandle_t sfxHandle, float volume, float minDist, float pitch, float maxDist, qboolean streamed)
{
    S_StartSound((float*)origin, entNum, entChannel, sfxHandle);
    // FIXME: partially implemented
}

/*
=================
S_AddLoopingSound
=================
*/
void S_AddLoopingSound(const vec3_t origin, const vec3_t velocity, sfxHandle_t sfxHandle, float volume, float minDist, float maxDist, float pitch, int flags)
{
    // FIXME: unimplemented
    S_AddLoopingSound(ENTITYNUM_WORLD, origin, velocity, sfxHandle);
}

/*
=================
S_StopAllSounds
=================
*/
void S_StopAllSounds(qboolean stop_music)
{
    // Call the original function
    S_StopAllSounds();
    // FIXME: stop music
}


/*
=================
S_ClearLoopingSounds
=================
*/
void S_ClearLoopingSounds(void)
{
    S_ClearLoopingSounds(qtrue);
}

/*
=================
S_Respatialize
=================
*/
void S_Respatialize(int entityNum, const vec3_t origin,
    vec3_t axis[3])
{
    S_Respatialize(entityNum, origin, axis, 0);
}
/*
=================
S_StartLocalSound
=================
*/
void S_StartLocalSound(const char* sound_name, qboolean force_load)
{
    // FIXME: unimplemented
}

sfxHandle_t	S_RegisterSound(const char* sample, qboolean compressed, qboolean streamed) {
    return S_RegisterSound(sample, compressed);
}

/*
=================
S_StopSound
=================
*/
void S_StopSound(int entnum, int channel)
{
    // FIXME: unimplemented
}

/*
=================
S_IsSoundPlaying
=================
*/
qboolean S_IsSoundPlaying(int channelNumber, const char* name)
{
    channel_t* v;
    if (channelNumber >= MAX_CHANNELS) {
        return qfalse;
    }

	v = &s_channels[channelNumber];

    return v->thesfx ? qtrue : qfalse;
}

/*
=================
MUSIC_Pause
=================
*/
void MUSIC_Pause()
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_Unpause
=================
*/
void MUSIC_Unpause()
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_LoadSoundtrackFile
=================
*/
qboolean MUSIC_LoadSoundtrackFile(const char* filename)
{
    // FIXME: unimplemented
    STUB();
    return qfalse;
}

/*
=================
MUSIC_SongValid
=================
*/
qboolean MUSIC_SongValid(const char* mood)
{
    // FIXME: unimplemented
    STUB();
    return qfalse;
}

/*
=================
MUSIC_Loaded
=================
*/
qboolean MUSIC_Loaded(void)
{
    // FIXME: unimplemented
    STUB();
    return qfalse;
}

/*
=================
Music_Update
=================
*/
void Music_Update(void)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_SongEnded
=================
*/
void MUSIC_SongEnded(void)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
S_StartSound
=================
*/
void MUSIC_NewSoundtrack(const char* name)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_UpdateMood
=================
*/
void MUSIC_UpdateMood(int current, int fallback)
{
    // FIXME: unimplemented
    //STUB();
}

/*
=================
MUSIC_UpdateVolume
=================
*/
void MUSIC_UpdateVolume(float volume, float fade_time)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_StopAllSongs
=================
*/
void MUSIC_StopAllSongs(void)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_FreeAllSongs
=================
*/
void MUSIC_FreeAllSongs(void)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_Playing
=================
*/
qboolean MUSIC_Playing(void)
{
    // FIXME: unimplemented
    STUB();
    return qfalse;
}

/*
=================
MUSIC_FindSong
=================
*/
int MUSIC_FindSong(const char* name)
{
    // FIXME: unimplemented
    STUB();
    return 0;
}

/*
=================
MUSIC_CurrentSongChannel
=================
*/
int MUSIC_CurrentSongChannel(void)
{
    // FIXME: unimplemented
    STUB();
    return 0;
}

/*
=================
MUSIC_StopChannel
=================
*/
void MUSIC_StopChannel(int channel_number)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_PlaySong
=================
*/
qboolean MUSIC_PlaySong(const char* alias)
{
    // FIXME: unimplemented
    STUB();
    return qfalse;
}

/*
=================
MUSIC_UpdateMusicVolumes
=================
*/
void MUSIC_UpdateMusicVolumes(void)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
MUSIC_CheckForStoppedSongs
=================
*/
void MUSIC_CheckForStoppedSongs(void)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
S_IsSoundRegistered
=================
*/
qboolean S_IsSoundRegistered(const char* name)
{
    // FIXME: unimplemented
    return qfalse;
}
/*
=================
S_GetSoundTime
=================
*/
float S_GetSoundTime(sfxHandle_t handle)
{
    // FIXME: unimplemented
    STUB();
    return 0.0;
}

/*
=================
S_SetGlobalAmbientVolumeLevel
=================
*/
void S_SetGlobalAmbientVolumeLevel(float volume)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
S_SetReverb
=================
*/
void S_SetReverb(int reverb_type, float reverb_level)
{
    // FIXME: unimplemented
    STUB();
}

/*
=================
S_EndRegistration
=================
*/
void S_EndRegistration(void)
{
    // FIXME: unimplemented
}

void S_UpdateEntity(int entityNum, const vec3_t origin, const vec3_t velocity, qboolean use_listener)
{
    // FIXME: unimplemented
}

void S_FadeSound(float fTime)
{
    // FIXME: unimplemented
}

void S_TriggeredMusic_PlayIntroMusic() {
    // FIXME: unimplemented
}

void callbackServer(int entnum, int channel_number, const char* name) {
    if (com_sv_running->integer) {
        SV_SoundCallback(entnum, channel_number, name);
    }
}

void S_ChannelFree_Callback(channel_t* v) {
    if (v->entnum & S_FLAG_DO_CALLBACK) {
        callbackServer(v->entnum & ~S_FLAG_DO_CALLBACK, v - s_channels, v->thesfx->soundName);
    }
}

void S_LoadData(soundsystemsavegame_t* pSave) {
    // FIXME: unimplemented
}
