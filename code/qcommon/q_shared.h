/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#ifndef __Q_SHARED_H
#define __Q_SHARED_H

// q_shared.h -- included first by ALL program modules.
// A user mod should never modify this file

#if defined(__cplusplus)
extern "C" {
#endif

#define PRODUCT_NAME            "OpenMoHAA"
#define PRODUCT_VERSION         "0.1"

#ifdef SVN_VERSION
# define Q3_VERSION PRODUCT_NAME " " SVN_VERSION
#else
# define Q3_VERSION PRODUCT_NAME " " PRODUCT_VERSION
#endif

#define CLIENT_WINDOW_TITLE     "OpenMoHAA"
#define CLIENT_WINDOW_MIN_TITLE "OpenMoHAA"
// 1.32 released 7-10-2002

#define BASEGAME              "main"

#define MAX_TEAMNAME 32

#ifdef _MSC_VER

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4032)
#pragma warning(disable : 4051)
#pragma warning(disable : 4057)		// slightly different base types
#pragma warning(disable : 4100)		// unreferenced formal parameter
#pragma warning(disable : 4115)
#pragma warning(disable : 4125)		// decimal digit terminates octal escape sequence
#pragma warning(disable : 4127)		// conditional expression is constant
#pragma warning(disable : 4136)
#pragma warning(disable : 4152)		// nonstandard extension, function/data pointer conversion in expression
//#pragma warning(disable : 4201)
//#pragma warning(disable : 4214)
#pragma warning(disable : 4244)
#pragma warning(disable : 4142)		// benign redefinition
//#pragma warning(disable : 4305)		// truncation from const double to float
//#pragma warning(disable : 4310)		// cast truncates constant value
//#pragma warning(disable:  4505) 	// unreferenced local function has been removed
#pragma warning(disable : 4514)
#pragma warning(disable : 4702)		// unreachable code
#pragma warning(disable : 4711)		// selected for automatic inline expansion
#pragma warning(disable : 4220)		// varargs matches remaining parameters
//#pragma intrinsic( memset, memcpy )
#endif

//Ignore __attribute__ on non-gcc platforms
#ifndef __GNUC__
#ifndef __attribute__
#define __attribute__(x)
#endif
#endif

/**********************************************************************
  VM Considerations

  The VM can not use the standard system headers because we aren't really
  using the compiler they were meant for.  We use bg_lib.h which contains
  prototypes for the functions we define for our own use in bg_lib.c.

  When writing mods, please add needed headers HERE, do not start including
  stuff like <stdio.h> in the various .c files that make up each of the VMs
  since you will be including system headers files can will have issues.

  Remember, if you use a C library function that is not defined in bg_lib.c,
  you will have to add your own version for support in the VM.

 **********************************************************************/

#ifndef Q3_VM

#ifdef _DEBUG_MEM
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#define Z_Malloc malloc
#define Z_TagMalloc(size, tag) malloc( size )
#define Z_Free(ptr) free(ptr)
#define Hunk_Alloc(size) malloc(size)
#define Hunk_AllocateTempMemory(size) malloc(size)
#define Hunk_FreeTempMemory(ptr) free(ptr)
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

#ifdef _MSC_VER
#include <io.h>

	typedef __int64 int64_t;
	typedef __int32 int32_t;
	typedef __int16 int16_t;
	typedef __int8 int8_t;
	typedef unsigned __int64 uint64_t;
	typedef unsigned __int32 uint32_t;
	typedef unsigned __int16 uint16_t;
	typedef unsigned __int8 uint8_t;

	// vsnprintf is ISO/IEC 9899:1999
	// abstracting this to make it portable
	int Q_vsnprintf( char *str, size_t size, const char *format, va_list ap );
#else
#include <stdint.h>

#define Q_vsnprintf vsnprintf
#endif

#define HAVE_STDINT_H
#define _HAVE_STDINT_H 1

#endif

#include "q_platform.h"

//=============================================================

typedef unsigned char 		byte;
typedef unsigned char		uchar;

#define qfalse 0
#define qtrue 1

typedef int	qboolean;

typedef int		qhandle_t;
typedef int		sfxHandle_t;
typedef int		fileHandle_t;
typedef int		clipHandle_t;

#define PAD(x,y) (((x)+(y)-1) & ~((y)-1))

#ifdef __GNUC__
#define ALIGN(x) __attribute__((aligned(x)))
#else
#define ALIGN(x)
#endif

#define PAD(base, alignment)	(((base)+(alignment)-1) & ~((alignment)-1))
#define PADLEN(base, alignment)	(PAD((base), (alignment)) - (base))

#define PADP(base, alignment)	((void *) PAD((intptr_t) (base), (alignment)))

#ifdef __GNUC__
#define QALIGN(x) __attribute__((aligned(x)))
#else
#define QALIGN(x)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef BIT
#define BIT(x)				(1 << x)
#endif

#define STRING(s)			#s
// expand constants before stringifying them
#define XSTRING(s)			STRING(s)

#define	MAX_QINT			0x7fffffff
#define	MIN_QINT			(-MAX_QINT-1)

#define ARRAY_LEN(x)			(sizeof(x) / sizeof(*(x)))
#define STRARRAY_LEN(x)			(ARRAY_LEN(x) - 1)

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

#ifndef WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif

// plane sides
typedef enum
{
	SIDE_FRONT = 0,
	SIDE_BACK = 1,
	SIDE_ON = 2,
	SIDE_CROSS = 3
} planeSide_t;

typedef enum
{
	SOLID_NOT,			// no interaction with other objects
	SOLID_TRIGGER,		// only touch when inside, after moving
	SOLID_BBOX,			// touch on edge
	SOLID_BSP			// bsp clip, touch on edge
} solid_t;

// the game guarantees that no string from the network will ever
// exceed MAX_STRING_CHARS
#define	MAX_STRING_CHARS	2048	// max length of a string passed to Cmd_TokenizeString
#define	MAX_STRING_TOKENS	1024	// max tokens resulting from Cmd_TokenizeString
#define	MAX_TOKEN_CHARS		1024	// max length of an individual token

#define	MAX_INFO_STRING		1350
#define	MAX_INFO_KEY		  1024
#define	MAX_INFO_VALUE		1024

#define	BIG_INFO_STRING		8192  // used for system info key only
#define	BIG_INFO_KEY		8192
#define	BIG_INFO_VALUE		8192


#define	MAX_QPATH			256		// max length of a quake game pathname
#ifdef PATH_MAX
#define MAX_OSPATH			PATH_MAX
#else
#define	MAX_OSPATH			256		// max length of a filesystem pathname
#endif

#define	MAX_NAME_LENGTH		32		// max length of a client name

#define	MAX_SAY_TEXT	150

// paramters for command buffer stuffing
typedef enum {
	EXEC_NOW,			// don't return until completed, a VM should NEVER use this,
						// because some commands might cause the VM to be unloaded...
	EXEC_INSERT,		// insert at current position, but don't run yet
	EXEC_APPEND			// add to end of the command buffer (normal case)
} cbufExec_t;


//
// these aren't needed by any of the VMs.  put in another header?
//
#define	MAX_MAP_AREA_BYTES		32		// bit vector of area visibility


// print levels from renderer (FIXME: set up for game / cgame?)
typedef enum {
	PRINT_ALL,
	PRINT_DEVELOPER,		// only print when "developer 1"
	PRINT_DEVELOPER_2,		// print when "developer 2"
	PRINT_WARNING,
	PRINT_ERROR
} printParm_t;


#ifdef ERR_FATAL
#undef ERR_FATAL			// this is be defined in malloc.h
#endif

// parameters to the main Error routine
typedef enum {
	ERR_FATAL,					// exit the entire game with a popup window
	ERR_DROP,					// print to console and disconnect from game
	ERR_SERVERDISCONNECT,		// don't kill server
	ERR_DISCONNECT,				// client disconnected from the server
	ERR_NEED_CD					// pop up the need-cd dialog
} errorParm_t;


// font rendering values used by ui and cgame

#define PROP_GAP_WIDTH			5
#define PROP_SPACE_WIDTH		8
#define PROP_HEIGHT				30
#define PROP_SMALL_SIZE_SCALE	0.75

#define BLINK_DIVISOR			200
#define PULSE_DIVISOR			250.0f

#define UI_LEFT			0x00000000	// default
#define UI_CENTER		0x00000001
#define UI_RIGHT		0x00000002
#define UI_FORMATMASK	0x00000007
#define UI_SMALLFONT	0x00000010
#define UI_BIGFONT		0x00000020	// default
#define UI_GIANTFONT	0x00000040
#define UI_DROPSHADOW	0x00000800
#define UI_BLINK		0x00001000
#define UI_INVERSE		0x00002000
#define UI_PULSE		0x00004000

#if defined(_DEBUG) && !defined(BSPC)
	#define HUNK_DEBUG
#endif

#define Com_Memset memset
#define Com_Memcpy memcpy

#define Com_Allocate malloc
#define Com_Dealloc free

#define CIN_system	1
#define CIN_loop	2
#define	CIN_hold	4
#define CIN_silent	8
#define CIN_shader	16

/*
==============================================================

MATHLIB

==============================================================
*/


typedef float vec_t;
typedef vec_t vec2_t[2];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];
typedef vec_t quat_t[4]; // | x y z w |
typedef vec_t vec5_t[5];
typedef vec_t matrix3x3_t[9];
typedef vec_t matrix_t[16];
typedef vec3_t axis_t[3];

typedef	int	fixed4_t;
typedef	int	fixed8_t;
typedef	int	fixed16_t;

#ifndef M_PI
#define M_PI		3.14159265358979323846f	// matches value in gcc v2 math.h
#endif

#ifndef M_SQRT2
#define M_SQRT2		1.414213562f
#endif

#ifndef M_SQRT1_2
#define M_SQRT1_2	7.0710678118654752440E-1
#endif

