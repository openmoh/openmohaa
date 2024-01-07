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

typedef struct {
    char  *funcname;
    void **funcptr;
    bool   required;
} extensions_table_t;

#    define MAX_MUSIC_SONGS 16

static int   s_iNextLoopingWarning = 0;
static int   s_iReverbType         = 0;
static float s_fReverbLevel        = 0;
static bool  s_bReverbChanged      = false;
static bool  s_bFading             = false;
static float s_fFadeVolume         = 1.f;

cvar_t *s_milesdriver;
cvar_t *s_openaldevice;
cvar_t *s_reverb;
cvar_t *s_show_cpu;
cvar_t *s_show_num_active_sounds;
cvar_t *s_show_sounds;
cvar_t *s_speaker_type;
cvar_t *s_obstruction_cal_time;
cvar_t *s_lastSoundTime;

static float reverb_table[] = {
    0.5f,   0.25f,        0.417f, 0.653f,      0.208f,      0.5f,   0.403f, 0.5f,   0.5f,
    0.361f, 0.5f,         0.153f, 0.361f,      0.44400001f, 0.25f,  0.111f, 0.111f, 0.19400001f,
    1.0f,   0.097000003f, 0.208f, 0.65200001f, 1.0f,        0.875f, 0.139f, 0.486f,
};

static vec3_t       vec_zero                  = {0, 0, 0};
int                 s_iNumMilesAudioProviders = 0;
bool                s_bProvidersEmunerated    = false;
static bool         al_initialized            = false;
static bool         al_use_reverb             = false;
static float        al_current_volume         = 0;
static unsigned int al_frequency              = 22050;
static void        *al_context_id             = NULL;
static ALCdevice   *al_device                 = NULL;

static ALboolean (*_alutLoadMP3_LOKI)(unsigned int buffer, const byte *data, int length);
static void (*_alReverbScale_LOKI)();
static void (*_alReverbDelay_LOKI)();

static qboolean music_active            = qfalse;
int             music_current_mood      = 0;
int             music_fallback_mood     = 0;
float           old_music_volume        = 1.f;
float           music_volume            = 1.f;
float           new_music_volume        = 1.f;
float           music_volume_fade_time  = 0;
long int        music_volume_start_time = 0;
int             music_volume_direction  = 0;
int             music_volume_changed    = 0;
int             music_loaded            = 0;
int             music_numsongs          = 0;
int             music_currentsong       = 0;

song_t            music_songs[MAX_MUSIC_SONGS];
openal_internal_t openal;
static float      s_fFadeStartTime;
static float      s_fFadeStopTime;
static char       current_soundtrack[128];

static void __alDieIfError(char *file)
{
    // FIXME: unimplemented
}

static void S_OPENAL_NukeSource(ALuint *srcptr)
{
    // FIXME: unimplemented
}

static void S_OPENAL_NukeBuffer(ALuint *bufptr)
{
    // FIXME: unimplemented
}

static void S_OPENAL_NukeChannel(openal_channel *channel)
{
    // FIXME: unimplemented
}

static void S_OPENAL_NukeContext()
{
    // FIXME: unimplemented
}

static bool S_OPENAL_InitContext()
{
    // FIXME: unimplemented
    return false;
}

static bool S_OPENAL_InitExtensions()
{
    // FIXME: unimplemented
    return false;
}

static bool S_OPENAL_InitChannel(int idx, openal_channel *chan)
{
    // FIXME: unimplemented
    return false;
}

qboolean S_OPENAL_Init()
{
    // FIXME: unimplemented
    return qfalse;
}

void S_OPENAL_Shutdown()
{
    // FIXME: unimplemented
}

void S_FadeSound(float fTime)
{
    // FIXME: unimplemented
}

float S_GetBaseVolume()
{
    // FIXME: unimplemented
    return 0;
}

qboolean S_NeedFullRestart()
{
    // FIXME: unimplemented
    return qfalse;
}

void S_PrintInfo()
{
    // FIXME: unimplemented
}

