/*
===========================================================================
Copyright (C) 2024 the OpenMoHAA team

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

#if USE_SOUND_NEW

#    include "snd_local.h"
#    include "snd_openal_new.h"
#    include "client.h"
#    include "cl_ui.h"

typedef struct {
    vec3_t origin;
    vec3_t velocity;
    sfx_t *sfx;
    int    mergeFrame;
} loopSound_t;

qboolean s_bLastInitSound  = qfalse;
qboolean s_bSoundStarted   = qfalse;
qboolean s_bSoundPaused    = qfalse;
qboolean s_bTryUnpause     = qfalse;
int      s_iListenerNumber = 0;
float    s_fAmbientVolume  = 1.f;

int number_of_sfx_infos = 0;

cvar_t *s_volume;
cvar_t *s_loadas8bit;
cvar_t *s_khz;
cvar_t *s_show;
cvar_t *s_testsound;
cvar_t *s_separation;
cvar_t *s_musicVolume;
cvar_t *s_ambientVolume;

sfx_info_t sfx_infos[MAX_SFX_INFOS];

sfx_t s_knownSfx[MAX_SFX];
int   s_numSfx;

s_entity_t s_entity[MAX_GENTITIES];

static int      s_registrationSequence;
static qboolean s_inRegistration;

cvar_t *s_mixPreStep;
cvar_t *s_dialogscale;

int         numLoopSounds;
loopSound_t loopSounds[MAX_LOOP_SOUNDS];

/*
==============
S_SoundInfo_f
==============
*/
void S_SoundInfo_f()
{
    S_PrintInfo();
}

/*
==============
S_SoundDump_f
==============
*/
void S_SoundDump_f()
{
    S_DumpInfo();
}

/*
==============
S_Init
==============
*/
void S_Init(qboolean full_startup)
{
    int     iStart, iEnd;
    cvar_t *cv;

    Com_Printf("------- Sound Initialization (%s) -------\n", full_startup ? "full" : "partial");

    iStart = Sys_Milliseconds();

    s_volume         = Cvar_Get("s_volume", "0.9", CVAR_ARCHIVE);
    s_musicVolume    = Cvar_Get("s_musicvolume", "0.9", CVAR_ARCHIVE);
    s_ambientVolume  = Cvar_Get("s_ambientvolume", "1.00", CVAR_ARCHIVE);
    s_separation     = Cvar_Get("s_separation", "0.5", CVAR_ARCHIVE);
    s_khz            = Cvar_Get("s_khz", "11", CVAR_ARCHIVE | CVAR_SOUND_LATCH);
    s_loadas8bit     = Cvar_Get("s_loadas8bit", "0", CVAR_ARCHIVE | CVAR_LATCH);
    s_mixPreStep     = Cvar_Get("s_mixPreStep", "0.05", CVAR_ARCHIVE);
    s_show           = Cvar_Get("s_show", "0", CVAR_CHEAT);
    s_testsound      = Cvar_Get("s_testsound", "0", CVAR_CHEAT);
    s_dialogscale    = Cvar_Get("s_dialogscale", "1", CVAR_ARCHIVE);
    s_bLastInitSound = false;

    cv = Cvar_Get("s_initsound", "1", 0);

    if (cv->integer) {
        Cmd_AddCommand("play", S_Play);
        Cmd_AddCommand("soundlist", S_SoundList);
        Cmd_AddCommand("soundinfo", S_SoundInfo_f);
        Cmd_AddCommand("sounddump", S_SoundDump_f);

        if (S_Driver_Init()) {
            s_bLastInitSound = true;
            s_bSoundStarted  = true;

            if (full_startup) {
                s_numSfx = 0;
                S_StopAllSounds(true);
            }

            S_FindName("***DEFAULT***", -1);
            S_SoundInfo_f();
        }
    } else {
        Com_Printf("Sound Not Initializing.\n");
        s_bSoundStarted = 0;
    }

    iEnd = Sys_Milliseconds();
    Com_Printf("------- Sound Initialization Complete ------- %i ms\n", iEnd - iStart);
}

/*
==============
S_Shutdown
==============
*/
void S_Shutdown(qboolean full_shutdown)
{
    int    i;
    sfx_t *sfx;

    if (!s_bSoundStarted) {
        return;
    }

    Com_Printf("------- Sound Shutdown (%s) -------\n", full_shutdown ? "full" : "partial");

    S_Driver_Shutdown();
    s_bSoundStarted = false;

    Cmd_RemoveCommand("play");
    Cmd_RemoveCommand("music");
    Cmd_RemoveCommand("stopsound");
    Cmd_RemoveCommand("soundlist");
    Cmd_RemoveCommand("soundinfo");

    if (full_shutdown) {
        for (i = 0; i < s_numSfx; i++) {
            sfx = &s_knownSfx[i];

            if (sfx->name[0]) {
                if (sfx->data) {
                    Z_Free(sfx->data);
                }

                *sfx = {};
            }
        }

        s_numSfx = 0;
    }

    Com_Printf("------- Sound Shutdown Complete -------\n");
}