#define NUMVERTEXNORMALS	162
extern	vec3_t	bytedirs[NUMVERTEXNORMALS];

// all drawing is done to a 640*480 virtual screen size
// and will be automatically scaled to the real resolution
#define	SCREEN_WIDTH		640
#define	SCREEN_HEIGHT		480

#define TINYCHAR_WIDTH		(SMALLCHAR_WIDTH)
#define TINYCHAR_HEIGHT		(SMALLCHAR_HEIGHT/2)

#define SMALLCHAR_WIDTH		8
#define SMALLCHAR_HEIGHT	16

#define BIGCHAR_WIDTH		16
#define BIGCHAR_HEIGHT		16

#define	GIANTCHAR_WIDTH		32
#define	GIANTCHAR_HEIGHT	48

extern	vec4_t		colorBlack;
extern	vec4_t		colorRed;
extern	vec4_t		colorGreen;
extern	vec4_t		colorBlue;
extern	vec4_t		colorYellow;
extern	vec4_t		colorMagenta;
extern	vec4_t		colorCyan;
extern	vec4_t		colorWhite;
extern	vec4_t		colorLtGrey;
extern	vec4_t		colorMdGrey;
extern	vec4_t		colorDkGrey;

enum {
	MESSAGE_YELLOW = 1,
	MESSAGE_CHAT_WHITE,
	MESSAGE_WHITE,
	MESSAGE_CHAT_RED,
	MESSAGE_MAX
};

#define Q_COLOR_ESCAPE	'^'
#define Q_IsColorString(p)	( p && *(p) == Q_COLOR_ESCAPE && *((p)+1) && isalnum(*((p)+1)) ) // ^[0-9a-zA-Z]

#define COLOR_BLACK		'0'
#define COLOR_RED		'1'
#define COLOR_GREEN		'2'
#define COLOR_YELLOW	'3'
#define COLOR_BLUE		'4'
#define COLOR_CYAN		'5'
#define COLOR_MAGENTA	'6'
#define COLOR_WHITE		'7'
#define ColorIndex(c)	( ( (c) - '0' ) & 7 )

#define S_COLOR_BLACK	"^0"
#define S_COLOR_RED		"^1"
#define S_COLOR_GREEN	"^2"
#define S_COLOR_YELLOW	"^3"
#define S_COLOR_BLUE	"^4"
#define S_COLOR_CYAN	"^5"
#define S_COLOR_MAGENTA	"^6"
#define S_COLOR_WHITE	"^7"

#define HUD_MESSAGE_YELLOW			"\x01"
#define HUD_MESSAGE_WHITE			"\x03"
#define HUD_MESSAGE_CHAT_WHITE		"\x02"
#define HUD_MESSAGE_CHAT_RED		"\x04"

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

extern vec4_t	g_color_table[8];

#define	MAKERGB( v, r, g, b ) v[0]=r;v[1]=g;v[2]=b
#define	MAKERGBA( v, r, g, b, a ) v[0]=r;v[1]=g;v[2]=b;v[3]=a

#define DEG2RAD( a ) ( ( (a) * M_PI ) / 180.0F )
#define RAD2DEG( a ) ( ( (a) * 180.0f ) / M_PI )

#define Q_max(a, b)      ((a) > (b) ? (a) : (b))
#define Q_min(a, b)      ((a) < (b) ? (a) : (b))
#define Q_bound(a, b, c) (Q_max(a, Q_min(b, c)))
#define Q_clamp(a, b, c) ((b) >= (c) ? (a)=(b) : (a) < (b) ? (a)=(b) : (a) > (c) ? (a)=(c) : (a))
#define Q_lerp(from, to, frac) (from + ((to - from) * frac))

#define LERP( a, b, w ) ( ( a ) * ( 1.0f - ( w ) ) + ( b ) * ( w ) )
#define LUMA( red, green, blue ) ( 0.2126f * ( red ) + 0.7152f * ( green ) + 0.0722f * ( blue ) )

#ifndef min
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

#define bound(a,minval,maxval)  ( ((a) > (minval)) ? ( ((a) < (maxval)) ? (a) : (maxval) ) : (minval) )

struct cplane_s;

extern	vec3_t	vec3_origin;
extern	vec3_t	axisDefault[3];
extern matrix_t matrixIdentity;

#define	nanmask (255<<23)

#define	IS_NAN(x) (((*(int *)&x)&nanmask)==nanmask)

static ID_INLINE long Q_ftol(float f)
{
#if id386_sse && defined(_MSC_VER)
	static int      tmp;
	__asm fld f
	__asm fistp tmp
	__asm mov eax, tmp
#else
	return (long)f;
#endif
}

#if idppc

static ID_INLINE float Q_rsqrt( float number ) {
		float x = 0.5f * number;
                float y;
#ifdef __GNUC__
                asm("frsqrte %0,%1" : "=f" (y) : "f" (number));
#else
		y = __frsqrte( number );
#endif
		return y * (1.5f - (x * y * y));
	}

#ifdef __GNUC__
static ID_INLINE float Q_fabs(float x) {
    float abs_x;

    asm("fabs %0,%1" : "=f" (abs_x) : "f" (x));
    return abs_x;
}
#else
#define Q_fabs __fabsf
#endif

#else
float Q_fabs( float f );
float Q_rsqrt( float f );		// reciprocal square root
#endif

#define SQRTFAST( x ) ( (x) * Q_rsqrt( x ) )

byte ClampByte(int i);
signed char ClampChar( int i );
signed short ClampShort( int i );

double		dEpsilon( void );
double 		dIdentity( void );
double		dSign( const double number );
double		dClamp( const double value, const double min, const double max );
double		dDistance( const double value1, const double value2 );
qboolean	dCloseEnough( const double value1, const double value2, const double epsilon );
qboolean	dSmallEnough( const double value, const double epsilon );

float		fEpsilon( void );
float 		fIdentity( void );
float		fSign( const float number );
float		fClamp( const float value, const float min, const float max );
float		fDistance( const float value1, const float value2 );
qboolean	fCloseEnough( const float value1, const float value2, const float epsilon );
qboolean	fSmallEnough( const float value, const float epsilon );

int			iSign( const int number );
int			iClamp( const int value, const int min, const int max );

// this isn't a real cheap function to call!
int DirToByte( vec3_t dir );
void ByteToDir( int b, vec3_t dir );

#if	1

#define ftol(x)		((int)(x))

#define DotProduct(x,y)			((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define DotProduct2D(x,y)		((x)[0]*(y)[0]+(x)[1]*(y)[1])
#define VectorSubtract(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2])
#define VectorAdd(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2])
#define VectorCopy(a,b)			((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2])
#define	VectorScale(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s))
#define	VectorMA(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s))

#else

#define DotProduct(x,y)			_DotProduct(x,y)
#define VectorSubtract(a,b,c)	_VectorSubtract(a,b,c)
#define VectorAdd(a,b,c)		_VectorAdd(a,b,c)
#define VectorCopy(a,b)			_VectorCopy(a,b)
#define	VectorScale(v, s, o)	_VectorScale(v,s,o)
#define	VectorMA(v, s, b, o)	_VectorMA(v,s,b,o)

#endif

#ifdef Q3_VM
#ifdef VectorCopy
#undef VectorCopy
// this is a little hack to get more efficient copies in our interpreter
typedef struct {
	float	v[3];
} vec3struct_t;
#define VectorCopy(a,b)	(*(vec3struct_t *)b=*(vec3struct_t *)a)
#endif
#endif

#define VectorClear2D(a)			((a)[0]=(a)[1]=0)
#define VectorLength2DSquared(a)	((a)[0]*(a)[0]+(a)[1]*(a)[1])
#define VectorLength2D(a)			(sqrt(VectorLength2DSquared(a)))
#define VectorSet2D(v, x, y)		((v)[0]=(x), (v)[1]=(y))

#define VectorClear(a)			((a)[0]=(a)[1]=(a)[2]=0)
#define VectorNegate(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2])
#define VectorSet(v, x, y, z)	((v)[0]=(x), (v)[1]=(y), (v)[2]=(z))
#define Vector4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define	SnapVector(v) {v[0]=((int)(v[0]));v[1]=((int)(v[1]));v[2]=((int)(v[2]));}

#define DotProduct4(x,y)		((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2]+(x)[3]*(y)[3])
#define VectorSubtract4(a,b,c)	((c)[0]=(a)[0]-(b)[0],(c)[1]=(a)[1]-(b)[1],(c)[2]=(a)[2]-(b)[2],(c)[3]=(a)[3]-(b)[3])
#define VectorAdd4(a,b,c)		((c)[0]=(a)[0]+(b)[0],(c)[1]=(a)[1]+(b)[1],(c)[2]=(a)[2]+(b)[2],(c)[3]=(a)[3]+(b)[3])
#define VectorCopy4(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])
#define	VectorScale4(v, s, o)	((o)[0]=(v)[0]*(s),(o)[1]=(v)[1]*(s),(o)[2]=(v)[2]*(s),(o)[3]=(v)[3]*(s))
#define	VectorMA4(v, s, b, o)	((o)[0]=(v)[0]+(b)[0]*(s),(o)[1]=(v)[1]+(b)[1]*(s),(o)[2]=(v)[2]+(b)[2]*(s),(o)[3]=(v)[3]+(b)[3]*(s))
#define VectorClear4(a)			((a)[0]=(a)[1]=(a)[2]=(a)[3]=0)
#define VectorNegate4(a,b)		((b)[0]=-(a)[0],(b)[1]=-(a)[1],(b)[2]=-(a)[2],(b)[3]=-(a)[3])
#define VectorSet4(v,x,y,z,w)	((v)[0]=(x),(v)[1]=(y),(v)[2]=(z),(v)[3]=(w))

#define VectorCopy5(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3],(b)[4]=(a)[4])

