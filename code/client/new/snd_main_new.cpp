#include "../snd_local.h"

/*
=================
S_IsSoundPlaying
=================
*/
int S_IsSoundPlaying(int channelNumber, const char* name)
{
    // FIXME: stub
    STUB();
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
qboolean MUSIC_LoadSoundtrackFile(const char* filename)
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
qboolean MUSIC_SongValid(const char* mood)
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
qboolean MUSIC_Loaded(void)
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
void Music_Update(void)
{
    // FIXME: stub
    STUB();
}

/*
=================
MUSIC_SongEnded
=================
*/
void MUSIC_SongEnded(void)
{
    // FIXME: stub
    STUB();
}

/*
=================
S_StartSound
=================
*/
void MUSIC_NewSoundtrack(const char* name)
{
    // FIXME: stub
    STUB();
}

/*
=================
MUSIC_UpdateMood
=================
*/
void MUSIC_UpdateMood(int current, int fallback)
{
    // FIXME: stub
    //STUB();
}

/*
=================
MUSIC_UpdateVolume
=================
*/
void MUSIC_UpdateVolume(float volume, float fade_time)
{
    // FIXME: stub
    STUB();
}

/*
=================
MUSIC_StopAllSongs
=================
*/
void MUSIC_StopAllSongs(void)
{
    // FIXME: stub
    STUB();
}

/*
=================
MUSIC_FreeAllSongs
=================
*/
void MUSIC_FreeAllSongs(void)
{
    // FIXME: stub
    STUB();
}

/*
=================
MUSIC_Playing
=================
*/
qboolean MUSIC_Playing(void)
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
int MUSIC_FindSong(const char* name)
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
int MUSIC_CurrentSongChannel(void)
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
void MUSIC_StopChannel(int channel_number)
{
    // FIXME: stub
    STUB();
}

/*
=================
MUSIC_PlaySong
=================
*/
qboolean MUSIC_PlaySong(const char* alias)
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
void MUSIC_UpdateMusicVolumes(void)
{
    // FIXME: stub
    STUB();
}

/*
=================
MUSIC_CheckForStoppedSongs
=================
*/
void MUSIC_CheckForStoppedSongs(void)
{
    // FIXME: stub
    STUB();
}

/*
=================
S_IsSoundRegistered
=================
*/
qboolean S_IsSoundRegistered(const char* name)
{
    // FIXME: stub
    return qfalse;
}
/*
=================
S_GetSoundTime
=================
*/
float S_GetSoundTime(sfxHandle_t handle)
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
void S_SetGlobalAmbientVolumeLevel(float volume)
{
    // FIXME: stub
    STUB();
}

/*
=================
S_SetReverb
=================
*/
void S_SetReverb(int reverb_type, float reverb_level)
{
    // FIXME: stub
    STUB();
}

/*
=================
S_EndRegistration
=================
*/
void S_EndRegistration(void)
{
    // FIXME: stub
}

void S_UpdateEntity(int entityNum, const vec3_t origin, const vec3_t velocity, qboolean use_listener)
{
    // FIXME: stub
}

void S_FadeSound(float fTime)
{
    // FIXME: stub
}