static void S_DumpStatus(const char *pszChanName, int iChanNum, openal_channel *channel)
{
    // FIXME: unimplemented
}

void S_DumpInfo()
{
    // FIXME: unimplemented
}

static void S_OPENAL_Pitch()
{
    // FIXME: unimplemented
}

static bool S_OPENAL_LoadMP3(const char *_path, openal_channel *chan)
{
    // FIXME: unimplemented
    return false;
}

static void S_OPENAL_PlayMP3()
{
    // FIXME: unimplemented
}

static void S_OPENAL_StopMP3()
{
    // FIXME: unimplemented
}

void MUSIC_Pause()
{
    // FIXME: unimplemented
}

void MUSIC_Unpause()
{
    // FIXME: unimplemented
}

void S_PauseSound()
{
    // FIXME: unimplemented
}

void S_UnpauseSound()
{
    // FIXME: unimplemented
}

static qboolean S_OPENAL_ShouldPlay(sfx_t *pSfx)
{
    // FIXME: unimplemented
    return qfalse;
}

static qboolean S_OPENAL_ShouldStart(float *vOrigin, float fMinDist, float fMaxDist)
{
    // FIXME: unimplemented
    return qfalse;
}

static int S_OPENAL_PickChannelBase(int iEntNum, int iEntChannel, int iFirstChannel, int iLastChannel)
{
    // FIXME: unimplemented
    return 0;
}

static int S_OPENAL_PickChannel3D(int iEntNum, int iEntChannel)
{
    // FIXME: unimplemented
    return 0;
}

static int S_OPENAL_PickChannel2D(int iEntNum, int iEntChannel)
{
    // FIXME: unimplemented
    return 0;
}

static int S_OPENAL_PickChannel2DStreamed(int iEntNum, int iEntChannel)
{
    // FIXME: unimplemented
    return 0;
}

void callbackServer(int entnum, int channel_number, const char *name)
{
    // FIXME: unimplemented
}

static void S_OPENAL_Start2DSound(
    float *vOrigin,
    int    iEntNum,
    int    iEntChannel,
    sfx_t *pSfx,
    float  fVolume,
    float  fMinDistance,
    float  fPitch,
    float  fMaxDistance
)
{
    // FIXME: unimplemented
}

void S_OPENAL_StartSound(
    float      *vOrigin,
    int         iEntNum,
    int         iEntChannel,
    sfxHandle_t sfxHandle,
    float       fVolume,
    float       fMinDist,
    float       fPitch,
    float       fMaxDist,
    qboolean    bStreamed
)
{
    // FIXME: unimplemented
}

void S_OPENAL_AddLoopingSound(
    float      *vOrigin,
    float      *vVelocity,
    sfxHandle_t sfxHandle,
    float       fVolume,
    float       fMinDist,
    float       fMaxDist,
    float       fPitch,
    int         iFlags
)
{
    // FIXME: unimplemented
}

void S_OPENAL_StopLoopingSound(openal_loop_sound_t *pLoopSound)
{
    // FIXME: unimplemented
}

void S_OPENAL_ClearLoopingSounds()
{
    // FIXME: unimplemented
}

void S_OPENAL_StopLoopingSounds()
{
    // FIXME: unimplemented
}

void S_OPENAL_StopSound(int iEntNum)
{
    // FIXME: unimplemented
}

void S_OPENAL_StopAllSounds(qboolean bStopMusic)
{
    // FIXME: unimplemented
}

static int S_OPENAL_Start2DLoopSound(
    openal_loop_sound_t *pLoopSound, float fVolume, float fVolumeToPlay, float fMinDistance, float *vLoopOrigin
)
{
    // FIXME: unimplemented
    return 0;
}

static int S_OPENAL_Start3DLoopSound(
    openal_loop_sound_t *pLoopSound,
    float                fVolumeToPlay,
    float                fMinDistance,
    float                fMaxDistance,
    float               *vLoopOrigin,
    float               *vListenerOrigin
)
{
    // FIXME: unimplemented
    return 0;
}

