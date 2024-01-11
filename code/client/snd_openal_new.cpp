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
#    include "../server/server.h"

#    include <sys/stat.h>

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
static ALCcontext  *al_context_id             = NULL;
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

static void S_OPENAL_PlayMP3();
static void S_OPENAL_StopMP3();
static void S_OPENAL_Pitch();
static void S_OPENAL_reverb(int iChannel, int iReverbType, float fReverbLevel);

#    define alDieIfError() __alDieIfError(__FILE__, __LINE__)

static void __alDieIfError(const char *file, int line)
{
    ALint alErr = qalGetError();
    if (alErr) {
        Com_DPrintf("OpenAL error, %s, line %i: [%s].\n", file, line, qalGetString(alErr));
    }
}

static void S_OPENAL_NukeSource(ALuint *srcptr)
{
    ALuint source;

    source = *srcptr;
    if (!*srcptr) {
        return;
    }

    if (!qalIsSource(*srcptr)) {
        return;
    }

    alDieIfError();
    qalSourceStop(source);

    alDieIfError();
    qalSourcei(source, AL_BUFFER, 0);

    alDieIfError();
    qalDeleteSources(1, srcptr);

    alDieIfError();
    *srcptr = 0;
}

static void S_OPENAL_NukeBuffer(ALuint *bufptr)
{
    if (!*bufptr) {
        return;
    }

    if (!qalIsBuffer(*bufptr)) {
        return;
    }

    alDieIfError();
    qalDeleteBuffers(1, bufptr);

    alDieIfError();
    *bufptr = 0;
    // FIXME: unimplemented
}

static void S_OPENAL_NukeChannel(openal_channel *channel)
{
    if (!channel) {
        return;
    }

    S_OPENAL_NukeSource(&channel->source);
    S_OPENAL_NukeBuffer(&channel->buffer);

    if (channel->bufferdata) {
        delete[] channel->bufferdata;
    }
}

static void S_OPENAL_NukeContext()
{
    int i;

    Com_Printf("OpenAL: Destroying channels...\n");

    for (i = 0; i < MAX_OPENAL_CHANNELS; ++i) {
        S_OPENAL_NukeChannel(openal.channel[i]);
    }

    Com_Printf("OpenAL: Channels destroyed successfully.\n");

    for (i = 0; i < s_numSfx; i++) {
        S_OPENAL_NukeBuffer(&s_knownSfx[i].buffer);
    }

    S_OPENAL_NukeBuffer(&openal.movieSFX.buffer);

    if (al_context_id) {
        Com_Printf("OpenAL: Destroying context...\n");

        qalcSuspendContext(al_context_id);
        qalcMakeContextCurrent(NULL);
        qalcDestroyContext(al_context_id);
        al_context_id = NULL;

        Com_Printf("OpenAL: Context destroyed successfully.\n");
    }

    if (al_device) {
        Com_Printf("OpenAL: Closing device...\n");

        qalcCloseDevice(al_device);
        al_device = NULL;

        Com_Printf("OpenAL: Device closed successfully.\n");
    }
}

static bool S_OPENAL_InitContext()
{
    const char *dev;
    int         attrlist[6];

    dev = 0;
    if (s_openaldevice) {
        dev = s_openaldevice->string;
    }

    if (dev && !*dev) {
        dev = NULL;
    }

    Com_Printf("OpenAL: Opening device %s...\n", dev ? dev : "{default}");

    al_device = qalcOpenDevice(dev);
    if (!al_device) {
        Com_Printf("OpenAL: Could not open device\n");
        S_OPENAL_NukeContext();
        return false;
    }

    Com_Printf("OpenAL: Device opened successfully.\n");
    al_frequency = 22050;
    if (s_khz->integer == 11) {
        al_frequency = 11025;
    } else if (s_khz->integer == 44) {
        al_frequency = 44100;
    }

    attrlist[0] = 256;
    attrlist[1] = al_frequency;
    attrlist[2] = 258;
    attrlist[3] = 0;
    attrlist[4] = 0;
    attrlist[5] = 0;

    Com_Printf("OpenAL: Creating AL context...\n");
    al_context_id = qalcCreateContext(al_device, attrlist);
    if (!al_context_id) {
        Com_Printf("OpenAL: Could not create context\n");
        S_OPENAL_NukeContext();
        return false;
    }

    Com_Printf("OpenAL: Context created successfully.\n");

    qalcMakeContextCurrent(al_context_id);
    alDieIfError();

    Com_Printf("AL_VENDOR: %s\n", qalGetString(AL_VENDOR));
    alDieIfError();

    Com_Printf("AL_VERSION: %s\n", qalGetString(AL_VERSION));
    alDieIfError();

    Com_Printf("AL_RENDERER: %s\n", qalGetString(AL_RENDERER));
    alDieIfError();

    Com_Printf("AL_EXTENSIONS: %s\n", qalGetString(AL_EXTENSIONS));
    alDieIfError();

    qalDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
    alDieIfError();

    qalcProcessContext(al_context_id);
    alDieIfError();

    return true;
}

static bool S_OPENAL_InitExtensions()
{
    extensions_table_t extensions_table[4] = {
        "alutLoadMP3_LOKI",
        (void **)_alutLoadMP3_LOKI,
        true,
        "alReverbScale_LOKI",
        (void **)_alReverbScale_LOKI,
        true,
        "alReverbDelay_LOKI",
        (void **)_alReverbDelay_LOKI,
        true
    };
    extensions_table_t *i;

    for (i = extensions_table; i->funcname; ++i) {
        *i->funcptr = NULL;
    }

    for (i = extensions_table; i->funcname; ++i) {
        Com_Printf("AL extension: Looking up %ssymbol \"%s\"...", i->required ? "required " : "", i->funcname);

        *i->funcptr = qalGetProcAddress(i->funcname);
        if (!*i->funcptr) {
            Com_Printf("...not found! [%s]\n", qalGetError());
            if (i->required) {
                S_OPENAL_NukeContext();
                return false;
            }
            continue;
        }

        Com_Printf("...found.\n");
    }

    qalGetError();
    return true;
}

