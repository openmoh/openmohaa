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
#    include "snd_codec.h"

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
// Added in OPM
cvar_t *s_openaldriver;

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
static int
S_OPENAL_SpatializeStereoSound(const vec3_t listener_origin, const vec3_t listener_left, const vec3_t origin);
static void S_OPENAL_reverb(int iChannel, int iReverbType, float fReverbLevel);
static bool S_OPENAL_LoadMP3_Codec(const char *_path, openal_channel *chan);

#    define alDieIfError() __alDieIfError(__FILE__, __LINE__)

#    if defined(_WIN64)
#        define ALDRIVER_DEFAULT "OpenAL64.dll"
#    elif defined(_WIN32)
#        define ALDRIVER_DEFAULT "OpenAL32.dll"
#    elif defined(__APPLE__)
#        define ALDRIVER_DEFAULT "/System/Library/Frameworks/OpenAL.framework/OpenAL"
#    elif defined(__OpenBSD__)
#        define ALDRIVER_DEFAULT "libopenal.so"
#    else
#        define ALDRIVER_DEFAULT "libopenal.so.1"
#    endif

/*
==============
__alDieIfError
==============
*/
static void __alDieIfError(const char *file, int line)
{
    ALint alErr = qalGetError();
    if (alErr) {
        if (s_show_sounds->integer) {
            Com_DPrintf("OpenAL error, %s, line %i: [%s].\n", file, line, qalGetString(alErr));
        }
    }
}

/*
==============
S_OPENAL_NukeSource
==============
*/
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

/*
==============
S_OPENAL_NukeBuffer
==============
*/
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
}

/*
==============
S_OPENAL_NukeChannel
==============
*/
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

/*
==============
S_OPENAL_NukeContext
==============
*/
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

/*
==============
S_OPENAL_InitContext
==============
*/
static bool S_OPENAL_InitContext()
{
    const char *dev;
    int         attrlist[6];

    dev = NULL;
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

/*
==============
S_OPENAL_InitExtensions
==============
*/
static bool S_OPENAL_InitExtensions()
{
    Com_Printf("AL extensions ignored\n");
    return true;

    extensions_table_t extensions_table[4] = {
        "alutLoadMP3_LOKI",
        (void **)&_alutLoadMP3_LOKI,
        true,
        "alReverbScale_LOKI",
        (void **)&_alReverbScale_LOKI,
        true,
        "alReverbDelay_LOKI",
        (void **)&_alReverbDelay_LOKI,
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

/*
==============
S_OPENAL_InitChannel
==============
*/
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
    qalSourcei(chan->source, AL_SOURCE_RELATIVE, true);
    alDieIfError();

    return true;
}

/*
==============
S_OPENAL_Init
==============
*/
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
    //
    // Added in OPM
    //  Initialize the AL driver DLL
    s_openaldriver = Cvar_Get("s_openaldriver", ALDRIVER_DEFAULT, CVAR_ARCHIVE | CVAR_LATCH | CVAR_PROTECTED);

    if (!QAL_Init(s_openaldriver->string)) {
        Com_Printf("Failed to load library: \"%s\".\n", s_openaldriver->string);
        if (!Q_stricmp(s_openaldriver->string, ALDRIVER_DEFAULT) || !QAL_Init(ALDRIVER_DEFAULT)) {
            return qfalse;
        }
    }

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
        if (!S_OPENAL_InitChannel(i + MAX_OPENAL_CHANNELS_3D, &openal.chan_2D[i])) {
            return false;
        }
    }

    for (i = 0; i < MAX_OPENAL_CHANNELS_2D_STREAM; i++) {
        if (!S_OPENAL_InitChannel(i + MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D, &openal.chan_2D_stream[i])) {
            return false;
        }
    }

    for (i = 0; i < MAX_OPENAL_SONGS; i++) {
        if (!S_OPENAL_InitChannel(
                i + MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D + MAX_OPENAL_CHANNELS_2D_STREAM,
                &openal.chan_song[i]
            )) {
            return false;
        }
    }

    if (!S_OPENAL_InitChannel(OPENAL_CHANNEL_MP3_ID, &openal.chan_mp3)) {
        return false;
    }

    if (!S_OPENAL_InitChannel(OPENAL_CHANNEL_TRIGGER_MUSIC_ID, &openal.chan_trig_music)) {
        return false;
    }

    if (!S_OPENAL_InitChannel(OPENAL_CHANNEL_MOVIE_ID, &openal.chan_movie)) {
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

    // Added in OPM
    S_CodecInit();

    return true;
}

/*
==============
S_OPENAL_Shutdown
==============
*/
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

/*
==============
S_FadeSound
==============
*/
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

/*
==============
S_GetBaseVolume
==============
*/
float S_GetBaseVolume()
{
    return s_volume->value * s_fFadeVolume;
}

/*
==============
S_NeedFullRestart
==============
*/
qboolean S_NeedFullRestart()
{
    return Cvar_Get("s_initsound", "1", 0)->integer != s_bLastInitSound;
}

/*
==============
S_PrintInfo
==============
*/
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

/*
==============
S_DumpStatus
==============
*/
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

/*
==============
S_DumpInfo
==============
*/
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

/*
==============
S_OPENAL_Pitch
==============
*/
static void S_OPENAL_Pitch()
{
    Com_Printf("S_OPENAL_Pitch() needs to be implemented!\n");
}

/*
==============
S_OPENAL_LoadMP3
==============
*/
static bool S_OPENAL_LoadMP3(const char *_path, openal_channel *chan)
{
    return S_OPENAL_LoadMP3_Codec(_path, chan);

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

/*
==============
S_OPENAL_PlayMP3
==============
*/
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

/*
==============
S_OPENAL_StopMP3
==============
*/
static void S_OPENAL_StopMP3()
{
    S_OPENAL_NukeChannel(&openal.chan_mp3);
}

/*
==============
MUSIC_Pause
==============
*/
void MUSIC_Pause()
{
    int i;

    for (i = 0; i < MAX_OPENAL_SONGS; i++) {
        openal.chan_song[i].pause();
    }
}

/*
==============
MUSIC_Unpause
==============
*/
void MUSIC_Unpause()
{
    int i;

    for (i = 0; i < MAX_OPENAL_SONGS; i++) {
        if (openal.chan_song[i].is_paused()) {
            openal.chan_song[i].play();
        }
    }
}

/*
==============
S_PauseSound
==============
*/
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

/*
==============
S_UnpauseSound
==============
*/
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

/*
==============
S_OPENAL_ShouldPlay
==============
*/
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

/*
==============
S_OPENAL_ShouldStart
==============
*/
static qboolean S_OPENAL_ShouldStart(const vec3_t vOrigin, float fMinDist, float fMaxDist)
{
    vec3_t vDir;
    vec3_t vListenerOrigin;
    vec3_t alvec;

    if (!al_initialized) {
        return false;
    }

    qalGetListenerfv(AL_POSITION, alvec);

    //VectorScale(alvec, 52.49f, vListenerOrigin);
    VectorCopy(alvec, vListenerOrigin);
    VectorSubtract(vOrigin, vListenerOrigin, vDir);

    return Square(fMaxDist) > VectorLengthSquared(vDir);
}

/*
==============
S_OPENAL_PickChannelBase
==============
*/
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

/*
==============
S_OPENAL_PickChannel3D
==============
*/
static int S_OPENAL_PickChannel3D(int iEntNum, int iEntChannel)
{
    return S_OPENAL_PickChannelBase(iEntNum, iEntChannel, 0, MAX_OPENAL_CHANNELS_3D - 1);
}

/*
==============
S_OPENAL_PickChannel2D
==============
*/
static int S_OPENAL_PickChannel2D(int iEntNum, int iEntChannel)
{
    return S_OPENAL_PickChannelBase(
        iEntNum, iEntChannel, MAX_OPENAL_CHANNELS_3D, MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D - 1
    );
}

/*
==============
S_OPENAL_PickChannel2DStreamed
==============
*/
static int S_OPENAL_PickChannel2DStreamed(int iEntNum, int iEntChannel)
{
    return S_OPENAL_PickChannelBase(
        iEntNum,
        iEntChannel,
        MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D,
        MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D + MAX_OPENAL_CHANNELS_2D_STREAM - 1
    );
}

/*
==============
callbackServer
==============
*/
void callbackServer(int entnum, int channel_number, const char *name)
{
    if (!com_sv_running->integer) {
        return;
    }

    SV_SoundCallback(entnum, channel_number, name);
}

/*
==============
S_OPENAL_Start2DSound
==============
*/
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
            iRealEntNum,
            S_ChannelNumToName(iEntChannel)
        );
        return;
    }

    if (iEntNum & S_FLAG_DO_CALLBACK) {
        callbackServer(iRealEntNum, iFreeChannel, pSfx->name);
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
    if (iRealEntNum == ENTITYNUM_NONE) {
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
            pChannel->iTime           = cl.serverTime - 1;
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

/*
==============
S_OPENAL_StartSound
==============
*/
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
    int             iChannel;
    openal_channel *pChannel;
    sfx_info_t     *pSfxInfo;
    sfx_t          *pSfx;
    ALint           state;
    bool            bOnlyUpdate;
    bool            bSupportWaitTillSoundDone;

    bOnlyUpdate = false;
    pSfx        = &s_knownSfx[sfxHandle];
    if (bStreamed) {
        pSfx->iFlags |= SFX_FLAG_NO_DATA;
    }

    if (!S_OPENAL_ShouldPlay(pSfx)) {
        Com_DPrintf("^~^~^ Not playing sound '%s'\n", pSfx->name);
        return;
    }

    if ((pSfx->iFlags & (SFX_FLAG_NO_OFFSET | SFX_FLAG_NO_DATA | SFX_FLAG_MP3)) || iEntChannel == CHAN_MENU
        || iEntChannel == CHAN_LOCAL) {
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
    if (iChannel < 0) {
        Com_DPrintf(
            "Couldn't play %s sound '%s' for entity %i on channel %s\n",
            (pSfx->iFlags & SFX_FLAG_NO_DATA) ? "3Dstreamed" : "3D",
            pSfx->name,
            iEntNum,
            S_ChannelNumToName(iEntChannel)
        );
        return;
    }

    if (bSupportWaitTillSoundDone) {
        callbackServer(iEntNum, iChannel, pSfx->name);
    }

    pChannel                = &openal.chan_3D[iChannel];
    pChannel->fNewPitchMult = fPitch;
    pChannel->iEntChannel   = iEntChannel;
    pChannel->iFlags &= ~(CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_LOCAL_LISTENER);
    state = pChannel->get_state();
    // Fixed in OPM
    //  Setup the channel for 3D
    pChannel->set_3d();

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

    if (fMinDist < 0.0) {
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
            fMaxDist
        );
    }

    pChannel->set_velocity(0, 0, 0);
    if (iEntNum == ENTITYNUM_NONE) {
        if (vOrigin) {
            //pChannel->vOrigin[0] = -vOrigin[0];
            //pChannel->vOrigin[1] = vOrigin[2];
            //pChannel->vOrigin[2] = -vOrigin[1];
            VectorCopy(vOrigin, pChannel->vOrigin);
        } else {
            //VectorClear(vOrigin);
            // Fixed in OPM
            //  Tiny mistake found in original where the vOrigin parameter is set to 0
            VectorClear(pChannel->vOrigin);
        }

        /*
        pChannel->set_position(
            pChannel->vOrigin[0] / 52.49f, pChannel->vOrigin[1] / 52.49f, pChannel->vOrigin[2] / 52.49f
        );
        */
        pChannel->set_position(pChannel->vOrigin[0], pChannel->vOrigin[1], pChannel->vOrigin[2]);
        pChannel->iFlags |= CHANNEL_FLAG_NO_ENTITY;
        pChannel->iEntNum = 0;
    } else {
        pChannel->iEntNum = iEntNum;
        if (vOrigin) {
            //pChannel->vOrigin[0] = -vOrigin[0];
            //pChannel->vOrigin[1] = vOrigin[2];
            //pChannel->vOrigin[2] = -vOrigin[1];
            VectorCopy(vOrigin, pChannel->vOrigin);

            bSupportWaitTillSoundDone = cl.serverTime - 1 < 0;
            pChannel->iTime           = cl.serverTime - 1;
            if (bSupportWaitTillSoundDone) {
                pChannel->iTime = 0;
            }
        } else {
            VectorClear(pChannel->vOrigin);
            pChannel->iTime = 0;
        }
        pChannel->set_position(pChannel->vOrigin[0], pChannel->vOrigin[1], pChannel->vOrigin[2]);
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

    if (!bOnlyUpdate && S_OPENAL_ShouldStart(pChannel->vOrigin, pChannel->fMinDist, pChannel->fMaxDist)) {
        pChannel->play();
        pChannel->iEndTime   = cl.serverTime + (int)pChannel->pSfx->time_length + 250;
        pChannel->iBaseRate  = pChannel->sample_playback_rate();
        pChannel->iStartTime = cl.serverTime;
    }
}

/*
==============
S_OPENAL_AddLoopingSound
==============
*/
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
    int                  i;
    int                  iFreeLoopSound;
    sfx_t               *pSfx;
    openal_loop_sound_t *pLoopSound;

    iFreeLoopSound = -1;
    pSfx           = &s_knownSfx[sfxHandle];
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
                iFreeLoopSound       = i;
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

    pLoopSound             = &openal.loop_sounds[iFreeLoopSound];
    pLoopSound->vOrigin[0] = -vOrigin[0];
    pLoopSound->vOrigin[1] = vOrigin[2];
    pLoopSound->vOrigin[2] = -vOrigin[1];
    //pLoopSound->vVelocity[0] = -vVelocity[0] / 52.49f / 500.f;
    //pLoopSound->vVelocity[1] = vVelocity[2] / 52.49f / 500.f;
    //pLoopSound->vVelocity[2] = -vVelocity[1] / 52.49f / 500.f;
    VectorCopy(vVelocity, pLoopSound->vVelocity);
    pLoopSound->pSfx        = pSfx;
    pLoopSound->bInUse      = true;
    pLoopSound->iStartTime  = cls.realtime;
    pLoopSound->fBaseVolume = fVolume;
    pLoopSound->fMinDist    = fMinDist;
    pLoopSound->fMaxDist    = fMaxDist;
    pLoopSound->fPitch      = fPitch;
    pLoopSound->iFlags      = iFlags;
    pLoopSound->bCombine    = VectorCompare(vVelocity, vec_zero) == 0;

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

/*
==============
S_OPENAL_StopLoopingSound
==============
*/
void S_OPENAL_StopLoopingSound(openal_loop_sound_t *pLoopSound)
{
    bool            bMayStop;
    int             i;
    openal_channel *pChannel;

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
            Com_DPrintf(
                "%d (#%i) - stopped loop - %s\n",
                cl.serverTime,
                pLoopSound->iChannel,
                openal.channel[pLoopSound->iChannel]->pSfx->name
            );
        }
        openal.channel[pLoopSound->iChannel]->force_free();
    }
    pLoopSound->pSfx     = NULL;
    pLoopSound->bPlaying = false;

    if (pLoopSound->bCombine) {
        for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
            if (openal.loop_sounds[i].pSfx == pLoopSound->pSfx) {
                openal.loop_sounds[i].bPlaying = false;
                openal.loop_sounds[i].pSfx     = NULL;
            }
        }
    }
}

