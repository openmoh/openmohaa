#pragma once

#ifdef __cplusplus
extern "C" {
#endif

	void Sys_InitLocalization();
	void Sys_ShutLocalization();
	const char* Sys_LV_ConvertString(const char* var);
	const char* Sys_LV_CL_ConvertString(const char* var);

#ifdef __cplusplus
}
#endif