static bool S_OPENAL_InitChannel(int idx, openal_channel *chan)
{
    openal.channel[idx]   = chan;
    chan->vOrigin[2]      = 0.0;
    chan->vOrigin[1]      = 0.0;
    chan->vOrigin[0]      = 0.0;
    chan->fVolume         = 1.0;
    chan->fNewPitchMult   = 0.0;
    chan->fMinDist        = 0.0;
    chan->fMaxDist        = 0.0;
    chan->pSfx            = 0;
    chan->iEntNum         = 0;
    chan->iEntChannel     = 0;
    chan->iBaseRate       = 0;
    chan->iStartTime      = 0;
    chan->iTime           = 0;
    chan->iEndTime        = 0;
    chan->iFlags          = 0;
    chan->iPausedOffset   = 0;
    chan->source          = 0;
    chan->buffer          = 0;
    chan->bufferdata      = 0;
    chan->fading          = FADE_NONE;
    chan->fade_time       = 0;
    chan->fade_start_time = 0;
    chan->song_number     = 0;

    qalGenSources(1, &chan->source);
    alDieIfError();
    qalSourcei(chan->source, 514, 1);
    alDieIfError();

    return true;
}

qboolean S_OPENAL_Init()
{
    int i;

    if (al_initialized) {
        Com_DPrintf("S_OPENAL_Init(): Called when sound is already initialized!\n");
        return qtrue;
    }

    for (i = 0; i < MAX_OPENAL_CHANNELS; i++) {
        openal.channel[i] = NULL;
    }

    for (i = 0; i < MAX_OPENAL_LOOP_SOUNDS; i++) {
        openal.loop_sounds[i] = {};
    }

    s_milesdriver            = Cvar_Get("s_milesdriver", "auto", CVAR_SOUND_LATCH | CVAR_ARCHIVE);
    s_openaldevice           = Cvar_Get("s_openaldevice", "", CVAR_SOUND_LATCH | CVAR_ARCHIVE);
    s_reverb                 = Cvar_Get("s_reverb", "0", CVAR_SOUND_LATCH | CVAR_ARCHIVE);
    s_show_cpu               = Cvar_Get("s_show_cpu", "0", 0);
    s_show_num_active_sounds = Cvar_Get("s_show_num_active_sounds", "0", 0);
    s_show_sounds            = Cvar_Get("s_show_sounds", "0", 0);
    s_speaker_type           = Cvar_Get("s_speaker_type", "0", CVAR_ARCHIVE);
    s_obstruction_cal_time   = Cvar_Get("s_obstruction_cal_time", "500", CVAR_ARCHIVE);

    if (!Cvar_Get("s_initsound", "1", 0)->integer) {
        Com_Printf("OpenAL: s_initsound set to zero...disabling audio.\n");
        return true;
    }

    if (!S_OPENAL_InitContext()) {
        Com_Printf("OpenAL: initialization failed. No audio will play.\n");
        return false;
    }

    if (!S_OPENAL_InitExtensions()) {
        Com_Printf("OpenAL: A required extension is missing. No audio will play.\n");
        return false;
    }

    al_use_reverb = false;
    if (s_reverb->integer) {
        STUB_DESC("reenable reverb support later.");

        if (al_use_reverb) {
            S_OPENAL_SetReverb(s_iReverbType, s_fReverbLevel);
        } else {
            Com_Printf("OpenAL: No reverb support. Reverb is disabled.\n");
        }
    }

    al_current_volume = s_volume->value * s_volume->value;
    qalListenerf(AL_GAIN, al_current_volume);
    alDieIfError();

    for (i = 0; i < MAX_OPENAL_CHANNELS_3D; i++) {
        if (!S_OPENAL_InitChannel(i, &openal.chan_3D[i])) {
            return false;
        }
    }

    for (i = 0; i < MAX_OPENAL_CHANNELS_2D; i++) {
        if (!S_OPENAL_InitChannel(i, &openal.chan_2D[i])) {
            return false;
        }
    }

    for (i = 0; i < MAX_OPENAL_CHANNELS_2D_STREAM; i++) {
        if (!S_OPENAL_InitChannel(i, &openal.chan_2D_stream[i])) {
            return false;
        }
    }

    for (i = 0; i < MAX_OPENAL_SONGS; i++) {
        if (!S_OPENAL_InitChannel(i, &openal.chan_song[i])) {
            return false;
        }
    }

    if (!S_OPENAL_InitChannel(OPENAL_CHANNEL_MP3_ID, &openal.chan_mp3)) {
        return false;
    }

    if (!S_OPENAL_InitChannel(OPENAL_CHANNEL_TRIGGER_MUSIC_ID, &openal.chan_mp3)) {
        return false;
    }

    if (!S_OPENAL_InitChannel(OPENAL_CHANNEL_MOVIE_ID, &openal.chan_mp3)) {
        return false;
    }

    Cmd_AddCommand("playmp3", S_OPENAL_PlayMP3);
    Cmd_AddCommand("stopmp3", S_OPENAL_StopMP3);
    Cmd_AddCommand("loadsoundtrack", S_loadsoundtrack);
    Cmd_AddCommand("playsong", S_PlaySong);
    Cmd_AddCommand("pitch", S_OPENAL_Pitch);
    Cmd_AddCommand("tmstart", S_TriggeredMusic_Start);
    Cmd_AddCommand("tmstartloop", S_TriggeredMusic_StartLoop);
    Cmd_AddCommand("tmstop", S_TriggeredMusic_Stop);

    S_OPENAL_ClearLoopingSounds();
    load_sfx_info();
    s_bProvidersEmunerated = true;
    al_initialized         = true;

    return true;
}