/*
==============
S_OPENAL_ClearLoopingSounds
==============
*/
void S_OPENAL_ClearLoopingSounds()
{
    int i;

    for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
        openal.loop_sounds[i].bInUse = false;
    }
}

/*
==============
S_OPENAL_StopLoopingSounds
==============
*/
void S_OPENAL_StopLoopingSounds()
{
    int i;

    for (i = 0; i < (MAX_OPENAL_CHANNELS_3D + MAX_OPENAL_CHANNELS_2D); i++) {
        openal.loop_sounds[i].bInUse = false;
        S_OPENAL_StopLoopingSound(&openal.loop_sounds[i]);
    }
}

/*
==============
S_OPENAL_StopSound
==============
*/
void S_OPENAL_StopSound(int iEntNum, int iEntChannel)
{
    int i;

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        openal_channel *pChannel = openal.channel[i];
        if (!pChannel->is_free() && pChannel->iEntNum == iEntNum && pChannel->iEntChannel == iEntChannel) {
            pChannel->end_sample();
            break;
        }
    }
}

/*
==============
S_OPENAL_StopAllSounds
==============
*/
void S_OPENAL_StopAllSounds(qboolean bStopMusic)
{
    int i;

    if (!s_bSoundStarted) {
        return;
    }

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        openal_channel *pChannel = openal.channel[i];
        if (pChannel) {
            pChannel->force_free();
        }
    }

    if (bStopMusic) {
        MUSIC_FreeAllSongs();
        S_TriggeredMusic_Stop();
    }
}

/*
==============
S_OPENAL_Start2DLoopSound
==============
*/
static int S_OPENAL_Start2DLoopSound(
    openal_loop_sound_t *pLoopSound, float fVolume, float fVolumeToPlay, float fMinDistance, const vec3_t vLoopOrigin
)
{
    int             iChannel;
    int             iSoundOFfset;
    openal_channel *pChannel;

    if (pLoopSound->pSfx->iFlags & SFX_FLAG_NO_DATA) {
        iChannel = S_OPENAL_PickChannel2DStreamed(0, 0);
    } else {
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

    pChannel->iEntNum     = 0;
    pChannel->iEntChannel = 0;
    pChannel->pSfx        = pLoopSound->pSfx;
    pChannel->iFlags |= CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_NO_ENTITY;
    pChannel->iBaseRate = pChannel->sample_playback_rate();
    VectorCopy(vLoopOrigin, pChannel->vOrigin);
    pChannel->set_sample_offset(
        (int)(pLoopSound->pSfx->info.width * pLoopSound->pSfx->info.rate
              * (float)(cls.realtime - pLoopSound->iStartTime) / 1000.0)
        % pLoopSound->pSfx->length
    );

    pChannel->set_sample_loop_count(0);
    pChannel->fVolume = fVolumeToPlay;
    pChannel->set_gain(fVolumeToPlay);
    pChannel->start_sample();
    if (s_show_sounds->integer > 0) {
        Com_DPrintf("%d (#%i) - %s (vol %f)\n", cl.serverTime, pLoopSound->iChannel, pLoopSound->pSfx->name, fVolume);
    }

    return iChannel;
}

/*
==============
S_OPENAL_Start3DLoopSound
==============
*/
static int S_OPENAL_Start3DLoopSound(
    openal_loop_sound_t *pLoopSound,
    float                fVolumeToPlay,
    float                fMinDistance,
    float                fMaxDistance,
    const vec3_t         vLoopOrigin,
    const vec3_t         vListenerOrigin
)
{
    int             iChannel;
    vec3_t          vDir;
    int             iSoundOffset;
    openal_channel *pChan3D;

    if (pLoopSound->pSfx->iFlags & SFX_FLAG_NO_DATA) {
        return -1;
    }

    iChannel = S_OPENAL_PickChannel3D(0, 0);
    if (iChannel < 0) {
        Com_DPrintf("Could not find a free channel\n");
        return iChannel;
    }

    pChan3D = &openal.chan_3D[iChannel];
    pChan3D->force_free();
    pChan3D->iEntNum     = 0;
    pChan3D->iEntChannel = 0;
    pChan3D->set_3d();

    if (!pChan3D->set_sfx(pLoopSound->pSfx)) {
        Com_DPrintf("Set sample error - %s\n", pLoopSound->pSfx->name);
        return -1;
    }

    /*
    pChan3D->set_position(vLoopOrigin[0] / 52.49f, pLoopSound->vVelocity[1] / 52.49f, vLoopOrigin[2] / 52.49f);
    pChan3D->set_velocity(
        pLoopSound->vVelocity[0] / 52.49f, vLoopOrigin[1] / 52.49f, pLoopSound->vVelocity[2] / 52.49f
    );
    */
    pChan3D->set_position(vLoopOrigin[0], pLoopSound->vVelocity[1], vLoopOrigin[2]);
    pChan3D->set_velocity(pLoopSound->vVelocity[0], vLoopOrigin[1], pLoopSound->vVelocity[2]);
    pChan3D->pSfx = pLoopSound->pSfx;
    pChan3D->iFlags |= CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_NO_ENTITY;
    pChan3D->iBaseRate = pChan3D->sample_playback_rate();

    iSoundOffset = (int)((int)pLoopSound->pSfx->info.width * pLoopSound->pSfx->info.rate
                         * (float)(cls.realtime - pLoopSound->iStartTime) / 1000.0)
                 % pLoopSound->pSfx->length;
    pChan3D->set_sample_offset(iSoundOffset);
    pChan3D->set_sample_loop_count(0);
    pChan3D->fVolume = fVolumeToPlay;
    pChan3D->set_gain(fVolumeToPlay);
    pChan3D->play();

    S_OPENAL_reverb(iChannel, s_iReverbType, s_fReverbLevel);

    return iChannel;
}

/*
==============
S_OPENAL_UpdateLoopSound
==============
*/
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
    openal_channel *pChannel;
    float           fVolume;
    float           fMaxVolume;
    vec3_t          vDir;
    float           fDistance;

    pChannel = openal.channel[pLoopSound->iChannel];
    if (!pChannel) {
        return false;
    }

    if (pChannel->pSfx != pLoopSound->pSfx) {
        pLoopSound->bPlaying = 0;
        return false;
    }

    pChannel->iStartTime = cl.serverTime;

    if (pLoopSound->pSfx->iFlags & (SFX_FLAG_NO_OFFSET | SFX_FLAG_NO_DATA | SFX_FLAG_MP3)
        || (pLoopSound->iFlags & LOOPSOUND_FLAG_NO_PAN)) {
        vec3_t vOrigin;
        int    iPan;

        pChannel->fVolume = fVolumeToPlay / 84.0;
        fVolume           = S_GetBaseVolume() * pChannel->fVolume;
        fMaxVolume        = fVolume;

        if (pLoopSound->iFlags & LOOPSOUND_FLAG_NO_PAN) {
            // Center the sound
            iPan = 64;
        } else {
            VectorCopy(vLoopOrigin, vOrigin);
            iPan = S_OPENAL_SpatializeStereoSound(vListenerOrigin, vTempAxis, vOrigin);

            VectorSubtract(vListenerOrigin, vOrigin, vDir);
            // Clamp the volume by distance
            fDistance = VectorLength(vDir);
            if (pChannel->fMinDist >= fDistance) {
                fVolume = fMaxVolume;
            } else {
                fVolume = pChannel->fMinDist / fDistance * fMaxVolume;
            }
        }

        pChannel->set_gain(fVolume);
        pChannel->set_sample_pan(iPan);
    } else {
        //pChannel->set_position(vLoopOrigin[0] / 52.49f, vLoopOrigin[1] / 52.49f, vLoopOrigin[2] / 52.49f);
        pChannel->set_position(vLoopOrigin[0], vLoopOrigin[1], vLoopOrigin[2]);
        pChannel->fVolume = fVolumeToPlay;
        pChannel->set_gain(fVolumeToPlay);
    }

    if (s_bReverbChanged) {
        // Make sure to update the reverb
        S_OPENAL_reverb(pLoopSound->iChannel, s_iReverbType, s_fReverbLevel);
    }

    return true;
}