static bool S_OPENAL_UpdateLoopSound(
    openal_loop_sound_t *pLoopSound,
    float                fVolumeToPlay,
    float                fMinDistance,
    float                fMaxDistance,
    float               *vListenerOrigin,
    float               *vTempAxis,
    float               *vLoopOrigin
)
{
    // FIXME: unimplemented
    return false;
}

void S_OPENAL_AddLoopSounds(float *vTempAxis)
{
    // FIXME: unimplemented
}

void S_OPENAL_Respatialize(int iEntNum, float *vHeadPos, float (*vAxis)[3])
{
    // FIXME: unimplemented
}

static int S_OPENAL_SpatializeStereoSound(float *listener_origin, float *listener_left, float *origin)
{
    // FIXME: unimplemented
    return 0;
}

static void S_OPENAL_reverb(int iChannel, int iReverbType, float fReverbLevel)
{
    // FIXME: unimplemented
}

void S_OPENAL_SetReverb(int iType, float fLevel)
{
    // FIXME: unimplemented
}

void S_OPENAL_Update()
{
    // FIXME: unimplemented
}

qboolean S_IsSoundPlaying(int channel_number, const char *sfxName)
{
    // FIXME: unimplemented
    return qfalse;
}

static void S_StoreBase(channelbasesavegame_t *pBase, openal_channel *pChannel)
{
    // FIXME: unimplemented
}

static void
S_StartSoundFromBase(channelbasesavegame_t *pBase, openal_channel *pChannel, sfx_t *pSfx, bool bStartUnpaused)
{
    // FIXME: unimplemented
}

static void S_LoadBase(channelbasesavegame_t *pBase, openal_channel *pChannel, bool bStartUnpaused)
{
    // FIXME: unimplemented
}

void S_SaveData(soundsystemsavegame_t *pSave)
{
    // FIXME: unimplemented
}

void S_ReLoad(soundsystemsavegame_t *pSave)
{
    // FIXME: unimplemented
}

static void S_InitBase(channelbasesavegame_t *pBase)
{
    // FIXME: unimplemented
}

void S_LoadData(soundsystemsavegame_t *pSave)
{
    // FIXME: unimplemented
}

void openal_channel::set_velocity(float v0, float v1, float v2)
{
    // FIXME: unimplemented
}

void openal_channel::set_position(float v0, float v1, float v2)
{
    // FIXME: unimplemented
}

void openal_channel::set_gain(float gain)
{
    // FIXME: unimplemented
}

void openal_channel::set_no_3d()
{
    // FIXME: unimplemented
}

void openal_channel::set_3d()
{
    // FIXME: unimplemented
}

void openal_channel::play()
{
    // FIXME: unimplemented
}

void openal_channel::pause()
{
    // FIXME: unimplemented
}

void openal_channel::stop()
{
    // FIXME: unimplemented
}

ALint openal_channel::get_state()
{
    // FIXME: unimplemented
    return 0;
}

bool openal_channel::is_free()
{
    // FIXME: unimplemented
    return false;
}

bool openal_channel::is_paused()
{
    // FIXME: unimplemented
    return false;
}

bool openal_channel::is_playing()
{
    // FIXME: unimplemented
    return false;
}

void openal_channel::force_free()
{
    // FIXME: unimplemented
}

bool openal_channel::set_sfx(sfx_t *pSfx)
{
    // FIXME: unimplemented
    return false;
}

void openal_channel::start_sample()
{
    // FIXME: unimplemented
}

void openal_channel::stop_sample()
{
    // FIXME: unimplemented
}

void openal_channel::resume_sample()
{
    // FIXME: unimplemented
}

void openal_channel::end_sample()
{
    // FIXME: unimplemented
}

void openal_channel::set_sample_pan(S32 pan)
{
    // FIXME: unimplemented
}

void openal_channel::set_sample_playback_rate(S32 rate)
{
    // FIXME: unimplemented
}