void S_OPENAL_Shutdown()
{
    if (!al_initialized) {
        Com_DPrintf("S_OPENAL_Shutdown(): Called when sound is NOT initialized!\n");
        return;
    }

    S_OPENAL_StopAllSounds(true);

    Cmd_RemoveCommand("playmp3");
    Cmd_RemoveCommand("stopmp3");
    Cmd_RemoveCommand("loadsoundtrack");
    Cmd_RemoveCommand("playsong");
    Cmd_RemoveCommand("pitch");
    Cmd_RemoveCommand("tmstart");
    Cmd_RemoveCommand("tmstartloop");
    Cmd_RemoveCommand("tmstop");

    S_OPENAL_NukeContext();

    s_bProvidersEmunerated = false;
    al_initialized         = false;
}

void S_FadeSound(float fTime)
{
    Com_Printf("Called FadeSound with: %f\n", fTime);

    if (fTime > 0) {
        s_bFading        = true;
        s_fFadeStartTime = cls.realtime;
        s_fFadeVolume    = 1;
        s_fFadeStopTime  = cls.realtime + fTime;
    } else {
        s_fFadeVolume = 1;
        s_bFading     = false;
    }
}

float S_GetBaseVolume()
{
    return s_volume->value * s_fFadeVolume;
}

qboolean S_NeedFullRestart()
{
    return Cvar_Get("s_initsound", "1", 0)->integer != s_bLastInitSound;
}

void S_PrintInfo()
{
    const char *dev;

    Com_Printf("----- Sound Info -----\n");

    if (s_bSoundStarted) {
        dev = NULL;
        if (s_openaldevice) {
            dev = s_openaldevice->string;
        }
        if (!dev || !*dev) {
            dev = "{default}";
        }
        Com_Printf("device - %s\n", dev);
        if (al_use_reverb) {
            Com_Printf("reverb - ON\n");
        } else {
            Com_Printf("reverb - OFF\n");
        }
        Com_Printf("samplebits - 16\n");
        Com_Printf("speed - %d\n", al_frequency);
        if (s_loadas8bit->integer) {
            Com_Printf("Can NOT force all sounds to 8 bit in OpenAL, I think.\n");
        }
    } else {
        Com_Printf("sound system not started\n");
    }

    Com_Printf("----------------------\n");
}

static void S_DumpStatus(const char *pszChanName, int iChanNum, openal_channel *channel)
{
    sfx_t *sfx;
    ALint  status;

    sfx = channel->pSfx;

    qalGetSourceiv(channel->source, AL_SOURCE_STATE, &status);
    alDieIfError();

    if (status == AL_PLAYING || status == AL_PAUSED) {
        const char *pszMode = status == AL_PLAYING ? "playing" : "paused";

        if (sfx) {
            if (sfx != (sfx_t *)-16 && sfx->name[0]) {
                Com_Printf("%s channel %i - %s sfx %s\n", pszChanName, iChanNum, pszMode, sfx->name);
            } else {
                Com_Printf("%s channel %i - %s with nameless sfx\n", pszChanName, iChanNum, pszMode);
            }
        } else {
            Com_Printf("%s channel %i - %s with NULL sfx\n", pszChanName, iChanNum, pszMode);
        }
    }
}

void S_DumpInfo()
{
    int i;

    for (i = 0; i < MAX_OPENAL_CHANNELS_3D; i++) {
        S_DumpStatus("3D", i, openal.channel[i]);
    }

    for (i = 0; i < MAX_OPENAL_CHANNELS_2D; i++) {
        S_DumpStatus("2D", i, openal.channel[MAX_OPENAL_CHANNELS_3D + i]);
    }

    for (i = 0; i < MAX_OPENAL_CHANNELS_2D_STREAM; i++) {
        S_DumpStatus("2D stream", i, openal.channel[MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D + i]);
    }

    for (i = 0; i < MAX_OPENAL_MISC_CHANNELS; i++) {
        S_DumpStatus(
            "Misc",
            i,
            openal.channel[MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D + MAX_OPENAL_CHANNELS_2D_STREAM + i]
        );
    }
}

static void S_OPENAL_Pitch()
{
    Com_Printf("S_OPENAL_Pitch() needs to be implemented!\n");
}

static bool S_OPENAL_LoadMP3(const char *_path, openal_channel *chan)
{
    char   path[MAX_QPATH];
    FILE  *in;
    size_t len;
    size_t rc;
    bool   failed;

    chan->stop();

    qalSourcei(chan->source, AL_BUFFER, 0);
    alDieIfError();

    S_OPENAL_NukeBuffer(&chan->buffer);
    alDieIfError();

    Q_strncpyz(path, _path, sizeof(path));
    path[MAX_QPATH - 1] = 0;

    FS_CorrectCase(path);

    in = fopen(path, "rb");
    if (!in) {
        Com_DPrintf("Failed to open MP3 \"%s\" for playback\n", path);
        return false;
    }

    fseek(in, 0, SEEK_END);
    len = ftell(in);
    fseek(in, 0, SEEK_SET);

    chan->bufferdata = new ALubyte[len];
    rc               = fread(chan->bufferdata, 1, len, in);
    fclose(in);

    if (rc != len) {
        delete[] chan->bufferdata;
        chan->bufferdata = NULL;
        Com_DPrintf("Failed to read MP3 \"%s\" from disk\n", path);

        return false;
    }

    qalGenBuffers(1, &chan->buffer);
    alDieIfError();

    failed = !_alutLoadMP3_LOKI(chan->buffer, chan->bufferdata, rc);
    alDieIfError();

    delete[] chan->bufferdata;
    chan->bufferdata = NULL;

    if (failed) {
        Com_DPrintf("Failed to decode MP3 file \"%s\"\n", path);
        return false;
    }

    qalSourcei(chan->source, AL_BUFFER, chan->buffer);
    alDieIfError();

    chan->set_no_3d();

    return true;
}

