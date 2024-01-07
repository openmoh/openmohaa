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

sfx_t *S_FindName(const char *name, int sequenceNumber);
void   S_DefaultSound(sfx_t *sfx);

void S_LoadData(soundsystemsavegame_t *pSave);
void S_SaveData(soundsystemsavegame_t *pSave);
void S_ClearSoundBuffer();

#define S_StopAllSounds2 S_StopAllSounds

#ifdef __cplusplus
}
#endif