// just in case you do't want to use the macros
vec_t _DotProduct( const vec3_t v1, const vec3_t v2 );
void _VectorSubtract( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorAdd( const vec3_t veca, const vec3_t vecb, vec3_t out );
void _VectorCopy( const vec3_t in, vec3_t out );
void _VectorScale( const vec3_t in, float scale, vec3_t out );
void _VectorMA( const vec3_t veca, float scale, const vec3_t vecb, vec3_t vecc );
vec_t Q_rint( vec_t in );

unsigned ColorBytes3 (float r, float g, float b);
unsigned ColorBytes4 (float r, float g, float b, float a);

float NormalizeColor( const vec3_t in, vec3_t out );
void ClampColor(vec4_t color);

float RadiusFromBounds( const vec3_t mins, const vec3_t maxs );
void ClearBounds( vec3_t mins, vec3_t maxs );
void ZeroBounds(vec3_t mins, vec3_t maxs);
void AddPointToBounds( const vec3_t v, vec3_t mins, vec3_t maxs );
void BoundsAdd(vec3_t mins, vec3_t maxs, const vec3_t mins2, const vec3_t maxs2);

#if !defined( Q3_VM ) || ( defined( Q3_VM ) && defined( __Q3_VM_MATH ) )
static ID_INLINE int VectorCompare( const vec3_t v1, const vec3_t v2 ) {
	if (v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2]) {
		return 0;
	}
	return 1;
}

static ID_INLINE int VectorCompare2D( const vec2_t v1, const vec2_t v2 ) {
	if( v1[ 0 ] != v2[ 0 ] || v1[ 1 ] != v2[ 1 ] ) {
		return 0;
	}
	return 1;
}

static ID_INLINE int VectorCompare4(const vec4_t v1, const vec4_t v2)
{
	if(v1[0] != v2[0] || v1[1] != v2[1] || v1[2] != v2[2] || v1[3] != v2[3])
	{
		return 0;
	}
	return 1;
}

static ID_INLINE int VectorCompare5( const vec5_t v1, const vec5_t v2 )
{
	if( v1[ 0 ] != v2[ 0 ] || v1[ 1 ] != v2[ 1 ] || v1[ 2 ] != v2[ 2 ] || v1[ 3 ] != v2[ 3 ] || v1[ 4 ] != v2[ 4 ] )
	{
		return 0;
	}
	return 1;
}

static ID_INLINE int VectorCompareEpsilon( const vec3_t v1, const vec3_t v2, float fEpsilon )
{
	int i;

	for( i = 0; i < 3; i++ )
	{
		if( fabs( v1[ i ] - v2[ i ] ) > fEpsilon ) {
			return 0;
		}
	}
	return 1;
}

static ID_INLINE vec_t VectorLength( const vec3_t v ) {
	return (vec_t)sqrt (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static ID_INLINE vec_t VectorLengthSquared( const vec3_t v ) {
	return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

static ID_INLINE vec_t Distance( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return VectorLength( v );
}

static ID_INLINE vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 ) {
	vec3_t	v;

	VectorSubtract (p2, p1, v);
	return v[0]*v[0] + v[1]*v[1] + v[2]*v[2];
}

// fast vector normalize routine that does not check to make sure
// that length != 0, nor does it return length, uses rsqrt approximation
static ID_INLINE void VectorNormalizeFast( vec3_t v )
{
	float ilength;

	ilength = Q_rsqrt( DotProduct( v, v ) );

	v[0] *= ilength;
	v[1] *= ilength;
	v[2] *= ilength;
}

static ID_INLINE void VectorInverse( vec3_t v ){
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

static ID_INLINE void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross ) {
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

#else
int VectorCompare( const vec3_t v1, const vec3_t v2 );

vec_t VectorLength( const vec3_t v );

vec_t VectorLengthSquared( const vec3_t v );

vec_t Distance( const vec3_t p1, const vec3_t p2 );

vec_t DistanceSquared( const vec3_t p1, const vec3_t p2 );

void VectorNormalizeFast( vec3_t v );

void VectorInverse( vec3_t v );

void CrossProduct( const vec3_t v1, const vec3_t v2, vec3_t cross );

#endif

vec_t VectorNormalize (vec3_t v);		// returns vector length
vec_t VectorNormalize2( const vec3_t v, vec3_t out );
vec_t VectorNormalize2D( vec2_t v );
vec_t VectorNormalize2D2( const vec2_t v, vec2_t out );
void VectorPackTo01( vec3_t v );
void Vector4Scale( const vec4_t in, vec_t scale, vec4_t out );
void VectorRotate( vec3_t in, vec3_t matrix[3], vec3_t out );

int NearestPowerOfTwo(int val);
int Q_log2(int val);

float Q_acos(float c);

int		Q_rand( int *seed );
float	Q_random( int *seed );
float	Q_crandom( int *seed );

#define random()	((rand () & 0x7fff) / ((float)0x7fff))
#define random2()	((rand () & 0x7fff) * (1.0 / ((float)0x7fff)))
#define crandom()	(2.0 * (random() - 0.5))

float grandom( void );
float erandom( float mean );

#define AnglesToMat AnglesToAxis

void vectoangles( const vec3_t value1, vec3_t angles );
void VectorToAngles( const vec3_t vec, vec3_t angles );
void AnglesToAxis( const vec3_t angles, vec3_t axis[3] );

void	R_ConcatRotations( float in1[ 3 ][ 3 ], float in2[ 3 ][ 3 ], float out[ 3 ][ 3 ] );
void	R_ConcatTransforms( float in1[ 3 ][ 4 ], float in2[ 3 ][ 4 ], float out[ 3 ][ 4 ] );

void	MatrixToEulerAngles( const float mat[ 3 ][ 3 ], vec3_t ang );
void	QuatToMat( const float q[ 4 ], float m[ 3 ][ 3 ] );
void	EulerToQuat( float ang[ 3 ], float q[ 4 ] );
void	MatToQuat( float srcMatrix[ 3 ][ 3 ], float destQuat[ 4 ] );

float ProjectPointOnLine( vec_t *i_vStart, vec_t *i_vEnd, vec_t *i_vPoint, vec_t *o_vProj );
float ProjectLineOnPlane( vec_t *vPlaneNorm, float fPlaneDist, vec_t *vStart, vec_t *vEnd, vec_t *vProj );

float anglemod( float a );
float angledist( float ang );

void AxisClear( vec3_t axis[3] );
void AxisCopy( const vec3_t in[3], vec3_t out[3] );

void SetPlaneSignbits( struct cplane_s *out );
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, struct cplane_s *plane);

void  CalculateRotatedBounds( vec3_t angles, vec3_t mins, vec3_t maxs );
void  CalculateRotatedBounds2( float trans[ 3 ][ 3 ], vec3_t mins, vec3_t maxs );
int   BoundingBoxToInteger( vec3_t mins, vec3_t maxs );
void  IntegerToBoundingBox( int num, vec3_t mins, vec3_t maxs );
qboolean BoundsIntersect(const vec3_t mins, const vec3_t maxs,
		const vec3_t mins2, const vec3_t maxs2);
qboolean BoundsIntersectSphere(const vec3_t mins, const vec3_t maxs,
		const vec3_t origin, vec_t radius);
qboolean BoundsIntersectPoint(const vec3_t mins, const vec3_t maxs,
		const vec3_t origin);

float	AngleMod(float a);
float	LerpAngle (float from, float to, float frac);
float	LerpAngleFromCurrent( float from, float to, float current, float frac );
float	AngleSubtract( float a1, float a2 );
void	AnglesSubtract( vec3_t v1, vec3_t v2, vec3_t v3 );

float AngleNormalize360 ( float angle );
float AngleNormalize180 ( float angle );
float AngleDelta ( float angle1, float angle2 );

qboolean PlaneFromPoints( vec4_t plane, const vec3_t a, const vec3_t b, const vec3_t c );
qboolean PlanesGetIntersectionPoint(const vec4_t plane1, const vec4_t plane2, const vec4_t plane3, vec3_t out);
void PlaneIntersectRay(const vec3_t rayPos, const vec3_t rayDir, const vec4_t plane, vec3_t res);
void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal );
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees );
void RotateAroundDirection( vec3_t axis[3], float yaw );
void MakeNormalVectors( const vec3_t forward, vec3_t right, vec3_t up );
// perpendicular vector could be replaced by this

//int	PlaneTypeForNormal (vec3_t normal);

void  MatrixTransformVector( const vec3_t in, const float mat[ 3 ][ 3 ], vec3_t out );
void  MatrixTransformVectorRight( const float mat[ 3 ][ 3 ], const vec3_t in, vec3_t out );
void Matrix3x3Multiply( const float in1[ 3 ][ 3 ], const float in2[ 3 ][ 3 ], float out[ 3 ][ 3 ] );
void AngleVectors( const vec3_t angles, vec3_t forward, vec3_t right, vec3_t up );
void AngleVectorsLeft( const vec3_t angles, vec3_t forward, vec3_t left, vec3_t up );
void PerpendicularVector( vec3_t dst, const vec3_t src );

void            GetPerpendicularViewVector(const vec3_t point, const vec3_t p1, const vec3_t p2, vec3_t up);
void            ProjectPointOntoVector(vec3_t point, vec3_t vStart, vec3_t vEnd, vec3_t vProj);

float           pointToLineDistance(const vec3_t point, const vec3_t p1, const vec3_t p2);
float           VectorMinComponent(vec3_t v);
float           VectorMaxComponent(vec3_t v);

static ID_INLINE void VectorLerp(const vec3_t from, const vec3_t to, float frac, vec3_t out)
{
	out[0] = from[0] + ((to[0] - from[0]) * frac);
	out[1] = from[1] + ((to[1] - from[1]) * frac);
	out[2] = from[2] + ((to[2] - from[2]) * frac);
}

vec_t           DistanceBetweenLineSegmentsSquared(const vec3_t sP0, const vec3_t sP1,
												   const vec3_t tP0, const vec3_t tP1, float *s, float *t);
