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

#include "qal.h"

#undef OPENAL

typedef int          S32;
typedef unsigned int U32;

#define MAX_OPENAL_CHANNELS_3D        32
#define MAX_OPENAL_CHANNELS_2D        32
#define MAX_OPENAL_CHANNELS_2D_STREAM 32
#define MAX_OPENAL_POSITION_CHANNELS  (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D + MAX_OPENAL_CHANNELS_2D_STREAM)
#define MAX_OPENAL_SONGS              2
#define MAX_OPENAL_MISC_CHANNELS      3
#define MAX_OPENAL_CHANNELS                                                                             \
    (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D + MAX_OPENAL_CHANNELS_2D_STREAM + MAX_OPENAL_SONGS \
     + MAX_OPENAL_MISC_CHANNELS)
#define MAX_OPENAL_LOOP_SOUNDS 64

#define OPENAL_CHANNEL_MP3_ID \
    (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D + MAX_OPENAL_CHANNELS_2D_STREAM + MAX_OPENAL_SONGS)
#define OPENAL_CHANNEL_TRIGGER_MUSIC_ID (OPENAL_CHANNEL_MP3_ID + 1)
#define OPENAL_CHANNEL_MOVIE_ID         (OPENAL_CHANNEL_TRIGGER_MUSIC_ID + 1)

typedef enum {
    FADE_NONE,
    FADE_IN,
    FADE_OUT
} fade_t;

typedef struct {
    vec3_t vOrigin;
    vec3_t vRelativeOrigin;
    vec3_t vVelocity;

    sfx_t   *pSfx;
    qboolean bPlaying;

    int   iChannel;
    float fVolume;
    float fPitch;

    int      iStartTime;
    qboolean bInUse;
    qboolean bCombine;

    float fBaseVolume;
    float fMinDist;
    float fMaxDist;
    int   iFlags;
} openal_loop_sound_t;

struct openal_channel {
    sfx_t *pSfx;
    int    iEntNum;
    int    iEntChannel;
    vec3_t vOrigin;

    float fVolume;
    int   iBaseRate;
    float fNewPitchMult;
    float fMinDist;
    float fMaxDist;

    int iStartTime;
    int iTime;
    int iEndTime;
    int iFlags;
    int iPausedOffset;

    int song_number;

    fade_t fading;
    int    fade_time;
    int    fade_start_time;

    ALuint   source;
    ALuint   buffer;
    ALubyte *bufferdata;

public:
    void play();
    void stop();
    void pause();

    void  set_no_3d();
    void  set_3d();
    ALint get_state();

    void set_gain(float gain);
    void set_velocity(float v0, float v1, float v2);
    void set_position(float v0, float v1, float v2);

    bool is_free();
    bool is_paused();
    bool is_playing();

    void force_free();
    bool set_sfx(sfx_t *pSfx);

    void start_sample();
    void stop_sample();
    void resume_sample();
    void end_sample();

    void set_sample_pan(S32 pan);
    void set_sample_playback_rate(S32 pan);
    S32  sample_playback_rate();

    S32  sample_volume();
    U32  sample_offset();
    U32  sample_ms_offset();
    U32  sample_loop_count();
    void set_sample_offset(U32 offset);
    void set_sample_ms_offset(U32 offset);
    void set_sample_loop_count(S32 count);
    void set_sample_loop_block(S32 start_offset, S32 end_offset);

    U32 sample_status();
};

struct openal_internal_t {
    openal_channel chan_3D[MAX_OPENAL_CHANNELS_3D];
    openal_channel chan_2D[MAX_OPENAL_CHANNELS_2D];
    openal_channel chan_2D_stream[MAX_OPENAL_CHANNELS_2D_STREAM];
    openal_channel chan_song[MAX_OPENAL_SONGS];
    openal_channel chan_mp3;
    openal_channel chan_trig_music;
    openal_channel chan_movie;

    // Pointers to channels
    openal_channel *channel[MAX_OPENAL_CHANNELS];

    openal_loop_sound_t loop_sounds[MAX_OPENAL_LOOP_SOUNDS];

    openal_channel movieChannel;
    sfx_t          movieSFX;

    char tm_filename[MAX_RES_NAME];
    int  tm_loopcount;
};

#ifdef __cplusplus
extern "C" {
#endif

qboolean S_OPENAL_Init();
void     S_OPENAL_Shutdown();
void     S_OPENAL_StartSound(
        const vec3_t vOrigin,
        int          iEntNum,
        int          iEntChannel,
        sfxHandle_t  sfxHandle,
        float        fVolume,
        float        fMinDist,
        float        fPitch,
        float        fMaxDist,
        qboolean     bStreamed
    );
void S_OPENAL_AddLoopingSound(
    const vec3_t vOrigin,
    const vec3_t vVelocity,
    sfxHandle_t  sfxHandle,
    float        fVolume,
    float        fMinDist,
    float        fMaxDist,
    float        fPitch,
    int          iFlags
);
void S_OPENAL_ClearLoopingSounds();
void S_OPENAL_StopSound(int iEntNum, int iEntChannel);
void S_OPENAL_StopAllSounds(qboolean bStopMusic);
void S_OPENAL_Respatialize(int iEntNum, const vec3_t vHeadPos, const vec3_t vAxis[3]);
void S_OPENAL_SetReverb(int iType, float fLevel);
void S_OPENAL_Update();

#ifdef __cplusplus
}
#endif
