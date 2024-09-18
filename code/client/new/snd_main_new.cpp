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
#include "../client.h"

#if defined(NO_MODERN_DMA) && NO_MODERN_DMA

qboolean s_bSoundPaused = qfalse;

void S_Init2()
{
    S_Init();
    SND_setup();

    // HACK: S_RegisterSound returns 0 when unsuccessful, or it returns the the sfx handle
    // But the first sfx handle is also 0...
    S_RegisterSound("sound/null.wav", qfalse);

    Cmd_AddCommand("tmstart", S_TriggeredMusic_Start);
    Cmd_AddCommand("tmstartloop", S_TriggeredMusic_StartLoop);
    Cmd_AddCommand("tmstop", S_TriggeredMusic_Stop);
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
    if (!sfxHandle) {
        return;
    }

    // FIXME: unimplemented

    if (VectorCompare(origin, vec3_origin)) {
        // Consider it to be a local sound, uses the player origin
        S_AddLoopingSound(cl.snap.ps.clientNum, cl.snap.ps.origin, velocity, sfxHandle);
        return;
    }

    S_AddLoopingSound(ENTITYNUM_WORLD, origin, velocity, sfxHandle);
}

/*
=================
S_StopAllSounds
=================
*/
void S_StopAllSounds2(qboolean stop_music)
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
S_StartLocalSoundChannel
=================
*/
void S_StartLocalSound(const char* sound_name, qboolean force_load)
{
    sfxHandle_t h;

    h = S_RegisterSound(sound_name, qfalse);

    if (h) {
        S_StartLocalSound(h, CHAN_LOCAL_SOUND);
    }
}

/*
=================
S_StartLocalSound
=================
*/
void S_StartLocalSoundChannel(const char* sound_name, qboolean force_load, int channel)
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
==============
S_CurrentSoundtrack
==============
*/
const char* S_CurrentSoundtrack()
{
    return "";
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

/*
==============
S_TriggeredMusic_Start
==============
*/
void S_TriggeredMusic_Start()
{
    if (Cmd_Argc() != 2) {
        Com_Printf("tmstart <sound file>\n");
        return;
    }

    S_StartBackgroundTrack(Cmd_Argv(1), "");
}

/*
==============
S_TriggeredMusic_StartLoop
==============
*/
void S_TriggeredMusic_StartLoop()
{
    if (Cmd_Argc() != 2) {
        Com_Printf("tmstartloop <sound file>\n");
        return;
    }

    S_StartBackgroundTrack(Cmd_Argv(1), Cmd_Argv(1));
}

/*
==============
S_TriggeredMusic_Stop
==============
*/
void S_TriggeredMusic_Stop()
{
    S_StopBackgroundTrack();
}

/*
==============
S_TriggeredMusic_PlayIntroMusic
==============
*/
void S_TriggeredMusic_PlayIntroMusic() {
    S_StartBackgroundTrack("sound/music/mus_MainTheme.mp3", "");
}

/*
==============
S_TriggeredMusic_SetupHandle
==============
*/
void S_TriggeredMusic_SetupHandle(const char* pszName, int iLoopCount, int iOffset, qboolean autostart) {
    // FIXME: unimplemented
}

/*
==============
S_GetMusicFilename
==============
*/
const char* S_GetMusicFilename() {
    // FIXME: unimplemented
    return "";
}

/*
==============
S_GetMusicLoopCount
==============
*/
int S_GetMusicLoopCount() {
    // FIXME: unimplemented
    return 0;
}

/*
==============
S_GetMusicOffset
==============
*/
unsigned int S_GetMusicOffset() {
    // FIXME: unimplemented
    return 0;
}

/*
==============
callbackServer
==============
*/
void callbackServer(int entnum, int channel_number, const char* name) {
    if (com_sv_running->integer) {
        SV_SoundCallback(entnum, channel_number, name);
    }
}

/*
==============
S_ChannelFree_Callback
==============
*/
void S_ChannelFree_Callback(channel_t* v) {
    if (v->entnum & S_FLAG_DO_CALLBACK) {
        callbackServer(v->entnum & ~S_FLAG_DO_CALLBACK, v - s_channels, v->thesfx->soundName);
    }
}

/*
==============
S_LoadData
==============
*/
void S_LoadData(soundsystemsavegame_t* pSave) {
    // FIXME: unimplemented
}

/*
==============
S_SaveData
==============
*/
void S_SaveData(soundsystemsavegame_t* pSave) {
    // FIXME: unimplemented
}

/*
==============
S_ReLoad
==============
*/
void S_ReLoad(soundsystemsavegame_t* pSave) {
    // FIXME: unimplemented
}

/*
==============
S_StopMovieAudio
==============
*/
void S_StopMovieAudio() {
}

/*
==============
S_CurrentMoviePosition
==============
*/
int S_CurrentMoviePosition() {
    return 0;
}

/*
==============
S_SetupMovieAudio
==============
*/
void S_SetupMovieAudio(const char* pszMovieName) {
}

#endif
