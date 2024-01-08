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

#pragma once

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int   format;
    float rate;

    float width;
    int   channels;
    int   samples;

    int dataofs;
    int datasize;
} wavinfo_t;

typedef struct sfx_s {
    int iFlags;

    int   length;
    int   width;
    byte *data;
    char  name[64];

    int   registration_sequence;
    int   sfx_info_index;
    float time_length;

    wavinfo_t    info;
    unsigned int buffer;
} sfx_t;

typedef struct {
    unsigned short wFormatTag;
    unsigned short nChannels;
    unsigned int   nSamplesPerSec;
    unsigned int   nAvgBytesPerSec;
    unsigned short nBlockAlign;
    unsigned short wBitsPerSample;
    unsigned short cbSize;
} wavinfo_x_t;

typedef struct {
    char name[64];

    int loop_start;
    int loop_end;

    int   max_number_playing;
    float max_factor;
} sfx_info_t;

typedef struct {
    vec3_t   position;
    vec3_t   velocity;
    int      time;
    qboolean use_listener;
} s_entity_t;

typedef struct {
    char alias[32];
    char path[64];

    int   mood_num;
    int   flags;
    float volume;
    float fadetime;

    int current_pos;
    int current_state;
} song_t;

typedef struct {
    int  iFlags;
    char szName[64];
} sfxsavegame_t;

typedef struct {
    qboolean      bPlaying;
    int           iStatus;
    sfxsavegame_t sfx;

    int    iEntNum;
    int    iEntChannel;
    vec3_t vOrigin;
    float  fVolume;
    int    iBaseRate;
    float  fNewPitchMult;
    float  fMinDist;
    float  fMaxDist;

    int iStartTime;
    int iTime;
    int iNextCheckObstructionTime;
    int iEndTime;

    int iFlags;
    int iOffset;
    int iLoopCount;
} channelbasesavegame_t;

typedef struct {
    channelbasesavegame_t Channels[96];
} soundsystemsavegame_t;

enum sfx_flags_t {
    SFX_FLAG_DEFAULT_SOUND = 1
};

// The current sound driver.
// Currently OPENAL
#define SOUND_DRIVER OPENAL

//
// snd_dma_new.cpp
//

sfx_t *S_FindName(const char *name, int sequenceNumber);
void   S_DefaultSound(sfx_t *sfx);

void S_LoadData(soundsystemsavegame_t *pSave);
void S_SaveData(soundsystemsavegame_t *pSave);
void S_ClearSoundBuffer();

//
// snd_mem.c
//
qboolean S_LoadSound(const char *fileName, sfx_t *sfx, int streamed, qboolean force_load);

#define S_StopAllSounds2 S_StopAllSounds

//
// Driver-specific functions
//
#define S_Call_SndDriver(driver, func)  S_##driver##_##func
#define S_Call_SndDriverX(driver, func) S_Call_SndDriver(driver, func)

#define S_Driver_Init                   S_Call_SndDriverX(SOUND_DRIVER, Init)
#define S_Driver_Shutdown               S_Call_SndDriverX(SOUND_DRIVER, Shutdown)
#define S_Driver_StartSound             S_Call_SndDriverX(SOUND_DRIVER, StartSound)
#define S_Driver_AddLoopingSound        S_Call_SndDriverX(SOUND_DRIVER, AddLoopingSound)
#define S_Driver_ClearLoopingSounds     S_Call_SndDriverX(SOUND_DRIVER, ClearLoopingSounds)
#define S_Driver_StopSound              S_Call_SndDriverX(SOUND_DRIVER, StopSound)
#define S_Driver_StopAllSounds          S_Call_SndDriverX(SOUND_DRIVER, StopAllSounds)
#define S_Driver_Respatialize           S_Call_SndDriverX(SOUND_DRIVER, Respatialize)
#define S_Driver_SetReverb              S_Call_SndDriverX(SOUND_DRIVER, SetReverb)
#define S_Driver_Update                 S_Call_SndDriverX(SOUND_DRIVER, Update)

void S_PrintInfo();
void S_DumpInfo();

extern cvar_t *s_show_sounds;

#ifdef __cplusplus
}
#endif