/*
==============
S_OPENAL_AddLoopSounds
==============
*/
void S_OPENAL_AddLoopSounds(const vec3_t vTempAxis)
{
    int                  i, j;
    static int           iLoopFrame = 0;
    float                fDistance;
    int                  iChannel;
    vec3_t               vListenerOrigin;
    vec3_t               vLoopOrigin;
    openal_loop_sound_t *pLoopSound;
    float                fTotalVolume;
    float                fVolumeToPlay;
    float                fMinDistance, fMaxDistance;
    float                fVolume;
    float                fPitch;
    float                fMaxVolume, fMaxFactor;
    openal_channel      *pChannel;
    bool                 bAlreadyAdded[MAX_OPENAL_LOOP_SOUNDS] = {false};
    vec3_t               alvec;

    qalGetListenerfv(AL_POSITION, alvec);
    //VectorScale(alvec, 52.49f, vListenerOrigin);

    for (i = 0; i < MAX_OPENAL_LOOP_SOUNDS; i++) {
        vec3_t vDir;

        if (bAlreadyAdded[i]) {
            continue;
        }

        pLoopSound = &openal.loop_sounds[i];
        if (!pLoopSound->pSfx) {
            continue;
        }
        pChannel = openal.channel[pLoopSound->iChannel];

        fMinDistance = pLoopSound->fMinDist;
        if (fMinDistance < 0) {
            fMinDistance = 200;
        }

        fMaxDistance = pLoopSound->fMaxDist;
        if (fMaxDistance < 0) {
            fMaxDistance = fMinDistance * 64;
        }

        fVolume = pLoopSound->fBaseVolume;
        if (fVolume < 0) {
            fVolume = 1;
        }
        fVolume = fVolume * s_fAmbientVolume;

        fTotalVolume = 0.0;
        fMaxVolume   = 0.0;

        if (pLoopSound->bPlaying) {
            pChannel->fNewPitchMult = pLoopSound->fPitch;
        }

        if (pLoopSound->bCombine) {
            for (j = 0; j < MAX_LOOP_SOUNDS; j++) {
                openal_loop_sound_t *pLoopSound2 = &openal.loop_sounds[j];

                if (pLoopSound2->pSfx == pLoopSound->pSfx) {
                    VectorSubtract(pLoopSound2->vOrigin, vListenerOrigin, vDir);
                    VectorCopy(vDir, pLoopSound2->vRelativeOrigin);
                    fDistance = VectorLength(pLoopSound2->vRelativeOrigin);

                    if (fDistance <= fMinDistance) {
                        fVolumeToPlay = fVolume;
                    } else if (fDistance >= fMaxDistance) {
                        fVolumeToPlay = 0;
                    } else {
                        fVolumeToPlay = fMinDistance * fMinDistance * fVolume / (fDistance * fDistance);
                    }

                    if (fMaxVolume < fVolumeToPlay) {
                        fMaxVolume = fVolumeToPlay;
                    }
                    fTotalVolume += fVolumeToPlay;
                    bAlreadyAdded[j] = true;
                }
            }
        } else {
            VectorSubtract(pLoopSound->vOrigin, vListenerOrigin, vDir);
            VectorCopy(vDir, pLoopSound->vRelativeOrigin);
            fDistance = VectorLength(pLoopSound->vRelativeOrigin);

            if (fDistance <= fMinDistance) {
                fTotalVolume = fVolume;
            } else if (fDistance >= fMaxDistance) {
                fTotalVolume = 0;
            } else {
                fTotalVolume = fMinDistance * fMinDistance * fVolume / (fDistance * fDistance);
            }
            pLoopSound->fVolume = fTotalVolume;
            fMaxVolume          = fTotalVolume;
        }

        fMaxFactor = sfx_infos[pLoopSound->pSfx->sfx_info_index].max_factor;
        if (fMaxFactor >= 1 && fMaxVolume * fMaxFactor < fTotalVolume) {
            fTotalVolume = fMaxVolume * fMaxFactor;
        }

        if (fTotalVolume <= 0 && !(pLoopSound->iFlags & LOOPSOUND_FLAG_NO_PAN)) {
            if (pLoopSound->bPlaying) {
                if (s_show_sounds->integer > 0) {
                    Com_DPrintf(
                        "%d (#%i) - stopped loop - %s\n",
                        cl.serverTime,
                        pLoopSound->iChannel,
                        openal.channel[pLoopSound->iChannel]->pSfx->name
                    );
                }

                pChannel->stop();
                pLoopSound->bPlaying = false;
                if (pLoopSound->bCombine) {
                    for (j = 0; j < MAX_LOOP_SOUNDS; j++) {
                        openal_loop_sound_t *pLoopSound2 = &openal.loop_sounds[j];

                        if (pLoopSound2->pSfx == pLoopSound->pSfx) {
                            pLoopSound2->bPlaying = false;
                        }
                    }
                }
            }

            continue;
        }

        VectorClear(vLoopOrigin);

        if (pLoopSound->bCombine) {
            for (j = 0; j < MAX_LOOP_SOUNDS; j++) {
                openal_loop_sound_t *pLoopSound2 = &openal.loop_sounds[j];

                if (pLoopSound2->pSfx == pLoopSound->pSfx) {
                    VectorNormalize(pLoopSound2->vRelativeOrigin);

                    VectorScale(
                        pLoopSound2->vRelativeOrigin, pLoopSound2->fVolume / fTotalVolume, pLoopSound2->vRelativeOrigin
                    );
                    VectorAdd(pLoopSound2->vRelativeOrigin, vLoopOrigin, vLoopOrigin);
                }
            }

            VectorNormalize(vLoopOrigin);
            VectorMA(vListenerOrigin, fMinDistance * 0.5f, vLoopOrigin, vLoopOrigin);
        } else {
            VectorCopy(pLoopSound->vOrigin, vLoopOrigin);
        }

        if (pLoopSound->bPlaying) {
            S_OPENAL_UpdateLoopSound(
                pLoopSound,
                S_GetBaseVolume() * 84.0 * fTotalVolume,
                fMinDistance,
                fMaxDistance,
                vListenerOrigin,
                vTempAxis,
                vLoopOrigin
            );

            continue;
        }

        if (s_show_sounds->integer > 0) {
            Com_DPrintf("%d (#%i) - started loop - %s\n", cl.serverTime, pLoopSound->iChannel, pLoopSound->pSfx->name);
        }

        if (pLoopSound->pSfx->iFlags & (SFX_FLAG_NO_OFFSET | SFX_FLAG_NO_DATA | SFX_FLAG_MP3)
            || (pLoopSound->iFlags & LOOPSOUND_FLAG_NO_PAN)) {
            iChannel = S_OPENAL_Start2DLoopSound(
                pLoopSound, fVolume, S_GetBaseVolume() * 84.0 * fTotalVolume, fMinDistance, vLoopOrigin
            );
        } else {
            iChannel = S_OPENAL_Start3DLoopSound(
                pLoopSound,
                S_GetBaseVolume() * 84.0 * fTotalVolume,
                fMinDistance,
                fMaxDistance,
                vLoopOrigin,
                vListenerOrigin
            );
        }

        if (iChannel < 0) {
            continue;
        }

        pLoopSound->bPlaying = 1;
        pLoopSound->iChannel = iChannel;

        if (pLoopSound->bCombine) {
            for (j = 0; j < MAX_LOOP_SOUNDS; j++) {
                openal_loop_sound_t *pLoopSound2 = &openal.loop_sounds[j];

                if (pLoopSound2->pSfx == pLoopSound->pSfx) {
                    pLoopSound2->bPlaying = true;
                    pLoopSound2->iChannel = iChannel;
                }
            }
        }
    }
}