static void S_OPENAL_PlayMP3()
{
    const char *path;

    if (Cmd_Argc() != 2) {
        Com_Printf("playmp3 <mp3 file>\n");
        return;
    }

    path = Cmd_Argv(1);
    if (!S_OPENAL_LoadMP3(path, &openal.chan_mp3)) {
        Com_Printf("Failed to play mp3 - %s\n", path);
        return;
    }

    openal.chan_mp3.play();
    Com_Printf("Playing mp3 - %s\n", path);
}

static void S_OPENAL_StopMP3()
{
    S_OPENAL_NukeChannel(&openal.chan_mp3);
}

void MUSIC_Pause()
{
    int i;

    for (i = 0; i < MAX_OPENAL_SONGS; i++) {
        openal.chan_song[i].pause();
    }
}

void MUSIC_Unpause()
{
    int i;

    for (i = 0; i < MAX_OPENAL_SONGS; i++) {
        if (openal.chan_song[i].is_paused()) {
            openal.chan_song[i].play();
        }
    }
}

void S_PauseSound()
{
    int i;

    if (!s_bSoundStarted) {
        return;
    }

    s_bSoundPaused = true;

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        openal_channel *pChannel = openal.channel[i];
        if (!pChannel) {
            continue;
        }

        if (pChannel->iEntChannel == CHAN_MENU) {
            continue;
        }

        if (!pChannel->is_playing()) {
            continue;
        }

        pChannel->pause();
    }

    if (openal.chan_mp3.is_playing()) {
        openal.chan_mp3.pause();
    }

    MUSIC_Pause();
    S_TriggeredMusic_Pause();
}

void S_UnpauseSound()
{
    int i;

    if (!s_bSoundStarted) {
        return;
    }

    s_bSoundPaused = true;

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        openal_channel *pChannel = openal.channel[i];
        if (!pChannel) {
            continue;
        }

        if (!pChannel->is_paused()) {
            continue;
        }

        pChannel->play();
    }

    if (openal.chan_mp3.is_paused()) {
        openal.chan_mp3.play();
    }

    MUSIC_Unpause();
    S_TriggeredMusic_Unpause();

    s_bSoundPaused = false;
}

static qboolean S_OPENAL_ShouldPlay(sfx_t *pSfx)
{
    if (sfx_infos[pSfx->sfx_info_index].max_number_playing <= 0) {
        return qtrue;
    }

    int iRemainingTimesToPlay;
    int i;

    iRemainingTimesToPlay = sfx_infos[pSfx->sfx_info_index].max_number_playing;

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        openal_channel *pChannel = openal.channel[i];
        if (!pChannel) {
            continue;
        }

        if (pChannel->pSfx == pSfx && pChannel->is_playing()) {
            iRemainingTimesToPlay--;
            if (!iRemainingTimesToPlay) {
                return qfalse;
            }
        }
    }

    return qtrue;
}

static qboolean S_OPENAL_ShouldStart(const vec3_t vOrigin, float fMinDist, float fMaxDist)
{
    vec3_t vDir;
    vec3_t vListenerOrigin;
    vec3_t alvec;

    if (!al_initialized) {
        return false;
    }

    qalGetListenerfv(AL_POSITION, alvec);

    VectorScale(alvec, 52.49f, vListenerOrigin);
    VectorSubtract(vOrigin, vListenerOrigin, vDir);

    return Square(fMaxDist) > VectorLengthSquared(vDir);
}

static int S_OPENAL_PickChannelBase(int iEntNum, int iEntChannel, int iFirstChannel, int iLastChannel)
{
    int             i;
    int             iBestChannel;
    openal_channel *pChannel;

    iBestChannel = -1;
    if (iEntNum != ENTITYNUM_NONE && iEntChannel) {
        bool bStoppedChannel = false;

        for (i = iFirstChannel; i <= iLastChannel; i++) {
            pChannel = openal.channel[i];
            if (!pChannel) {
                continue;
            }

            if (pChannel->is_free()) {
                iBestChannel = i;
                continue;
            }

            if (pChannel->iEntNum == iEntNum && pChannel->iEntChannel == iEntChannel) {
                pChannel->end_sample();
                bStoppedChannel = 1;
                iBestChannel    = i;
                break;
            }
        }

        if (!bStoppedChannel) {
            for (i = 0; i < iFirstChannel; ++i) {
                pChannel = openal.channel[i];
                if (!pChannel || pChannel->is_free()) {
                    continue;
                }

                if (pChannel->iEntNum == iEntNum && pChannel->iEntChannel == iEntChannel) {
                    bStoppedChannel = 1;
                    break;
                }
            }

            if (!bStoppedChannel) {
                i = iLastChannel;
                while (++i <= 95) {
                    pChannel = openal.channel[i];
                    if (!pChannel || pChannel->is_free()) {
                        continue;
                    }

                    if (pChannel->iEntNum == iEntNum && pChannel->iEntChannel == iEntChannel) {
                        break;
                    }
                }
            }
        }
    }

    if (iBestChannel < 0) {
        int iBestTime = 0x7FFFFFFF;

        for (i = iFirstChannel; i <= iLastChannel; i++) {
            pChannel = openal.channel[i];
            if (!pChannel) {
                continue;
            }

            if (pChannel->is_free()) {
                return i;
            }

            if (pChannel->iEntNum == s_iListenerNumber && iEntNum != pChannel->iEntNum) {
                continue;
            }

            if (pChannel->iEntChannel < iEntChannel
                || (pChannel->iEntChannel == iEntChannel && pChannel->iStartTime < iBestTime)) {
                iBestChannel = i;
                iEntChannel  = pChannel->iEntChannel;
                iBestTime    = pChannel->iStartTime;
            }
        }
    }

    return iBestChannel;
}

