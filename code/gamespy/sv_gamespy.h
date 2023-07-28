#pragma once

#ifdef __cplusplus
extern "C" {
#endif

extern const char* GS_GetGameKey(unsigned int index);
extern unsigned int GS_GetGameID(unsigned int index);
extern const char* GS_GetGameName(unsigned int index);
extern const char* GS_GetCurrentGameKey();
extern unsigned int GS_GetCurrentGameID();
extern const char* GS_GetCurrentGameName();

#ifdef __cplusplus
}
#endif