/*
==============
S_OPENAL_Respatialize
==============
*/
void S_OPENAL_Respatialize(int iEntNum, const vec3_t vHeadPos, const vec3_t vAxis[3])
{
    int             i;
    vec3_t          vOrigin;
    vec3_t          vVelocity;
    vec3_t          vEntOrigin;
    vec3_t          vEntVelocity;
    vec3_t          vDir;
    vec3_t          vUp;
    vec3_t          vListenerOrigin;
    int             iPan;
    vec3_t          vTempAxis;
    float           fMaxVolume;
    float           fVolume;
    float           fDist;
    openal_channel *pChannel;
    vec3_t          alvec {0};
    vec3_t          alorientation[2];

    if (cls.no_menus) {
        return;
    }

    s_iListenerNumber = iEntNum;

    //
    // Velocity
    //
    qalListenerfv(AL_VELOCITY, alvec);
    alDieIfError();

    //
    // Position
    //
    //alvec[0] = -vHeadPos[0] / 52.49f;
    //alvec[1] = vHeadPos[2] / 52.49f;
    //alvec[2] = -vHeadPos[1] / 52.49f;
    VectorCopy(vHeadPos, alvec);
    VectorCopy(alvec, vListenerOrigin);
    qalListenerfv(AL_POSITION, alvec);
    alDieIfError();

    //
    // Orientation
    //
    /*
    alorientation[0][0] = -vAxis[0][0];
    alorientation[0][1] = vAxis[2][0];
    alorientation[0][2] = -vAxis[1][0];
    alorientation[1][0] = -vAxis[0][2];
    alorientation[1][1] = vAxis[2][2];
    alorientation[1][2] = -vAxis[1][2];
    */
    alorientation[0][0] = vAxis[0][0];
    alorientation[0][1] = vAxis[0][1];
    alorientation[0][2] = vAxis[0][2];
    alorientation[1][0] = vAxis[2][0];
    alorientation[1][1] = vAxis[2][1];
    alorientation[1][2] = vAxis[2][2];
    qalListenerfv(AL_ORIENTATION, (const ALfloat *)alorientation);
    alDieIfError();

    //vTempAxis[0] = -vAxis[0][1];
    //vTempAxis[1] = vAxis[2][1];
    //vTempAxis[2] = -vAxis[1][1];
    VectorCopy(vAxis[0], vTempAxis);

    fVolume = 1;
    iPan    = 64;

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        pChannel   = openal.channel[i];
        fMaxVolume = S_GetBaseVolume() * pChannel->fVolume;

        if (!pChannel) {
            continue;
        }

        if (!pChannel->is_playing()) {
            continue;
        }

        if (pChannel->iFlags & CHANNEL_FLAG_PAUSED) {
            continue;
        }

        if (pChannel->iFlags & CHANNEL_FLAG_NO_ENTITY) {
            VectorCopy(pChannel->vOrigin, vOrigin);

            if (pChannel->iFlags & CHANNEL_FLAG_LOCAL_LISTENER) {
                VectorCopy(vListenerOrigin, vOrigin);
                if (i >= MAX_OPENAL_CHANNELS_3D) {
                    fVolume = fMaxVolume;
                    iPan    = 64;
                } else {
                    //pChannel->set_position(vOrigin[0] / 52.49f, vOrigin[1] / 52.49f, vOrigin[2] / 52.49f);
                    pChannel->set_position(vOrigin[0], vOrigin[1], vOrigin[2]);
                }
            } else {
                if (i >= MAX_OPENAL_CHANNELS_3D) {
                    iPan = S_OPENAL_SpatializeStereoSound(vListenerOrigin, vTempAxis, vOrigin);
                    VectorSubtract(vListenerOrigin, vOrigin, vDir);

                    fDist = VectorLength(vDir);
                    if (fDist <= pChannel->fMinDist + 0.001f) {
                        fVolume = fMaxVolume;
                    } else if (fDist >= pChannel->fMaxDist - 0.001f) {
                        fVolume = 0;
                    } else {
                        fVolume = (1.0 - (fDist - pChannel->fMinDist) / (pChannel->fMaxDist - pChannel->fMinDist))
                                * fMaxVolume;
                    }
                } else {
                    //pChannel->set_position(vOrigin[0] / 52.49f, vOrigin[1] / 52.49f, vOrigin[2] / 52.49f);
                    pChannel->set_position(vOrigin[0], vOrigin[1], vOrigin[2]);
                }
            }
        } else if (pChannel->iFlags & CHANNEL_FLAG_LOCAL_LISTENER) {
            VectorCopy(vListenerOrigin, vOrigin);
            if (i >= MAX_OPENAL_CHANNELS_3D) {
                fVolume = fMaxVolume;
                iPan    = 64;
            } else {
                //pChannel->set_position(vOrigin[0] / 52.49f, vOrigin[1] / 52.49f, vOrigin[2] / 52.49f);
                pChannel->set_position(vOrigin[0], vOrigin[1], vOrigin[2]);
            }
        } else {
            if (s_entity[pChannel->iEntNum].time < pChannel->iTime) {
                VectorCopy(pChannel->vOrigin, vOrigin);
                if (!(pChannel->iFlags & CHANNEL_FLAG_LOOPING)) {
                    pChannel->end_sample();
                    continue;
                }
            } else {
                VectorCopy(s_entity[pChannel->iEntNum].position, vEntOrigin);
                //vOrigin[1] = vEntOrigin[2] / 52.49f;
                //vOrigin[0] = -vEntOrigin[0] / 52.49f;
                //vOrigin[2] = -vEntOrigin[1] / 52.49f;
                VectorCopy(vEntOrigin, vOrigin);
                VectorCopy(vOrigin, pChannel->vOrigin);
                pChannel->iTime = s_entity[pChannel->iEntNum].time;
            }

            if (s_entity[pChannel->iEntNum].use_listener) {
                VectorCopy(vListenerOrigin, vOrigin);
            }

            if (pChannel->iEntNum == s_iListenerNumber) {
                if (vListenerOrigin[0] == vOrigin[0] && vListenerOrigin[2] == vOrigin[2]) {
                    float fDelta = vListenerOrigin[1] - vOrigin[1];

                    if (fDelta > 89.9f && fDelta < 90.09f) {
                        VectorCopy(vListenerOrigin, vOrigin);
                    }
                }
            }

            if (i >= MAX_OPENAL_CHANNELS_3D) {
                iPan = S_OPENAL_SpatializeStereoSound(vListenerOrigin, vTempAxis, vOrigin);
                VectorSubtract(vListenerOrigin, vOrigin, vDir);
                fDist = VectorLength(vDir);
                if (fDist <= pChannel->fMinDist + 0.001f) {
                    fVolume = fMaxVolume;
                } else if (fDist >= pChannel->fMaxDist - 0.001f) {
                    fVolume = 0;
                } else {
                    fVolume =
                        (1.0 - (fDist - pChannel->fMinDist) / (pChannel->fMaxDist - pChannel->fMinDist)) * fMaxVolume;
                }
            } else {
                //pChannel->set_position(vOrigin[0] / 52.49f, vOrigin[1] / 52.49f, vOrigin[2] / 52.49f);
                pChannel->set_position(vOrigin[0], vOrigin[1], vOrigin[2]);
                VectorCopy(s_entity[pChannel->iEntNum].velocity, vEntVelocity);

                //vVelocity[0] = -vEntVelocity[0] / 52.49f / 500.f;
                //vVelocity[1] = vEntVelocity[2] / 52.49f / 500.f;
                //vVelocity[2] = -vEntVelocity[1] / 52.49f / 500.f;
                VectorCopy(vEntVelocity, vVelocity);
                //pChannel->set_velocity(vVelocity[0] / 52.49f, vVelocity[1] / 52.49f, vVelocity[2] / 52.49f);
                pChannel->set_velocity(vVelocity[0], vVelocity[1], vVelocity[2]);
            }
        }

        if (i >= MAX_OPENAL_CHANNELS_3D) {
            pChannel->set_gain(fVolume);
            pChannel->set_sample_pan(iPan);
        }

        if (s_bReverbChanged) {
            S_OPENAL_reverb(i, s_iReverbType, s_fReverbLevel);
        }
    }

    S_OPENAL_AddLoopSounds(vTempAxis);
    s_bReverbChanged = false;
}

/*
==============
S_OPENAL_SpatializeStereoSound
==============
*/
static int S_OPENAL_SpatializeStereoSound(const vec3_t listener_origin, const vec3_t listener_left, const vec3_t origin)
{
    float  lscale, rscale;
    vec3_t source_vec;
    int    pan;

    VectorSubtract(origin, listener_origin, source_vec);
    VectorNormalize(source_vec);

    pan = s_separation->value + (1.f - s_separation->value) * -DotProduct(listener_left, source_vec);

    if (pan < 0) {
        pan = 0;
    }

    return pan * 128.f;
}

/*
==============
S_OPENAL_reverb
==============
*/
static void S_OPENAL_reverb(int iChannel, int iReverbType, float fReverbLevel)
{
    // No reverb.
}

/*
==============
S_OPENAL_SetReverb
==============
*/
void S_OPENAL_SetReverb(int iType, float fLevel)
{
    s_fReverbLevel = fLevel;
    s_iReverbType  = iType;
    if (al_use_reverb) {
        s_bReverbChanged = true;
    }
}

