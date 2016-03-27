#ifndef __CL_SOUND_H__
#define __CL_SOUND_H__

extern float *sound_global_volume;

void CL_FadeSound2(float time, float min_volume);
void CL_RestoreSound(float time, float max_volume);
void CL_SoundProcessing(int frametime);

#endif // __CL_SOUND_H__