static int S_OPENAL_PickChannel3D(int iEntNum, int iEntChannel)
{
    return S_OPENAL_PickChannelBase(iEntNum, iEntChannel, 0, MAX_OPENAL_CHANNELS_3D - 1);
}

static int S_OPENAL_PickChannel2D(int iEntNum, int iEntChannel)
{
    return S_OPENAL_PickChannelBase(
        iEntNum, iEntChannel, MAX_OPENAL_CHANNELS_3D, MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D - 1
    );
}

static int S_OPENAL_PickChannel2DStreamed(int iEntNum, int iEntChannel)
{
    return S_OPENAL_PickChannelBase(
        iEntNum,
        iEntChannel,
        MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D,
        MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D + MAX_OPENAL_CHANNELS_2D_STREAM - 1
    );
}

void callbackServer(int entnum, int channel_number, const char *name)
{
    if (!com_sv_running->integer) {
        return;
    }

    SV_SoundCallback(entnum, channel_number, name);
}

static void S_OPENAL_Start2DSound(
    const vec3_t vOrigin,
    int          iEntNum,
    int          iEntChannel,
    sfx_t       *pSfx,
    float        fVolume,
    float        fMinDistance,
    float        fPitch,
    float        fMaxDistance
)
{
    int             iRealEntNum;
    int             iFreeChannel;
    openal_channel *pChannel;
    float           fRealVolume;
    bool            bSupportWaitTillSoundDone;

    iRealEntNum = iEntNum & ~S_FLAG_DO_CALLBACK;

    if (pSfx->iFlags & SFX_FLAG_NO_DATA) {
        iFreeChannel = S_OPENAL_PickChannel2DStreamed(iRealEntNum, iEntChannel);
    } else {
        iFreeChannel = S_OPENAL_PickChannel2D(iRealEntNum, iEntChannel);
    }

    if (iFreeChannel < 0) {
        Com_DPrintf(
            "Couldn't play %s sound '%s' for entity %i on channel %s\n",
            (pSfx->iFlags & SFX_FLAG_NO_DATA) ? "2Dstreamed" : "2D",
            pSfx->name,
            iEntNum & ~S_FLAG_DO_CALLBACK,
            S_ChannelNumToName(iEntChannel)
        );
        return;
    }

    if (iEntNum & S_FLAG_DO_CALLBACK) {
        callbackServer(iEntNum, iFreeChannel, pSfx->name);
    }

    pChannel = openal.channel[iFreeChannel];
    pChannel->force_free();

    if (fVolume < 0) {
        fVolume = 1;
    }

    fRealVolume       = S_GetBaseVolume() * fVolume;
    pChannel->fVolume = fVolume;

    if (pChannel->iEntChannel == CHAN_LOCAL) {
        pChannel->iFlags |= CHANNEL_FLAG_LOCAL_LISTENER;
    } else {
        pChannel->iFlags &= ~CHANNEL_FLAG_LOCAL_LISTENER;
    }
    pChannel->iFlags &= ~CHANNEL_FLAG_PAUSED;

    if (fMinDistance < 0.0) {
        fMinDistance = 200.0;
    }
    pChannel->fMinDist = fMinDistance;

    if (fMaxDistance < 0.0) {
        fMaxDistance = pChannel->fMinDist * 64;
    }
    pChannel->fMaxDist = fMaxDistance;

    pChannel->fNewPitchMult = fPitch;
    pChannel->pSfx          = pSfx;
    pChannel->iEntNum       = iRealEntNum;
    pChannel->iEntChannel   = iEntChannel;
    if (iEntNum == ENTITYNUM_NONE) {
        VectorClear(pChannel->vOrigin);
        pChannel->iFlags |= CHANNEL_FLAG_NO_ENTITY;
        pChannel->iEntNum = 0;

        if (vOrigin) {
            pChannel->vOrigin[0] = -vOrigin[0];
            pChannel->vOrigin[1] = vOrigin[2];
            pChannel->vOrigin[2] = -vOrigin[1];
        }
    } else {
        pChannel->iFlags &= ~CHANNEL_FLAG_NO_ENTITY;
        pChannel->iEntNum = iRealEntNum;
        if (vOrigin) {
            pChannel->vOrigin[0] = -vOrigin[0];
            pChannel->vOrigin[1] = vOrigin[2];
            pChannel->vOrigin[2] = -vOrigin[1];

            bSupportWaitTillSoundDone = cl.serverTime - 1 < 0;
            pChannel->iTime = cl.serverTime - 1;
            if (bSupportWaitTillSoundDone) {
                pChannel->iTime = 0;
            }
        } else {
            VectorClear(pChannel->vOrigin);
            pChannel->iTime = 0;
        }
    }

    pChannel->iStartTime = cl.serverTime;
    pChannel->iEndTime   = (int)(cl.serverTime + pSfx->time_length + 250.f);

    if (iFreeChannel > MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D) {
        // streamed
        if (!pChannel->set_sfx(pSfx)) {
            Com_DPrintf("Set stream error - %s\n", pSfx->name);
            return;
        }

        pChannel->iBaseRate = pChannel->sample_playback_rate();
        pChannel->set_no_3d();
        fRealVolume = fRealVolume * 84.f;
        pChannel->set_gain(fRealVolume);
        pChannel->play();
    } else {
        pChannel->stop();
        pChannel->set_no_3d();
        pChannel->set_sfx(pSfx);
        pChannel->set_gain(fRealVolume);
        pChannel->play();
    }

    if (s_show_sounds->integer > 0) {
        Com_DPrintf(
            "2D - %d (#%i) - %s (vol %f, mindist %f, maxdist %f)\n",
            cl.serverTime,
            iFreeChannel,
            pSfx->name,
            fVolume,
            fMinDistance,
            fMaxDistance
        );
    }
}

