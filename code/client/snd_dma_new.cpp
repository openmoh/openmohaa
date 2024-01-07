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

#include "snd_local.h"
#include "snd_openal_new.h"

typedef struct {
    vec3_t origin;
    vec3_t velocity;
    sfx_t *sfx;
    int    mergeFrame;
} loopSound_t;

#    define MAX_SFX         1400
#    define MAX_SFX_INFOS   1000
#    define MAX_LOOP_SOUNDS 64

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

void S_SoundInfo_f()
{
    // FIXME: unimplemented
}

void S_SoundDump_f()
{
    // FIXME: unimplemented
}

void S_Init(qboolean full_startup)
{
    // FIXME: unimplemented
}

void S_Shutdown(qboolean full_shutdown)
{
    // FIXME: unimplemented
}

qboolean S_NameExists(const char *name)
{
    // FIXME: unimplemented
    return qfalse;
}

sfx_t *S_FindName(const char *name, int sequenceNumber)
{
    // FIXME: unimplemented
    return NULL;
}

void S_BeginRegistration()
{
    // FIXME: unimplemented
}

void S_DefaultSound(sfx_t *sfx)
{
    // FIXME: unimplemented
}

qboolean S_IsSoundRegistered(const char *name)
{
    // FIXME: unimplemented
    return qfalse;
}

sfxHandle_t S_RegisterSound(const char *name, int streamed, qboolean force_load)
{
    // FIXME: unimplemented
    return 0;
}

float S_GetSoundTime(sfxHandle_t handle)
{
    // FIXME: unimplemented
    return 0;
}

void S_EndRegistration()
{
    // FIXME: unimplemented
}

void S_UpdateEntity(int entityNum, const vec3_t origin, const vec3_t vel, qboolean use_listener)
{
    // FIXME: unimplemented
}

void S_SetGlobalAmbientVolumeLevel(float volume)
{
    // FIXME: unimplemented
}

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
    // FIXME: unimplemented
}

void S_StartLocalSound(const char *sound_name, qboolean force_load)
{
    // FIXME: unimplemented
}

void S_StartLocalSoundChannel(const char *sound_name, qboolean force_load, soundChannel_t channel)
{
    // FIXME: unimplemented
}

void S_StopAllSounds(qboolean stop_music)
{
    // FIXME: unimplemented
}

void S_ClearLoopingSounds()
{
    // FIXME: unimplemented
}

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
    // FIXME: unimplemented
}

void S_Respatialize(int entityNum, const vec3_t head, vec3_t axis[3])
{
    // FIXME: unimplemented
}

void S_Update()
{
    // FIXME: unimplemented
}

void S_StopSound(int entnum, int channel)
{
    // FIXME: unimplemented
}

void S_SetReverb(int reverb_type, float reverb_level)
{
    // FIXME: unimplemented
}

void S_Play()
{
    // FIXME: unimplemented
}

void S_SoundList()
{
    // FIXME: unimplemented
}

void S_ClearSoundBuffer()
{
    // TODO: Remove once AL is fully implemented
}

#endif