/*
==============
S_NameExists
==============
*/
qboolean S_NameExists(const char *name)
{
    int i;

    if (strlen(name) >= MAX_RES_NAME) {
        Com_DPrintf("Sound name too long: %s", name);
        return qfalse;
    }

    for (i = 0; i < s_numSfx; i++) {
        if (!strcmp(s_knownSfx[i].name, name)) {
            return qtrue;
        }
    }

    return qfalse;
}

/*
==============
S_FindName
==============
*/
sfx_t *S_FindName(const char *name, int sequenceNumber)
{
    int    i;
    sfx_t *sfx;

    if (!name) {
        Com_DPrintf("S_FindName: NULL\n");
        return NULL;
    }

    if (!name[0]) {
        Com_DPrintf("S_FindName: empty name\n");
        return NULL;
    }

    if (strlen(name) >= MAX_RES_NAME) {
        Com_DPrintf("Sound name too long: %s", name);
        return NULL;
    }

    for (i = 0; i < s_numSfx; i++) {
        sfx = &s_knownSfx[i];

        if (!strcmp(sfx->name, name)) {
            if (sfx->registration_sequence != -1) {
                sfx->registration_sequence = sequenceNumber;
            }

            return sfx;
        }
    }

    sfx = &s_knownSfx[0];

    for (i = 0; i < s_numSfx; i++) {
        sfx = &s_knownSfx[i];
        if (!sfx->name[0]) {
            break;
        }
    }

    if (i == s_numSfx) {
        if (i == MAX_SFX) {
            Com_Error(ERR_FATAL, "S_FindName: out of sfx_t");
        }

        sfx = &s_knownSfx[s_numSfx];
        s_numSfx++;
    }

    *sfx = {};
    // Fixed in OPM
    //  strcpy was used before
    //strcpy(sfx->name, name);
    Q_strncpyz(sfx->name, name, sizeof(sfx->name));
    sfx->registration_sequence = sequenceNumber;

    return sfx;
}

/*
==============
S_BeginRegistration
==============
*/
void S_BeginRegistration()
{
    Com_Printf("------- Sound Begin Registration -------\n");

    S_StopAllSounds(true);

    ++s_registrationSequence;
    s_inRegistration                    = true;
    s_knownSfx[0].registration_sequence = s_registrationSequence;

    Com_Printf("------- Sound Begin Registration Complete -------\n");
}

/*
==============
S_DefaultSound
==============
*/
void S_DefaultSound(sfx_t *sfx)
{
    sfx->data = NULL;
    sfx->iFlags |= SFX_FLAG_DEFAULT_SOUND;
}

/*
==============
S_IsSoundRegistered
==============
*/
qboolean S_IsSoundRegistered(const char *name)
{
    char szCacheName[MAX_QPATH];
    int  i;

    if (!s_bSoundStarted) {
        return qfalse;
    }

    for (i = 0; name[i] && i < MAX_QPATH; i++) {
        szCacheName[i] = tolower(name[i]);
    }
    szCacheName[i] = 0;

    if (i >= MAX_QPATH) {
        Com_Printf("Sound name exceeds MAX_QPATH\n");
        return qfalse;
    }

    return S_NameExists(szCacheName);
}

/*
==============
S_RegisterSound
==============
*/
sfxHandle_t S_RegisterSound(const char *name, int streamed, qboolean force_load)
{
    char   szCacheName[MAX_QPATH];
    sfx_t *sfx;
    int    i;

    if (!s_bSoundStarted) {
        Com_Printf("ERROR:  Trying to register sound %s when soundsystem hasn't started.\n", name);
        return 0;
    }

    if (!name) {
        return 0;
    }

    for (i = 0; name[i] && i < MAX_QPATH; i++) {
        szCacheName[i] = tolower(name[i]);
    }
    szCacheName[i] = 0;

    if (i >= MAX_QPATH) {
        Com_Printf("Sound name exceeds MAX_QPATH\n");
        return 0;
    }

    sfx = S_FindName(szCacheName, s_registrationSequence);
    if (!sfx) {
        return 0;
    }

    if (!sfx->data) {
        if (!S_LoadSound(sfx->name, sfx, streamed, force_load)) {
            Com_DPrintf("Couldn't load sound: %s\n", sfx->name);
            if (!S_LoadSound("sound/default.wav", sfx, streamed, 0)) {
                Com_DPrintf("Couldn't load sound: sound/default.wav\n");
                sfx->iFlags |= SFX_FLAG_DEFAULT_SOUND;
                return 0;
            }
        }

        sfx->iFlags &= ~SFX_FLAG_DEFAULT_SOUND;
        sfx->sfx_info_index = 0;

        for (i = 0; i < number_of_sfx_infos; i++) {
            if (!Q_stricmp(sfx->name, sfx_infos[i].name)) {
                sfx->sfx_info_index = i;
                break;
            }
        }
    }

    return sfx - s_knownSfx;
}