void S_OPENAL_StartSound(
    const vec3_t vOrigin,
    int          iEntNum,
    int          iEntChannel,
    sfxHandle_t  sfxHandle,
    float        fVolume,
    float        fMinDist,
    float        fPitch,
    float        fMaxDist,
    qboolean     bStreamed
)
{
    int iChannel;
    openal_channel* pChannel;
    sfx_info_t* pSfxInfo;
    sfx_t* pSfx;
    ALint state;
    bool bOnlyUpdate;
    bool bSupportWaitTillSoundDone;

    bOnlyUpdate = false;
    pSfx = &s_knownSfx[sfxHandle];
    if (bStreamed) {
        pSfx->iFlags |= SFX_FLAG_NO_DATA;
    }

    if (!S_OPENAL_ShouldPlay(pSfx))
    {
        Com_DPrintf("^~^~^ Not playing sound '%s'\n", pSfx->name);
        return;
    }

    if ((pSfx->iFlags & (SFX_FLAG_NO_OFFSET | SFX_FLAG_NO_DATA | SFX_FLAG_MP3)) || iEntChannel == CHAN_MENU || iEntChannel == CHAN_LOCAL)
    {
        S_OPENAL_Start2DSound(vOrigin, iEntNum, iEntChannel, pSfx, fVolume, fMinDist, fPitch, fMaxDist);
        return;
    }

    bSupportWaitTillSoundDone = (iEntNum & S_FLAG_DO_CALLBACK) != 0;
    iEntNum &= ~S_FLAG_DO_CALLBACK;

    pSfxInfo = &sfx_infos[pSfx->sfx_info_index];
    if (pSfx->iFlags & SFX_FLAG_NO_DATA) {
        Com_DPrintf("3D sounds not supported - couldn't play '%s'\n", pSfx->name);
        return;
    }

    iChannel = S_OPENAL_PickChannel3D(iEntNum, iEntChannel);
    if (iChannel < 0)
    {
        Com_DPrintf("Couldn't play %s sound '%s' for entity %i on channel %s\n", (pSfx->iFlags & SFX_FLAG_NO_DATA) ? "3Dstreamed" : "3D", pSfx->name, iEntNum, S_ChannelNumToName(iEntChannel));
        return;
    }

    if (bSupportWaitTillSoundDone) {
        callbackServer(iEntNum, iChannel, pSfx->name);
    }

    pChannel = &openal.chan_3D[iChannel];
    pChannel->fNewPitchMult = fPitch;
    pChannel->iEntChannel = iEntChannel;
    pChannel->iFlags &= ~(CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_LOCAL_LISTENER);
    state = pChannel->get_state();

    if (pChannel->iEntNum == iEntNum && (state == AL_PLAYING || state == AL_PAUSED) && pChannel->pSfx == pSfx) {
        bOnlyUpdate = true;
    } else {
        pChannel->stop();
        pChannel->iFlags &= ~(CHANNEL_FLAG_LOOPING | CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_LOCAL_LISTENER);
        if (!pChannel->set_sfx(pSfx)) {
            Com_DPrintf("Set sample error - %s\n", pSfx->name);
            return;
        }
    }

    if (fMinDist < 0.0)
    {
        fMinDist = 200.0;
        fMaxDist = 12800.0;
    }
    pChannel->fMinDist = fMinDist;
    pChannel->fMaxDist = fMaxDist;

    if (fVolume < 0.0) {
        fVolume = 1.0;
    }

    pChannel->fVolume = S_GetBaseVolume() * fVolume;
    pChannel->set_gain(pChannel->fVolume);

    if (s_show_sounds->integer > 0) {
        Com_DPrintf(
            "%d (#%i) - %s (vol %f, mindist %f, maxdist %f)\n",
            cl.serverTime,
            iChannel,
            pSfx->name,
            pChannel->fVolume,
            fMinDist,
            fMaxDist);
    }

    pChannel->set_velocity(0, 0, 0);
    if (iEntNum == ENTITYNUM_NONE) {
        if (vOrigin) {
            pChannel->vOrigin[0] = -vOrigin[0];
            pChannel->vOrigin[1] = vOrigin[2];
            pChannel->vOrigin[2] = -vOrigin[1];
        } else {
            //VectorClear(vOrigin);
            // Fixed in OPM
            //  Tiny mistake found in original where the vOrigin parameter is set to 0
            VectorClear(pChannel->vOrigin);
        }

        pChannel->set_position(pChannel->vOrigin[0] / 52.49f, pChannel->vOrigin[1] / 52.49f, pChannel->vOrigin[2] / 52.49f);
        pChannel->iFlags |= CHANNEL_FLAG_NO_ENTITY;
        pChannel->iEntNum = 0;
    } else {
        pChannel->iEntNum = iEntNum;
        if (vOrigin) {
            pChannel->vOrigin[0] = -*vOrigin;
            pChannel->vOrigin[1] = vOrigin[2];
            pChannel->vOrigin[2] = -vOrigin[1];
            bSupportWaitTillSoundDone = cl.serverTime - 1 < 0;
            pChannel->iTime = cl.serverTime - 1;
            if (bSupportWaitTillSoundDone) {
                pChannel->iTime = 0;
            }
        } else {
            VectorClear(pChannel->vOrigin);
            pChannel->iTime = 0;
        }
    }

    if (pSfxInfo->loop_start != -1) {
        pChannel->set_sample_loop_block(pSfxInfo->loop_start, pSfxInfo->loop_end);
        pChannel->set_sample_loop_count(0);

        pChannel->iFlags |= CHANNEL_FLAG_LOOPING;
        if (s_show_sounds->integer) {
            Com_DPrintf("loopblock - %d to %d\n", pSfxInfo->loop_start, pSfxInfo->loop_end);
        }
    } else {
        pChannel->set_sample_loop_count(1);
    }

    if (!bOnlyUpdate && S_OPENAL_ShouldStart(pChannel->vOrigin, pChannel->fMinDist, pChannel->fMaxDist))
    {
        pChannel->play();
        pChannel->iEndTime = cl.serverTime + (int)pChannel->pSfx->time_length + 250;
        pChannel->iBaseRate = pChannel->sample_playback_rate();
        pChannel->iStartTime = cl.serverTime;
    }
}