vec_t           DistanceBetweenLineSegments(const vec3_t sP0, const vec3_t sP1,
											const vec3_t tP0, const vec3_t tP1, float *s, float *t);

void			VectorMatrixInverse( void* DstMatrix, const void* SrcMatrix );

void            MatrixIdentity(matrix_t m);
void            MatrixClear(matrix_t m);
void            MatrixCopy(const matrix_t in, matrix_t out);
qboolean        MatrixCompare(const matrix_t a, const matrix_t b);
void            MatrixTransposeIntoXMM(const matrix_t m);
void            MatrixTranspose(const matrix_t in, matrix_t out);

// invert any m4x4 using Kramer's rule.. return qtrue if matrix is singular, else return qfalse
qboolean        MatrixInverse(matrix_t m);
void            MatrixSetupXRotation(matrix_t m, vec_t degrees);
void            MatrixSetupYRotation(matrix_t m, vec_t degrees);
void            MatrixSetupZRotation(matrix_t m, vec_t degrees);
void            MatrixSetupTranslation(matrix_t m, vec_t x, vec_t y, vec_t z);
void            MatrixSetupScale(matrix_t m, vec_t x, vec_t y, vec_t z);
void            MatrixSetupShear(matrix_t m, vec_t x, vec_t y);
void            Matrix4x4Multiply(const matrix_t a, const matrix_t b, matrix_t out);
void			MatrixMultiply( const float in1[ 3 ][ 3 ], const float in2[ 3 ][ 3 ], float out[ 3 ][ 3 ] );
void            MatrixMultiply2( matrix_t m, const matrix_t m2 );
void            MatrixMultiplyRotation(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll);
void            MatrixMultiplyZRotation(matrix_t m, vec_t degrees);
void            MatrixMultiplyTranslation(matrix_t m, vec_t x, vec_t y, vec_t z);
void            MatrixMultiplyScale(matrix_t m, vec_t x, vec_t y, vec_t z);
void            MatrixMultiplyShear(matrix_t m, vec_t x, vec_t y);
void            MatrixToAngles(const matrix_t m, vec3_t angles);
void            MatrixFromAngles(matrix_t m, vec_t pitch, vec_t yaw, vec_t roll);
void            MatrixFromVectorsFLU(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up);
void            MatrixFromVectorsFRU(matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up);
void            MatrixFromQuat(matrix_t m, const quat_t q);
void            MatrixFromPlanes(matrix_t m, const vec4_t left, const vec4_t right, const vec4_t bottom, const vec4_t top,
								 const vec4_t near, const vec4_t far);
void            MatrixToVectorsFLU(const matrix_t m, vec3_t forward, vec3_t left, vec3_t up);
void            MatrixToVectorsFRU(const matrix_t m, vec3_t forward, vec3_t right, vec3_t up);
void            MatrixSetupTransformFromVectorsFLU(matrix_t m, const vec3_t forward, const vec3_t left, const vec3_t up, const vec3_t origin);
void            MatrixSetupTransformFromVectorsFRU(matrix_t m, const vec3_t forward, const vec3_t right, const vec3_t up, const vec3_t origin);
void            MatrixSetupTransformFromRotation(matrix_t m, const matrix_t rot, const vec3_t origin);
void            MatrixSetupTransformFromQuat(matrix_t m, const quat_t quat, const vec3_t origin);
void            MatrixAffineInverse(const matrix_t in, matrix_t out);
void            MatrixTransformNormal(const matrix_t m, const vec3_t in, vec3_t out);
void            MatrixTransformNormal2(const matrix_t m, vec3_t inout);
void            MatrixTransformPoint(const matrix_t m, const vec3_t in, vec3_t out);
void            MatrixTransformPoint2(const matrix_t m, vec3_t inout);
void            MatrixTransform4(const matrix_t m, const vec4_t in, vec4_t out);
void            MatrixTransformPlane(const matrix_t m, const vec4_t in, vec4_t out);
void            MatrixTransformPlane2(const matrix_t m, vec3_t inout);
void            MatrixPerspectiveProjection(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far);
void            MatrixPerspectiveProjectionLH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far);
void            MatrixPerspectiveProjectionRH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far);
void            MatrixPerspectiveProjectionFovYAspectLH(matrix_t m, vec_t fov, vec_t aspect, vec_t near, vec_t far);
void            MatrixPerspectiveProjectionFovXYLH(matrix_t m, vec_t fovX, vec_t fovY, vec_t near, vec_t far);
void            MatrixPerspectiveProjectionFovXYRH(matrix_t m, vec_t fovX, vec_t fovY, vec_t near, vec_t far);
void            MatrixPerspectiveProjectionFovXYInfiniteRH(matrix_t m, vec_t fovX, vec_t fovY, vec_t near);
void            MatrixOrthogonalProjection(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far);

void			MatrixOrthogonalProjectionLH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far);
void			MatrixOrthogonalProjectionRH(matrix_t m, vec_t left, vec_t right, vec_t bottom, vec_t top, vec_t near, vec_t far);

void			MatrixPlaneReflection(matrix_t m, const vec4_t plane);

void            MatrixLookAtLH(matrix_t output, const vec3_t pos, const vec3_t dir, const vec3_t up);
void            MatrixLookAtRH(matrix_t m, const vec3_t eye, const vec3_t dir, const vec3_t up);
void            MatrixScaleTranslateToUnitCube(matrix_t m, const vec3_t mins, const vec3_t maxs);
void            MatrixCrop(matrix_t m, const vec3_t mins, const vec3_t maxs);

void			TransposeMatrix( float in[ 3 ][ 3 ], float out[ 3 ][ 3 ] );

static ID_INLINE void AnglesToMatrix(const vec3_t angles, matrix_t m)
{
	MatrixFromAngles(m, angles[PITCH], angles[YAW], angles[ROLL]);
}


#define QuatSet(q,x,y,z,w)	((q)[0]=(x),(q)[1]=(y),(q)[2]=(z),(q)[3]=(w))
#define QuatCopy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

#define QuatCompare(a,b)	((a)[0]==(b)[0] && (a)[1]==(b)[1] && (a)[2]==(b)[2] && (a)[3]==(b)[3])

static ID_INLINE void QuatClear(quat_t q)
{
	q[0] = 0;
	q[1] = 0;
	q[2] = 0;
	q[3] = 1;
}

/*
static ID_INLINE int QuatCompare(const quat_t a, const quat_t b)
{
	if(a[0] != b[0] || a[1] != b[1] || a[2] != b[2] || a[3] != b[3])
	{
		return 0;
	}
	return 1;
}
*/

static ID_INLINE void QuatCalcW(quat_t q)
{
#if 1
	vec_t           term = 1.0f - (q[0] * q[0] + q[1] * q[1] + q[2] * q[2]);

	if(term < 0.0)
		q[3] = 0.0;
	else
		q[3] = -sqrt(term);
#else
	q[3] = sqrt(fabs(1.0f - (q[0] * q[0] + q[1] * q[1] + q[2] * q[2])));
#endif
}

static ID_INLINE void QuatInverse(quat_t q)
{
	q[0] = -q[0];
	q[1] = -q[1];
	q[2] = -q[2];
}

static ID_INLINE void QuatAntipodal(quat_t q)
{
	q[0] = -q[0];
	q[1] = -q[1];
	q[2] = -q[2];
	q[3] = -q[3];
}

static ID_INLINE vec_t QuatLength(const quat_t q)
{
	return (vec_t) sqrt(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
}

vec_t           QuatNormalize(quat_t q);

void            QuatFromAngles(quat_t q, vec_t pitch, vec_t yaw, vec_t roll);

static ID_INLINE void AnglesToQuat(const vec3_t angles, quat_t q)
{
	QuatFromAngles(q, angles[PITCH], angles[YAW], angles[ROLL]);
}

void            QuatFromMatrix(quat_t q, const matrix_t m);
void            QuatToVectorsFLU(const quat_t quat, vec3_t forward, vec3_t left, vec3_t up);
void            QuatToVectorsFRU(const quat_t quat, vec3_t forward, vec3_t right, vec3_t up);
void            QuatToAxis(const quat_t q, vec3_t axis[3]);
void            QuatToAngles(const quat_t q, vec3_t angles);

// wombat: pretty straightforward
void			QuatToRotAngle( const quat_t q, vec_t *angle );
void			QuatToRotAngleAxis( const quat_t q, vec_t *angle, vec3_t axis );

void			QuatFromRotAngleAxis( quat_t q, vec_t angle, const vec3_t axis );

// Quaternion multiplication, analogous to the matrix multiplication routines.

// qa = rotate by qa, then qb
void            QuatMultiply0(quat_t qa, const quat_t qb);

// qc = rotate by qa, then qb
void            QuatMultiply1(const quat_t qa, const quat_t qb, quat_t qc);

// qc = rotate by qa, then by inverse of qb
void            QuatMultiply2(const quat_t qa, const quat_t qb, quat_t qc);

// qc = rotate by inverse of qa, then by qb
void            QuatMultiply3(const quat_t qa, const quat_t qb, quat_t qc);

// qc = rotate by inverse of qa, then by inverse of qb
void            QuatMultiply4(const quat_t qa, const quat_t qb, quat_t qc);

void QuaternionMultiply( quat_t output, quat_t first, quat_t second );

void            QuatSlerp(const quat_t from, const quat_t to, float frac, quat_t out);
void            QuatTransformVector(const quat_t q, const vec3_t in, vec3_t out);
int Q_isnan( float x );

//=============================================


typedef struct
{
	qboolean        frameMemory;
	int             currentElements;
	int             maxElements;	// will reallocate and move when exceeded
	void          **elements;
} growList_t;

// you don't need to init the growlist if you don't mind it growing and moving
// the list as it expands
void            Com_InitGrowList(growList_t * list, int maxElements);
void            Com_DestroyGrowList(growList_t * list);
int             Com_AddToGrowList(growList_t * list, void *data);
void           *Com_GrowListElement(const growList_t * list, int index);
int             Com_IndexForGrowListElement(const growList_t * list, const void *element);

unsigned long long rdtsc( void );


//=============================================

float Com_Clamp( float min, float max, float value );

const char	*COM_SkipPath( const char *pathname );
const char	*COM_GetExtension( const char *name );
void	COM_StripExtension(const char *in, char *out, int destsize);
void	COM_DefaultExtension( char *path, int maxSize, const char *extension );

void	COM_BeginParseSession( const char *name );
int		COM_GetCurrentParseLine( void );
const char	*COM_Parse( char **data_p );
char	*COM_ParseExt( char **data_p, qboolean allowLineBreak );
char	*COM_ParseExtIgnoreQuotes( char **data_p, qboolean allowLineBreak );
const char *COM_GetToken( const char **data_p, qboolean crossline );
void	Com_SkipRestOfLine(char **data);
void	Com_SkipBracedSection(char **program);
void	Com_Parse1DMatrix(char **buf_p, int x, float *m, qboolean checkBrackets);
int		COM_Compress( char *data_p );
void	COM_ParseError( char *format, ... ) __attribute__ ((format (printf, 1, 2)));
void	COM_ParseWarning( char *format, ... ) __attribute__ ((format (printf, 1, 2)));
//int		COM_ParseInfos( char *buf, int max, char infos[][MAX_INFO_STRING] );

#define MAX_TOKENLENGTH		1024

#ifndef TT_STRING
//token types
#define TT_STRING					1			// string
#define TT_LITERAL					2			// literal
#define TT_NUMBER					3			// number
#define TT_NAME						4			// name
#define TT_PUNCTUATION				5			// punctuation
#endif

typedef struct pc_token_s
{
	int type;
	int subtype;
	int intvalue;
	float floatvalue;
	char string[MAX_TOKENLENGTH];
} pc_token_t;

// data is an in/out parm, returns a parsed out token

void	COM_MatchToken( char**buf_p, char *match );

qboolean SkipBracedSection (char **program);
qboolean SkipBracedSectionEx (char **program, int depth);
void SkipRestOfLine ( char **data );

void Parse1DMatrix (char **buf_p, int x, float *m);
void Parse2DMatrix (char **buf_p, int y, int x, float *m);
void Parse3DMatrix (char **buf_p, int z, int y, int x, float *m);

int Com_HexStrToInt( const char *str );

void	QDECL Com_sprintf (char *dest, int size, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));

