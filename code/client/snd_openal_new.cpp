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

#include "snd_local.h"
#include "snd_openal_new.h"
#include "client.h"
#include "../server/server.h"
#include "snd_codec.h"

typedef struct {
    const char *funcname;
    void      **funcptr;
    bool        required;
} extensions_table_t;

#define MAX_MUSIC_SONGS 16

static int   s_iNextLoopingWarning = 0;
static int   s_iReverbType         = 0;
static float s_fReverbLevel        = 0;
static bool  s_bReverbChanged      = false;
static bool  s_bFading             = false;
static float s_fFadeVolume         = 1.f;
static float s_fVelocityScale      = 0.005f;

static constexpr float s_fVolumeGain = 1.f; // = 84.f;

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
cvar_t *s_alAvailableDevices;

static float reverb_table[] = {
    0.5f,   0.25f,        0.417f, 0.653f,      0.208f,      0.5f,   0.403f, 0.5f,   0.5f,
    0.361f, 0.5f,         0.153f, 0.361f,      0.44400001f, 0.25f,  0.111f, 0.111f, 0.19400001f,
    1.0f,   0.097000003f, 0.208f, 0.65200001f, 1.0f,        0.875f, 0.139f, 0.486f,
};

static vec3_t       vec_zero                   = {0, 0, 0};
int                 s_iNumMilesAudioProviders  = 0;
bool                s_bProvidersEmunerated     = false;
static bool         al_initialized             = false;
static bool         al_use_reverb              = false;
static float        al_current_volume          = 0;
static unsigned int al_frequency               = 22050;
static ALCcontext  *al_context_id              = NULL;
static ALCdevice   *al_device                  = NULL;
static ALsizei      al_default_resampler_index = 0;
static ALsizei      al_resampler_index         = 0;

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

static qboolean enumeration_ext     = qfalse;
static qboolean enumeration_all_ext = qfalse;

static qboolean ima4_ext         = qfalse;
static qboolean soft_block_align = qfalse;

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
static void   S_OPENAL_reverb(int iChannel, int iReverbType, float fReverbLevel);
static bool   S_OPENAL_LoadMP3_Codec(const char *_path, sfx_t *pSfx);
static ALuint S_OPENAL_Format(float width, int channels);

#define alDieIfError() __alDieIfError(__FILE__, __LINE__)

#if defined(_WIN64)
#    define ALDRIVER_DEFAULT "OpenAL64.dll"
#elif defined(_WIN32)
#    define ALDRIVER_DEFAULT "OpenAL32.dll"
#elif defined(__APPLE__)
#    define ALDRIVER_DEFAULT "libopenal.1.dylib"
#elif defined(__OpenBSD__)
#    define ALDRIVER_DEFAULT "libopenal.so"
#else
#    define ALDRIVER_DEFAULT "libopenal.so.1"
#endif

//
// alext
//
#ifdef AL_SOFT_source_resampler
LPALGETSTRINGISOFT qalGetStringiSOFT;
#endif

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

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS; ++i) {
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
    int         attrlist[12];

    Com_DPrintf("OpenAL: Context initialization\n");

    dev = NULL;
    if (s_openaldevice) {
        dev = s_openaldevice->string;
    }

    if (dev && !*dev) {
        dev = NULL;
    }

    // Device enumeration support
    enumeration_all_ext = qalcIsExtensionPresent(NULL, "ALC_ENUMERATE_ALL_EXT");
    enumeration_ext     = qalcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");

    if (enumeration_ext || enumeration_all_ext) {
        char        devicenames[16384] = "";
        const char *devicelist;
#ifdef _WIN32
        const char *defaultdevice;
#endif
        int curlen;

        // get all available devices + the default device name.
        if (enumeration_all_ext) {
            devicelist = qalcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER);
#ifdef _WIN32
            defaultdevice = qalcGetString(NULL, ALC_DEFAULT_ALL_DEVICES_SPECIFIER);
#endif
        } else {
            // We don't have ALC_ENUMERATE_ALL_EXT but normal enumeration.
            devicelist = qalcGetString(NULL, ALC_DEVICE_SPECIFIER);
#ifdef _WIN32
            defaultdevice = qalcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
#endif
            enumeration_ext = qtrue;
        }

#ifdef _WIN32
        Com_DPrintf("OpenAL: Default playback device: \"%s\"\n", defaultdevice);
#endif

#ifdef _WIN32
        // check whether the default device is generic hardware. If it is, change to
        // Generic Software as that one works more reliably with various sound systems.
        // If it's not, use OpenAL's default selection as we don't want to ignore
        // native hardware acceleration.
        if (!dev && defaultdevice && !strcmp(defaultdevice, "Generic Hardware")) {
            dev = "Generic Software";
        }
#endif

        // dump a list of available devices to a cvar for the user to see.

        if (devicelist) {
            while ((curlen = strlen(devicelist))) {
                Q_strcat(devicenames, sizeof(devicenames), devicelist);
                Q_strcat(devicenames, sizeof(devicenames), "\n");

                devicelist += curlen + 1;
            }
        }

        Com_Printf("OpenAL: List of available devices:\n%s\n", devicenames);

        s_alAvailableDevices = Cvar_Get("s_alAvailableDevices", devicenames, CVAR_ROM | CVAR_NORESTART);
    }

    Com_Printf("OpenAL: Opening device \"%s\"...\n", dev ? dev : "{default}");

    al_device = qalcOpenDevice(dev);
    if (!al_device && dev) {
        Com_Printf("Failed to open OpenAL device '%s', trying default.\n", dev);
        al_device = qalcOpenDevice(NULL);
    }

    if (!al_device) {
        Com_Printf("OpenAL: Could not open device\n");
        S_OPENAL_NukeContext();
        return false;
    }

    Com_Printf("OpenAL: Device opened successfully.\n");
    switch (s_khz->integer) {
    case 11:
        al_frequency = 11025;
        break;
    default:
    case 22:
        al_frequency = 22050;
        break;
    case 44:
        al_frequency = 44100;
        break;
    }

    attrlist[0] = ALC_FREQUENCY;
    attrlist[1] = al_frequency;
    attrlist[2] = ALC_SYNC;
    attrlist[3] = qfalse;

    s_speaker_type->modified = false;

#ifdef ALC_SOFT_output_mode
    attrlist[4] = ALC_OUTPUT_MODE_SOFT;
    // Disable HRTF by default
    // For example, actual speakers that are recognized as headphones by the OS
    // will not get forced HRTF
    attrlist[6] = ALC_HRTF_SOFT;
    attrlist[7] = ALC_FALSE;

    switch (s_speaker_type->integer) {
    // Two speakers
    default:
    case 0:
        attrlist[5] = ALC_STEREO_SOFT;
        break;
    // Headphones
    case 1:
        attrlist[5] = ALC_STEREO_HRTF_SOFT;
        // Allow HRTF mixing (without forcing in case it's unsupported)
        attrlist[7] = ALC_DONT_CARE_SOFT;
        break;
    // Surround
    case 2:
        attrlist[5] = ALC_SURROUND_5_1_SOFT;
        break;
    // Quad speakers
    case 3:
        attrlist[5] = ALC_QUAD_SOFT;
        break;
    }
#else
#    pragma message("OpenAL: ALC_OUTPUT_MODE_SOFT unavailable")

    Com_Printf(
        "OpenAL: ALC_OUTPUT_MODE_SOFT is unavailable. The speaker type will be ignored, fallback to normal stereo.\n"
    );
    attrlist[4] = 0;
    attrlist[5] = 0;
#endif

#ifdef ALC_SOFT_output_limiter
    // Disable limiter
    attrlist[8] = ALC_OUTPUT_LIMITER_SOFT;
    attrlist[9] = ALC_FALSE;
#endif
    attrlist[10] = 0;
    attrlist[11] = 0;

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
    /*
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
    */

    extensions_table_t extensions_table[] = {
#ifdef AL_SOFT_source_resampler
        extensions_table_t {
                            "alGetStringiSOFT", (void **)&qalGetStringiSOFT,
                            false, },
#endif
        extensions_table_t {NULL, NULL, NULL}
    };

    extensions_table_t *i;

    for (i = extensions_table; i->funcname; ++i) {
        *i->funcptr = NULL;
    }

    for (i = extensions_table; i->funcname; ++i) {
        Com_Printf("AL extension: Looking up %ssymbol \"%s\"...", i->required ? "required " : "", i->funcname);

        *i->funcptr = qalGetProcAddress(i->funcname);
        if (!*i->funcptr) {
            Com_Printf("...not found! %d [%s]\n", qalGetError(), qalGetString(qalGetError()));
            if (i->required) {
                S_OPENAL_NukeContext();
                return false;
            }
            continue;
        }

        Com_Printf("...found.\n");
    }

    ima4_ext         = qalIsExtensionPresent("AL_EXT_IMA4");
    soft_block_align = qalIsExtensionPresent("AL_SOFT_block_alignment");

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
    openal.channel[idx] = chan;
    VectorClear(chan->vOrigin);
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

#ifdef AL_SOFT_source_resampler
    qalSourcei(chan->source, AL_SOURCE_RESAMPLER_SOFT, al_resampler_index);
    alDieIfError();