/*
==============
S_OPENAL_Update
==============
*/
void S_OPENAL_Update()
{
    int             i;
    openal_channel *pChannel;

    if (cl.snap.ps.stats[STAT_CINEMATIC]) {
        S_SetGlobalAmbientVolumeLevel(0.5f);
    } else {
        S_SetGlobalAmbientVolumeLevel(1.f);
    }

    if (paused->integer && !s_bSoundPaused) {
        S_PauseSound();
    } else if (!paused->integer && s_bSoundPaused) {
        S_UnpauseSound();
    }

    if (s_bFading) {
        s_fFadeVolume = 1.f - (cls.realtime - s_fFadeStartTime) / (s_fFadeStopTime - s_fFadeStartTime);
        if (s_fFadeVolume < 0) {
            s_fFadeVolume = 0;
        }
        music_volume_changed = true;
    }

    if (s_volume->modified) {
        if (s_volume->value > 1) {
            Cvar_Set("s_volume", "1.0");
        } else if (s_volume->value < 0) {
            Cvar_Set("s_volume", "0.0");
        }

        music_volume_changed = true;
        s_volume->modified   = 0;
        al_current_volume    = Square(s_volume->value * s_volume->value);
        qalListenerf(AL_GAIN, al_current_volume);
        alDieIfError();
    }

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        pChannel = openal.channel[i];
        if (!pChannel) {
            continue;
        }

        if (!pChannel->is_playing()) {
            continue;
        }

        if (pChannel->fNewPitchMult <= 0) {
            continue;
        }

        pChannel->set_sample_playback_rate(pChannel->iBaseRate * pChannel->fNewPitchMult);
        pChannel->fNewPitchMult = 0;
    }

    if (s_speaker_type->modified) {
        if (s_speaker_type->integer) {
            Com_Printf("FIXME: Allow different speaker types in OpenAL code.\n");
            Cvar_Set("s_speaker_type", "0");
        }
        s_speaker_type->modified = false;
    }

    if (s_reverb->modified) {
        s_reverb->modified = false;
        Com_Printf("FIXME: Allow reverb toggle at runtime in OpenAL code.\n");
    }

    if (s_show_num_active_sounds->integer == 1) {
        int num = 0;

        for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
            pChannel = openal.channel[i];
            if (pChannel && pChannel->is_playing()) {
                ++num;
            }
        }
        Com_DPrintf("Number of active sounds = %d\n", num);
    }

    Music_Update();

    for (i = 0; i < MAX_LOOP_SOUNDS; i++) {
        if (!openal.loop_sounds[i].bInUse) {
            S_OPENAL_StopLoopingSound(&openal.loop_sounds[i]);
        }
    }
}

/*
==============
S_IsSoundPlaying
==============
*/
qboolean S_IsSoundPlaying(int channel_number, const char *sfxName)
{
    openal_channel *pChannel = openal.channel[channel_number];
    if (!pChannel) {
        return false;
    }

    if (s_bSoundPaused) {
        return false;
    }

    if (!pChannel->is_playing()) {
        return false;
    }

    if (!pChannel->pSfx || pChannel->pSfx == (sfx_t *)-16) {
        return false;
    }

    return !strcmp(sfxName, pChannel->pSfx->name);
}

/*
==============
S_StoreBase
==============
*/
static void S_StoreBase(channelbasesavegame_t *pBase, openal_channel *pChannel)
{
    if (!pChannel) {
        return;
    }

    if (pChannel->iEntChannel == CHAN_MENU || pChannel->is_free()) {
        pBase->bPlaying      = false;
        pBase->iOffset       = 0;
        pBase->iLoopCount    = 0;
        pBase->sfx.szName[0] = 0;
        pBase->sfx.iFlags    = 0;
        pBase->fNewPitchMult = 1.f;
        pBase->iBaseRate     = pChannel->iBaseRate;
        pBase->iStatus       = 0;
    } else {
        pBase->bPlaying   = true;
        pBase->iOffset    = pChannel->sample_offset();
        pBase->iLoopCount = pChannel->sample_loop_count();
        memcpy(pBase->sfx.szName, pChannel->pSfx->name, sizeof(pBase->sfx.szName));
        pBase->sfx.iFlags    = pChannel->pSfx->iFlags;
        pBase->iBaseRate     = pChannel->iBaseRate;
        pBase->iStatus       = 0;
        pBase->fNewPitchMult = (float)pBase->iBaseRate / (float)pChannel->sample_playback_rate();
    }

    pBase->iStartTime                = pChannel->iStartTime - cl.serverTime;
    pBase->iEndTime                  = pChannel->iEndTime - cl.serverTime;
    pBase->iEntChannel               = pChannel->iEntChannel;
    pBase->iEntNum                   = pChannel->iEntNum;
    pBase->iFlags                    = pChannel->iFlags;
    pBase->fMaxDist                  = pChannel->fMaxDist;
    pBase->fMinDist                  = pChannel->fMinDist;
    pBase->iNextCheckObstructionTime = 0;
    VectorCopy(pChannel->vOrigin, pBase->vOrigin);
    pBase->iTime   = pChannel->iTime - cl.serverTime;
    pBase->fVolume = pChannel->fVolume;
}

/*
==============
S_StartSoundFromBase
==============
*/
static void
S_StartSoundFromBase(channelbasesavegame_t *pBase, openal_channel *pChannel, sfx_t *pSfx, bool bStartUnpaused)
{
    if (!pChannel->set_sfx(pSfx)) {
        Com_DPrintf("Set sample error - %s\n", pSfx->name);
        pChannel->iFlags &= ~CHANNEL_FLAG_PLAYABLE;
        return;
    }

    pChannel->iBaseRate = pChannel->sample_playback_rate();
    pChannel->set_gain(pChannel->fVolume);
    pChannel->set_sample_offset(pBase->iOffset);
    pChannel->set_sample_playback_rate(pChannel->iBaseRate * pBase->fNewPitchMult);

    if (sfx_infos[pSfx->sfx_info_index].loop_start != -1) {
        pChannel->set_sample_loop_block(
            sfx_infos[pSfx->sfx_info_index].loop_start, sfx_infos[pSfx->sfx_info_index].loop_end
        );
        pChannel->set_sample_loop_count(1);
        pChannel->iFlags |= CHANNEL_FLAG_LOOPING;
        if (s_show_sounds->integer > 0) {
            Com_DPrintf(
                "loopblock - %d to %d\n",
                sfx_infos[pSfx->sfx_info_index].loop_start,
                sfx_infos[pSfx->sfx_info_index].loop_end
            );
        }
    } else {
        pChannel->set_sample_loop_count(1);
    }

    if (bStartUnpaused) {
        pChannel->resume_sample();
    } else {
        pChannel->iFlags |= CHANNEL_FLAG_PLAYABLE;
    }
}

/*
==============
S_LoadBase
==============
*/
static void S_LoadBase(channelbasesavegame_t *pBase, openal_channel *pChannel, bool bStartUnpaused)
{
    sfxHandle_t handle;

    if (!pChannel) {
        return;
    }

    if (!pBase->bPlaying) {
        return;
    }

    if (strstr(pBase->sfx.szName, "null.wav")) {
        return;
    }

    handle = S_RegisterSound(pBase->sfx.szName, (pBase->sfx.iFlags & SFX_FLAG_DEFAULT_SOUND), false);

    pChannel->iBaseRate     = pBase->iBaseRate;
    pChannel->iStartTime    = pBase->iStartTime;
    pChannel->iEndTime      = pBase->iEndTime;
    pChannel->iEntChannel   = pBase->iEntChannel;
    pChannel->iEntNum       = pBase->iEntNum;
    pChannel->iFlags        = pBase->iFlags;
    pChannel->fMaxDist      = pBase->fMaxDist;
    pChannel->fMinDist      = pBase->fMinDist;
    pChannel->fNewPitchMult = pBase->fNewPitchMult;
    VectorCopy(pBase->vOrigin, pChannel->vOrigin);
    pChannel->iTime   = pBase->iTime;
    pChannel->fVolume = pBase->fVolume;
    pChannel->pSfx    = &s_knownSfx[handle];

    S_StartSoundFromBase(pBase, pChannel, &s_knownSfx[handle], bStartUnpaused);
}

/*
==============
S_SaveData
==============
*/
void S_SaveData(soundsystemsavegame_t *pSave)
{
    int  i;
    bool bSoundWasUnpaused;

    bSoundWasUnpaused = !s_bSoundPaused;
    if (!s_bSoundPaused) {
        S_PauseSound();
    }

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        S_StoreBase(&pSave->Channels[i], openal.channel[i]);
    }

    if (bSoundWasUnpaused) {
        S_UnpauseSound();
    }
}

/*
==============
S_ReLoad
==============
*/
void S_ReLoad(soundsystemsavegame_t *pSave)
{
    int  i;
    bool bSoundWasUnpaused;

    bSoundWasUnpaused = !s_bSoundPaused;
    if (!s_bSoundPaused) {
        S_PauseSound();
    }

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        S_LoadBase(&pSave->Channels[i], openal.channel[i], bSoundWasUnpaused);
    }

    if (bSoundWasUnpaused) {
        S_UnpauseSound();
    }
}

/*
==============
S_InitBase
==============
*/
static void S_InitBase(channelbasesavegame_t *pBase)
{
    if (!pBase->bPlaying) {
        return;
    }

    if (strstr(pBase->sfx.szName, "null.wav")) {
        return;
    }

    SV_AddSvsTimeFixup(&pBase->iStartTime);
    SV_AddSvsTimeFixup(&pBase->iEndTime);
    SV_AddSvsTimeFixup(&pBase->iTime);
}

/*
==============
S_LoadData
==============
*/
void S_LoadData(soundsystemsavegame_t *pSave)
{
    int i;

    for (i = 0; i < MAX_OPENAL_POSITION_CHANNELS; i++) {
        S_InitBase(&pSave->Channels[i]);
    }
}

/*
==============
openal_channel::set_velocity
==============
*/
void openal_channel::set_velocity(float v0, float v1, float v2)
{
    qalSource3f(source, AL_VELOCITY, v0, v1, v2);
    alDieIfError();
}

/*
==============
openal_channel::set_position
==============
*/
void openal_channel::set_position(float v0, float v1, float v2)
{
    qalSource3f(source, AL_POSITION, v0, v1, v2);
    alDieIfError();
}

/*
==============
openal_channel::set_gain
==============
*/
void openal_channel::set_gain(float gain)
{
    qalSourcef(source, AL_GAIN, gain);
    alDieIfError();
}

/*
==============
openal_channel::set_no_3d
==============
*/
void openal_channel::set_no_3d()
{
    qalSource3f(source, AL_POSITION, 0, 0, 0);
    alDieIfError();
    qalSource3f(source, AL_VELOCITY, 0, 0, 0);
    alDieIfError();
    qalSourcei(source, AL_SOURCE_RELATIVE, true);
    alDieIfError();
    qalSourcei(source, AL_LOOPING, false);
    alDieIfError();
    qalSourcei(source, AL_ROLLOFF_FACTOR, 0);
    alDieIfError();
    qalSourcef(source, AL_GAIN, S_GetBaseVolume());
    alDieIfError();
}

/*
==============
openal_channel::set_3d
==============
*/
void openal_channel::set_3d()
{
    qalSourcei(source, AL_SOURCE_RELATIVE, false);
    alDieIfError();
    qalSourcei(source, AL_LOOPING, false);
    alDieIfError();
    qalSourcef(source, AL_ROLLOFF_FACTOR, 0.019f);
    alDieIfError();
    qalSourcef(source, AL_GAIN, S_GetBaseVolume());
    alDieIfError();
}