char *Com_SkipTokens( char *s, int numTokens, char *sep );
char *Com_SkipCharset( char *s, char *sep );
void Com_BackslashToSlash( char *str );

void Com_RandomBytes( byte *string, int len );

// mode parm for FS_FOpenFile
typedef enum {
	FS_READ,
	FS_WRITE,
	FS_APPEND,
	FS_APPEND_SYNC
} fsMode_t;

typedef enum {
	FS_SEEK_CUR,
	FS_SEEK_END,
	FS_SEEK_SET
} fsOrigin_t;

//=============================================

int Q_isprint( int c );
int Q_islower( int c );
int Q_isupper( int c );
int Q_isalpha( int c );

// portable case insensitive compare
int		Q_stricmp (const char *s1, const char *s2);
int		Q_strncmp( const char *s1, const char *s2, size_t n );
int		Q_stricmpn (const char *s1, const char *s2, size_t n);
char	*Q_strlwr( char *s1 );
char	*Q_strupr( char *s1 );
char	*Q_strrchr( const char* string, int c );
const char	*Q_stristr( const char *s, const char *find);
qboolean Q_strreplace(char *dest, int destsize, const char *find, const char *replace);

// buffer size safe library replacements
void	Q_strncpyz( char *dest, const char *src, size_t destsize );
void	Q_strcat( char *dest, int size, const char *src );

// strlen that discounts Quake color sequences
int Q_PrintStrlen( const char *string );
// removes color sequences from string
char *Q_CleanStr( char *string );

//=============================================

extern qboolean bigendian;

short	BigShort( short l );
short	LittleShort( short l );
int	BigLong( int l );
int	LittleLong( int l );
float	BigFloat( float l );
float	LittleFloat( float l );
unsigned short	LittleUnsignedShort( unsigned short l );
unsigned short	BigUnsignedShort( unsigned short l );

void	Swap_Init( void );

//=============================================

// 64-bit integers for global rankings interface
// implemented as a struct for qvm compatibility
typedef struct
{
	byte	b0;
	byte	b1;
	byte	b2;
	byte	b3;
	byte	b4;
	byte	b5;
	byte	b6;
	byte	b7;
} qint64;

const char *va( const char *format, ... ) __attribute__ ((format (printf, 1, 2)));

#define TRUNCATE_LENGTH	64
void Com_TruncateLongString( char *buffer, const char *s );

//=============================================

int MusicMood_NameToNum( const char * name );
const char * MusicMood_NumToName( int num );
int EAXMode_NameToNum( const char * name );
const char * EAXMode_NumToName( int num );

//
// key / value info strings
//
char *Info_ValueForKey( const char *s, const char *key );
void Info_RemoveKey( char *s, const char *key );
void Info_RemoveKey_big( char *s, const char *key );
void Info_SetValueForKey( char *s, const char *key, const char *value );
void Info_SetValueForKey_Big( char *s, const char *key, const char *value );
qboolean Info_Validate( const char *s );
void Info_NextPair( const char **s, char *key, char *value );

// this is only here so the functions in q_shared.c and bg_*.c can link
void	QDECL Com_Error( int level, const char *error, ... ) __attribute__ ((format (printf, 2, 3)));
void	QDECL Com_Printf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));
void	QDECL Com_DPrintf( const char *msg, ... ) __attribute__ ((format (printf, 1, 2)));


/*
==========================================================

CVARS (console variables)

Many variables can be used for cheating purposes, so when
cheats is zero, force all unspecified variables to their
default values.
==========================================================
*/

#define	CVAR_ARCHIVE			1			// set to cause it to be saved to vars.rc
											// used for system variables, not for player
											// specific configurations
#define	CVAR_USERINFO			2			// sent to server on connect or change
#define	CVAR_SERVERINFO			4			// sent in response to front end requests
#define	CVAR_SYSTEMINFO			8			// these cvars will be duplicated on all clients
#define	CVAR_INIT				16			// don't allow change from console at all,
											// but can be set from the command line
#define	CVAR_LATCH				32			// will only change when C code next does
											// a Cvar_Get(), so it can't be changed
											// without proper initialization.  modified
											// will be set, even though the value hasn't
											// changed yet
#define	CVAR_ROM				64			// display only, cannot be set by user at all
#define	CVAR_USER_CREATED		128			// created by a set command
#define	CVAR_TEMP				256			// can be set even when cheats are disabled, but is not archived
#define CVAR_CHEAT				512			// can not be changed if cheats are disabled
#define CVAR_NORESTART			1024		// do not clear when a cvar_restart is issued

#define CVAR_SERVER_CREATED		2048		// cvar was created by a server the client connected to.
#define	CVAR_SOUND_LATCH		4096		// specifically for sound will only change 
											// when C code next does a Cvar_Get(), so it 
											// can't be changed without proper initialization.
											// modified will be set, even though the value hasn't
											// changed yet

#define	CVAR_TERRAIN_LATCH		8192		// specifically for terrain will only change

#define	CVAR_SAVEGAME			16384		// cvar will be saved when saving to file

#define CVAR_NONEXISTENT		0xFFFFFFFF	// Cvar doesn't exist.

// nothing outside the Cvar_*() functions should modify these fields!
typedef struct cvar_s {
	char		*name;
	char		*string;
	char		*resetString;		// cvar_restart will reset to this value
	char		*latchedString;		// for CVAR_LATCH vars
	int			flags;
	qboolean	modified;			// set each time the cvar is changed
	int			modificationCount;	// incremented each time the cvar is changed
	float		value;				// atof( string )
	int			integer;			// atoi( string )
	struct cvar_s *next;
	struct cvar_s *hashNext;
} cvar_t;

#define	MAX_CVAR_VALUE_STRING	256

typedef int	cvarHandle_t;

// the modules that run in the virtual machine can't access the cvar_t directly,
// so they must ask for structured updates
typedef struct {
	cvarHandle_t	handle;
	int			modificationCount;
	float		value;
	int			integer;
	char		string[MAX_CVAR_VALUE_STRING];
	char		latchedString[MAX_CVAR_VALUE_STRING];
} vmCvar_t;

/*
==============================================================

COLLISION DETECTION

==============================================================
*/

#include "surfaceflags.h"			// shared with the q3map utility

// plane types are used to speed some tests
// 0-2 are axial planes
#define	PLANE_X			0
#define	PLANE_Y			1
#define	PLANE_Z			2
#define	PLANE_NON_AXIAL	3
#define PLANE_NON_PLANAR 4


/*
=================
PlaneTypeForNormal
=================
*/

//#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )
static ID_INLINE int PlaneTypeForNormal(vec3_t normal)
{
	if(normal[0] == 1.0)
		return PLANE_X;

	if(normal[1] == 1.0)
		return PLANE_Y;

	if(normal[2] == 1.0)
		return PLANE_Z;

	if(normal[0] == 0.0 && normal[1] == 0.0 && normal[2] == 0.0)
		return PLANE_NON_PLANAR;

	return PLANE_NON_AXIAL;
}

// plane_t structure
// !!! if this is changed, it must be changed in asm code too !!!
typedef struct cplane_s {
	vec3_t	normal;
	float	dist;
	byte	type;			// for fast side tests: 0,1,2 = axial, 3 = nonaxial
	byte	signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
	byte	pad[2];
} cplane_t;


