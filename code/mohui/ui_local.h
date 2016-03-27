/*
===========================================================================
Copyright (C) 2010-2011 wombat

This file is part of OpenMohaa source code.

OpenMohaa source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

OpenMohaa source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
#ifndef __UI_LOCAL2_H__
#define __UI_LOCAL2_H__

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "../renderercommon/tr_types.h"
#include "../uilib/ui_public.h"
#include "../client/keycodes.h"
#include "../game/bg_public.h"
#include "../qcommon/tiki.h"


#define UI_MAX_URCSIZE		131072
#define UI_MAX_MENUS		16
#define UI_MAX_NAME			128
#define UI_MAX_RESOURCES	64
#define UI_MAX_SELECT		32
#define UI_MAX_FILES		256

#define UI_CHECKBOX_SIZE	16

typedef void (*voidfunc_f)(void);
//typedef struct tiki_s tiki_t;

typedef enum uiBorderstyle_s {
	UI_BORDER_NONE,
	UI_BORDER_RAISED,
	UI_BORDER_INDENT,
	UI_BORDER_3D
} uiBorderstyle_t;

typedef enum uiMenuDirection_s {
	UI_NONE,
	UI_FROM_TOP,
	UI_FROM_BOTTOM,
	UI_FROM_LEFT,
	UI_FROM_RIGHT
} uiMenuDirection_t;

typedef enum uiAlign_s {
	UI_ALIGN_NONE,
	UI_ALIGN_LEFT,
	UI_ALIGN_CENTER,
	UI_ALIGN_RIGHT
} uiAlign_t;

typedef enum uiResType_s {
	UI_RES_LABEL,
	UI_RES_BUTTON,
	UI_RES_BINDBUTTON,
	UI_RES_CHECKBOX,
	UI_RES_FIELD,
	UI_RES_LIST,
	UI_RES_LISTBOX,
	UI_RES_SLIDER,
	UI_RES_PULLDOWN,
	UI_RES_SERVERLIST,
	UI_RES_LANSERVERLIST,
	UI_RES_BINDLIST,
} uiResType_t;

typedef enum uiSlidertype_s {
	UI_ST_FLOAT
} uiSlidertype_t;

typedef enum {
	STATBAR_NONE,
	STATBAR_HORIZONTAL,
	STATBAR_VERTICAL,
	STATBAR_VERTICAL_REVERSE,
	STATBAR_VERTICAL_STAGGER_EVEN,
	STATBAR_VERTICAL_STAGGER_ODD,
	STATBAR_CIRCULAR,
	STATBAR_NEEDLE,
	STATBAR_ROTATOR,
	STATBAR_COMPASS,
	STATBAR_SPINNER,
	STATBAR_HEADING_SPINNER
} uiStatbar_t;

typedef struct uiResource_s {
	uiResType_t			type;
	char				name[UI_MAX_NAME];
//	uiMenuDirection_t	align[2];
	int					rect[4];
	vec4_t				fgcolor;
	vec4_t				bgcolor;
	uiBorderstyle_t		borderstyle;
	qhandle_t			shader;
	qhandle_t			hoverShader;
	qboolean			hoverDraw;
	qhandle_t			tileShader;

	qhandle_t			checked_shader;
	qhandle_t			unchecked_shader;
	char				*clicksound;
	char				*stuffcommand;
	char				*hovercommand;
	cvar_t				*enabledcvar;
	qboolean			enablewithcvar;
	cvar_t				*linkcvar;
	char				*linkcvarname;
	qboolean			linkcvartoshader;
	char				*linkstring1[UI_MAX_SELECT];
	char				*linkstring2[UI_MAX_SELECT];

	char				*title;
	uiAlign_t			align;
	qhandle_t			menushader;
	qhandle_t			selmenushader;
	int					selentries;

	uiSlidertype_t		slidertype;
	float				flRange[2];
	float				flStepsize;

	qboolean			rendermodel;
	vec3_t				modeloffset;
	vec3_t				modelrotateoffset;
	vec3_t				modelangles;
	float				modelscale;
	char				*modelanim;
// storing extra memory for each resource is too memory-expensive.
// letting this point to uis.menuFont for now
	fontInfo_t			*font;	
	int					ordernumber;

	qboolean			active;
	qboolean			lastState;
	qboolean			pressed;

	qboolean			statvar;
	uiStatbar_t			statbar;
	statIndex_t			playerstat;
	statIndex_t			maxplayerstat;
	statIndex_t			itemstat;
	qhandle_t			statbarshader;
	qhandle_t			statbartileshader;
	qhandle_t			statbarshaderflash;
	float				statbarRange[2];
	int					statbarendangles[2];
	float				rotatorsize[2];
	int					invmodelhand;
	float				fadein;

	// serverlist
	int					hoverListEntry;
	int					selectedListEntry;
} uiResource_t;

typedef struct uiMenu_s {
	char			name[UI_MAX_NAME];
	int				size[2];
	int				offset[2];
	uiAlign_t		align[2];
	uiMenuDirection_t	motion;
	int				time;
	vec4_t			fgcolor;
	vec4_t			bgcolor;
	uiBorderstyle_t	borderstyle;
	vec4_t			bgfill;
	qboolean		fullscreen;
	int				vidmode;
	float			fadein;
	char			include[UI_MAX_NAME];
	char			postinclude[UI_MAX_NAME];
	qboolean		virtualres;

	uiResource_t	resources[UI_MAX_RESOURCES];
	int				resPtr;

	qboolean		standard;
} uiMenu_t;

typedef enum {
	HUD_AMMO,
	HUD_COMPASS,
	HUD_FRAGLIMIT,
	HUD_HEALTH,
	HUD_ITEMS,
	HUD_SCORE,
	HUD_STOPWATCH,
	HUD_TIMELIMIT,
	HUD_WEAPONS,
	HUD_MAX
} uiHudMenus_t; 

typedef struct uiMenulist_s {
	char *name;
	char *file;

	struct uiMenulist_s *next;
} uiMenulist_t;

// ui_quarks.c
typedef struct {
	int					frametime;
	int					realtime;
	int					cursorx;
	int					cursory;

	glconfig_t			glconfig;
	qboolean			debug;
	qhandle_t			whiteShader;
	qhandle_t			blackShader;
	qhandle_t			charset;

	qhandle_t			cursor;
	float				xscale;
	float				yscale;
	float				bias;
	qboolean			firstdraw;

	uiMenuCommand_t		activemenu;

	// wombat
	fontInfo_t			menuFont;

	sfxHandle_t			main_theme;
	sfxHandle_t			menu_apply;
	sfxHandle_t			menu_back;
	sfxHandle_t			menu_select;

	uiMenu_t			*stack[UI_MAX_MENUS];
	int					MSP; // 'Menu Stack Pointer'

	// menus that are always used
	uiMenu_t			main; // main menu
	uiMenu_t			connecting; // connecting screen, used by UI_DrawConnectScreen
	uiMenu_t			loading;
	// for other windows, we use the cache.
	uiMenu_t			cache[UI_MAX_MENUS];
	int					CP; // 'Cache Pointer'
	
	// HUD
	uiMenu_t			hudMenus[HUD_MAX];
	uiMenu_t			crosshair;
	uiMenu_t			scoreboard;
	qboolean			showscores;

	// menu list
	uiMenulist_t		menuList[UI_MAX_FILES];
	int					MLP;	// menu list pointer

	// su44
	char				currentViewModelWeaponName[MAX_QPATH]; // eg BAR
	char				currentViewModelWeaponURC[MAX_QPATH]; // eg hud_ammo_BAR
	gametype_t			gameType;
} uiStatic_t;


// ui_main.c
void *UI_Alloc( int size );
void UI_Free( void *p );
void UI_InitMemory( void );
qboolean UI_OutOfMemory( void );
// ui_quarks.c
//
extern uiStatic_t uis;

void UI_DrawHUD( playerState_t *ps );
void UI_Init( void );
void UI_Shutdown( void );
void UI_KeyEvent( int key, int down );
void UI_MouseEvent( int dx, int dy );
void UI_Refresh( int realtime );
qboolean UI_IsFullscreen( void );
void UI_SetActiveMenu( uiMenuCommand_t menu );
qboolean UI_ConsoleCommand( int realTime );
void UI_DrawConnectScreen( qboolean overlay );

void UI_SetColor( const float *rgba );
void UI_UpdateScreen( void );
void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader );
void UI_RotatedPic( float x, float y, float w, float h, qhandle_t hShader, float angle );
void UI_CmdExecute( const char *text );
void UI_DrawBox( int x, int y, int w, int h, qboolean ctrCoord, int refx, int refy );
void UI_AdjustFrom640( float *x, float *y, float *w, float *h );

// ui_urc.c
void	UI_LoadINC( const char *name, uiMenu_t *menu, qboolean post );
void	UI_LoadURC( const char *name, uiMenu_t *menu );
void	UI_PushMenu( const char *name );
void	UI_PopMenu( void );
void	UI_FindMenus( void );

// ui_model.c
void	UI_RenderModel(uiResource_t *res);

#endif
