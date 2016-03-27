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

// hook.cpp : Hooks & Detours

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include "windows.h"
#else
#include <sys/mman.h>
#endif

#include "glb_local.h"
#include <float.h>

#include "hook.h"
#include "md5.h"

//#pragma warning(disable : 4172)
#pragma warning(disable : 4005 4083 4244 4047 4018 4700)

int bDataCompare( const uint8_t* pData, const uint8_t* bMask, const char* szMask )
{
    for(;*szMask;++szMask,++pData,++bMask)
        if(*szMask=='x' && *pData!=*bMask ) 
            return 0;
    return (*szMask) == 0;
}

uintptr_t dwFindPattern( uintptr_t dwAddress, uintptr_t dwLen, uint8_t *bMask, const char * szMask )
{
    DWORD i;

	for( i=0; i < dwLen; i++)
        if( bDataCompare( (BYTE*)( dwAddress+i ),bMask,szMask) )
            return (DWORD)(dwAddress+i);
    
    return 0;
}

void ReadMemory( uintptr_t dwBaseAddress, uint8_t buffer[], uintptr_t dwLen )
{
#ifndef WIN32
	int ret = unprotect( GET_PAGE( (void *)dwBaseAddress ), 4096 );

	if ( ret == -1 ) {
		handle_error( "Read memory error #1" );
	}

	if ( ( ( (uint)dwBaseAddress ) & 0xfff ) > 0xff9 )
		ret = unprotect( GET_PAGE( (void *)dwBaseAddress + 4096 ), 4096 );

	if ( ret == -1 ) {
		handle_error( "Read memory error #2" );
	}
#endif

	memcpy( ( void* )buffer, ( void* )dwBaseAddress, dwLen );
}

void WriteMemory( uintptr_t dwBaseAddress, uint8_t buffer[], uintptr_t dwLen )
{
#ifdef WIN32
	DWORD old_protection;
	DWORD old_protectionRevert;

	VirtualProtect( (LPVOID)dwBaseAddress, dwLen+5, PAGE_EXECUTE_READWRITE, &old_protection);
#else

	int ret = 0;

	ret = unprotect( GET_PAGE( (void *)dwBaseAddress ), 4096 );

	if ( ret == -1 ) {
		Com_Printf( "dwBaseAddress = %p\n", dwBaseAddress );
		handle_error( "Write memory error #1" );
	}

	if ( ( ( (uint)dwBaseAddress ) & 0xfff ) > 0xff9 )
		ret = unprotect( GET_PAGE( (void *)dwBaseAddress + 4096 ), 4096 );

	if ( ret == -1 ) {
		Com_Printf( "dwBaseAddress = %p\n", dwBaseAddress );
		handle_error( "Write memory error #2" );
	}

#endif

	memcpy( (void*)dwBaseAddress, (void*)buffer, dwLen );

#ifdef WIN32
	VirtualProtect( (LPVOID)dwBaseAddress, dwLen+5, old_protection, &old_protectionRevert);
#endif
}

