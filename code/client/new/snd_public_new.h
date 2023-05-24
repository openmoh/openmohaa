#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Music soundtrack

void MUSIC_Pause();
void MUSIC_Unpause();
qboolean MUSIC_LoadSoundtrackFile(const char* filename);
qboolean MUSIC_SongValid(const char* mood);
qboolean MUSIC_Loaded(void);
void Music_Update(void);
void MUSIC_SongEnded(void);
void MUSIC_NewSoundtrack(const char* name);
void MUSIC_UpdateMood(int current, int fallback);
void MUSIC_UpdateVolume(float volume, float fade_time);
void MUSIC_StopAllSongs(void);
void MUSIC_FreeAllSongs(void);
qboolean MUSIC_Playing(void);
int MUSIC_FindSong(const char* name);
int MUSIC_CurrentSongChannel(void);
void MUSIC_StopChannel(int channel_number);
qboolean MUSIC_PlaySong(const char* alias);
void MUSIC_UpdateMusicVolumes(void);
void MUSIC_CheckForStoppedSongs(void);

float S_GetSoundTime(sfxHandle_t handle);
void S_SetGlobalAmbientVolumeLevel(float volume);
void S_SetReverb(int reverb_type, float reverb_level);
int S_IsSoundPlaying(int channelNumber, const char* name);

void S_RespatializeOld(int entityNum, const vec3_t origin, vec3_t axis[3]);
void S_UpdateEntity(int entityNum, const vec3_t origin, const vec3_t velocity, qboolean use_listener);
void S_FadeSound(float fTime);

#ifdef __cplusplus
}
#endif