/*
==============
openal_channel::play
==============
*/
void openal_channel::play()
{
    qalSourcePlay(source);
    alDieIfError();
}

/*
==============
openal_channel::pause
==============
*/
void openal_channel::pause()
{
    qalSourcePause(source);
    alDieIfError();
}

/*
==============
openal_channel::stop
==============
*/
void openal_channel::stop()
{
    qalSourceStop(source);
    alDieIfError();
}

/*
==============
openal_channel::get_state
==============
*/
ALint openal_channel::get_state()
{
    ALint retval;

    qalGetSourcei(source, AL_SOURCE_STATE, &retval);
    alDieIfError();

    return retval;
}

/*
==============
openal_channel::is_free
==============
*/
bool openal_channel::is_free()
{
    ALint state = get_state();

    return state == AL_INITIAL || state == AL_STOPPED;
}

/*
==============
openal_channel::is_paused
==============
*/
bool openal_channel::is_paused()
{
    ALint state = get_state();

    return state == AL_PAUSED;
}

/*
==============
openal_channel::is_playing
==============
*/
bool openal_channel::is_playing()
{
    ALint state = get_state();

    return state == AL_PLAYING;
}

/*
==============
openal_channel::force_free
==============
*/
void openal_channel::force_free()
{
    stop();
}

/*
==============
openal_channel::set_sfx
==============
*/
bool openal_channel::set_sfx(sfx_t *pSfx)
{
    ALfloat freq = 0;

    this->pSfx = pSfx;
    if (!pSfx->buffer || !qalIsBuffer(pSfx->buffer)) {
        if (pSfx->iFlags & SFX_FLAG_MP3) {
            qalGenBuffers(1, &pSfx->buffer);
            alDieIfError();

            if (!_alutLoadMP3_LOKI(pSfx->buffer, pSfx->data, pSfx->length)) {
                qalDeleteBuffers(1, &pSfx->buffer);
                alDieIfError();

                Com_Printf("OpenAL: Failed to load MP3.\n");
                return false;
            }

            alDieIfError();
        } else {
            ALenum fmt = 0;

            if (pSfx->info.channels == 1) {
                if (pSfx->info.width == 1) {
                    fmt = AL_FORMAT_MONO8;
                } else if (pSfx->info.width == 2) {
                    fmt = AL_FORMAT_MONO16;
                }
            } else if (pSfx->info.channels == 2) {
                if (pSfx->info.width == 1) {
                    fmt = AL_FORMAT_STEREO8;
                } else if (pSfx->info.width == 2) {
                    fmt = AL_FORMAT_STEREO16;
                }
            }

            if (!fmt) {
                Com_Printf(
                    "OpenAL: Bad Wave file (%d channels, %d bits) [%s].\n",
                    pSfx->info.channels,
                    (int)(pSfx->info.width * 8.f),
                    pSfx->name
                );
                return false;
            }

            qalGenBuffers(1, &pSfx->buffer);
            alDieIfError();

            qalBufferData(
                pSfx->buffer,
                fmt,
                &pSfx->data[pSfx->info.dataofs],
                pSfx->info.samples * pSfx->info.width,
                pSfx->info.rate
            );
            alDieIfError();
        }
    }

    qalSourceStop(source);
    alDieIfError();

    qalSourcei(source, AL_BUFFER, pSfx->buffer);
    alDieIfError();

    // Get the base frequency
    qalGetBufferfv(pSfx->buffer, AL_FREQUENCY, &freq);
    alDieIfError();

    iBaseRate = freq;

    return true;
}

/*
==============
openal_channel::start_sample
==============
*/
void openal_channel::start_sample()
{
    play();
}

/*
==============
openal_channel::stop_sample
==============
*/
void openal_channel::stop_sample()
{
    pause();
}

/*
==============
openal_channel::resume_sample
==============
*/
void openal_channel::resume_sample()
{
    play();
}

/*
==============
openal_channel::end_sample
==============
*/
void openal_channel::end_sample()
{
    stop();
}

/*
==============
openal_channel::set_sample_pan
==============
*/
void openal_channel::set_sample_pan(S32 pan) {}

/*
==============
openal_channel::set_sample_playback_rate
==============
*/
void openal_channel::set_sample_playback_rate(S32 rate) {}

/*
==============
openal_channel::sample_playback_rate
==============
*/
S32 openal_channel::sample_playback_rate()
{
    return 22050;
}

/*
==============
openal_channel::sample_volume
==============
*/
S32 openal_channel::sample_volume()
{
    STUB_DESC("sample_volume");
    return 127;
}

/*
==============
openal_channel::sample_offset
==============
*/
U32 openal_channel::sample_offset()
{
    STUB_DESC("sample_offset");
    return 127;
}

/*
==============
openal_channel::sample_ms_offset
==============
*/
U32 openal_channel::sample_ms_offset()
{
    STUB_DESC("sample_ms_offset");
    return 127;
}

/*
==============
openal_channel::sample_loop_count
==============
*/
U32 openal_channel::sample_loop_count()
{
    ALuint queued;
    ALuint processed;
    S32    left;

    qalGetSourceiv(source, AL_BUFFERS_QUEUED, (ALint *)&queued);
    alDieIfError();
    qalGetSourcei(source, AL_BUFFERS_PROCESSED, (ALint *)&processed);
    alDieIfError();

    left = queued + ~processed;
    if (left < 0) {
        left = 0;
    }
    return left;
}

/*
==============
openal_channel::set_sample_offset
==============
*/
void openal_channel::set_sample_offset(U32 offset)
{
    STUB_DESC("sample_offset");
}

/*
==============
openal_channel::set_sample_ms_offset
==============
*/
void openal_channel::set_sample_ms_offset(U32 offset)
{
    STUB_DESC("sample_ms_offset");
}

/*
==============
openal_channel::set_sample_loop_count
==============
*/
void openal_channel::set_sample_loop_count(S32 count)
{
    ALuint processed = 0;

    stop();

    qalGetSourceiv(source, AL_BUFFERS_PROCESSED, (ALint *)&processed);
    alDieIfError();

    for (ALuint i = 0; i < processed; i++) {
        ALuint bufName;

        qalSourceUnqueueBuffers(source, 1, &bufName);
        alDieIfError();
    }

    for (S32 i = 0; i < count; i++) {
        qalSourceQueueBuffers(source, 1, &buffer);
        alDieIfError();
    }
}

/*
==============
openal_channel::set_sample_loop_block
==============
*/
void openal_channel::set_sample_loop_block(S32 start_offset, S32 end_offset)
{
    STUB_DESC("sample_loop_block");
}

/*
==============
openal_channel::sample_status
==============
*/
U32 openal_channel::sample_status()
{
    STUB_DESC("sample_status");
    return 127;
}

/*
==============
MUSIC_LoadSoundtrackFile
==============
*/
qboolean MUSIC_LoadSoundtrackFile(const char *filename)
{
    song_t *psong = NULL;
    char    args[MAX_MUSIC_SONGS][MAX_RES_NAME];
    int     numargs;
    char    com_token[MAX_STRING_CHARS];
    char    alias[128];
    char    file[128];
    char    load_path[128];
    char   *buffer;
    char    path[MAX_RES_NAME];
    int     i;
    byte   *data;

    if (strrchr(filename, '.')) {
        Com_sprintf(path, sizeof(path), "%s", filename);
    } else {
        Com_sprintf(path, sizeof(path), "%s.mus", filename);
    }

    FS_ReadFile(path, (void **)&data);
    if (!data) {
        Com_DPrintf("Couldn't load %s\n", path);
        return false;
    }
    Com_DPrintf("SOUNDTRACK: Loading %s\n", path);

    MUSIC_StopAllSongs();
    music_numsongs = 0;

    buffer       = (char *)data;
    load_path[0] = 0;

    while (1) {
        strcpy(com_token, COM_GetToken(&buffer, true));
        if (!com_token[0]) {
            break;
        }

        if (strlen(com_token) >= MAX_RES_NAME) {
            Com_Printf("MUSIC_LoadSoundtrackFile: argument too long, truncating in %s\n", path);
            com_token[MAX_RES_NAME - 1] = 0;
        }

        numargs = 1;
        strcpy(args[0], com_token);

        while (1) {
            strcpy(com_token, COM_GetToken(&buffer, false));
            if (!com_token[0]) {
                break;
            }

            if (strlen(com_token) >= MAX_RES_NAME) {
                Com_Printf("MUSIC_LoadSoundtrackFile: argument too long, truncating in %s\n", path);
                com_token[MAX_RES_NAME - 1] = 0;
            }

            strcpy(args[numargs], com_token);
            numargs++;
        }

        if (!Q_stricmp(args[0], "path")) {
            strcpy(load_path, args[1]);
            if (load_path[strlen(load_path) - 1] != '/' && load_path[strlen(load_path) - 1] != '\\') {
                strcat(load_path, "/");
            }
        } else if (args[0][0] == '!') {
            for (i = 0; i < music_numsongs; i++) {
                psong = &music_songs[i];
                if (!Q_stricmp(psong->alias, &args[0][1])) {
                    break;
                }
            }

            if (i == music_numsongs) {
                Com_Printf("MUSIC_LoadSoundtrackFile: song %s not found, command skipped in %s.\n", &args[0][1], path);
                continue;
            }

            if (!Q_stricmp(args[1], "volume")) {
                psong->volume = atoi(args[2]);
            } else if (!Q_stricmp(args[1], "fadetime")) {
                psong->fadetime = atoi(args[2]);
            } else if (!Q_stricmp(args[1], "loop")) {
                psong->flags |= 1;
            } else if (!Q_stricmp(args[1], "restart")) {
                psong->flags |= 2;
            } else if (!Q_stricmp(args[1], "interrupt")) {
                psong->fadetime = 0;
                psong->flags |= 4 | 2;
            } else {
                Com_Printf(
                    "MUSIC_LoadSoundtrackFile: unknown command %s for song %s in %s.\n", args[1], &args[0][1], path
                );
            }
        } else {
            if (numargs > 1) {
                strcpy(alias, args[0]);
                strcpy(file, load_path);
                strcat(file, args[1]);
            } else {
                strcpy(file, load_path);
                strcat(file, args[1]);

                strncpy(alias, args[0], strlen(args[0]) - 4);
                file[strlen(args[0]) + MAX_RES_NAME * 2 - 4] = 0;
            }

            if (music_numsongs >= MAX_MUSIC_SONGS) {
                Com_Printf("MUSIC_LoadSoundtrackFile: Too many songs in %s, skipping %s.\n", path, alias);
                continue;
            }

            psong = &music_songs[music_numsongs];
            strcpy(psong->alias, alias);
            strcpy(psong->path, file);
            music_songs[music_numsongs].fadetime    = 1.0;
            music_songs[music_numsongs].volume      = 1.0;
            music_songs[music_numsongs].flags       = 0;
            music_songs[music_numsongs].current_pos = 0;
            music_songs[music_numsongs].mood_num    = MusicMood_NameToNum(alias);
            music_numsongs++;
        }
    }

    if (!music_numsongs) {
        Com_Printf("MUSIC_LoadSoundtrackFile: could not load %s, no songs defined.\n", path);
        FS_FreeFile(data);
        return false;
    }

    music_currentsong = -1;
    FS_FreeFile(data);

    if (music_current_mood == mood_none) {
        MUSIC_UpdateMood(mood_normal, mood_normal);
    }

    return true;
}