BYTE *detour_function( void *orig, void *detour, int len )
{
	if ( orig == NULL || detour == NULL ) {
		return NULL;
	}

	unsigned char *tramp;
	unsigned char *tmp;

#ifdef WIN32
	DWORD old_protection;
	DWORD old_protectionRevert;
#endif

	if ( len<5 )
	{
		Com_Printf( "detour_function : length must be 5 or above (len=%d)! (orig=0x%p|detour=0x%p)\n", len, orig, detour );

		cgi.Error( ERR_FATAL, "Wrong length\n" );

		return NULL;
	}

	tramp = (unsigned char *)malloc( len + 5 + ( 2 * sizeof(DWORD) ) );

	*( DWORD * )tramp = (DWORD)orig; // store the original function address in the trampoline
	*( DWORD * )( tramp + 4 ) = len; // store the length in the trampoline
	tramp += 8;

	memset( tramp, 0x90, len+5 );

	memcpy( tramp, orig, len );
	tramp[len] = 0xE9;

	*( (void **)( tramp + len + 1 ) ) = (void *)( ( ( (unsigned int)orig ) + len ) - (unsigned int)( tramp + len + 5 ) );

#ifdef WIN32
	VirtualProtect( (LPVOID)orig, len+10, PAGE_EXECUTE_READWRITE, &old_protection);
#else
	unprotect( GET_PAGE( orig ), 4096 );

	if ( ( ( (uint)orig ) & 0xfff ) > 0xff9 )
		unprotect( GET_PAGE( orig + 4096 ), 4096 );
#endif

	*( (unsigned char *)orig ) = 0xE9;
	*( (void **)( (unsigned int)orig + 1 ) ) = (void*)( ( (unsigned int)detour ) - ( ( (unsigned int)orig ) + 5 ) );
	tmp = (unsigned char*)( ( (unsigned int)orig ) + 5 );

	for ( ; len>5; len--, tmp++ )
		*tmp = 0xCC;

#ifdef WIN32
	VirtualProtect( (LPVOID)orig, len+10, old_protection, &old_protectionRevert);
#endif

	return tramp;
}

void detour_remove( void * orig, void * detour )
{
	if ( orig == NULL || detour == NULL ) {
		return;
	}

#ifdef WIN32
	DWORD old_protection;
	DWORD old_protectionRevert;
#endif

	DWORD dwOld;
	int len;

	dwOld = *( DWORD * )( ( DWORD )detour - 8 ); // retrieve the original function address from the detour
	len = *( DWORD * ) ( ( DWORD )detour - 4 ); // retrieve the length from the detour

#ifdef WIN32
	VirtualProtect( (LPVOID)dwOld, len+10, PAGE_EXECUTE_READWRITE, &old_protection);
#else

	unprotect( GET_PAGE( dwOld ), 4096 );
	if ( ( ( (uint)dwOld ) & 0xfff ) > 0xff9 )
		unprotect( GET_PAGE( dwOld + 4096 ), 4096 );
#endif

	memcpy( (void*)dwOld, detour, len );

	free( ( void* )( ( DWORD )detour - 8 ) );

#ifdef WIN32
	VirtualProtect( (LPVOID)orig, len+10, old_protection, &old_protectionRevert);
#endif
}

DWORD DetourPatchcall( PBYTE pSrc, PBYTE pDst, BOOL bForce )
{
	DWORD dwOrgCallDest = 0;
	DWORD dwOldProtection = 0;
	DWORD dwRel = 0;

	if(!pSrc || !pDst) return (0);

	//type call <rel const>
	if( *pSrc == 0xE8 || bForce )
	{
		if( *pSrc == 0xE8 ){
			dwRel = *(DWORD*)( pSrc + 1 );
			dwOrgCallDest = ( (DWORD)pSrc + dwRel + 0x5 );
		} else {
			dwOrgCallDest = 1; // return 1 means no original call at pSrc
		}

#ifdef WIN32
		if( !VirtualProtect( (PVOID)pSrc, 0x1000, PAGE_EXECUTE_READWRITE, &dwOldProtection ) )
			return (0);
#else
		unprotect( GET_PAGE( pSrc ), 4096 );

		if ( ( ( (unsigned long)pSrc ) & 0xfff ) > 0xff9 )
			unprotect( GET_PAGE( pSrc + 4096 ), 4096 );
#endif

		*pSrc = 0xE8;
		*( DWORD* )( pSrc + 1 ) = (DWORD)( pDst - pSrc - 5 );

#ifdef WIN32
		VirtualProtect( (PVOID)pSrc, 0x1000, dwOldProtection, &dwOldProtection );
#endif

		return dwOrgCallDest;
	}

	return (0);
}