// a trace is returned when a box is swept through the world
typedef struct {
	qboolean	allsolid;	// if true, plane is not valid
	qboolean	startsolid;	// if true, the initial point was in a solid area
	float		fraction;	// time completed, 1.0 = didn't hit anything
	vec3_t		endpos;		// final position
	cplane_t	plane;		// surface normal at impact, transformed to world space
	int			surfaceFlags;	// surface hit
	int			shaderNum;
	int			contents;	// contents on other side of surface hit
	int			entityNum;	// entity the contacted surface is a part of

	int			location;
	struct gentity_s *ent;
} trace_t;

// trace->entityNum can also be 0 to (MAX_GENTITIES-1)
// or ENTITYNUM_NONE, ENTITYNUM_WORLD

typedef struct {
	int version;
	int time;
	int mapTime;
	char comment[ 64 ];
	char mapName[ 64 ];
	char saveName[ 64 ];
	char tm_filename[ 64 ];
	int tm_loopcount;
	int tm_offset;
} savegamestruct_t;

// markfragments are returned by CM_MarkFragments()
typedef struct {
	int		firstPoint;
	int		numPoints;
} markFragment_t;

typedef struct treadMark_s {
	int			iReferenceNumber;
	int			iLastTime;
	qhandle_t	hTreadShader;
	int			iState;
	float		fWidth;
	vec3_t		vStartDir;
	vec3_t		vStartVerts[ 2 ];
	float		fStartTexCoord;
	float		fStartAlpha;
	vec3_t		vMidPos;
	vec3_t		vMidVerts[ 2 ];
	float		fMidTexCoord;
	float		fMidAlpha;
	vec3_t		vEndPos;
	vec3_t		vEndVerts[ 2 ];
	float		fEndTexCoord;
	float		fEndAlpha;
} treadMark_t;

typedef struct {
	vec3_t p;
	quat_t q;
} bone_t;

typedef struct {
	vec3_t		origin;
	vec3_t		axis[ 3 ];
} orientation_t;

//=====================================================================


// in order from highest priority to lowest
// if none of the catchers are active, bound key strings will be executed
#define KEYCATCH_CONSOLE		0x0001
#define	KEYCATCH_UI				0x0002
#define	KEYCATCH_MESSAGE		0x0004
#define	KEYCATCH_CGAME			0x0008


// sound channels
// channel 0 never willingly overrides
// other channels will allways override a playing sound on that channel
/*typedef enum {
	CHAN_AUTO,
	CHAN_LOCAL,		// menu sounds, etc
	CHAN_WEAPON,
	CHAN_VOICE,
	CHAN_ITEM,
	CHAN_BODY,
	CHAN_LOCAL_SOUND,	// chat messages, etc
	CHAN_ANNOUNCER		// announcer voices, etc
} soundChannel_t;*/
typedef enum {
	CHAN_AUTO,
	CHAN_BODY,
	CHAN_ITEM,
	CHAN_WEAPONIDLE,
	CHAN_VOICE,
	CHAN_LOCAL,
	CHAN_WEAPON,
	CHAN_DIALOG_SECONDARY,
	CHAN_DIALOG,
	CHAN_MENU,
	CHAN_LOCAL_SOUND,	// chat messages, etc
	CHAN_ANNOUNCER,		// announcer voices, etc
	CHAN_MAX
} soundChannel_t;

#define DEFAULT_MIN_DIST  -1.0
#define DEFAULT_VOL  -1.0

#define LEVEL_WIDE_MIN_DIST  10000	// full volume the entire level
#define LEVEL_WIDE_STRING "levelwide"

#define  SOUND_SYNCH             0x1
#define  SOUND_SYNCH_FADE        0x2
#define  SOUND_RANDOM_PITCH_20   0x4
#define  SOUND_RANDOM_PITCH_40   0x8
#define  SOUND_LOCAL_DIALOG      0x10

typedef enum
{
	mood_none,
	mood_normal,
	mood_action,
	mood_suspense,
	mood_mystery,
	mood_success,
	mood_failure,
	mood_surprise,
	mood_special,
	mood_aux1,
	mood_aux2,
	mood_aux3,
	mood_aux4,
	mood_aux5,
	mood_aux6,
	mood_aux7,
	mood_totalnumber
} music_mood_t;

typedef enum
{
	eax_generic,
	eax_paddedcell,
	eax_room,
	eax_bathroom,
	eax_livingroom,
	eax_stoneroom,
	eax_auditorium,
	eax_concerthall,
	eax_cave,
	eax_arena,
	eax_hangar,
	eax_carpetedhallway,
	eax_hallway,
	eax_stonecorridor,
	eax_alley,
	eax_forest,
	eax_city,
	eax_mountains,
	eax_quarry,
	eax_plain,
	eax_parkinglot,
	eax_sewerpipe,
	eax_underwater,
	eax_drugged,
	eax_dizzy,
	eax_psychotic,
	eax_totalnumber
} eax_mode_t;

/*
========================================================================

  ELEMENTS COMMUNICATED ACROSS THE NET

========================================================================
*/

#define	ANGLE2SHORT(x)	((int)((x)*65536/360) & 65535)
#define	BYTE2ANGLE(x)	((x)*(360.0/255))
#define	SHORT2ANGLE(x)	((x)*(360.0/65536))

#define	SNAPFLAG_RATE_DELAYED	1
#define	SNAPFLAG_NOT_ACTIVE		2	// snapshot used during connection and for zombies
#define SNAPFLAG_SERVERCOUNT	4	// toggled every map_restart so transitions can be detected

//
// per-level limits
//
#define	MAX_CLIENTS			64		// absolute limit
#define MAX_LOCATIONS		64

#define MAX_MAP_BOUNDS          16384
#define MIN_MAP_BOUNDS          ( -MAX_MAP_BOUNDS )
#define MAP_SIZE                ( MAX_MAP_BOUNDS - MIN_MAP_BOUNDS )

#define	GENTITYNUM_BITS		10		// don't need to send any more
#define	MAX_GENTITIES		(1<<GENTITYNUM_BITS)

// entitynums are communicated with GENTITY_BITS, so any reserved
// values that are going to be communcated over the net need to
// also be in this range
#define	ENTITYNUM_NONE		(MAX_GENTITIES-1)
#define	ENTITYNUM_WORLD		(MAX_GENTITIES-2)
#define	ENTITYNUM_MAX_NORMAL	(MAX_GENTITIES-2)

#define MAX_SERVER_SOUNDS			64
#define MAX_SERVER_SOUNDS_BITS		(MAX_SERVER_SOUNDS-1)

#define	MAX_MODELS			1024		// these are sent over the net as 8 bits
#define	MAX_SOUNDS			512		// so they cannot be blindly increased
#define MAX_OBJECTIVES		20
#define MAX_LIGHTSTYLES		32
#define MAX_WEAPONS			48

#define	MAX_CONFIGSTRINGS	2736

// these are the only configstrings that the system reserves, all the
// other ones are strictly for servergame to clientgame communication
#define	CS_SERVERINFO		0		// an info string with all the serverinfo cvars
#define	CS_SYSTEMINFO		1		// an info string for server system to client system configuration (timescale, etc)

#define	RESERVED_CONFIGSTRINGS	2	// game can't modify below this, only the system can

#define	MAX_GAMESTATE_CHARS	32000
typedef struct {
	int			stringOffsets[MAX_CONFIGSTRINGS];
	char		stringData[MAX_GAMESTATE_CHARS];
	int			dataCount;
} gameState_t;

//=========================================================

// maybe this is better put somewhere else...
typedef struct server_sound_s {
	float origin[3];
	int entity_number;
	int channel;
	short int sound_index;
	float volume;
	float min_dist;
	float maxDist;
	float pitch;
	qboolean stop_flag;
	qboolean streamed;
} server_sound_t;

typedef struct usereyes_s {
	signed char ofs[ 3 ];
	float angles[ 2 ];
} usereyes_t;

// bit field limits
#define	MAX_STATS				32
#define	MAX_ACTIVEITEMS			8
#define	MAX_AMMO				16
#define	MAX_AMMO_AMOUNT			16
#define MAX_MAX_AMMO_AMOUNT		16

#define	MAX_PERSISTANT			16
#define	MAX_POWERUPS			16

#define	MAX_PS_EVENTS			2

#define PS_PMOVEFRAMECOUNTBITS	6

// playerState_t is the information needed by both the client and server
// to predict player motion and actions
// nothing outside of pmove should modify these, or some degree of prediction error
// will occur

// you can't add anything to this without modifying the code in msg.c

// playerState_t is a full superset of entityState_t as it is used by players,
// so if a playerState_t is transmitted, the entityState_t can be fully derived
// from it.

typedef struct playerState_s {
	int			commandTime;	// cmd->serverTime of last executed command
	int			pm_type;
	int			bobCycle;		// for view bobbing and footstep generation
	int			pm_flags;		// ducked, jump_held, etc
	int			pm_time;

	vec3_t		origin;
	vec3_t		velocity;

	int			gravity;
	int			speed;
	int			delta_angles[3];	// add to command angles to get view direction
									// changed by spawns, rotating objects, and teleporters

	int			groundEntityNum;	// ENTITYNUM_NONE = in air

	qboolean	walking;
	qboolean	groundPlane;
	int			feetfalling;
	float		falldir[3];
	trace_t		groundTrace;

	int			clientNum;		// ranges from 0 to MAX_CLIENTS-1

	vec3_t		viewangles;		// for fixed views
	int			viewheight;

	float		fLeanAngle;
	int			iViewModelAnim;
	int			iViewModelAnimChanged;

	int			stats[ MAX_STATS ];
	int			activeItems[ 8 ];
	int			ammo_name_index[ 16 ];
	int			ammo_amount[ 16 ];
	int			max_ammo_amount[ 16 ];

	int			current_music_mood;
	int			fallback_music_mood;
	float		music_volume;
	float		music_volume_fade_time;
	int			reverb_type;
	float		reverb_level;
	float		blend[4];
	float		fov;
	float		camera_origin[3];
	float		camera_angles[3];
	float		camera_time;
	float		camera_offset[3];
	float		camera_posofs[3];
	int			camera_flags;
	float		damage_angles[3];

	// not communicated over the net at all
	int			ping;			// server to game info for scoreboard
	float		vEyePos[3];
} playerState_t;