/*
==============
MUSIC_SongValid
==============
*/
qboolean MUSIC_SongValid(const char *mood)
{
    return MUSIC_FindSong(mood) != -1;
}

/*
==============
MUSIC_Loaded
==============
*/
qboolean MUSIC_Loaded()
{
    return music_loaded;
}

/*
==============
Music_Update
==============
*/
void Music_Update()
{
    int currentsong;

    if (!s_bSoundStarted) {
        return;
    }

    if (!music_active) {
        return;
    }

    if (s_bSoundPaused) {
        return;
    }

    MUSIC_CheckForStoppedSongs();

    if (!MUSIC_Loaded() && music_active && strlen(current_soundtrack)) {
        MUSIC_LoadSoundtrackFile(current_soundtrack);
        music_loaded = true;
    }

    if (music_currentsong >= 0) {
        currentsong = music_songs[music_currentsong].mood_num;
    } else {
        currentsong = -1;
    }

    if (music_current_mood == mood_none) {
        if (MUSIC_Playing()) {
            MUSIC_StopAllSongs();
        }
    } else if (music_current_mood != currentsong) {
        const char *mood = MusicMood_NumToName(music_current_mood);
        if (MUSIC_SongValid(mood) && MUSIC_Loaded() && strlen(current_soundtrack)) {
            Com_DebugPrintf("Playing %s.\n", mood);
            MUSIC_PlaySong(mood);
        }
    }

    if (new_music_volume != music_volume) {
        if (music_volume_fade_time > 0) {
            if (music_volume_direction == 0) {
                music_volume = (cls.realtime - music_volume_start_time) * (new_music_volume - old_music_volume)
                                 / (music_volume_fade_time * 1000.f)
                             + old_music_volume;

                if (music_volume > new_music_volume) {
                    music_volume         = new_music_volume;
                    music_volume_changed = 1;
                } else {
                    music_volume_changed = 1;
                }
            } else if (music_volume_direction == 1) {
                music_volume = 1.0
                             - (cls.realtime - music_volume_start_time) * (old_music_volume - new_music_volume)
                                   / (music_volume_fade_time * 1000.f);

                if (music_volume >= new_music_volume) {
                    music_volume_changed = true;
                } else {
                    music_volume         = new_music_volume;
                    music_volume_changed = true;
                }
            } else {
                music_volume_changed = true;
            }
        } else {
            music_volume         = new_music_volume;
            music_volume_changed = true;
        }
    }

    MUSIC_UpdateMusicVolumes();
}

/*
==============
MUSIC_SongEnded
==============
*/
void MUSIC_SongEnded()
{
    Com_DPrintf(
        "MUSIC: Song ended, changing from [ %s ] to [ %s ]\n",
        MusicMood_NumToName(music_current_mood),
        MusicMood_NumToName(music_fallback_mood)
    );
    music_current_mood = music_fallback_mood;
}

/*
==============
MUSIC_NewSoundtrack
==============
*/
void MUSIC_NewSoundtrack(const char *name)
{
    if (!s_bSoundStarted) {
        return;
    }

    if (!Q_stricmp(name, current_soundtrack)) {
        return;
    }

    // Fixed in OPM
    //  Make sure to not get past the end of the buffer
    //strcpy(current_soundtrack, name);
    Q_strncpyz(current_soundtrack, name, sizeof(current_soundtrack));

    if (!Q_stricmp(current_soundtrack, "none") || !Q_stricmp(current_soundtrack, "")) {
        music_active = qfalse;
        if (MUSIC_Playing()) {
            MUSIC_StopAllSongs();
        }
    } else {
        music_active = qtrue;
        MUSIC_LoadSoundtrackFile(name);
        music_loaded = qtrue;
    }
}

/*
==============
MUSIC_UpdateMood
==============
*/
void MUSIC_UpdateMood(int current, int fallback)
{
    static int last_current_mood  = -1;
    static int last_fallback_mood = -1;
    static int current_mood       = -1;
    static int fallback_mood      = -1;
    qboolean   was_action;

    if (current == current_mood && fallback == fallback_mood) {
        return;
    }

    was_action          = current == last_current_mood && fallback == last_fallback_mood && current_mood == mood_action;
    last_current_mood   = current_mood;
    last_fallback_mood  = fallback_mood;
    current_mood        = current;
    music_current_mood  = current;
    fallback_mood       = fallback;
    music_fallback_mood = fallback;
    music_active        = qtrue;

    Com_DPrintf(
        "MUSIC: changing from [ %s | %s ] to [ %s | %s ]\n",
        MusicMood_NumToName(last_current_mood),
        MusicMood_NumToName(last_fallback_mood),
        MusicMood_NumToName(current_mood),
        MusicMood_NumToName(fallback)
    );

    if (was_action) {
        int songnum = MUSIC_FindSong(MusicMood_NumToName(current_mood));

        if (songnum != -1 && (music_songs[songnum].flags & 4)) {
            Com_DPrintf(
                "MUSIC: restoring music from action state, skipping [ %s ] for [ %s ]\n",
                MusicMood_NumToName(current_mood),
                MusicMood_NumToName(fallback_mood)
            );
            music_current_mood = music_fallback_mood;
        }
    }
}

/*
==============
MUSIC_UpdateVolume
==============
*/
void MUSIC_UpdateVolume(float volume, float fade_time)
{
    if (new_music_volume == volume && music_volume_fade_time == fade_time) {
        return;
    }

    old_music_volume        = music_volume;
    new_music_volume        = volume;
    music_volume_fade_time  = fade_time;
    music_volume_start_time = cls.realtime;

    if (volume > music_volume) {
        music_volume_direction = 0;
    } else if (volume < music_volume) {
        music_volume_direction = 1;
    }
}

/*
==============
MUSIC_StopAllSongs
==============
*/
void MUSIC_StopAllSongs()
{
    for (int i = 0; i < MAX_OPENAL_SONGS; i++) {
        MUSIC_StopChannel(i);
    }

    music_currentsong = -1;
}

/*
==============
MUSIC_FreeAllSongs
==============
*/
void MUSIC_FreeAllSongs()
{
    MUSIC_StopAllSongs();
    MUSIC_UpdateMood(mood_none, mood_none);
    current_soundtrack[0] = 0;
    music_loaded          = false;
}

/*
==============
MUSIC_Playing
==============
*/
qboolean MUSIC_Playing()
{
    return MUSIC_CurrentSongChannel() != -1;
}

/*
==============
MUSIC_FindSong
==============
*/
int MUSIC_FindSong(const char *name)
{
    int i;

    for (i = 0; i < music_numsongs; i++) {
        if (!Q_stricmp(music_songs[i].alias, name)) {
            return i;
        }
    }

    return -1;
}

/*
==============
S_loadsoundtrack
==============
*/
void S_loadsoundtrack()
{
    if (Cmd_Argc() != 2) {
        Com_Printf("loadsoundtrack <sound track file>\n");
        return;
    }

    MUSIC_LoadSoundtrackFile(Cmd_Argv(1));
    music_loaded = true;
    Q_strncpyz(current_soundtrack, Cmd_Argv(1), sizeof(current_soundtrack));
}

/*
==============
S_CurrentSoundtrack
==============
*/
const char *S_CurrentSoundtrack()
{
    return current_soundtrack;
}

/*
==============
S_PlaySong
==============
*/
void S_PlaySong()
{
    if (Cmd_Argc() != 2) {
        Com_Printf("playsong <song alias>\n");
        return;
    }

    MUSIC_PlaySong(Cmd_Argv(1));
    music_active = true;
}

/*
==============
MUSIC_CurrentSongChannel
==============
*/
int MUSIC_CurrentSongChannel()
{
    int channel_number = -1;
    int ch_idx         = 0;

    for (ch_idx = 0; ch_idx < MAX_OPENAL_SONGS; ch_idx++) {
        if (openal.chan_song[ch_idx].is_playing() && openal.chan_song[ch_idx].song_number == music_currentsong) {
            channel_number = ch_idx;
        }
    }

    return channel_number;
}

/*
==============
MUSIC_StopChannel
==============
*/
void MUSIC_StopChannel(int channel_number)
{
    openal_channel *channel = &openal.chan_song[channel_number];

    channel->pause();
    if (music_songs[channel->song_number].flags & 2) {
        music_songs[channel->song_number].current_pos = 0;
    } else {
        music_songs[channel->song_number].current_pos = channel->sample_ms_offset();
    }

    channel->stop();
}