void S_OPENAL_AddLoopingSound(
    const vec3_t vOrigin,
    const vec3_t vVelocity,
    sfxHandle_t  sfxHandle,
    float        fVolume,
    float        fMinDist,
    float        fMaxDist,
    float        fPitch,
    int          iFlags
)
{
    int i;
    int iFreeLoopSound;
    sfx_t* pSfx;
    openal_loop_sound_t* pLoopSound;

    iFreeLoopSound = -1;
    pSfx = &s_knownSfx[sfxHandle];
    if (!pSfx) {
        return;
    }

    for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
        pLoopSound = &openal.loop_sounds[i];
        if (pLoopSound->pSfx == pSfx && !pLoopSound->bInUse) {
            iFreeLoopSound = i;
            break;
        }

    }

    if (iFreeLoopSound < 0) {
        for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
            pLoopSound = &openal.loop_sounds[i];
            if (!pLoopSound->pSfx && !pLoopSound->bInUse) {
                iFreeLoopSound = i;
                pLoopSound->bPlaying = false;
                break;
            }
        }
    }

    if (iFreeLoopSound < 0) {
        if (cls.realtime >= s_iNextLoopingWarning) {
            Com_DPrintf("Too many looping sounds\n");
            s_iNextLoopingWarning = cls.realtime + 1000;
        }
        return;
    }

    pLoopSound = &openal.loop_sounds[iFreeLoopSound];
    pLoopSound->vOrigin[0] = -vOrigin[0];
    pLoopSound->vOrigin[1] = vOrigin[2];
    pLoopSound->vOrigin[2] = -vOrigin[1];
    pLoopSound->vVelocity[0] = -vVelocity[0] / 52.49f / 500.f;
    pLoopSound->vVelocity[1] = vVelocity[2] / 52.49f / 500.f;
    pLoopSound->vVelocity[2] = -vVelocity[1] / 52.49f / 500.f;
    pLoopSound->pSfx = pSfx;
    pLoopSound->bInUse = true;
    pLoopSound->iStartTime = cls.realtime;
    pLoopSound->fBaseVolume = fVolume;
    pLoopSound->fMinDist = fMinDist;
    pLoopSound->fMaxDist = fMaxDist;
    pLoopSound->fPitch = fPitch;
    pLoopSound->iFlags = iFlags;
    pLoopSound->bCombine = VectorCompare(vVelocity, vec_zero) == 0;

    if (pLoopSound->bCombine) {
        for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
            if (openal.loop_sounds[i].pSfx == pSfx && openal.loop_sounds[i].bInUse) {
                pLoopSound->iStartTime = openal.loop_sounds[i].iStartTime;
                if (openal.loop_sounds[i].bPlaying) {
                    pLoopSound->bPlaying = true;
                    pLoopSound->iChannel = openal.loop_sounds[i].iChannel;
                }
            }
        }
    }
}

void S_OPENAL_StopLoopingSound(openal_loop_sound_t *pLoopSound)
{
    bool bMayStop;
    int i;
    openal_channel* pChannel;

    if (!pLoopSound->bPlaying) {
        return;
    }
    bMayStop = true;

    if (pLoopSound->bCombine) {
        for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
            if (openal.loop_sounds[i].pSfx == pLoopSound->pSfx && openal.loop_sounds[i].bInUse) {
                bMayStop = false;
                break;
            }
        }
    }

    if (bMayStop) {
        if (s_show_sounds->integer > 0) {
            Com_DPrintf("%d (#%i) - stopped loop - %s\n", cl.serverTime, pLoopSound->iChannel, openal.channel[pLoopSound->iChannel]->pSfx->name);
        }
        openal.channel[pLoopSound->iChannel]->force_free();
    }
    pLoopSound->pSfx = NULL;
    pLoopSound->bPlaying = false;

    if (pLoopSound->bCombine) {
        for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
            if (openal.loop_sounds[i].pSfx == pLoopSound->pSfx)
            {
                openal.loop_sounds[i].bPlaying = false;
                openal.loop_sounds[i].pSfx = NULL;
            }
        }
    }
}

void S_OPENAL_ClearLoopingSounds()
{
    int i;

    for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
        openal.loop_sounds[i].bInUse = false;
    }
}

void S_OPENAL_StopLoopingSounds()
{
    int i;

    for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
        openal.loop_sounds[i].bInUse = false;
        S_OPENAL_StopLoopingSound(&openal.loop_sounds[i]);
    }
}

void S_OPENAL_StopSound(int iEntNum, int iEntChannel)
{
    int i;

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        openal_channel* pChannel = openal.channel[i];
        if (!pChannel->is_free() && pChannel->iEntNum == iEntNum && pChannel->iEntChannel == iEntChannel) {
            pChannel->end_sample();
            break;
        }
    }
}