//====================================================================

#define	BUTTON_ATTACKRIGHT_BITINDEX 		0
#define	BUTTON_ATTACKLEFT_BITINDEX  		1
#define	BUTTON_SNEAK_BITINDEX				2
#define	BUTTON_RUN_BITINDEX					3
#define	BUTTON_HOLSTERWEAPON_BITINDEX		4
#define	BUTTON_USE_BITINDEX					5
#define	BUTTON_TALK_BITINDEX				6			// displays talk balloon and disables actions
#define	BUTTON_ANY_BITINDEX					11			// any key whatsoever
#define BUTTON_MOUSE_BITINDEX				14			// mouse move

//
// usercmd_t->button bits, many of which are generated by the client system,
// so they aren't game/cgame only definitions
//
#define	BUTTON_ATTACK		1	// +/-attackprimary
#define	BUTTON_ATTACK2		2	// +/-attacksecondary
#define	BUTTON_RUN			4	// +/-speed
										// walking can't just be infered from MOVE_RUN
										// because a key pressed late in the frame will
										// only generate a small move value for that frame
										// walking will use different animations and
										// won't generate footsteps

#define	BUTTON_USE			8
#define	BUTTON_LEANLEFT		16	// +/-leanleft
#define BUTTON_LEANRIGHT	32	// +/-leanright
#define	BUTTON_NEGATIVE		64

#define BUTTON_GETFLAG		128
#define BUTTON_GUARDBASE	256
#define BUTTON_PATROL		512
#define BUTTON_FOLLOWME		1024

#define BUTTON_HOLSTER		32768

#define	BUTTON_TALK				( 1 << BUTTON_TALK_BITINDEX )			// displays talk balloon and disables actions
#define	BUTTON_ANY				( 1 << BUTTON_ANY_BITINDEX )				// any key whatsoever
#define BUTTON_MOUSE			( 1 << BUTTON_MOUSE_BITINDEX )

#define	MOVE_RUN			120			// if forwardmove or rightmove are >= MOVE_RUN,
										// then BUTTON_WALKING should be set

// usercmd_t is sent to the server each client frame
typedef struct usercmd_s {
	int					serverTime;
	byte				msec;
	short unsigned int	buttons;
	short int			angles[3];
	//int	weapon; // su44: there is no 'weapon' field
	// in MoHAA's usercmd_s - weapon commands are encoded 
	// into buttons bits. See CL_CmdButtons from cl_input.c.
	signed char	forwardmove, rightmove, upmove;
} usercmd_t;

//===================================================================

//
// Animation flags
//
#define  MDL_ANIM_DELTA_DRIVEN   ( 1 << 0 )
#define  MDL_ANIM_DEFAULT_ANGLES ( 1 << 3 )
#define  MDL_ANIM_NO_TIMECHECK   ( 1 << 4 )

// if entityState->solid == SOLID_BMODEL, modelindex is an inline model number
#define	SOLID_BMODEL	0xffffff

// renderfx flags (entityState_t::renderfx)
// su44: moved it here, in MoHAA (and FAKK)
// render flags are set by "renderEffects" event
// and sent to cgame trough entityState_t
// TODO: find out rest of them
#define	RF_THIRD_PERSON			(1<<0)   	// don't draw through eyes, only mirrors (player bodies, chat sprites)
#define	RF_FIRST_PERSON			(1<<1)		// only draw through eyes (view weapon, damage blood blob)
#define	RF_DEPTHHACK			(1<<2)		// hack the z-depth so that view weapons do not clip into walls
#define	RF_VIEWLENSFLARE		(1<<3)		// View dependent lensflare
#define	RF_FRAMELERP			(1<<4)		// interpolate between current and next state
#define	RF_BEAM					(1<<5)		// draw a beam between origin and origin2
#define RF_DONTDRAW				(1<<7)		// don't draw this entity but send it over
#define RF_LENSFLARE			(1<<8)		// add a lens flare to this
#define RF_EXTRALIGHT			(1<<9)		// use good lighting on this entity
#define RF_DETAIL				(1<<10)		// Culls a model based on the distance away from you
#define RF_SHADOW				(1<<11)		// whether or not to draw a shadow
#define	RF_PORTALSURFACE		(1<<12)   	// don't draw, but use to set portal views
#define	RF_SKYORIGIN   			(1<<13)   	// don't draw, but use to set sky portal origin and coordinate system
#define	RF_SKYENTITY   			(1<<14)   	// this entity is only visible through a skyportal
#define	RF_LIGHTOFFSET   		(1<<15)   	// this entity has a light offset
#define	RF_CUSTOMSHADERPASS		(1<<16)		// draw the custom shader on top of the base geometry
#define	RF_MINLIGHT				(1<<17)		// allways have some light (viewmodel, some items)
#define	RF_FULLBRIGHT			(1<<18)		// allways have full lighting
#define RF_LIGHTING_ORIGIN		(1<<19)		// use refEntity->lightingOrigin instead of refEntity->origin
// for lighting.  This allows entities to sink into the floor
// with their origin going solid, and allows all parts of a
// player to get the same lighting
#define	RF_SHADOW_PLANE			(1<<20)		// use refEntity->shadowPlane
#define	RF_WRAP_FRAMES			(1<<21)		// mod the model frames by the maxframes to allow continuous
// animation without needing to know the frame count
//#define RF_PORTALENTITY			(1<<22)		// this entity should only be drawn from a portal
//#define RF_DUALENTITY			(1<<23)		// this entity is drawn both in the portal and outside it.
#define RF_ADDITIVE_DLIGHT		(1<<22)		// this entity has an additive dynamic light
#define RF_LIGHTSTYLE_DLIGHT	(1<<23)		// this entity has a dynamic light that uses a light style
#define RF_SHADOW_PRECISE		(1<<24)		// this entity can have a precise shadow applied to it
#define RF_INVISIBLE			(1<<25)		// This entity is invisible, and only negative lights will light it up
#define RF_VIEWMODEL			(1<<26)		// This entity is invisible, and only negative lights will light it up
#define RF_PRECISESHADOW		(1<<28)		// This entity is invisible, and only negative lights will light it up
#define RF_ALWAYSDRAW			(1<<30)
//
// use this mask when propagating renderfx from one entity to another
//
#define  RF_FLAGS_NOT_INHERITED ( RF_LENSFLARE | RF_VIEWLENSFLARE | RF_BEAM | RF_EXTRALIGHT | RF_SKYORIGIN | RF_SHADOW | RF_SHADOW_PRECISE | RF_SHADOW_PLANE )


//#define RF_ADDICTIVEDYNAMICLIGHT ?????

#define RF_FORCENOLOD		1024

#define BEAM_LIGHTNING_EFFECT   (1<<0)
#define BEAM_USEMODEL           (1<<1)
#define BEAM_PERSIST_EFFECT     (1<<2)
#define BEAM_SPHERE_EFFECT      (1<<3)
#define BEAM_RANDOM_DELAY       (1<<4)
#define BEAM_TOGGLE             (1<<5)
#define BEAM_RANDOM_TOGGLEDELAY (1<<6)
#define BEAM_WAVE_EFFECT        (1<<7)
#define BEAM_USE_NOISE          (1<<8)
#define BEAM_PARENT             (1<<9)
#define BEAM_TILESHADER         (1<<10)
#define BEAM_OFFSET_ENDPOINTS   (1<<11)

typedef enum {
	TR_STATIONARY,
	TR_INTERPOLATE,				// non-parametric, but interpolate between snapshots
	TR_LINEAR,
	TR_LINEAR_STOP,
	TR_SINE,					// value = base + sin( time / duration ) * delta
	TR_GRAVITY,
	TR_LERP
} trType_t;

typedef struct {
	int trTime;
	vec3_t trDelta;
} trajectory_t;

typedef struct frameInfo_s {
	int index;
	float time;
	float weight;
} frameInfo_t;

#define NUM_BONE_CONTROLLERS 5

#define NUM_MORPH_CONTROLLERS 10

#define MAX_MODEL_SURFACES   32 // this needs to be the same in qfiles.h for TIKI_MAX_SURFACES

#define  MDL_SURFACE_SKINOFFSET_BIT0  ( 1 << 0 )
#define  MDL_SURFACE_SKINOFFSET_BIT1  ( 1 << 1 )
#define  MDL_SURFACE_NODRAW           ( 1 << 2 )
#define  MDL_SURFACE_SURFACETYPE_BIT0 ( 1 << 3 )
#define  MDL_SURFACE_SURFACETYPE_BIT1 ( 1 << 4 )
#define  MDL_SURFACE_SURFACETYPE_BIT2 ( 1 << 5 )
#define  MDL_SURFACE_CROSSFADE_SKINS  ( 1 << 6 )
#define  MDL_SURFACE_SKIN_NO_DAMAGE   ( 1 << 7 )

#define CROUCH_HEIGHT		36
#define CROUCH_EYE_HEIGHT	30
#define STAND_HEIGHT			72
#define STAND_EYE_HEIGHT	66

#define MAX_FRAMEINFOS			16
#define FRAMEINFO_BLEND			( MAX_FRAMEINFOS >> 1 )

// entityState_t is the information conveyed from the server
// in an update message about entities that the client will
// need to render in some way
// Different eTypes may use the information in different ways
// The messages are delta compressed, so it doesn't really matter if
// the structure size is fairly large