#endif

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

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS; i++) {
        openal.channel[i] = NULL;
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_LOOP_SOUNDS; i++) {
        openal.loop_sounds[i] = {};
    }

    s_milesdriver            = Cvar_Get("s_milesdriver", "auto", CVAR_SOUND_LATCH | CVAR_ARCHIVE);
    s_openaldevice           = Cvar_Get("s_openaldevice", "", CVAR_SOUND_LATCH);
    s_reverb                 = Cvar_Get("s_reverb", "0", CVAR_SOUND_LATCH | CVAR_ARCHIVE);
    s_show_cpu               = Cvar_Get("s_show_cpu", "0", 0);
    s_show_num_active_sounds = Cvar_Get("s_show_num_active_sounds", "0", 0);
    s_show_sounds            = Cvar_Get("s_show_sounds", "0", 0);
    s_speaker_type           = Cvar_Get("s_speaker_type", "0", CVAR_ARCHIVE);
    s_obstruction_cal_time   = Cvar_Get("s_obstruction_cal_time", "500", CVAR_ARCHIVE);
    //
    // Added in OPM
    //  Initialize the AL driver DLL
    s_openaldriver = Cvar_Get("s_openaldriver", ALDRIVER_DEFAULT, CVAR_LATCH | CVAR_PROTECTED);

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
    s_reverb->modified = false;

    al_current_volume = Square(s_volume->value);
    qalListenerf(AL_GAIN, al_current_volume);
    alDieIfError();

#ifdef AL_SOFT_source_resampler
    if (qalGetStringiSOFT) {
        size_t numResamplers;
        size_t i;

        al_default_resampler_index = qalGetInteger(AL_DEFAULT_RESAMPLER_SOFT);
        alDieIfError();
        al_resampler_index = al_default_resampler_index;
        numResamplers      = qalGetInteger(AL_NUM_RESAMPLERS_SOFT);
        alDieIfError();

        for (i = 0; i < numResamplers; i++) {
            const ALchar *resamplerName = qalGetStringiSOFT(AL_RESAMPLER_NAME_SOFT, i);
            if (Q_stristr(resamplerName, "spline")) {
                Com_Printf("OpenAL: Using %s as the resampler.\n", resamplerName);
                al_resampler_index = i;
                break;
            }
        }
    }