/*
==============
S_GetSoundTime
==============
*/
float S_GetSoundTime(sfxHandle_t handle)
{
    return s_knownSfx[handle].time_length;
}

/*
==============
S_EndRegistration
==============
*/
void S_EndRegistration()
{
    int    i;
    sfx_t *sfx;

    Com_Printf("------- Sound End Registration -------\n");
    s_inRegistration = qfalse;

    for (i = 0; i < s_numSfx; i++) {
        sfx = &s_knownSfx[i];

        if (!sfx->name[0]) {
            continue;
        }

        if (sfx->registration_sequence && sfx->registration_sequence != s_registrationSequence) {
            if (sfx->data) {
                Z_Free(sfx->data);
            }

            *sfx = {};
        }
    }

    Com_Printf("------- Sound End Registration Complete -------\n");
}

/*
==============
S_UpdateEntity
==============
*/
void S_UpdateEntity(int entityNum, const vec3_t origin, const vec3_t vel, qboolean use_listener)
{
    if (entityNum > MAX_GENTITIES) {
        Com_Error(ERR_DROP, "S_UpdateEntityPosition: bad entitynum %i", entityNum);
    }

    VectorCopy(origin, s_entity[entityNum].position);
    VectorCopy(vel, s_entity[entityNum].velocity);
    s_entity[entityNum].use_listener = use_listener;
    s_entity[entityNum].time         = cl.serverTime;
}

/*
==============
S_SetGlobalAmbientVolumeLevel
==============
*/
void S_SetGlobalAmbientVolumeLevel(float volume)
{
    s_fAmbientVolume = volume;
}

/*
==============
S_StartSound
==============
*/
void S_StartSound(
    const vec3_t origin,
    int          entnum,
    int          entchannel,
    sfxHandle_t  sfxHandle,
    float        volume,
    float        min_dist,
    float        pitch,
    float        maxDist,
    int          streamed
)
{
    if (s_show_sounds->integer > 0 && sfxHandle > 0 && sfxHandle < s_numSfx) {
        Com_DPrintf("S_StartSound: %s\n", s_knownSfx[sfxHandle].name);
    }

    if (!s_bSoundStarted) {
        if (s_show_sounds->integer > 0) {
            Com_DPrintf("^~^~^ sound system not yet started\n");
        }
        return;
    }

    if (server_loading) {
        if (s_show_sounds->integer > 0) {
            Com_DPrintf("^~^~^ cannot start sounds while loading a server\n");
        }
        return;
    }

    if (sfxHandle < 0 || sfxHandle >= s_numSfx) {
        Com_Error(ERR_DROP, "S_StartSound: handle %i out of range", sfxHandle);
    }

    if (!sfxHandle) {
        if (s_show_sounds->integer > 0) {
            Com_DPrintf("^~^~^ null handle\n");
        }
        return;
    }

    if (entnum && (entchannel == CHAN_DIALOG || entchannel == CHAN_DIALOG_SECONDARY) && s_dialogscale->value > 0) {
        volume *= s_dialogscale->value;
    }

    S_Driver_StartSound(origin, entnum, entchannel, sfxHandle, volume, min_dist, pitch, maxDist, streamed);
}

/*
==============
S_StartLocalSound
==============
*/
void S_StartLocalSound(const char *sound_name, qboolean force_load)
{
    sfxHandle_t      sfxHandle;
    const char      *name;
    AliasListNode_t *pSoundAlias = NULL;

    if (!s_bSoundStarted) {
        return;
    }

    if (!sound_name) {
        return;
    }

    name = Alias_FindRandom(sound_name, &pSoundAlias);
    if (!name) {
        name = sound_name;
    }

    sfxHandle = S_RegisterSound(name, pSoundAlias ? pSoundAlias->streamed : qfalse, force_load);

    if (!pSoundAlias) {
        S_StartSound(NULL, s_iListenerNumber, CHAN_MENU, sfxHandle, -1.0, -1.0, 1.0, -1.0, qfalse);
        return;
    }

    S_StartSound(
        0,
        s_iListenerNumber,
        CHAN_MENU,
        sfxHandle,
        pSoundAlias->volume + random() * pSoundAlias->volumeMod,
        -1.0,
        pSoundAlias->pitch + random() * pSoundAlias->pitchMod,
        pSoundAlias->maxDist,
        pSoundAlias->streamed
    );
}