char *replace( char *string, const char *search, const char *replace )
{
	static char strings[ 2 ][ MAX_STRING_TOKENS ];
	static int index = 0;
	char *buffer;

	buffer = strings[ index & 1 ];
	index++;

	char *p;

	if( !( p = strstr( string, search ) ) ) {
		return string;
	}

	strncpy( buffer, string, p - string ); // Copy characters from 'str' start to 'orig' st$
	buffer[ p - string ] = '\0';

	sprintf( buffer + ( p - string ), "%s%s", replace, p + strlen( search ) );

	return buffer;
}

void replace2( char *s, const char *s1, const char *s2 )
{
	char *p, *sp, *dp;
	int len, cnt;

	for( ;; )
	{
		if( ( p = strstr( s, s1 ) ) == NULL ) return;

		len = strlen( s2 ) - strlen( s1 );
		if( len )
		{
			// Locate the string and replace string length, requiring shift action
			sp = p + strlen( s1 );  // From the SP referring location
			dp = sp + len;  // Move to the location referred to dp
			cnt = strlen( sp ) + 1; // The length
			memmove( p + strlen( s1 ) + len, p + strlen( s1 ), cnt );
		}
		memcpy( p, s2, strlen( s2 ) );
	}
}

const char *itemName[255] =
{
	"",
	"Papers",
	"Colt 45",
	"Walther P38",
	"Hi-Standard Silenced",
	"M1 Garand",
	"Mauser KAR 98K",
	"KAR98 - Sniper",
	"Springfield '03 Sniper",
	"Thompson",
	"MP40",
	"BAR",
	"StG 44",
	"Frag Grenade",
	"Stielhandgranate",
	"Bazooka",
	"Panzerschreck",
	"Shotgun",
	"unarmed",
	NULL
};

const char *itemPrefix[255] =
{
	"",
	"papers",
	"colt45",
	"p38",
	"histandard",
	"garand",
	"kar98",
	"kar98sniper",
	"springfield",
	"thompson",
	"mp40",
	"bar",
	"mp44",
	"fraggrenade",
	"stielhandgranate",
	"bazooka",
	"panzerschreck",
	"shotgun",
	"unarmed",
	NULL
};

void VM_AddViewModelPrefix( const char * weapon_name, const char * prefix )
{
	int i;
	char *weapon_name_allocated;
	char *weapon_prefix_allocated;

	for( i = 1; i < 255; i++ )
	{
		/* Don't add this more than once... */
		if( itemPrefix[i] != NULL && _stricmp( itemPrefix[i], prefix ) == 0 && _stricmp( itemName[i], weapon_name ) == 0 ) {
			return;
		}
	}

	/* Allocate the name so that it will be always present in the database */
	weapon_name_allocated = ( char * )malloc( strnlen( weapon_name, 255 ) );
	weapon_prefix_allocated = ( char * )malloc( strnlen( prefix, 255 ) );

	strcpy( weapon_name_allocated, weapon_name );
	strcpy( weapon_prefix_allocated, prefix );

	for( i = 1; i < 255; i++ )
	{
		if( itemName[i] == NULL || itemPrefix[i] == NULL )
		{
			itemName[i] = ( char * )weapon_name_allocated;
			itemPrefix[i] = ( char * )weapon_prefix_allocated;
			break;
		}
	}
}

int VM_GetVMAnimPrefixString( char * weapon_name )
{
	int i;

	if( weapon_name == NULL ) {
		return 18; // unarmed
	}

	for( i = 1; i < 255; i++ )
	{
		if( itemName[i] == NULL ) {
			continue;
		}

		if( _stricmp( itemName[i], weapon_name ) == 0 ) {
			return i;
		}
	}

	return 18;
}

void VM_VMInit( void )
{
	int i;

	for( i = 1; i < 255; i++ )
	{
		if( itemName[i] == NULL || itemPrefix[i] == NULL ) {
			break;
		}
	}

	for( ; i < 255; i ++ )
	{
		itemName[i] = NULL;
		itemPrefix[i] = NULL;
	}
}