#endif

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS_3D; i++) {
        if (!S_OPENAL_InitChannel(i, &openal.chan_3D[i])) {
            return false;
        }
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS_2D; i++) {
        if (!S_OPENAL_InitChannel(i + MAX_SOUNDSYSTEM_CHANNELS_3D, &openal.chan_2D[i])) {
            return false;
        }
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS_2D_STREAM; i++) {
        if (!S_OPENAL_InitChannel(
                i + MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D, &openal.chan_2D_stream[i]
            )) {
            return false;
        }
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_SONGS; i++) {
        if (!S_OPENAL_InitChannel(
                i + MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D + MAX_SOUNDSYSTEM_CHANNELS_2D_STREAM,
                &openal.chan_song[i]
            )) {
            return false;
        }
    }

    if (!S_OPENAL_InitChannel(SOUNDSYSTEM_CHANNEL_MP3_ID, &openal.chan_mp3)) {
        return false;
    }

    if (!S_OPENAL_InitChannel(SOUNDSYSTEM_CHANNEL_TRIGGER_MUSIC_ID, &openal.chan_trig_music)) {
        return false;
    }

    if (!S_OPENAL_InitChannel(SOUNDSYSTEM_CHANNEL_MOVIE_ID, &openal.chan_movie)) {
        return false;
    }

    //
    // Disable sound virtualization on special channels
    //  (triggered music, ambient sounds...)
    //
    for (i = 0; i < MAX_SOUNDSYSTEM_SONGS; i++) {
        openal.chan_song[i].set_no_virtualization();
    }
    openal.chan_mp3.set_no_virtualization();
    openal.chan_trig_music.set_no_virtualization();
    openal.chan_movie.set_no_virtualization();

    Cmd_AddCommand("playmp3", S_OPENAL_PlayMP3);
    Cmd_AddCommand("stopmp3", S_OPENAL_StopMP3);
    Cmd_AddCommand("loadsoundtrack", S_loadsoundtrack);
    Cmd_AddCommand("playsong", S_PlaySong);
    Cmd_AddCommand("pitch", S_OPENAL_Pitch);
    Cmd_AddCommand("tmstart", S_TriggeredMusic_Start);
    Cmd_AddCommand("tmstartloop", S_TriggeredMusic_StartLoop);
    Cmd_AddCommand("tmstop", S_TriggeredMusic_Stop);
    // Added in 2.0
    Cmd_AddCommand("tmvolume", S_TriggeredMusic_Volume);

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
    // Added in 2.0
    Cmd_RemoveCommand("tmvolume");

    S_OPENAL_NukeContext();

    s_bProvidersEmunerated = false;
    al_initialized         = false;

    QAL_Shutdown();
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
        // Fixed in OPM
        //  Make sure to restore the music volume
        //  in case the map restarts immediately after the intermission
        music_volume_changed = true;
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

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS_3D; i++) {
        S_DumpStatus("3D", i, openal.channel[i]);
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS_2D; i++) {
        S_DumpStatus("2D", i, openal.channel[MAX_SOUNDSYSTEM_CHANNELS_3D + i]);
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS_2D_STREAM; i++) {
        S_DumpStatus("2D stream", i, openal.channel[MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D + i]);
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_MISC_CHANNELS; i++) {
        S_DumpStatus(
            "Misc",
            i,
            openal.channel
                [MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D + MAX_SOUNDSYSTEM_CHANNELS_2D_STREAM + i]
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
    char       path[MAX_QPATH];
    snd_info_t info;
    ALuint     format;

    chan->stop();

    qalSourcei(chan->source, AL_BUFFER, 0);
    alDieIfError();

    Q_strncpyz(path, _path, sizeof(path));
    path[MAX_QPATH - 1] = 0;

    FS_CorrectCase(path);

    // Try to load
    chan->bufferdata = (ALubyte *)S_CodecLoad(path, &info);
    if (!chan->bufferdata) {
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
        qalBufferData(chan->buffer, format, chan->bufferdata, info.size, info.rate);
    }

    alDieIfError();

    // Free the memory
    Hunk_FreeTempMemory(chan->bufferdata);
    chan->bufferdata = NULL;

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

    for (i = 0; i < MAX_SOUNDSYSTEM_SONGS; i++) {
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

    for (i = 0; i < MAX_SOUNDSYSTEM_SONGS; i++) {
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

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
        openal_channel *pChannel = openal.channel[i];
        if (!pChannel) {
            continue;
        }

        if (pChannel->is_paused() || (pChannel->iFlags & CHANNEL_FLAG_PLAY_DEFERRED)) {
            pChannel->iFlags &= ~CHANNEL_FLAG_PLAY_DEFERRED;
            pChannel->play();
        }
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

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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
                for (i = iLastChannel + 1; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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
    return S_OPENAL_PickChannelBase(iEntNum, iEntChannel, 0, MAX_SOUNDSYSTEM_CHANNELS_3D - 1);
}

/*
==============
S_OPENAL_PickChannel2D
==============
*/
static int S_OPENAL_PickChannel2D(int iEntNum, int iEntChannel)
{
    return S_OPENAL_PickChannelBase(
        iEntNum, iEntChannel, MAX_SOUNDSYSTEM_CHANNELS_3D, MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D - 1
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
        MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D,
        MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D + MAX_SOUNDSYSTEM_CHANNELS_2D_STREAM - 1
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

    if (pSfx->iFlags & SFX_FLAG_STREAMED) {
        iFreeChannel = S_OPENAL_PickChannel2DStreamed(iRealEntNum, iEntChannel);
    } else {
        iFreeChannel = S_OPENAL_PickChannel2D(iRealEntNum, iEntChannel);
    }

    if (iFreeChannel < 0) {
        Com_DPrintf(
            "OpenAL: Couldn't play %s sound '%s' for entity %i on channel %s\n",
            (pSfx->iFlags & SFX_FLAG_STREAMED) ? "2Dstreamed" : "2D",
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
            VectorCopy(vOrigin, pChannel->vOrigin);
        }
    } else {
        pChannel->iFlags &= ~CHANNEL_FLAG_NO_ENTITY;
        pChannel->iEntNum = iRealEntNum;
        if (vOrigin) {
            VectorCopy(vOrigin, pChannel->vOrigin);

            bSupportWaitTillSoundDone = cl.serverTime - 1 < 0;
            pChannel->iTime           = cl.serverTime - 1;
            if (bSupportWaitTillSoundDone) {
                pChannel->iTime = 0;
            }

            if (s_entity[iEntNum].time < pChannel->iTime) {
                // Fixed in OPM
                //  Not sure if it's the real solution, but script_origin entities are usually
                //  never sent to client so the sound will immediately get stopped
                pChannel->iFlags |= CHANNEL_FLAG_MISSING_ENT;
            }
        } else {
            VectorClear(pChannel->vOrigin);
            pChannel->iTime = 0;
        }
    }

    pChannel->iStartTime = cl.serverTime;
    pChannel->iEndTime   = (int)(cl.serverTime + pSfx->time_length + 250.f);

    if (iFreeChannel > MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D) {
        // streamed
        if (!pChannel->set_sfx(pSfx)) {
            Com_DPrintf("OpenAL: Set stream error - %s\n", pSfx->name);
            return;
        }

        pChannel->iBaseRate = pChannel->sample_playback_rate();
        pChannel->set_no_3d();
        fRealVolume = fRealVolume * s_fVolumeGain;
    } else {
        pChannel->stop();
        pChannel->set_no_3d();
        pChannel->set_sfx(pSfx);
    }

    pChannel->set_gain(fRealVolume);
    pChannel->play();

    if (s_show_sounds->integer > 0) {
        Com_DPrintf(
            "OpenAL: 2D - %d (#%i) - %s (vol %f, mindist %f, maxdist %f)\n",
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
        pSfx->iFlags |= SFX_FLAG_STREAMED;
    }

    if (!S_OPENAL_ShouldPlay(pSfx)) {
        Com_DPrintf("OpenAL: ^~^~^ Not playing sound '%s'\n", pSfx->name);
        return;
    }

    if ((pSfx->iFlags & (SFX_FLAG_NO_OFFSET) || pSfx->iFlags & (SFX_FLAG_STREAMED | SFX_FLAG_MP3))
        || iEntChannel == CHAN_MENU || iEntChannel == CHAN_LOCAL) {
        S_OPENAL_Start2DSound(vOrigin, iEntNum, iEntChannel, pSfx, fVolume, fMinDist, fPitch, fMaxDist);
        return;
    }

    bSupportWaitTillSoundDone = (iEntNum & S_FLAG_DO_CALLBACK) != 0;
    iEntNum &= ~S_FLAG_DO_CALLBACK;

    pSfxInfo = &sfx_infos[pSfx->sfx_info_index];
    if (pSfx->iFlags & SFX_FLAG_STREAMED) {
        Com_DPrintf("OpenAL: 3D sounds not supported - couldn't play '%s'\n", pSfx->name);
        return;
    }

    iChannel = S_OPENAL_PickChannel3D(iEntNum, iEntChannel);
    if (iChannel < 0) {
        Com_DPrintf(
            "OpenAL: Couldn't play %s sound '%s' for entity %i on channel %s\n",
            (pSfx->iFlags & SFX_FLAG_STREAMED) ? "3Dstreamed" : "3D",
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
    state = pChannel->sample_status();

    if (pChannel->iEntNum == iEntNum && (state == AL_PLAYING || state == AL_PAUSED) && pChannel->pSfx == pSfx) {
        bOnlyUpdate = true;
    } else {
        pChannel->stop();
        pChannel->iFlags &= ~(CHANNEL_FLAG_LOOPING | CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_LOCAL_LISTENER);
        if (!pChannel->set_sfx(pSfx)) {
            Com_DPrintf("OpenAL: Set sample error - %s\n", pSfx->name);
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

    // Fixed in OPM
    //  Setup the channel for 3D
    pChannel->set_3d();

    if (s_show_sounds->integer > 0) {
        Com_DPrintf(
            "OpenAL: %d (#%i) - %s (vol %f, mindist %f, maxdist %f)\n",
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
            VectorCopy(vOrigin, pChannel->vOrigin);
        } else {
            //VectorClear(vOrigin);
            // Fixed in OPM
            //  Tiny mistake found in original where the vOrigin parameter is set to 0
            VectorClear(pChannel->vOrigin);
        }

        pChannel->set_position(pChannel->vOrigin[0], pChannel->vOrigin[1], pChannel->vOrigin[2]);
        pChannel->iFlags |= CHANNEL_FLAG_NO_ENTITY;
        pChannel->iEntNum = 0;
    } else {
        pChannel->iFlags &= ~CHANNEL_FLAG_NO_ENTITY;
        pChannel->iEntNum = iEntNum;
        if (vOrigin) {
            VectorCopy(vOrigin, pChannel->vOrigin);

            bSupportWaitTillSoundDone = cl.serverTime - 1 < 0;
            pChannel->iTime           = cl.serverTime - 1;
            if (bSupportWaitTillSoundDone) {
                pChannel->iTime = 0;
            }

            if (s_entity[iEntNum].time < pChannel->iTime) {
                // Fixed in OPM
                //  Not sure if it's the real solution, but script_origin entities are usually
                //  never sent to client so the sound will immediately get stopped
                pChannel->iFlags |= CHANNEL_FLAG_MISSING_ENT;
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
            Com_DPrintf("OpenAL: loopblock - %d to %d\n", pSfxInfo->loop_start, pSfxInfo->loop_end);
        }
    } else {
        pChannel->set_sample_loop_count(1);
    }

    if (!bOnlyUpdate && S_OPENAL_ShouldStart(pChannel->vOrigin, pChannel->fMinDist, pChannel->fMaxDist)) {
        // Fixed in OPM
        //  Make sure to set the pitch before playing the sound
        pChannel->iBaseRate = pSfx->info.rate;
        pChannel->set_sample_playback_rate(pChannel->iBaseRate * pChannel->fNewPitchMult);
        pChannel->fNewPitchMult = 0;

        pChannel->play();
        pChannel->iEndTime   = cl.serverTime + (int)pChannel->pSfx->time_length + 250;
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
    if (!pSfx || !pSfx->length) {
        return;
    }

    for (i = 0; i < (MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D); i++) {
        pLoopSound = &openal.loop_sounds[i];
        if (pLoopSound->pSfx == pSfx && !pLoopSound->bInUse) {
            iFreeLoopSound = i;
            break;
        }
    }

    if (iFreeLoopSound < 0) {
        for (i = 0; i < (MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D); i++) {
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
            Com_DPrintf("OpenAL: Too many looping sounds\n");
            s_iNextLoopingWarning = cls.realtime + 1000;
        }
        return;
    }

    pLoopSound = &openal.loop_sounds[iFreeLoopSound];
    VectorCopy(vOrigin, pLoopSound->vOrigin);
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
        for (i = 0; i < (MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D); i++) {
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
        for (i = 0; i < (MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D); i++) {
            if (openal.loop_sounds[i].pSfx == pLoopSound->pSfx && openal.loop_sounds[i].bInUse) {
                bMayStop = false;
                break;
            }
        }
    }

    if (bMayStop) {
        if (s_show_sounds->integer > 0) {
            Com_DPrintf(
                "OpenAL: %d (#%i) - stopped loop - %s\n",
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
        for (i = 0; i < (MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D); i++) {
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

    for (i = 0; i < (MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D); i++) {
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

    for (i = 0; i < (MAX_SOUNDSYSTEM_CHANNELS_3D + MAX_SOUNDSYSTEM_CHANNELS_2D); i++) {
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

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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

    S_OPENAL_StopLoopingSounds();

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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
    int             iSoundOffset;
    openal_channel *pChannel;

    if (pLoopSound->pSfx->iFlags & SFX_FLAG_STREAMED) {
        iChannel = S_OPENAL_PickChannel2DStreamed(0, 0);
    } else {
        iChannel = S_OPENAL_PickChannel2D(0, 0);
    }

    if (iChannel < 0) {
        Com_DPrintf("OpenAL: Could not find a free 2D sound channel\n");
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
        Com_DPrintf("OpenAL: Set sample error (loopsound) - %s\n", pLoopSound->pSfx->name);
        pChannel->iFlags &= ~CHANNEL_FLAG_PLAY_DEFERRED;
        return -1;
    }

    pChannel->iEntNum     = 0;
    pChannel->iEntChannel = 0;
    pChannel->pSfx        = pLoopSound->pSfx;
    pChannel->iFlags |= CHANNEL_FLAG_PAUSED | CHANNEL_FLAG_NO_ENTITY;
    pChannel->iBaseRate = pChannel->sample_playback_rate();
    VectorCopy(vLoopOrigin, pChannel->vOrigin);

    iSoundOffset = (int)(pLoopSound->pSfx->info.width * pLoopSound->pSfx->info.rate
                         * (float)(cls.realtime - pLoopSound->iStartTime) / 1000.0);
    pChannel->set_sample_offset(iSoundOffset % pLoopSound->pSfx->length);
    pChannel->set_sample_loop_count(0);
    pChannel->fVolume = fVolumeToPlay;
    pChannel->set_gain(fVolumeToPlay);
    pChannel->start_sample();
    if (s_show_sounds->integer > 0) {
        Com_DPrintf(
            "OpenAL: %d (#%i) - %s (vol %f)\n", cl.serverTime, pLoopSound->iChannel, pLoopSound->pSfx->name, fVolume
        );
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

    if (pLoopSound->pSfx->iFlags & SFX_FLAG_STREAMED) {
        return -1;
    }

    if (!pLoopSound->pSfx->data || !pLoopSound->pSfx->length) {
        return -1;
    }

    iChannel = S_OPENAL_PickChannel3D(0, 0);
    if (iChannel < 0) {
        Com_DPrintf("OpenAL: Could not find a free channel\n");
        return iChannel;
    }

    pChan3D = &openal.chan_3D[iChannel];
    pChan3D->force_free();
    pChan3D->iEntNum     = 0;
    pChan3D->iEntChannel = 0;
    pChan3D->fMinDist    = fMinDistance;
    pChan3D->fMaxDist    = fMaxDistance;
    pChan3D->set_3d();

    if (!pChan3D->set_sfx(pLoopSound->pSfx)) {
        Com_DPrintf("OpenAL: Set sample error - %s\n", pLoopSound->pSfx->name);
        return -1;
    }

    pChan3D->set_position(vLoopOrigin[0], vLoopOrigin[1], vLoopOrigin[2]);
    VectorScale(pLoopSound->vVelocity, s_fVelocityScale, pLoopSound->vVelocity);
    pChan3D->set_velocity(pLoopSound->vVelocity[0], pLoopSound->vVelocity[1], pLoopSound->vVelocity[2]);
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

    if (pLoopSound->pSfx->iFlags & (SFX_FLAG_NO_OFFSET) || pLoopSound->pSfx->iFlags & (SFX_FLAG_STREAMED | SFX_FLAG_MP3)
        || (pLoopSound->iFlags & LOOPSOUND_FLAG_NO_PAN)) {
        vec3_t vOrigin;
        int    iPan;

        pChannel->fVolume = fVolumeToPlay / s_fVolumeGain;
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
    bool                 bAlreadyAdded[MAX_SOUNDSYSTEM_LOOP_SOUNDS] = {false};
    vec3_t               alvec;

    qalGetListenerfv(AL_POSITION, alvec);
    VectorCopy(alvec, vListenerOrigin);

    for (i = 0; i < MAX_SOUNDSYSTEM_LOOP_SOUNDS; i++) {
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
                        "OpenAL: %d (#%i) - stopped loop - %s\n",
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
                S_GetBaseVolume() * s_fVolumeGain * fTotalVolume,
                fMinDistance,
                fMaxDistance,
                vListenerOrigin,
                vTempAxis,
                vLoopOrigin
            );

            continue;
        }

        if (s_show_sounds->integer > 0) {
            Com_DPrintf(
                "OpenAL: %d (#%i) - started loop - %s\n", cl.serverTime, pLoopSound->iChannel, pLoopSound->pSfx->name
            );
        }

        if (pLoopSound->pSfx->iFlags & (SFX_FLAG_NO_OFFSET)
            || pLoopSound->pSfx->iFlags & (SFX_FLAG_STREAMED | SFX_FLAG_MP3)
            || (pLoopSound->iFlags & LOOPSOUND_FLAG_NO_PAN)) {
            iChannel = S_OPENAL_Start2DLoopSound(
                pLoopSound, fVolume, S_GetBaseVolume() * s_fVolumeGain * fTotalVolume, fMinDistance, vLoopOrigin
            );
        } else {
            iChannel = S_OPENAL_Start3DLoopSound(
                pLoopSound,
                S_GetBaseVolume() * s_fVolumeGain * fTotalVolume,
                fMinDistance,
                fMaxDistance,
                vLoopOrigin,
                vListenerOrigin
            );
        }

        if (iChannel < 0) {
            continue;
        }

        pLoopSound->bPlaying = true;
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
    VectorCopy(s_entity[iEntNum].velocity, alvec);
    VectorScale(alvec, s_fVelocityScale, alvec);
    qalListenerfv(AL_VELOCITY, alvec);
    alDieIfError();

    //
    // Position
    //
    VectorCopy(vHeadPos, alvec);
    VectorCopy(alvec, vListenerOrigin);
    qalListenerfv(AL_POSITION, alvec);
    alDieIfError();

    //
    // Orientation
    //
    alorientation[0][0] = vAxis[0][0];
    alorientation[0][1] = vAxis[0][1];
    alorientation[0][2] = vAxis[0][2];
    alorientation[1][0] = vAxis[2][0];
    alorientation[1][1] = vAxis[2][1];
    alorientation[1][2] = vAxis[2][2];
    qalListenerfv(AL_ORIENTATION, (const ALfloat *)alorientation);
    alDieIfError();

    VectorCopy(vAxis[1], vTempAxis);

    fVolume = 1;
    iPan    = 64;

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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
                if (i >= MAX_SOUNDSYSTEM_CHANNELS_3D) {
                    fVolume = fMaxVolume;
                    iPan    = 64;
                } else {
                    pChannel->set_position(vOrigin[0], vOrigin[1], vOrigin[2]);
                }
            } else {
                if (i >= MAX_SOUNDSYSTEM_CHANNELS_3D) {
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
                    pChannel->set_position(vOrigin[0], vOrigin[1], vOrigin[2]);
                }
            }
        } else if (pChannel->iFlags & CHANNEL_FLAG_LOCAL_LISTENER) {
            VectorCopy(vListenerOrigin, vOrigin);
            if (i >= MAX_SOUNDSYSTEM_CHANNELS_3D) {
                fVolume = fMaxVolume;
                iPan    = 64;
            } else {
                pChannel->set_position(vOrigin[0], vOrigin[1], vOrigin[2]);
            }
        } else {
            if (s_entity[pChannel->iEntNum].time < pChannel->iTime) {
                VectorCopy(pChannel->vOrigin, vOrigin);
                if (!(pChannel->iFlags & CHANNEL_FLAG_LOOPING) && !(pChannel->iFlags & (CHANNEL_FLAG_MISSING_ENT))) {
                    pChannel->end_sample();
                    continue;
                }
            } else {
                pChannel->iFlags &= ~CHANNEL_FLAG_MISSING_ENT;

                VectorCopy(s_entity[pChannel->iEntNum].position, vEntOrigin);
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

            if (i >= MAX_SOUNDSYSTEM_CHANNELS_3D) {
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
                pChannel->set_position(vOrigin[0], vOrigin[1], vOrigin[2]);
                VectorCopy(s_entity[pChannel->iEntNum].velocity, vEntVelocity);
                VectorCopy(vEntVelocity, vVelocity);
                VectorScale(vVelocity, s_fVelocityScale, vVelocity);
                pChannel->set_velocity(vVelocity[0], vVelocity[1], vVelocity[2]);
            }
        }

        if (i >= MAX_SOUNDSYSTEM_CHANNELS_3D) {
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
    float  pan;

    VectorSubtract(origin, listener_origin, source_vec);
    VectorNormalize(source_vec);

    pan = s_separation->value + (1.f - s_separation->value) * -DotProduct(listener_left, source_vec);

    if (pan < 0) {
        pan = 0;
    }

    return ceilf(pan * 127);
}

/*
==============
S_OPENAL_reverb
==============
*/
static void S_OPENAL_reverb(int iChannel, int iReverbType, float fReverbLevel)
{
    // FIXME: Connect source to effect slot
    //  see https://github.com/kcat/openal-soft/blob/master/examples/alreverb.c

    // No reverb currently.
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

    // FIXME: generate effect and auxiliary effect slot
    //  or destroy them
    //  see https://github.com/kcat/openal-soft/blob/master/examples/alreverb.c
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
        al_current_volume    = Square(s_volume->value);
        qalListenerf(AL_GAIN, al_current_volume);
        alDieIfError();
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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
        Cbuf_AddText("snd_restart\n");
        s_speaker_type->modified = false;
    }

    if (s_reverb->modified) {
        s_reverb->modified = false;

        if (s_reverb->integer) {
            if (al_use_reverb) {
                S_OPENAL_SetReverb(s_iReverbType, s_fReverbLevel);
            } else {
                Com_Printf("OpenAL: No reverb support. Reverb is disabled.\n");
            }
        } else if (al_use_reverb) {
            S_OPENAL_SetReverb(0, 0);
        }
    }

    if (s_show_num_active_sounds->integer == 1) {
        int num = 0;

        for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
            pChannel = openal.channel[i];
            if (pChannel && pChannel->is_playing()) {
                ++num;
            }
        }
        Com_DPrintf("OpenAL: Number of active sounds = %d\n", num);
    }

    Music_Update();

    for (i = 0; i < MAX_LOOP_SOUNDS; i++) {
        if (!openal.loop_sounds[i].bInUse) {
            S_OPENAL_StopLoopingSound(&openal.loop_sounds[i]);
        }
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_CHANNELS; i++) {
        openal.channel[i]->update();
    }
}

/*
==============
S_OPENAL_GetMusicFilename
==============
*/
const char *S_OPENAL_GetMusicFilename()
{
    // Don't call into the channel if QAL initialisation did not succeed.
    if (!al_initialized) {
        return "";
    }

    if (!openal.chan_trig_music.is_playing() && !openal.chan_trig_music.is_paused()) {
        return "";
    }

    return openal.tm_filename;
}

/*
==============
S_OPENAL_GetMusicLoopCount
==============
*/
int S_OPENAL_GetMusicLoopCount()
{
    return openal.tm_loopcount;
}

/*
==============
S_OPENAL_GetMusicOffset
==============
*/
unsigned int S_OPENAL_GetMusicOffset()
{
    return openal.chan_trig_music.sample_offset();
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

    if (!pChannel->pSfx) {
        return false;
    }

    if (strcmp(sfxName, pChannel->pSfx->name)) {
        return false;
    }

    return true;
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
        Com_DPrintf("OpenAL: Set sample error - %s\n", pSfx->name);
        pChannel->iFlags &= ~CHANNEL_FLAG_PLAY_DEFERRED;
        return;
    }

    pChannel->iBaseRate = pChannel->sample_playback_rate();

    if (sfx_infos[pSfx->sfx_info_index].loop_start != -1) {
        pChannel->set_sample_loop_block(
            sfx_infos[pSfx->sfx_info_index].loop_start, sfx_infos[pSfx->sfx_info_index].loop_end
        );
        pChannel->set_sample_loop_count(1);
        pChannel->iFlags |= CHANNEL_FLAG_LOOPING;
        if (s_show_sounds->integer > 0) {
            Com_DPrintf(
                "OpenAL: loopblock - %d to %d\n",
                sfx_infos[pSfx->sfx_info_index].loop_start,
                sfx_infos[pSfx->sfx_info_index].loop_end
            );
        }
    } else {
        pChannel->set_sample_loop_count(1);
    }

    pChannel->set_gain(pChannel->fVolume);
    pChannel->set_sample_offset(pBase->iOffset);
    pChannel->set_sample_playback_rate(pChannel->iBaseRate * pBase->fNewPitchMult);

    if (bStartUnpaused) {
        pChannel->resume_sample();
    } else {
        pChannel->iFlags |= CHANNEL_FLAG_PLAY_DEFERRED;
    }

    if (pChannel->iEntNum != ENTITYNUM_NONE) {
        s_entity[pChannel->iEntNum].time = pChannel->iTime;
        VectorCopy(pChannel->vOrigin, s_entity[pChannel->iEntNum].position);
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

    handle = S_RegisterSound(pBase->sfx.szName, (pBase->sfx.iFlags & SFX_FLAG_STREAMED), false);

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

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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

    for (i = 0; i < MAX_SOUNDSYSTEM_POSITION_CHANNELS; i++) {
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
    qalSourcef(source, AL_ROLLOFF_FACTOR, 1.0f);
    alDieIfError();
    qalSourcef(source, AL_GAIN, S_GetBaseVolume());
    alDieIfError();
    qalSourcei(source, AL_DISTANCE_MODEL, AL_LINEAR_DISTANCE_CLAMPED);
    alDieIfError();
    //
    // Added in OPM
    //
    if (fMinDist > 0) {
        qalSourcef(source, AL_REFERENCE_DISTANCE, fMinDist);
        alDieIfError();
    }
    if (fMaxDist > 0) {
        qalSourcef(source, AL_MAX_DISTANCE, fMaxDist);
        alDieIfError();
    }
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
openal_channel::set_no_virtualization
==============
*/
void openal_channel::set_no_virtualization()
{
#if AL_SOFT_direct_channels_remix
    qalSourcei(source, AL_DIRECT_CHANNELS_SOFT, AL_REMIX_UNMATCHED_SOFT);
    alDieIfError();
#elif AL_SOFT_direct_channels
    qalSourcei(source, AL_DIRECT_CHANNELS_SOFT, AL_TRUE);
    alDieIfError();
#endif
}

/*
==============
openal_channel::set_virtualization
==============
*/
void openal_channel::set_virtualization()
{
#if AL_SOFT_direct_channels_remix || AL_SOFT_direct_channels
    qalSourcei(source, AL_DIRECT_CHANNELS_SOFT, AL_FALSE);
    alDieIfError();
#endif
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
openal_channel::is_free
==============
*/
bool openal_channel::is_free()
{
    ALint state = sample_status();

    return state == AL_INITIAL || state == AL_STOPPED;
}

/*
==============
openal_channel::is_paused
==============
*/
bool openal_channel::is_paused()
{
    ALint state = sample_status();

    return state == AL_PAUSED;
}

/*
==============
openal_channel::is_playing
==============
*/
bool openal_channel::is_playing()
{
    ALint state = sample_status();

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
    iFlags = 0;
}

/*
==============
openal_channel::set_sfx
==============
*/
bool openal_channel::set_sfx(sfx_t *pSfx)
{
    ALint freq = 0;

    assert(pSfx->length);

    this->pSfx = pSfx;
    if (!pSfx->buffer || !qalIsBuffer(pSfx->buffer)) {
        if (pSfx->iFlags & SFX_FLAG_MP3) {
            qalGenBuffers(1, &pSfx->buffer);
            alDieIfError();

            if (!S_OPENAL_LoadMP3_Codec(pSfx->name, pSfx)) {
                qalDeleteBuffers(1, &pSfx->buffer);
                alDieIfError();

                Com_DPrintf("OpenAL: Failed to load MP3.\n");
                return false;
            }

            alDieIfError();
        } else {
            ALenum fmt = S_OPENAL_Format(pSfx->info.width, pSfx->info.channels);

            if (!fmt) {
                Com_Printf(
                    "OpenAL: Bad Wave file (%d channels, %d bits) [%s].\n",
                    pSfx->info.channels,
                    (int)(pSfx->info.width * 8),
                    pSfx->name
                );
                return false;
            }

            if (pSfx->info.dataalign > 1 && !soft_block_align) {
                Com_DPrintf(
                    "OpenAL: Alignment specified but AL doesn't support block alignment (%d).", pSfx->info.dataalign
                );
                return false;
            }

            qalGenBuffers(1, &pSfx->buffer);
            alDieIfError();

#if AL_SOFT_block_alignment
            if (pSfx->info.dataalign > 1) {
                qalBufferi(pSfx->buffer, AL_UNPACK_BLOCK_ALIGNMENT_SOFT, pSfx->info.dataalign);
                alDieIfError();
            }
#endif

            qalBufferData(
                pSfx->buffer,
                fmt,
                &pSfx->data[pSfx->info.dataofs],
                pSfx->info.datasize - pSfx->info.dataofs,
                pSfx->info.rate
            );
            alDieIfError();
        }
    }

    qalSourceStop(source);
    alDieIfError();

    qalSourcei(source, AL_BUFFER, pSfx->buffer);
    alDieIfError();

    qalSourcef(source, AL_PITCH, 1.0);
    alDieIfError();

    // Get the base frequency
    qalGetBufferi(pSfx->buffer, AL_FREQUENCY, &freq);
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
void openal_channel::set_sample_pan(S32 pan)
{
    const float   panning           = (pan - 64) / 127.f;
    const ALfloat sourcePosition[3] = {panning, 0, sqrtf(1.f - Square(panning))};

    qalSourcef(source, AL_ROLLOFF_FACTOR, 0);
    qalSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
    qalSourcefv(source, AL_POSITION, sourcePosition);
}

/*
==============
openal_channel::set_sample_playback_rate
==============
*/
void openal_channel::set_sample_playback_rate(S32 rate)
{
    // Fixed in OPM
    //  Set the pitch in OpenAL
    qalSourcef(source, AL_PITCH, rate / (float)iBaseRate);
    alDieIfError();
}

/*
==============
openal_channel::sample_playback_rate
==============
*/
S32 openal_channel::sample_playback_rate()
{
    float pitch = 1;
    ALint freq  = 0;

    // Fixed in OPM
    //  The sample rate varies according to the pitch
    qalGetSourcef(source, AL_PITCH, &pitch);
    alDieIfError();

    return buffer_frequency() * pitch;
}

/*
==============
openal_channel::buffer_frequency
==============
*/
U32 openal_channel::buffer_frequency() const
{
    ALint freq = 0;

    if (buffer) {
        qalGetBufferi(buffer, AL_FREQUENCY, &freq);
        alDieIfError();
    } else if (pSfx) {
        qalGetBufferi(pSfx->buffer, AL_FREQUENCY, &freq);
        alDieIfError();
    } else {
        freq = 22050;
    }

    return freq;
}

/*
==============
openal_channel::sample_volume
==============
*/
S32 openal_channel::sample_volume()
{
    ALfloat gain = 0;

    qalGetSourcef(source, AL_GAIN, &gain);
    alDieIfError();

    return (gain / S_GetBaseVolume()) * 100;
}

/*
==============
openal_channel::sample_offset
==============
*/
U32 openal_channel::sample_offset()
{
    ALint offset = 0;

    qalGetSourcei(source, AL_SAMPLE_OFFSET, &offset);
    alDieIfError();

    return offset;
}

/*
==============
openal_channel::sample_ms_offset
==============
*/
U32 openal_channel::sample_ms_offset()
{
    if (!is_playing()) {
        return 0;
    }

    return sample_offset() * 1000ull / sample_playback_rate();
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
    qalSourcei(source, AL_SAMPLE_OFFSET, offset);
    alDieIfError();
}

/*
==============
openal_channel::set_sample_ms_offset
==============
*/
void openal_channel::set_sample_ms_offset(U32 offset)
{
    set_sample_offset(offset * sample_playback_rate() / 1000);
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

    if (buffer) {
        for (S32 i = 0; i < count; i++) {
            qalSourceQueueBuffers(source, 1, &buffer);
            alDieIfError();
        }
    }

    if (count == 0) {
        qalSourcei(source, AL_LOOPING, true);
        alDieIfError();
    } else {
        qalSourcei(source, AL_LOOPING, false);
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
    // FIXME: unimplemented
    STUB_DESC("sample_loop_block");
}

/*
==============
openal_channel::sample_status
==============
*/
U32 openal_channel::sample_status()
{
    ALint retval;

    qalGetSourcei(source, AL_SOURCE_STATE, &retval);
    alDieIfError();

    return retval;
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

    music_numsongs = 0;

    FS_ReadFile(path, (void **)&data);
    if (!data) {
        Com_DPrintf("OpenAL: Couldn't load %s\n", path);
        return false;
    }
    Com_DPrintf("SOUNDTRACK: Loading %s\n", path);

    MUSIC_StopAllSongs();

    buffer       = (char *)data;
    load_path[0] = 0;

    while (1) {
        Q_strncpyz(com_token, COM_GetToken(&buffer, true), sizeof(com_token));
        if (!com_token[0]) {
            break;
        }

        if (strlen(com_token) >= MAX_RES_NAME) {
            Com_Printf("MUSIC_LoadSoundtrackFile: argument too long, truncating in %s\n", path);
            com_token[MAX_RES_NAME - 1] = 0;
        }

        numargs = 1;
        Q_strncpyz(args[0], com_token, sizeof(args[0]));

        while (1) {
            Q_strncpyz(com_token, COM_GetToken(&buffer, false), sizeof(com_token));
            if (!com_token[0]) {
                break;
            }

            if (strlen(com_token) >= MAX_RES_NAME) {
                Com_Printf("MUSIC_LoadSoundtrackFile: argument too long, truncating in %s\n", path);
                com_token[MAX_RES_NAME - 1] = 0;
            }

            Q_strncpyz(args[numargs], com_token, sizeof(args[numargs]));
            numargs++;
        }

        if (!Q_stricmp(args[0], "path")) {
            Q_strncpyz(load_path, args[1], sizeof(load_path));
            if (load_path[strlen(load_path) - 1] != '/' && load_path[strlen(load_path) - 1] != '\\') {
                Q_strcat(load_path, sizeof(load_path), "/");
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
                psong->volume = atof(args[2]);
            } else if (!Q_stricmp(args[1], "fadetime")) {
                psong->fadetime = atof(args[2]);
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
                Q_strncpyz(alias, args[0], sizeof(alias));
                Q_strncpyz(file, load_path, sizeof(file));
                Q_strcat(file, sizeof(file), args[1]);
            } else {
                size_t len;

                Q_strncpyz(file, load_path, sizeof(file));
                Q_strcat(file, sizeof(file), args[1]);

                len = strlen(args[0]);
                if (len >= 4) {
                    Q_strncpyz(alias, args[0], len - 4);
                } else {
                    alias[0] = 0;
                }
            }

            if (music_numsongs >= MAX_MUSIC_SONGS) {
                Com_Printf("MUSIC_LoadSoundtrackFile: Too many songs in %s, skipping %s.\n", path, alias);
                continue;
            }

            psong = &music_songs[music_numsongs];
            Q_strncpyz(psong->alias, alias, sizeof(psong->alias));
            Q_strncpyz(psong->path, file, sizeof(psong->path));
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
    for (int i = 0; i < MAX_SOUNDSYSTEM_SONGS; i++) {
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

    // Added in OPM
    //  Clear the list of songs so the songs from previous maps
    //  won't get played
    music_numsongs = 0;
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

    for (ch_idx = 0; ch_idx < MAX_SOUNDSYSTEM_SONGS; ch_idx++) {
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
    int                          channel_number;
    song_t                      *song;
    int                          songnum;
    int                          channel_to_play_on;
    int                          fading_song;
    openal_channel_two_d_stream *song_channel;
    unsigned int                 loop_start;
    int                          rate;

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

    if (!song_channel->queue_stream(song->path)) {
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
        song_channel->set_gain(S_GetBaseVolume() * (song->volume * s_ambientVolume->value) * s_fVolumeGain);
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

        for (i = 0; i < MAX_SOUNDSYSTEM_SONGS; i++) {
            if (!openal.chan_song[i].is_playing() && !openal.chan_song[i].is_paused()) {
                continue;
            }

            if (openal.chan_song[i].fading != FADE_NONE) {
                continue;
            }

            openal.chan_song[i].set_gain(
                S_GetBaseVolume() * (music_songs[openal.chan_song[i].song_number].volume * s_ambientVolume->value)
                * s_fVolumeGain * music_volume
            );
        }
    }

    for (i = 0; i < MAX_SOUNDSYSTEM_SONGS; i++) {
        if (!openal.chan_song[i].is_playing() && !openal.chan_song[i].is_paused()) {
            continue;
        }

        switch (openal.chan_song[i].fading) {
        case fade_t::FADE_IN:
            max_volume = music_songs[openal.chan_song[i].song_number].volume * s_ambientVolume->value;
            new_volume = (unsigned int)(cls.realtime - openal.chan_song[i].fade_start_time)
                       / (openal.chan_song[i].fade_time * 1000.f) * max_volume;

            if (new_volume > max_volume) {
                openal.chan_song[i].set_gain(S_GetBaseVolume() * (max_volume * s_fVolumeGain * music_volume));
                openal.chan_song[i].fading = FADE_NONE;
            } else {
                openal.chan_song[i].set_gain(S_GetBaseVolume() * (new_volume * s_fVolumeGain * music_volume));
            }
            break;
        case fade_t::FADE_OUT:
            max_volume = music_songs[openal.chan_song[i].song_number].volume * s_ambientVolume->value;
            new_volume = (unsigned int)(cls.realtime - openal.chan_song[i].fade_start_time)
                       / (openal.chan_song[i].fade_time * 1000.f) * max_volume;

            if (new_volume > 0) {
                openal.chan_song[i].set_gain(S_GetBaseVolume() * (new_volume * s_fVolumeGain * music_volume));
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
                S_GetBaseVolume() * (openal.chan_trig_music.fVolume * s_musicVolume->value) * s_fVolumeGain
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

    for (i = 0; i < MAX_SOUNDSYSTEM_SONGS; i++) {
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
    void            *stream      = NULL;

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

    if (!openal.chan_trig_music.queue_stream(pszRealName)) {
        S_OPENAL_InitChannel(SOUNDSYSTEM_CHANNEL_TRIGGER_MUSIC_ID, &openal.chan_trig_music);
        Com_DPrintf("Could not start triggered music '%s'\n", pszName);
        return;
    }

    openal.chan_trig_music.fVolume = fVolume;
    openal.chan_trig_music.set_gain(S_GetBaseVolume() * (fVolume * s_musicVolume->value) * s_fVolumeGain);
    openal.chan_trig_music.set_sample_loop_count(iLoopCount);
    openal.chan_trig_music.set_sample_offset(iOffset);

    // Fixed in OPM
    //  Play the sound then pause it immediately
    //  So it can be unpaused upon loading from save
    openal.chan_trig_music.play();

    if (!autostart) {
        openal.chan_trig_music.pause();
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
    // Fixed in OPM
    //  Make sure to clear the triggered music in case snd_restart is called
    openal.tm_filename[0] = 0;
    openal.tm_loopcount   = 0;
}

/*
==============
S_TriggeredMusic_Volume

Added in 2.0
==============
*/
void S_TriggeredMusic_Volume()
{
    if (!s_bSoundStarted) {
        return;
    }

    if (Cmd_Argc() != 2) {
        Com_Printf("tmvolume <0-1>\n\n");
        return;
    }

    openal.chan_trig_music.fVolume = atof(Cmd_Argv(1));
    music_volume_changed           = true;
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

    openal.chan_trig_music.set_gain(
        S_GetBaseVolume() * (openal.chan_trig_music.fVolume * s_musicVolume->value) * s_fVolumeGain
    );
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
    openal.chan_movie.stop();
}

/*
==============
S_SetupMovieAudio
==============
*/
void S_SetupMovieAudio(const char *pszMovieName)
{
    char filename[MAX_QPATH];

    S_StopMovieAudio();

    COM_StripExtension(pszMovieName, filename, sizeof(filename));
    Q_strcat(filename, sizeof(filename), ".mp3");

    if (!openal.chan_movie.queue_stream(filename)) {
        COM_StripExtension(pszMovieName, filename, sizeof(filename));
        Q_strcat(filename, sizeof(filename), ".wav");

        if (!openal.chan_movie.queue_stream(filename)) {
            Com_DPrintf("Can't find any matching audio file for movie: %s\n", pszMovieName);
            return;
        }
    }

    openal.chan_movie.set_gain(S_GetBaseVolume() * 1.5f);
    openal.chan_movie.play();

    Com_DPrintf("Movie Audio setup: %s\n", filename);
}

/*
==============
S_CurrentMoviePosition
==============
*/
int S_CurrentMoviePosition()
{
    return openal.chan_movie.sample_ms_offset();
}

/*
=================
S_AL_Format
=================
*/
static ALuint S_OPENAL_Format(float width, int channels)
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
    } else if (width == 0.5) {
        if (ima4_ext && soft_block_align) {
            if (channels == 1) {
                format = AL_FORMAT_MONO_IMA4;
            } else if (channels == 2) {
                format = AL_FORMAT_STEREO_IMA4;
            }
        } else {
            // unsupported
            format = 0;
        }
    }

    return format;
}

/*
==============
S_OPENAL_LoadMP3_Codec
==============
*/
static bool S_OPENAL_LoadMP3_Codec(const char *_path, sfx_t *pSfx)
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
    qalGenBuffers(1, &pSfx->buffer);
    alDieIfError();

    // Fill the buffer
    if (info.size == 0) {
        // We have no data to buffer, so buffer silence
        byte dummyData[2] = {0};

        qalBufferData(pSfx->buffer, AL_FORMAT_MONO16, (void *)dummyData, 2, 22050);
    } else {
        qalBufferData(pSfx->buffer, format, data, info.size, info.rate);
    }

    alDieIfError();

    // Free the memory
    Hunk_FreeTempMemory(data);

    return true;
}

/*
==============
openal_channel::update
==============
*/
void openal_channel::update() {}

/*
==============
openal_channel_two_d_stream::stop
==============
*/
void openal_channel_two_d_stream::stop()
{
    openal_channel::stop();

    clear_stream();
}

/*
==============
openal_channel_two_d_stream::set_sfx
==============
*/
bool openal_channel_two_d_stream::set_sfx(sfx_t *pSfx)
{
    snd_stream_t *stream;
    unsigned int  bytesToRead, bytesRead;
    ALint         freq = 0;
    char          rawData[MAX_BUFFER_SAMPLES * 2 * 2];

    stop();

    sampleLoopCount = 1;

    this->pSfx = pSfx;
    Q_strncpyz(this->fileName, pSfx->name, sizeof(this->fileName));

    streamHandle = S_CodecLoad(pSfx->name, NULL);
    if (!streamHandle) {
        Com_DPrintf("OpenAL: Failed to load sound file.\n");
        return false;
    }

    stream              = (snd_stream_t *)streamHandle;
    pSfx->info.channels = stream->info.channels;
    pSfx->info.dataofs  = stream->info.dataofs;
    pSfx->info.datasize = stream->info.size;
    pSfx->info.rate     = stream->info.rate;
    pSfx->info.samples  = stream->info.samples;
    pSfx->info.width    = stream->info.width;

    pSfx->info.format = S_OPENAL_Format(stream->info.width, stream->info.channels);
    if (!pSfx->info.format) {
        Com_DPrintf(
            "OpenAL: Bad Wave file (%d channels, %d bits) [%s].\n",
            pSfx->info.channels,
            (int)(pSfx->info.width * 8.f),
            pSfx->name
        );

        S_CodecCloseStream(stream);
        return false;
    }

    qalSourceStop(source);
    alDieIfError();

    qalGenBuffers(MAX_STREAM_BUFFERS, buffers);
    alDieIfError();

    // Read a smaller sample
    bytesToRead      = Q_min(MAX_BUFFER_SAMPLES * stream->info.width * stream->info.channels, sizeof(rawData));
    bytesRead        = S_CodecReadStream(stream, bytesToRead, rawData);
    streamNextOffset = bytesRead;
    if (!bytesRead) {
        // Valid stream but no data?
        S_CodecCloseStream(stream);
        return true;
    }

#if AL_SOFT_block_alignment
    if (stream->info.dataalign > 1 && soft_block_align) {
        qalBufferi(buffers[currentBuf], AL_UNPACK_BLOCK_ALIGNMENT_SOFT, stream->info.dataalign);
        alDieIfError();
    }
#endif

    qalBufferData(buffers[currentBuf], pSfx->info.format, rawData, bytesRead, stream->info.rate);
    alDieIfError();

    qalSourceQueueBuffers(source, 1, &buffers[currentBuf]);
    alDieIfError();

    iBaseRate  = stream->info.rate;
    currentBuf = (currentBuf + 1) % MAX_STREAM_BUFFERS;
    streaming  = true;

    return true;
}

/*
==============
openal_channel_two_d_stream::set_sample_loop_count
==============
*/
void openal_channel_two_d_stream::set_sample_loop_count(S32 count)
{
    sampleLoopCount = count;
    sampleLooped    = 0;
}

/*
==============
openal_channel_two_d_stream::openal_channel_two_d_stream
==============
*/
openal_channel_two_d_stream::openal_channel_two_d_stream()
{
    int i;

    streamHandle = NULL;
    for (i = 0; i < MAX_STREAM_BUFFERS; i++) {
        buffers[i] = 0;
    }
    currentBuf       = 0;
    sampleLoopCount  = 0;
    sampleLooped     = 0;
    streamNextOffset = 0;
    streaming        = false;
}

/*
==============
openal_channel_two_d_stream::openal_channel_two_d_stream
==============
*/
openal_channel_two_d_stream::~openal_channel_two_d_stream()
{
    clear_stream();
}

/*
==============
openal_channel_two_d_stream::update
==============
*/
void openal_channel_two_d_stream::update()
{
    snd_stream_t *stream = (snd_stream_t *)streamHandle;
    unsigned int  bytesToRead, bytesRead;
    ALuint        format;
    ALint         numProcessedBuffers = 0, numQueuedBuffers = 0;
    ALint         state = sample_status();
    // 2 channels with a width of 2
    char rawData[MAX_BUFFER_SAMPLES * 2 * 2];

    if (!streaming || is_paused() || state == AL_INITIAL) {
        return;
    }

    qalGetSourcei(source, AL_BUFFERS_PROCESSED, &numProcessedBuffers);
    alDieIfError();

    qalGetSourcei(source, AL_BUFFERS_QUEUED, &numQueuedBuffers);
    alDieIfError();

    // Remove processed buffers after half of the queued buffers has been processed
    if (numProcessedBuffers >= Q_max(numQueuedBuffers / 2, 1)) {
        while (numProcessedBuffers-- > 0) {
            ALuint tmpBuffer;
            qalSourceUnqueueBuffers(source, 1, &tmpBuffer);
        }
    }

    if (numQueuedBuffers >= MAX_STREAM_BUFFERS) {
        return;
    }

    if (sampleLoopCount && sampleLooped >= sampleLoopCount) {
        if (!is_playing()) {
            // Can clear the stream
            clear_stream();
        }
        return;
    }

    bytesToRead = Q_min(MAX_BUFFER_SAMPLES * stream->info.width * stream->info.channels, sizeof(rawData));

    format = S_OPENAL_Format(stream->info.width, stream->info.channels);

    bytesRead = S_CodecReadStream(stream, bytesToRead, rawData);
    streamNextOffset += bytesRead;
    if (!bytesRead) {
        S_CodecCloseStream(stream);

        sampleLooped++;
        if (sampleLoopCount && sampleLooped >= sampleLoopCount) {
            // Finished the last loop
            streamHandle = NULL;
            return;
        }

        //
        // Looped, start again from the beginning
        //
        streamHandle = S_CodecLoad(this->fileName, NULL);
        if (!streamHandle) {
            clear_stream();
            return;
        }

        stream = (snd_stream_t *)streamHandle;
        //
        // Re-read the format, we never know...
        //
        format = S_OPENAL_Format(stream->info.width, stream->info.channels);

        bytesRead        = S_CodecReadStream(stream, bytesToRead, rawData);
        streamNextOffset = bytesRead;
        if (!bytesRead) {
            S_CodecCloseStream(stream);
            streamHandle = NULL;
            return;
        }
    }

#if AL_SOFT_block_alignment
    if (stream->info.dataalign > 1 && soft_block_align) {
        qalBufferi(buffers[currentBuf], AL_UNPACK_BLOCK_ALIGNMENT_SOFT, stream->info.dataalign);
        alDieIfError();
    }
#endif

    qalBufferData(buffers[currentBuf], format, rawData, bytesRead, stream->info.rate);
    alDieIfError();

    qalSourceQueueBuffers(source, 1, &buffers[currentBuf]);
    alDieIfError();

    if (!is_playing()) {
        // The sample has stopped during stream
        // Could be because the storage is slow enough for the buffer
        // or because the storage was powering on after standby
        play();
    }

    currentBuf = (currentBuf + 1) % MAX_STREAM_BUFFERS;
}

/*
==============
openal_channel_two_d_stream::sample_offset
==============
*/
U32 openal_channel_two_d_stream::sample_offset()
{
    snd_stream_t *stream              = (snd_stream_t *)streamHandle;
    ALint         playerByteOffset    = 0;
    ALint         numProcessedBuffers = 0;
    ALint         numQueuedBuffers    = 0;
    unsigned int  totalQueueLength    = 0;
    unsigned int  bitsPerSample       = 0;
    unsigned int  offset              = 0;
    ALint         bits = 0, channels = 0;

    if (!streaming) {
        // no stream
        return 0;
    }

    // Get the byte offset in the queue
    qalGetSourcei(source, AL_BYTE_OFFSET, &playerByteOffset);
    alDieIfError();

    totalQueueLength = getQueueLength();
    bitsPerSample    = getBitsPerSample();

    assert(playerByteOffset < totalQueueLength);
    assert(streamNextOffset >= totalQueueLength || stream);

    if (streamNextOffset >= totalQueueLength) {
        offset = streamNextOffset - totalQueueLength + playerByteOffset;
    } else if (stream) {
        // it probably means the sound is looped and will restart from the beginning
        if (streamNextOffset + playerByteOffset <= totalQueueLength) {
            // near the end of the stream
            if (stream->info.size > totalQueueLength) {
                offset = stream->info.size + streamNextOffset - totalQueueLength + playerByteOffset;
            } else {
                // the buffer is really small
                offset = (streamNextOffset - totalQueueLength + playerByteOffset) % stream->info.size;
            }
        } else {
            // it is past end of stream
            // so, start from the beginning of the stream
            offset = streamNextOffset + playerByteOffset - totalQueueLength;
        }

        assert(offset < stream->info.size);
    } else {
        offset = totalQueueLength - streamNextOffset + playerByteOffset;
    }

    return offset * 8 / bitsPerSample;
}

/*
==============
openal_channel_two_d_stream::buffer_frequency
==============
*/
U32 openal_channel_two_d_stream::buffer_frequency() const
{
    unsigned int bufferId;
    ALint        freq = 0;

    bufferId = (currentBuf - 1) % MAX_STREAM_BUFFERS;
    qalGetBufferi(buffers[bufferId], AL_FREQUENCY, &freq);
    alDieIfError();

    return freq;
}

/*
==============
openal_channel_two_d_stream::set_sample_offset
==============
*/
void openal_channel_two_d_stream::set_sample_offset(U32 offset)
{
    snd_stream_t *stream;
    unsigned int  bytesToRead, bytesRead;
    unsigned int  byteOffset;
    unsigned int  bitsPerSample;
    unsigned int  streamPosition;
    ALuint        format;
    ALint         numQueuedBuffers;
    bool          bWasPlaying;
    char          rawData[MAX_BUFFER_SAMPLES * 2 * 2];

    if (!streaming) {
        return;
    }

    stream         = (snd_stream_t *)streamHandle;
    streamPosition = getCurrentStreamPosition();
    bitsPerSample  = getBitsPerSample();
    byteOffset     = offset * bitsPerSample / 8;

    if (byteOffset >= streamPosition && byteOffset < streamNextOffset) {
        //
        // Sample is in the currently streamed data
        //
        qalSourcei(source, AL_BYTE_OFFSET, byteOffset - streamPosition);
        alDieIfError();

        return;
    }

    bWasPlaying = is_playing();

    if (stream) {
        if (byteOffset < streamPosition) {
            //
            // New offset is before the current offset,
            // reload the stream from the beginning
            //
            S_CodecCloseStream(stream);
            streamHandle = NULL;
        } else if (byteOffset >= stream->info.size) {
            //
            // This shouldn't happen unless the file is different
            // from the one used while saving
            //
            byteOffset %= stream->info.size;
            S_CodecCloseStream(stream);
            streamHandle = NULL;
        }
    }

    if (!streamHandle) {
        streamHandle = S_CodecLoad(this->fileName, NULL);
        if (!streamHandle) {
            clear_stream();
            return;
        }

        stream           = (snd_stream_t *)streamHandle;
        streamNextOffset = 0;
    }

    while (streamNextOffset <= byteOffset) {
        streamPosition = streamNextOffset;

        bytesToRead = Q_min(MAX_BUFFER_SAMPLES * stream->info.width * stream->info.channels, sizeof(rawData));
        bytesRead   = S_CodecReadStream(stream, bytesToRead, rawData);
        if (!bytesRead) {
            clear_stream();
            return;
        }

        streamNextOffset += bytesRead;
    }

    //
    // Stop the sound and remove all buffers from the queue
    //
    qalSourceStop(source);
    alDieIfError();

    //
    // Remove all buffers from the queue
    //
    numQueuedBuffers = 0;
    qalGetSourcei(source, AL_BUFFERS_QUEUED, &numQueuedBuffers);
    alDieIfError();

    while (numQueuedBuffers--) {
        ALuint b;
        qalSourceUnqueueBuffers(source, 1, &b);
        alDieIfError();
    }

    format = S_OPENAL_Format(stream->info.width, stream->info.channels);

    qalBufferData(buffers[currentBuf], format, rawData, bytesRead, stream->info.rate);
    alDieIfError();

    qalSourceQueueBuffers(source, 1, &buffers[currentBuf]);
    alDieIfError();
    currentBuf = (currentBuf + 1) % MAX_STREAM_BUFFERS;

    //
    // Now set the source offset
    //
    qalSourcei(source, AL_BYTE_OFFSET, byteOffset - streamPosition);
    alDieIfError();

    if (bWasPlaying) {
        //
        // The sound was stopped when re-streaming
        //
        play();
    }
}

/*
==============
openal_channel_two_d_stream::queue_stream
==============
*/
bool openal_channel_two_d_stream::queue_stream(const char *fileName)
{
    snd_stream_t *stream;
    unsigned int  bytesToRead, bytesRead;
    ALint         freq   = 0;
    ALuint        format = 0;
    ALuint        old    = 0;
    char          rawData[MAX_BUFFER_SAMPLES * 2 * 2];

    // Store the filename so it can be looped later
    Q_strncpyz(this->fileName, fileName, sizeof(this->fileName));
    clear_stream();

    sampleLoopCount = 1;

    //
    // Load the file
    //
    streamHandle = S_CodecLoad(this->fileName, NULL);
    if (!streamHandle) {
        return false;
    }
    stream = (snd_stream_t *)streamHandle;

    iStartTime = cl.serverTime;
    iEndTime   = (int)(cl.serverTime + (stream->info.samples / stream->info.rate * 1000.f));

    format = S_OPENAL_Format(stream->info.width, stream->info.channels);
    if (!format) {
        Com_Printf(
            "OpenAL: Bad Wave file (%d channels, %d bits) [%s].\n",
            pSfx->info.channels,
            (int)(pSfx->info.width * 8.f),
            pSfx->name
        );

        S_CodecCloseStream(stream);
        return false;
    }

    qalGenBuffers(MAX_STREAM_BUFFERS, buffers);
    alDieIfError();

    // Read a smaller sample
    bytesToRead = Q_min(MAX_BUFFER_SAMPLES * stream->info.width * stream->info.channels, sizeof(rawData));
    bytesRead   = S_CodecReadStream(stream, bytesToRead, rawData);
    if (!bytesRead) {
        // Valid stream but no data?
        return true;
    }

    streamNextOffset = bytesRead;

    qalBufferData(buffers[currentBuf], format, rawData, bytesRead, stream->info.rate);
    alDieIfError();

    qalSourceQueueBuffers(source, 1, &buffers[currentBuf]);
    alDieIfError();

    currentBuf = (currentBuf + 1) % MAX_STREAM_BUFFERS;

    streaming = true;

    return true;
}

/*
==============
openal_channel_two_d_stream::clear_stream
==============
*/
void openal_channel_two_d_stream::clear_stream()
{
    if (!streaming) {
        return;
    }

    qalSourceStop(source);
    qalSourcei(source, AL_BUFFER, 0);
    qalDeleteBuffers(MAX_STREAM_BUFFERS, buffers);

    if (streamHandle) {
        S_CodecCloseStream((snd_stream_t *)streamHandle);
    }

    fileName[0]  = 0;
    streamHandle = NULL;

    currentBuf       = 0;
    streamNextOffset = 0;
    streaming        = false;
    sampleLooped     = 0;
}

/*
==============
openal_channel_two_d_stream::getQueueLength

Return the total length of all buffers being streamed.
==============
*/
unsigned int openal_channel_two_d_stream::getQueueLength() const
{
    ALint        numQueuedBuffers = 0;
    unsigned int totalQueueLength = 0;
    unsigned int bufferId;
    unsigned int i;

    qalGetSourcei(source, AL_BUFFERS_QUEUED, &numQueuedBuffers);
    alDieIfError();

    for (i = 0; i < numQueuedBuffers; i++) {
        ALint bufferSize = 0;

        bufferId = (currentBuf - i - 1) % MAX_STREAM_BUFFERS;
        qalGetBufferi(buffers[bufferId], AL_SIZE, &bufferSize);

        totalQueueLength += bufferSize;
    }

    return totalQueueLength;
}

/*
==============
openal_channel_two_d_stream::getCurrentStreamPosition

Return the current position in the sound being streamed.
==============
*/
unsigned int openal_channel_two_d_stream::getCurrentStreamPosition() const
{
    unsigned int queueLength = getQueueLength();

    if (streamNextOffset >= queueLength) {
        return streamNextOffset - queueLength;
    } else if (streamHandle) {
        snd_stream_t *stream = (snd_stream_t *)streamHandle;
        return (stream->info.size + streamNextOffset - queueLength) % stream->info.size;
    } else {
        return queueLength - streamNextOffset;
    }
}

/*
==============
openal_channel_two_d_stream::getBitsPerSample

Return the number of bytes per sample.
It assumes that all pending buffers have the same channels and bits.
==============
*/
unsigned int openal_channel_two_d_stream::getBitsPerSample() const
{
    unsigned int bufferId;
    ALint        bits = 0, channels = 0;

    bufferId = (currentBuf - 1) % MAX_STREAM_BUFFERS;
    qalGetBufferi(buffers[bufferId], AL_BITS, &bits);
    alDieIfError();
    qalGetBufferi(buffers[bufferId], AL_CHANNELS, &channels);
    alDieIfError();

    return bits * channels;
}