/*
==============
S_StartLocalSoundChannel

Added in 2.0
Starts a local sound, with the specified channel
==============
*/
void S_StartLocalSoundChannel(const char *sound_name, qboolean force_load, soundChannel_t channel)
{
    sfxHandle_t      sfxHandle;
    const char      *name;
    AliasListNode_t *pSoundAlias;

    if (!s_bSoundStarted) {
        return;
    }

    if (!sound_name) {
        return;
    }

    name = Alias_FindRandom(sound_name, &pSoundAlias);
    if (!name) {
        name = sound_name;
    }

    sfxHandle = S_RegisterSound(name, pSoundAlias ? pSoundAlias->streamed : qfalse, force_load);

    if (!pSoundAlias) {
        S_StartSound(NULL, s_iListenerNumber, channel, sfxHandle, -1.0, -1.0, 1.0, -1.0, qfalse);
        return;
    }

    S_StartSound(
        0,
        s_iListenerNumber,
        channel,
        sfxHandle,
        pSoundAlias->volume + random() * pSoundAlias->volumeMod,
        -1.0,
        pSoundAlias->pitch + random() * pSoundAlias->pitchMod,
        pSoundAlias->maxDist,
        pSoundAlias->streamed
    );
}

/*
==============
S_StopAllSounds
==============
*/
void S_StopAllSounds(qboolean stop_music)
{
    if (!s_bSoundStarted) {
        return;
    }

    Com_Printf("------- S_StopAllSounds (%s) -------\n", stop_music ? "stop music" : "don't stop music");

    // Tell the driver to stop all sounds
    S_Driver_StopAllSounds(stop_music);

    Com_Printf("------- S_StopAllSounds Complete-------\n");
}

/*
==============
S_ClearLoopingSounds
==============
*/
void S_ClearLoopingSounds()
{
    if (!s_bSoundStarted) {
        return;
    }

    S_Driver_ClearLoopingSounds();
}

/*
==============
S_AddLoopingSound
==============
*/
void S_AddLoopingSound(
    const vec3_t origin,
    const vec3_t velocity,
    sfxHandle_t  sfxHandle,
    float        volume,
    float        min_dist,
    float        max_dist,
    float        pitch,
    int          flags
)
{
    if (!s_bSoundStarted) {
        return;
    }

    if (sfxHandle < 0 || sfxHandle >= s_numSfx) {
        Com_Error(ERR_DROP, "S_AddLoopingSound: handle %i out of range", sfxHandle);
    }

    if (!sfxHandle) {
        return;
    }

    S_Driver_AddLoopingSound(origin, velocity, sfxHandle, volume, min_dist, max_dist, pitch, flags);
}

/*
==============
S_Respatialize
==============
*/
void S_Respatialize(int entityNum, const vec3_t head, vec3_t axis[3])
{
    if (!s_bSoundStarted) {
        return;
    }

    S_Driver_Respatialize(entityNum, head, axis);
}

/*
==============
S_Update
==============
*/
void S_Update()
{
    if (!s_bSoundStarted) {
        return;
    }

    if (server_loading) {
        return;
    }

    S_Driver_Update();
}

/*
==============
S_StopSound
==============
*/
void S_StopSound(int entnum, int channel)
{
    if (!s_bSoundStarted) {
        return;
    }

    if (channel == CHAN_AUTO) {
        return;
    }

    S_Driver_StopSound(entnum, channel);
}

/*
==============
S_SetReverb
==============
*/
void S_SetReverb(int reverb_type, float reverb_level)
{
    if (!s_bSoundStarted) {
        return;
    }

    S_Driver_SetReverb(reverb_type, reverb_level);
}

/*
==============
S_Play
==============
*/
void S_Play()
{
    int  i;
    char name[MAX_QPATH];

    for (i = 1; i < Cmd_Argc(); i++) {
        if (strrchr(Cmd_Argv(i), '.')) {
            Q_strncpyz(name, Cmd_Argv(i), sizeof(name));
        } else {
            Com_sprintf(name, sizeof(name), "%s.wav", Cmd_Argv(1));
        }
    }

    S_StartLocalSound(name, qfalse);
}

/*
==============
S_SoundList
==============
*/
void S_SoundList()
{
    int    i;
    sfx_t *sfx;
    int    size;
    int    total = 0;

    for (i = 0; i < s_numSfx; i++) {
        sfx = &s_knownSfx[i];

        if (sfx->registration_sequence) {
            size = sfx->length * sfx->width;
            total += size;
            Com_Printf("(%2db) %6i : %s\n", sfx->width * 8, size, sfx->name);
        }
    }

    Com_Printf("Total resident: %i\n", total);
}

/*
==============
S_ClearSoundBuffer
==============
*/
void S_ClearSoundBuffer()
{
    // TODO: Remove once AL is fully implemented
}

#endif