S32 openal_channel::sample_playback_rate()
{
    // FIXME: unimplemented
    return 0;
}

S32 openal_channel::sample_volume()
{
    // FIXME: unimplemented
    return 0;
}

U32 openal_channel::sample_offset()
{
    // FIXME: unimplemented
    return 0;
}

U32 openal_channel::sample_ms_offset()
{
    // FIXME: unimplemented
    return 0;
}

U32 openal_channel::sample_loop_count()
{
    // FIXME: unimplemented
    return 0;
}

void openal_channel::set_sample_offset(U32 offset)
{
    // FIXME: unimplemented
}

void openal_channel::set_sample_ms_offset(U32 offset)
{
    // FIXME: unimplemented
}

void openal_channel::set_sample_loop_count(S32 count)
{
    // FIXME: unimplemented
}

void openal_channel::set_sample_loop_block(S32 start_offset, S32 end_offset)
{
    // FIXME: unimplemented
}

U32 openal_channel::sample_status()
{
    // FIXME: unimplemented
    return 0;
}

qboolean MUSIC_LoadSoundtrackFile(const char *filename)
{
    // FIXME: unimplemented
    return qfalse;
}

qboolean MUSIC_SongValid(const char *mood)
{
    // FIXME: unimplemented
    return qfalse;
}

qboolean MUSIC_Loaded()
{
    // FIXME: unimplemented
    return qfalse;
}

void Music_Update()
{
    // FIXME: unimplemented
}

void MUSIC_SongEnded()
{
    // FIXME: unimplemented
}

void MUSIC_NewSoundtrack(const char *name)
{
    // FIXME: unimplemented
}

void MUSIC_UpdateMood(int current, int fallback)
{
    // FIXME: unimplemented
}

void MUSIC_UpdateVolume(float volume, float fade_time)
{
    // FIXME: unimplemented
}

void MUSIC_StopAllSongs()
{
    // FIXME: unimplemented
}

void MUSIC_FreeAllSongs()
{
    // FIXME: unimplemented
}

qboolean MUSIC_Playing()
{
    // FIXME: unimplemented
    return qfalse;
}

int MUSIC_FindSong(const char *name)
{
    // FIXME: unimplemented
    return 0;
}

void S_loadsoundtrack()
{
    // FIXME: unimplemented
}

const char *S_CurrentSoundtrack()
{
    // FIXME: unimplemented
    return NULL;
}

void S_PlaySong()
{
    // FIXME: unimplemented
}

int MUSIC_CurrentSongChannel()
{
    // FIXME: unimplemented
    return 0;
}

void MUSIC_StopChannel(int channel_number)
{
    // FIXME: unimplemented
}

qboolean MUSIC_PlaySong(const char *alias)
{
    // FIXME: unimplemented
    return qfalse;
}

void MUSIC_UpdateMusicVolumes()
{
    // FIXME: unimplemented
}

void MUSIC_CheckForStoppedSongs()
{
    // FIXME: unimplemented
}

void S_TriggeredMusic_SetupHandle(const char *pszName, int iLoopCount, int iOffset, qboolean autostart)
{
    // FIXME: unimplemented
}

void S_TriggeredMusic_Start()
{
    // FIXME: unimplemented
}

void S_TriggeredMusic_StartLoop()
{
    // FIXME: unimplemented
}

void S_TriggeredMusic_Stop()
{
    // FIXME: unimplemented
}

void S_TriggeredMusic_Pause()
{
    // FIXME: unimplemented
}

void S_TriggeredMusic_Unpause()
{
    // FIXME: unimplemented
}

void S_TriggeredMusic_PlayIntroMusic()
{
    // FIXME: unimplemented
}

void S_StopMovieAudio()
{
    // FIXME: unimplemented
}

void S_SetupMovieAudio(const char *pszMovieName)
{
    // FIXME: unimplemented
}

int S_CurrentMoviePosition()
{
    // FIXME: unimplemented
    return 0;
}

#endif