typedef struct entityState_s {
	int		number;			// entity index
	int		eType;			// entityType_t
	int		eFlags;

	trajectory_t pos;

	vec3_t	netorigin;
	vec3_t	origin;
	vec3_t	origin2;

	vec3_t	netangles;
	vec3_t	angles;

	int		constantLight;	// r + (g<<8) + (b<<16) + (intensity<<24)
	int		loopSound;		// constantly loop this sound
	float	loopSoundVolume;
	float	loopSoundMinDist;
	float	loopSoundMaxDist;
	float	loopSoundPitch;
	int		loopSoundFlags;

	int		parent;
	int		tag_num;

	qboolean	attach_use_angles;
	vec3_t		attach_offset;

	int		beam_entnum;

	int		modelindex;
	int		usageIndex;
	int		skinNum;
	int		wasframe;
	frameInfo_t frameInfo[ MAX_FRAMEINFOS ];
	float	actionWeight;

	int		bone_tag[ NUM_BONE_CONTROLLERS ];
	vec3_t	bone_angles[ NUM_BONE_CONTROLLERS ];
	quat_t	bone_quat[ NUM_BONE_CONTROLLERS ]; // not sent over
	byte	surfaces[ 32 ];

	int		clientNum;			// 0 to (MAX_CLIENTS - 1), for players and corpses
	int		groundEntityNum;	// -1 = in air

	int		solid;				// for client side prediction, trap_linkentity sets this properly

	float	scale;
	float	alpha;
	int		renderfx;
	float	shader_data[ 2 ];
	float	shader_time;
	quat_t	quat;
	vec3_t	eyeVector;
} entityState_t;

typedef enum {
	CA_UNINITIALIZED,
	CA_DISCONNECTED, 	// not talking to a server
	CA_AUTHORIZING,		// not used any more, was checking cd key
	CA_CONNECTING,		// sending request packets to the server
	CA_CHALLENGING,		// sending challenge packets to the server
	CA_CONNECTED,		// netchan_t established, getting gamestate
	CA_LOADING,			// only during cgame initialization, never during main loop
	CA_PRIMED,			// got gamestate, waiting for first frame
	CA_ACTIVE,			// game views should be displayed
	CA_CINEMATIC		// playing a cinematic or a static pic, not connected to a server
} connstate_t;

typedef struct qtime_s {
	int		tm_sec;
	int		tm_min;
	int		tm_hour;
	int		tm_mday;
	int		tm_mon;
	int		tm_year;
	int		tm_wday;
	int		tm_yday;
	int		tm_isdst;
} qtime_t;

typedef struct {
	float			start[ 3 ];
	float			end[ 3 ];
	float			color[ 3 ];
	float			alpha;
	float			width;
	unsigned short	factor;
	unsigned short	pattern;
} debugline_t;

typedef struct {
	char	szText[ 64 ];
	float	pos[ 3 ];
	float	scale;
	float	color[ 4 ];
} debugstring_t;

// font support

typedef struct {
	float pos[ 2 ];
	float size[ 2 ];
} letterloc_t;

typedef struct {
	int				indirection[ 256 ];
	letterloc_t		locations[ 256 ];
	char			name[ 64 ];
	float			height;
	float			aspectRatio;
	void			*shader;
	int				trhandle;
} fontheader_t;

/*typedef struct hdelement_s {
	qhandle_t		hShader;
	char			shaderName[ 64 ];

	int				iX;
	int				iY;
	int				iWidth;
	int				iHeight;

	float			vColor[ 4 ];

	int				iHorizontalAlign;
	int				iVerticalAlign;
	qboolean		bVirtualScreen;

	char			string[ 2048 ];
	char			fontName[ 64 ];

	fontheader_t	*pFont;
} hdelement_t;*/

/*typedef struct {
	frameInfo_t	g_VMFrameInfo[ 16 ];

	int			g_iLastVMAnim;
	int			g_iLastVMAnimChanged;
	int			g_iCurrentVMAnimSlot;
	int			g_iCurrentVMDuration;

	qboolean	g_bCrossblending;

	int			g_iLastEquippedWeaponStat;
	char		g_szLastActiveItem[ 80 ];
	int			g_iLastAnimPrefixIndex;

	float		g_vCurrentVMPosOffset[ 3 ];
} clientAnim_t;*/

#define GLYPH_START 0
#define GLYPH_END 255
#define GLYPH_CHARSTART 32
#define GLYPH_CHAREND 127
#define GLYPHS_PER_FONT GLYPH_END - GLYPH_START + 1
typedef struct {
  int height;       // number of scan lines
  int top;          // top of glyph in buffer
  int bottom;       // bottom of glyph in buffer
  int pitch;        // width for copying
  int xSkip;        // x adjustment
  int imageWidth;   // width of actual image
  int imageHeight;  // height of actual image
  float s;          // x offset in image where glyph starts
  float t;          // y offset in image where glyph starts
  float s2;
  float t2;
  qhandle_t glyph;  // handle to the shader with the glyph
  char shaderName[32];
} glyphInfo_t;

typedef struct {
  glyphInfo_t glyphs [GLYPHS_PER_FONT];
  float glyphScale;
  char name[MAX_QPATH];
} fontInfo_t;

#define Square(x) ((x)*(x))

// real time
//=============================================

// server browser sources
// TTimo: AS_MPLAYER is no longer used
#define AS_LOCAL			0
#define AS_MPLAYER			99
#define AS_GLOBAL			2
#define AS_FAVORITES		3
#define AS_GAMESPY			1	// wombat: right now we use AS_GLOBAL for GS, too


// cinematic states
typedef enum {
	FMV_IDLE,
	FMV_PLAY,		// play
	FMV_EOF,		// all other conditions, i.e. stop/EOF/abort
	FMV_ID_BLT,
	FMV_ID_IDLE,
	FMV_LOOPED,
	FMV_ID_WAIT
} e_status;

typedef enum _flag_status {
	FLAG_ATBASE = 0,
	FLAG_TAKEN,			// CTF
	FLAG_TAKEN_RED,		// One Flag CTF
	FLAG_TAKEN_BLUE,	// One Flag CTF
	FLAG_DROPPED
} flagStatus_t;

#define	MAX_GLOBAL_SERVERS				2048
#define	MAX_OTHER_SERVERS					128
#define MAX_PINGREQUESTS					32
#define MAX_SERVERSTATUSREQUESTS	16

#define SAY_ALL		0
#define SAY_TEAM	1
#define SAY_TELL	2

#define CDKEY_LEN 16
#define CDCHKSUM_LEN 2

#define FLOAT_TO_INT( x, fracbits ) ( ( x ) * ( 1 << ( fracbits ) ) )

#define FLOAT_TO_PKT( x, dest, wholebits, fracbits )                            \
   {                                                                            \
   if ( ( x ) >= ( 1 << ( wholebits ) ) )                                       \
         {                                                                         \
      ( dest ) = FLOAT_TO_INT( ( 1 << ( wholebits ) ), ( fracbits ) ) - 1; \
         }                                                                         \
      else if ( ( x ) < 0 )                                                        \
      {                                                                         \
      ( dest ) = 0;                                                             \
      }                                                                         \
      else                                                                         \
      {                                                                         \
      ( dest ) = FLOAT_TO_INT( ( x ), ( fracbits ) );               \
      }                                                                         \
   }

#define SIGNED_FLOAT_TO_PKT( x, dest, wholebits, fracbits )                   \
   {                                                                          \
   float temp_x;                                                              \
   temp_x = ( x ) + ( 1 << ( wholebits ) );                                   \
   if ( temp_x >= ( 1 << ( ( wholebits ) + 1 ) ) )                            \
      ( dest ) = FLOAT_TO_INT( ( 1 << ( ( wholebits ) + 1 ) ), ( fracbits ) ) - 1;    \
      else if ( temp_x < 0 )                                                     \
      (dest) = 0;                                                             \
      else                                                                       \
      ( dest ) = FLOAT_TO_INT( temp_x, ( fracbits ) );                        \
   }

#define INT_TO_FLOAT( x, wholebits, fracbits ) ( ( float )( ( ( float )( x ) ) / ( float )( 1 << ( fracbits ) ) - ( float )( 1 << ( wholebits ) ) ) )
#define UINT_TO_FLOAT( x, fracbits ) ( ( float )( ( ( float )( x ) ) / ( float )( 1 << ( fracbits ) ) ) )

#define TRANSLATION_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 4, 11 )
#define PKT_TO_TRANSLATION( x ) UINT_TO_FLOAT( ( x ), 11 )

#define OFFSET_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 1, 14 )
#define PKT_TO_OFFSET( x ) UINT_TO_FLOAT( ( x ), 14 )

#define ROTATE_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 9, 6 )
#define PKT_TO_ROTATE( x ) UINT_TO_FLOAT( ( x ), 6 )

#define BASE_TO_PKT( x, dest ) SIGNED_FLOAT_TO_PKT( ( x ), ( dest ), 3, 4 )
#define PKT_TO_BASE( x ) INT_TO_FLOAT( ( x ), 3, 4 )

#define AMPLITUDE_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 4, 4 )
#define PKT_TO_AMPLITUDE( x ) UINT_TO_FLOAT( ( x ), 4 )

#define PHASE_TO_PKT( x, dest ) SIGNED_FLOAT_TO_PKT( ( x ), ( dest ), 3, 4 )
#define PKT_TO_PHASE( x ) INT_TO_FLOAT( ( x ), 3, 4 )

#define FREQUENCY_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 4, 4 )
#define PKT_TO_FREQUENCY( x ) UINT_TO_FLOAT( ( x ), 4 )

#define BEAM_PARM_TO_PKT( x, dest ) FLOAT_TO_PKT( ( x ), ( dest ), 4, 4 )
#define PKT_TO_BEAM_PARM( x ) UINT_TO_FLOAT( ( x ), 4 )

#define STUB() printf( "STUB: function %s in %s line %d.\n", __FUNCTION__, __FILE__, __LINE__ )
#define STUB_DESC( description ) printf( "STUB: %s in %s line %d.\n", ##description, __FILE__, __LINE__ )
#define UNIMPLEMENTED() Com_Printf( "FIXME: (%s) is unimplemented (file %s:%d)\n", __FUNCTION__, __FILE__, __LINE__ )

#if defined(__cplusplus)
}
#endif

#endif	// __Q_SHARED_H
