/*
* Game-Deception Blank Wrapper v2
* Copyright (c) Crusader 2002
*/

/*
* Useful ogl functions for half-life including hooked extensions
*/

#include "opengl_api.h"

/*
#pragma warning(disable:4100)
BOOL __stdcall DllMain (HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls (hOpenGL);
			return Init();

		case DLL_PROCESS_DETACH:
			if ( hOpenGL != NULL )
			{
				FreeLibrary(hOpenGL);
				hOpenGL = NULL;
			}
			break;
	}
	return TRUE;
}
#pragma warning(default:4100)
*/


void __cdecl add_log (const char * fmt, ...)
{
	va_list va_alist;
	char logbuf[256] = "";
    FILE * fp;
   
	va_start (va_alist, fmt);
	_vsnprintf (logbuf+strlen(logbuf), sizeof(logbuf) - strlen(logbuf), fmt, va_alist);
	va_end (va_alist);

	if ( (fp = fopen ("opengl32.log", "ab")) != NULL )
	{
		fprintf ( fp, "%s\n", logbuf );
		fclose (fp);
	}
}