/*
==============
MUSIC_PlaySong
==============
*/
qboolean MUSIC_PlaySong(const char *alias)
{
    int             channel_number;
    song_t         *song;
    int             songnum;
    int             channel_to_play_on;
    int             fading_song;
    openal_channel *song_channel;
    unsigned int    loop_start;
    int             rate;

    fading_song = 0;
    songnum     = MUSIC_FindSong(alias);

    if (songnum == -1) {
        return true;
    }

    song = &music_songs[songnum];

    if (MUSIC_Playing() && songnum == music_currentsong) {
        return true;
    }

    channel_number    = MUSIC_CurrentSongChannel();
    music_currentsong = songnum;

    if (channel_number != -1) {
        if (song->flags & 4) {
            MUSIC_StopChannel(channel_number);
        } else {
            song_channel                  = &openal.chan_song[channel_number];
            song_channel->fading          = FADE_OUT;
            song_channel->fade_time       = (int)song->fadetime;
            song_channel->fade_start_time = cls.realtime;
            fading_song                   = true;
        }
    }
    channel_to_play_on = (fading_song != 0) && (channel_number == 0);
    song_channel       = &openal.chan_song[channel_to_play_on];

    if (song_channel->is_playing() || song_channel->is_paused()) {
        MUSIC_StopChannel(channel_to_play_on);
    }

    //if (!S_OPENAL_LoadMP3(FS_BuildOSPath(Cvar_VariableString("fs_basepath"), FS_Gamedir(), song->path), song_channel)) {
    if (!S_OPENAL_LoadMP3(song->path, song_channel)) {
        Com_DPrintf("Could not start music file '%s'!", song->path);
        return false;
    }

    rate = song_channel->sample_playback_rate();

    song_channel->song_number = songnum;
    if (song->current_pos) {
        song_channel->set_sample_ms_offset(song->current_pos);
    } else {
        song_channel->set_sample_offset(rate * 0.063f);
    }

    if (song->flags & 1) {
        song_channel->set_sample_loop_count(0);
        song_channel->set_sample_loop_block(rate * 0.063f, -1);
    } else {
        song_channel->set_sample_loop_count(1);
    }

    if (fading_song) {
        song_channel->fading    = FADE_IN;
        song_channel->fade_time = (int)song->fadetime;
        song_channel->set_gain(0.0);
        song_channel->fade_start_time = cls.realtime;
    } else {
        song_channel->fading = FADE_NONE;
        song_channel->set_gain(S_GetBaseVolume() * (song->volume * s_ambientVolume->value) * 84.f);
    }

    song_channel->play();

    return true;
}

/*
==============
MUSIC_UpdateMusicVolumes
==============
*/
void MUSIC_UpdateMusicVolumes()
{
    int          i;
    unsigned int current_time;
    float        new_volume, max_volume;

    if (s_ambientVolume->modified || music_volume_changed) {
        s_ambientVolume->modified = false;

        for (i = 0; i < MAX_OPENAL_SONGS; i++) {
            if (!openal.chan_song[i].is_playing() && !openal.chan_song[i].is_paused()) {
                continue;
            }

            if (openal.chan_song[i].fading != FADE_NONE) {
                continue;
            }

            openal.chan_song[i].set_gain(
                S_GetBaseVolume() * (music_songs[openal.chan_song[i].song_number].volume * s_ambientVolume->value)
                * 84.0 * music_volume
            );
        }
    }

    for (i = 0; i < MAX_OPENAL_SONGS; i++) {
        if (!openal.chan_song[i].is_playing() && !openal.chan_song[i].is_paused()) {
            continue;
        }

        switch (openal.chan_song[i].fading) {
        case fade_t::FADE_IN:
            max_volume = music_songs[openal.chan_song[i].song_number].volume * s_ambientVolume->value;
            new_volume = (unsigned int)(cls.realtime - openal.chan_song[i].fade_start_time)
                       / (openal.chan_song[i].fade_time * 1000.f) * max_volume;

            if (new_volume > max_volume) {
                openal.chan_song[i].set_gain(S_GetBaseVolume() * (max_volume * 84.f * music_volume));
                openal.chan_song[i].fading = FADE_NONE;
            } else {
                openal.chan_song[i].set_gain(S_GetBaseVolume() * (new_volume * 84.f * music_volume));
            }
            break;
        case fade_t::FADE_OUT:
            max_volume = music_songs[openal.chan_song[i].song_number].volume * s_ambientVolume->value;
            new_volume = (unsigned int)(cls.realtime - openal.chan_song[i].fade_start_time)
                       / (openal.chan_song[i].fade_time * 1000.f) * max_volume;

            if (new_volume > 0) {
                openal.chan_song[i].set_gain(S_GetBaseVolume() * (new_volume * 84.f * music_volume));
            } else {
                MUSIC_StopChannel(i);
            }
            break;
        default:
            break;
        }
    }

    if (s_musicVolume->modified || music_volume_changed) {
        s_musicVolume->modified = false;
        if (openal.chan_trig_music.is_playing() || openal.chan_trig_music.is_paused()) {
            openal.chan_trig_music.set_gain(
                S_GetBaseVolume() * (openal.chan_trig_music.fVolume * s_musicVolume->value) * 84.f
            );
        }
    }

    music_volume_changed = false;
}

/*
==============
MUSIC_CheckForStoppedSongs
==============
*/
void MUSIC_CheckForStoppedSongs()
{
    int i;

    for (i = 0; i < MAX_OPENAL_SONGS; i++) {
        if (!openal.chan_song[i].is_playing()) {
            continue;
        }

        if (openal.chan_song[i].sample_loop_count()) {
            continue;
        }

        MUSIC_FindSong(MusicMood_NumToName(music_fallback_mood));
        if (!openal.chan_song[i].is_playing() && !openal.chan_song[i].is_paused()) {
            MUSIC_SongEnded();
        }
    }
}

/*
==============
S_TriggeredMusic_SetupHandle
==============
*/
void S_TriggeredMusic_SetupHandle(const char *pszName, int iLoopCount, int iOffset, qboolean autostart)
{
    char            *pszFilename;
    const char      *pszRealName;
    float            fVolume     = 1.0;
    AliasListNode_t *pSoundAlias = NULL;

    if (!s_bSoundStarted) {
        return;
    }

    if (openal.chan_trig_music.is_playing() || openal.chan_trig_music.is_paused()) {
        Com_DPrintf("Didn't start new triggered music because some was already playing\n");
        return;
    }

    openal.chan_trig_music.stop();
    // Fixed in OPM
    //  Use strncpy instead
    //strcpy(openal.tm_filename, pszName);
    Q_strncpyz(openal.tm_filename, pszName, sizeof(openal.tm_filename));

    openal.tm_loopcount = iLoopCount;
    openal.chan_trig_music.set_sample_loop_count(iLoopCount);
    pszRealName = Alias_FindRandom(pszName, &pSoundAlias);

    if (!pszRealName) {
        pszRealName = pszName;
    } else if (pSoundAlias) {
        fVolume = random() * pSoundAlias->volumeMod + pSoundAlias->volume;
    }

    pszFilename = FS_BuildOSPath(Cvar_VariableString("fs_basepath"), FS_Gamedir(), pszRealName);

    //if (!S_OPENAL_LoadMP3(pszFilename, &openal.chan_trig_music)) {
    if (!S_OPENAL_LoadMP3(pszRealName, &openal.chan_trig_music)) {
        S_OPENAL_InitChannel(OPENAL_CHANNEL_TRIGGER_MUSIC_ID, &openal.chan_trig_music);
        Com_DPrintf("Could not start triggered music '%s'\n", pszName);
        return;
    }

    openal.chan_trig_music.fVolume = fVolume;
    openal.chan_trig_music.set_gain(S_GetBaseVolume() * (fVolume * s_musicVolume->value) * 84.f);
    openal.chan_trig_music.set_sample_loop_count(iLoopCount);
    openal.chan_trig_music.set_sample_offset(iOffset);

    if (autostart) {
        openal.chan_trig_music.play();
    }
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

    S_TriggeredMusic_SetupHandle(Cmd_Argv(1), 1, 0, true);
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

    S_TriggeredMusic_SetupHandle(Cmd_Argv(1), 0, 0, true);
}

/*
==============
S_TriggeredMusic_Stop
==============
*/
void S_TriggeredMusic_Stop()
{
    if (!s_bSoundStarted) {
        return;
    }

    openal.chan_trig_music.stop();
}

/*
==============
S_TriggeredMusic_Pause
==============
*/
void S_TriggeredMusic_Pause()
{
    if (!s_bSoundStarted) {
        return;
    }

    if (openal.chan_trig_music.is_playing()) {
        openal.chan_trig_music.pause();
    }
}

/*
==============
S_TriggeredMusic_Unpause
==============
*/
void S_TriggeredMusic_Unpause()
{
    if (!s_bSoundStarted) {
        return;
    }

    if (openal.chan_trig_music.is_paused()) {
        openal.chan_trig_music.play();
    }

    openal.chan_trig_music.set_gain(S_GetBaseVolume() * (openal.chan_trig_music.fVolume * s_musicVolume->value) * 84.f);
}

/*
==============
S_TriggeredMusic_PlayIntroMusic
==============
*/
void S_TriggeredMusic_PlayIntroMusic()
{
    S_TriggeredMusic_SetupHandle("sound/music/mus_MainTheme.mp3", 0, 0, true);
}

/*
==============
S_StopMovieAudio
==============
*/
void S_StopMovieAudio()
{
    STUB_DESC("sound stuff.");
}

/*
==============
S_SetupMovieAudio
==============
*/
void S_SetupMovieAudio(const char *pszMovieName)
{
    STUB_DESC("sound stuff");
}

/*
==============
S_CurrentMoviePosition
==============
*/
int S_CurrentMoviePosition()
{
    STUB_DESC("sound stuff");
    return 0;
}

/*
=================
S_AL_Format
=================
*/
static ALuint S_OPENAL_Format(int width, int channels)
{
    ALuint format = AL_FORMAT_MONO16;

    // Work out format
    if (width == 1) {
        if (channels == 1) {
            format = AL_FORMAT_MONO8;
        } else if (channels == 2) {
            format = AL_FORMAT_STEREO8;
        }
    } else if (width == 2) {
        if (channels == 1) {
            format = AL_FORMAT_MONO16;
        } else if (channels == 2) {
            format = AL_FORMAT_STEREO16;
        }
    }

    return format;
}

/*
==============
S_OPENAL_LoadMP3_Codec
==============
*/
static bool S_OPENAL_LoadMP3_Codec(const char *_path, openal_channel *chan)
{
    void      *data;
    snd_info_t info;
    ALuint     format;

    // Try to load
    data = S_CodecLoad(_path, &info);
    if (!data) {
        return false;
    }

    format = S_OPENAL_Format(info.width, info.channels);

    // Create a buffer
    qalGenBuffers(1, &chan->buffer);
    alDieIfError();

    // Fill the buffer
    if (info.size == 0) {
        // We have no data to buffer, so buffer silence
        byte dummyData[2] = {0};

        qalBufferData(chan->buffer, AL_FORMAT_MONO16, (void *)dummyData, 2, 22050);
    } else {
        qalBufferData(chan->buffer, format, data, info.size, info.rate);
    }

    alDieIfError();

    // Free the memory
    Hunk_FreeTempMemory(data);

    qalSourcei(chan->source, AL_BUFFER, chan->buffer);
    alDieIfError();

    chan->set_no_3d();

    return true;
}

#endif