void S_OPENAL_StopAllSounds(qboolean bStopMusic)
{
    int i;

    if (!s_bSoundStarted) {
        return;
    }

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        openal_channel* pChannel = openal.channel[i];
        if (pChannel) {
            pChannel->force_free();
        }
    }

    if (bStopMusic) {
        MUSIC_FreeAllSongs();
        S_TriggeredMusic_Stop();
    }
    // FIXME: unimplemented
}

static int S_OPENAL_Start2DLoopSound(
    openal_loop_sound_t *pLoopSound, float fVolume, float fVolumeToPlay, float fMinDistance, const vec3_t vLoopOrigin
)
{
    int iChannel;
    int iSoundOFfset;
    openal_channel* pChannel;

    if (pLoopSound->pSfx->iFlags & SFX_FLAG_NO_DATA) {
        iChannel = S_OPENAL_PickChannel2DStreamed(0, 0);
    }
    else {
        iChannel = S_OPENAL_PickChannel2D(0, 0);
    }

    if (iChannel < 0) {
        Com_DPrintf("Could not find a free 2D sound channel\n");
        return iChannel;
    }

    pChannel = openal.channel[iChannel];
    pChannel->force_free();
    pChannel->fVolume = fVolume;

    if (pLoopSound->iFlags & LOOPSOUND_FLAG_NO_PAN) {
        pChannel->iFlags |= CHANNEL_FLAG_LOCAL_LISTENER;
    } else {
        pChannel->iFlags &= ~CHANNEL_FLAG_LOCAL_LISTENER;
    }
    pChannel->fMinDist = fMinDistance;

    if (!pChannel->set_sfx(pLoopSound->pSfx)) {
        Com_DPrintf("Set sample error\n");
        pChannel->iFlags &= ~CHANNEL_FLAG_PLAYABLE;
        return -1;
    }

    pChannel->iEntNum = 0;
    pChannel->iEntChannel = 0;
    pChannel->pSfx = pLoopSound->pSfx;
    pChannel->iFlags |= CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_NO_ENTITY;
    pChannel->iBaseRate = pChannel->sample_playback_rate();
    VectorCopy(vLoopOrigin, pChannel->vOrigin);
    pChannel->set_sample_offset((int)(pLoopSound->pSfx->info.width
        * pLoopSound->pSfx->info.rate
        * (float)(cls.realtime - pLoopSound->iStartTime)
        / 1000.0)
        % pLoopSound->pSfx->length);

    pChannel->set_sample_loop_count(0);
    pChannel->fVolume = fVolumeToPlay;
    pChannel->set_gain(fVolumeToPlay);
    pChannel->start_sample();
    if (s_show_sounds->integer > 0) {
        Com_DPrintf("%d (#%i) - %s (vol %f)\n", cl.serverTime, pLoopSound->iChannel, pLoopSound->pSfx->name, fVolume);
    }

    return iChannel;
}

static int S_OPENAL_Start3DLoopSound(
    openal_loop_sound_t *pLoopSound,
    float                fVolumeToPlay,
    float                fMinDistance,
    float                fMaxDistance,
    const vec3_t         vLoopOrigin,
    const vec3_t         vListenerOrigin
)
{
    int iChannel;
    vec3_t vDir;
    int iSoundOffset;
    openal_channel* pChan3D;

    if (pLoopSound->pSfx->iFlags & SFX_FLAG_NO_DATA) {
        return -1;
    }

    iChannel = S_OPENAL_PickChannel3D(0, 0);
    if (iChannel < 0)
    {
        Com_DPrintf("Could not find a free channel\n");
        return iChannel;
    }

    pChan3D = &openal.chan_3D[iChannel];
    pChan3D->force_free();
    pChan3D->iEntNum = 0;
    pChan3D->iEntChannel = 0;
    pChan3D->set_3d();

    if (!pChan3D->set_sfx(pLoopSound->pSfx)) {
        Com_DPrintf("Set sample error - %s\n", pLoopSound->pSfx->name);
        return -1;
    }

    pChan3D->set_position(vLoopOrigin[0] / 52.49f, pLoopSound->vVelocity[1] / 52.49f, vLoopOrigin[2] / 52.49f);
    pChan3D->set_velocity(pLoopSound->vVelocity[0] / 52.49f, vLoopOrigin[1] / 52.49f, pLoopSound->vVelocity[2] / 52.49f);
    pChan3D->pSfx = pLoopSound->pSfx;
    pChan3D->iFlags |= CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_NO_ENTITY;
    pChan3D->iBaseRate = pChan3D->sample_playback_rate();

    iSoundOffset = (int)((int)pLoopSound->pSfx->info.width
        * pLoopSound->pSfx->info.rate
        * (float)(cls.realtime - pLoopSound->iStartTime)
        / 1000.0)
        % pLoopSound->pSfx->length;
    pChan3D->set_sample_offset(iSoundOffset);
    pChan3D->set_sample_loop_count(0);
    pChan3D->fVolume = fVolumeToPlay;
    pChan3D->set_gain(fVolumeToPlay);
    pChan3D->play();

    S_OPENAL_reverb(iChannel, s_iReverbType, s_fReverbLevel);

    return iChannel;
}

static bool S_OPENAL_UpdateLoopSound(
    openal_loop_sound_t *pLoopSound,
    float                fVolumeToPlay,
    float                fMinDistance,
    float                fMaxDistance,
    const vec3_t         vListenerOrigin,
    const vec3_t         vTempAxis,
    const vec3_t         vLoopOrigin
)
{
    // FIXME: unimplemented
    return false;
}

void S_OPENAL_AddLoopSounds(const vec3_t vTempAxis)
{
    // FIXME: unimplemented
}

void S_OPENAL_Respatialize(int iEntNum, const vec3_t vHeadPos, const vec3_t vAxis[3])
{
    // FIXME: unimplemented
}

static int S_OPENAL_SpatializeStereoSound(const vec3_t listener_origin, const vec3_t listener_left, const vec3_t origin)
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
