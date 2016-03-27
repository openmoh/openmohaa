/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// hook.h : Hooks & Detours

#if defined ( CGAME_DLL )
#include "cgamex86.h"
#else
#include "ubersdk.h"
#endif

#include "asm.h"
#include "vector.h"

#if defined ( CGAME_DLL )
#define glbs cgi
#else
#define glbs ugi
#endif

//====================
//	CGM Messages
//====================

#define CGM_HUDDRAW_SHADER			27
#define CGM_HUDDRAW_ALIGN			28
#define CGM_HUDDRAW_RECT			29
#define CGM_HUDDRAW_VIRTUALSIZE		30
#define CGM_HUDDRAW_COLOR			31
#define CGM_HUDDRAW_ALPHA			32
#define CGM_HUDDRAW_STRING			33
#define CGM_HUDDRAW_FONT			34
#define CGM_HUDDRAW_TIMER			38
#define CGM_HUDDRAW_3D				39
#define CGM_HUDDRAW_BREAK3D			40
#define CGM_HUDDRAW_FADE			41
#define CGM_HUDDRAW_MOVE			42
#define CGM_HUDDRAW_SCALE			43
#define CGM_VIEWMODELANIM			44
#define CGM_VIEWMODELPREFIX			45
#define CGM_MODHEIGHTFLOAT			46
#define CGM_SETVMASPEED				47
#define CGM_HINTSTRING				48
#define CGM_PLAYLOCALSOUND			49
#define CGM_SETLOCALSOUNDRATE		50
#define CGM_STOPLOCALSOUND			51
#define CGM_HUDDRAW_RECTX			52
#define CGM_HUDDRAW_RECTY			53
#define CGM_HUDDRAW_RECTWH			54
#define CGM_REPLICATION				55
#define CGM_HUDDRAW_DELETE			56

//====================
//	Hooks & Detours
//====================

#define unprotect(addr,len) (mprotect(addr,len,PROT_READ|PROT_WRITE|PROT_EXEC))
#define GET_PAGE(a) ((void*)(((unsigned long)a)&0xfffff000))

#define handle_error(msg) \
	do { perror( msg ); exit( EXIT_FAILURE ); } while ( 0 )


//====================
//	Misc Functions
//====================

extern int bDataCompare( const uint8_t* pData, const uint8_t* bMask, const char* szMask );
extern uintptr_t dwFindPattern( uintptr_t dwAddress, uintptr_t dwLen, uint8_t *bMask, const char * szMask );

extern void ReadMemory( uintptr_t dwBaseAddress, uint8_t buffer[], uintptr_t dwLen );
extern void WriteMemory( uintptr_t dwBaseAddress, uint8_t buffer[], uintptr_t dwLen );

uint8_t *detour_function( void * orig, void * detour, int len );
void detour_remove( void * orig, void * detour );

extern uintptr_t DetourPatchcall( uintptr_t pSrc, uintptr_t pDst, uintptr_t bForce );

#define _wpm(a,b,s) WriteProcessMemory(GetCurrentProcess(), (PVOID)(a), (PBYTE)(b), (s), NULL )

char *replace( char *string, const char *search, const char *replace );
void replace2( char *s, const char *s1, const char *s2 );

//====================
//	Misc Functions
//====================
extern const char *itemName[];
extern const char *itemPrefix[];

void VM_AddViewModelPrefix( const char * weapon_name, const char * prefix );
int VM_GetVMAnimPrefixString( char * weapon_name );
void VM_VMInit( void );

//++++++++END+++++++++

#ifndef WIN32

void initsighandlers(void);
void resetsighandlers(void);
void sighandler(int sig, siginfo_t *info, void *secret);

#else

void initExceptions( void );

#endif

#ifndef __FUNCTION__
#define __FUNCTION__ const char * function
#endif
