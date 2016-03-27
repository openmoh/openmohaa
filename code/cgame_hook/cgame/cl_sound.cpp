#include "glb_local.h"
#include "cl_sound.h"

uint8_t *bFadeSoundOrig = ( uint8_t * )0x007F7371;
qboolean *bFadeSoundTrack = ( qboolean * )0x007F7410;
/*DWORD *sound_frame = (DWORD*)0x00FF2A14;
float *fade_soundorig_frame = (float*)0x007F5D68;
float *fade_soundorig_time = (float*)0x007F5D50;*/

qboolean bFadeSound;
float sound_fade_startvol;
float sound_fade_time;
float sound_fade_minvol;

qboolean bRestoreSound;
float sound_restore_startvol;
float sound_restore_time;
float sound_restore_maxvol;

float *sound_global_volume = (float*)0x00551A4C;

void CL_FadeSound2(float time, float min_volume)
{
	cgi.Printf("Called FadeSound with: %f %f\n", time, min_volume);

	if(bRestoreSound)
		bRestoreSound = 0;

	if(*sound_global_volume < min_volume)
	{
		*sound_global_volume = min_volume;
		return;
	}

	bFadeSound = 1;

	sound_fade_startvol = *sound_global_volume;
	sound_fade_time = time;
	sound_fade_minvol = min_volume;
}

void CL_RestoreSound(float time, float max_volume)
{
	cgi.Printf("Called RestoreSound with: %f %f\n", time, max_volume);

	if(bFadeSound || *bFadeSoundOrig)
	{
		*bFadeSoundOrig = false;
		bFadeSound = 0;
	}

	if(*sound_global_volume > max_volume)
	{
		*sound_global_volume = max_volume;
		return;
	}

	bRestoreSound = 1;

	sound_restore_startvol = *sound_global_volume;
	sound_restore_time = time;
	sound_restore_maxvol = max_volume;
}

void CL_SoundProcessing(int frametime)
{
	float volume = *sound_global_volume;

	if(bFadeSound)
	{
		volume -= (1.0f-sound_fade_minvol)/sound_fade_time * frametime;

		*sound_global_volume = volume;

		if(*sound_global_volume < sound_fade_minvol)
		{
			*sound_global_volume = sound_fade_minvol;
			bFadeSound = false;
			*bFadeSoundTrack = false;
		}

		*bFadeSoundTrack = true;
	}

	if(bRestoreSound)
	{
		volume += (sound_restore_maxvol)/sound_restore_time * frametime;

		*sound_global_volume = volume;

		if(*sound_global_volume > sound_restore_maxvol)
		{
			*sound_global_volume = sound_restore_maxvol;
			bRestoreSound = false;
			*bFadeSoundTrack = false;
		}

		*bFadeSoundTrack = true;
	}
}
