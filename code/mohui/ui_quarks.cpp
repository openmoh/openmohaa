/*
===========================================================================
Copyright (C) 2010-2012 Michael Rieder

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
along with OpenMohaa source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

// ui_quarks.c -- basic functionality

#include "ui_local.h"

uiStatic_t		uis;
playerState_t	*playerState;

vec4_t menu_text_color	    = {1.0f, 1.0f, 1.0f, 1.0f};
vec4_t menu_dim_color       = {0.0f, 0.0f, 0.0f, 0.75f};
vec4_t color_black	    = {0.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_white	    = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t color_yellow	    = {1.00f, 1.00f, 0.00f, 1.00f};
vec4_t color_blue	    = {0.00f, 0.00f, 1.00f, 1.00f};
vec4_t color_lightOrange    = {1.00f, 0.68f, 0.00f, 1.00f };
vec4_t color_orange	    = {1.00f, 0.43f, 0.00f, 1.00f};
vec4_t color_red	    = {1.00f, 0.00f, 0.00f, 1.00f};
vec4_t color_dim	    = {0.00f, 0.00f, 0.00f, 0.25f};
vec4_t color_gray	    = {0.00f, 0.00f, 0.00f, 0.6f};
vec4_t color_green	    = {0.10f, 0.70f, 0.10f, 1.00f};
vec4_t color_dkgreen	= {0.40f, 0.40f, 0.40f, 1.00f};

// current color scheme
vec4_t pulse_color          = {1.00f, 1.00f, 1.00f, 1.00f};
vec4_t text_color_disabled  = {0.50f, 0.50f, 0.50f, 1.00f};	// light gray
//vec4_t text_color_normal    = {1.00f, 0.43f, 0.00f, 1.00f};	// light orange
vec4_t text_color_normal    = {1.00f, 1.00f, 1.00f, 1.00f};		//white
vec4_t text_color_highlight = {1.00f, 1.00f, 0.00f, 1.00f};	// bright yellow
vec4_t listbar_color        = {1.00f, 0.43f, 0.00f, 0.30f};	// transluscent orange
vec4_t text_color_status    = {1.00f, 1.00f, 1.00f, 1.00f};	// bright white

static const char *gtstrings[] = {
	"Single Player",	// single player ffa
	"Free For All",				// free for all
	"Team Deathmatch",			// team deathmatch
	"Round-based Team",
	"Objective Match"
};

/*
=================
UI_ClampCvar
=================
*/
float UI_ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

void UI_ClearHashTable( void );

/*
=================
UI_Shutdown
=================
*/
void UI_Shutdown( void )
{
	UI_ClearHashTable();
	memset( &uis, 0, sizeof( uiStatic_t ) );
}


void UI_RegisterMedia( void ) {
	// FONTS
	uii.R_RegisterFont( "facfont-20", 0, &uis.menuFont );

	// SHADERS
	uis.whiteShader = uii.Rend_RegisterMaterial( "*white" );
	uis.blackShader = uii.Rend_RegisterMaterial( "textures/mohmenu/black.tga" );
	uis.cursor = uii.Rend_RegisterMaterial( "gfx/2d/mouse_cursor.tga" );

	// SOUNDS
	//if (ui_playmusic->integer)
	uis.main_theme	= uii.S_RegisterSound( "sound/music/mus_MainTheme_01d.mp3", qfalse, qfalse );
	uis.menu_apply	= uii.S_RegisterSound( "sound/menu/apply.wav", qfalse, qfalse );
	uis.menu_back	= uii.S_RegisterSound( "sound/menu/Back.wav", qfalse, qfalse );
	uis.menu_select	= uii.S_RegisterSound( "sound/menu/Scroll.wav", qfalse, qfalse );

	// STANDARD MENUS
	UI_LoadURC( "main", &uis.main );
	UI_LoadURC( "connecting", &uis.connecting );
	//UI_LoadURC( "loading_default", &uis.loading ); //preloading not necessary
	uis.main.standard			= qtrue;
	uis.connecting.standard		= qtrue;
	uis.loading.standard		= qtrue;
	
	// HUD MENUS
	UI_LoadURC( "hud_ammo_bar", &uis.hudMenus[HUD_AMMO] );
	UI_LoadURC( "hud_compass", &uis.hudMenus[HUD_COMPASS] );
	uii.Cvar_Set( "ui_compass_size", va( "%d", uis.hudMenus[HUD_COMPASS].size[0] ) );
	UI_LoadURC( "hud_fraglimit", &uis.hudMenus[HUD_FRAGLIMIT] );
	UI_LoadURC( "hud_health", &uis.hudMenus[HUD_HEALTH] );
	UI_LoadURC( "hud_items", &uis.hudMenus[HUD_ITEMS] );
	UI_LoadURC( "hud_score", &uis.hudMenus[HUD_SCORE] );
	UI_LoadURC( "hud_stopwatch", &uis.hudMenus[HUD_STOPWATCH] );
	UI_LoadURC( "hud_timelimit", &uis.hudMenus[HUD_TIMELIMIT] );
	UI_LoadURC( "hud_weapons", &uis.hudMenus[HUD_WEAPONS] );
	
	UI_LoadURC( "crosshair", &uis.crosshair );
	// by default load deatmatch scoreboard. We might need to change it for "objective" game type.
	UI_LoadURC( "dm_scoreboard", &uis.scoreboard );
	uis.gameType = GT_TEAM;
}

/*
=================
UI_Init
=================
*/
void UI_Init( void ) {

	UI_InitMemory();

//	UI_InitGameinfo();

	// cache redundant calulations
	uii.GetGlconfig( &uis.glconfig );

	// for 640x480 virtualized screen
	uis.xscale = uis.glconfig.vidWidth * (1.0/640.0);
	uis.yscale = uis.glconfig.vidHeight * (1.0/480.0);
	if ( uis.glconfig.vidWidth * 480 > uis.glconfig.vidHeight * 640 ) {
		// wide screen
		uis.bias = 0.5 * ( uis.glconfig.vidWidth - ( uis.glconfig.vidHeight * (640.0/480.0) ) );
		uis.xscale = uis.yscale;
	}
	else {
		// no wide screen
		uis.bias = 0;
	}

	// initialize the menu system
	UI_FindMenus();
	UI_RegisterMedia();

	uis.MSP	= -1;
	uis.CP	= -1;
	uis.MLP	= 0;
}

/*
=================
UI_KeyEvent
=================
*/
void UI_KeyEvent( int key, int down ) {
	int				i;
	uiMenu_t		*menu;
	uiResource_t	*res;
	char			buffer[64];

	if (uis.MSP == -1) {
		return;
	}

	if (!down) {
		menu = uis.stack[uis.MSP];

		for (i=0;i<=menu->resPtr;i++) {
			res = &menu->resources[i];
			if (res->pressed == qtrue) {
				res->pressed = qfalse;
				if ( uis.cursorx >= res->rect[0]+res->rect[2] && uis.cursorx <= res->rect[0]+res->rect[2]+128 )
					if ( uis.cursory >= res->rect[1] && uis.cursory <= res->rect[1]+ 16*res->selentries && res->linkstring2[(uis.cursory-res->rect[1])/16] )
						UI_CmdExecute( res->linkstring2[(uis.cursory-res->rect[1])/16] );
			}
		}
		return;
	}

	// menu system keys
	switch ( key )
	{
		case K_MOUSE2:
		case K_ESCAPE:
			//uii.S_StartLocalSound( uis.menu_back, CHAN_LOCAL_SOUND );
			UI_PopMenu();
			break;
		case K_MOUSE1:
		case K_ENTER:
			menu = uis.stack[uis.MSP];
			
			for (i=0;i<=menu->resPtr;i++) {
				res = &menu->resources[i];
				if (res->active ==qfalse)
					continue;
				switch (res->type) {
					case UI_RES_BUTTON:
						//uii.S_StartLocalSound( uis.menu_apply, CHAN_LOCAL_SOUND );
						if ( res->stuffcommand )
							UI_CmdExecute( res->stuffcommand );
						break;
					case UI_RES_CHECKBOX:
						if (res->linkcvar->integer)
							uii.Cvar_Set( res->linkcvarname, "0" );
						else
							uii.Cvar_Set( res->linkcvarname, "1" );
						break;
					case UI_RES_PULLDOWN:
						res->pressed = qtrue;
						break;
					case UI_RES_SERVERLIST:
					case UI_RES_LANSERVERLIST:
						res->selectedListEntry = res->hoverListEntry;
						break;
				}
			}
			break;
		case K_BACKSPACE:
			menu = uis.stack[uis.MSP];
			
			for (i=0;i<=menu->resPtr;i++) {
				res = &menu->resources[i];
				if (res->active ==qfalse)
					continue;
				if (res->type == UI_RES_FIELD) {
					int len;
					strcpy( buffer, res->linkcvar->string );
					len = strlen(res->linkcvar->string);
					if(len > 0) {
						buffer[len-1] = 0;
						uii.Cvar_Set( res->linkcvarname, buffer );
					}
				}
			}
			break;
		default:
			if ( key&K_CHAR_FLAG )
				break;
			menu = uis.stack[uis.MSP];

			for (i=0;i<=menu->resPtr;i++) {
				res = &menu->resources[i];
				if (res->active ==qfalse)
					continue;
				if( res->type == UI_RES_FIELD ) {
					strcpy( buffer, res->linkcvar->string );
					if ( (key >= 'a'&&key <= 'z') || (key >= '0'&&key <= '9')) {
						int len = strnlen(buffer,sizeof(buffer));
						// su44: we dont want buffer overflow errors...
						if(len != sizeof(buffer) - 1) {
							buffer[len] = key;
							buffer[len+1] = 0;
						} else {
							// su44: someone could change it to DPrintf, but right now there is no DPrintf in UI
							Com_Printf("UI_KeyEvent: maximum buffer size (%i) reached for editfield '%s' of menu '%s' (buffer text: '%s')\n",
								sizeof(buffer),res->name,menu->name,buffer);
						}
					}
					uii.Cvar_Set( res->linkcvarname, buffer );
				}
			}
			break;
	}
}

/*
=================
UI_MouseEvent
=================
*/
void UI_MouseEvent( int dx, int dy )
{
	int				i;
	uiMenu_t		*menu;
	uiResource_t	*res;

	if (uis.MSP == -1) {
		return;
	}
	
	// update mouse screen position
	uis.cursorx += dx;
	if (uis.cursorx < 0)
		uis.cursorx = 0;
	else if (uis.cursorx > SCREEN_WIDTH)
		uis.cursorx = SCREEN_WIDTH;

	uis.cursory += dy;
	if (uis.cursory < 0)
		uis.cursory = 0;
	else if (uis.cursory > SCREEN_HEIGHT)
		uis.cursory = SCREEN_HEIGHT;

	// region test the active menu items
	menu = uis.stack[uis.MSP];

	for (i=0; i<=menu->resPtr;i++) {
		res = &menu->resources[i];
		if (res->type != UI_RES_BUTTON
			&& res->type != UI_RES_CHECKBOX
			&& res->type != UI_RES_FIELD
			&& res->type != UI_RES_PULLDOWN
			&& res->type != UI_RES_SERVERLIST
			&& res->type != UI_RES_LANSERVERLIST)
			continue;
		if (	uis.cursorx >= res->rect[0]
				&& uis.cursorx <= res->rect[2]+res->rect[0]
				&& uis.cursory >= res->rect[1]
				&& uis.cursory <= res->rect[3]+res->rect[1] ) {

					res->active = qtrue;
					if ( res->lastState == qfalse && res->hovercommand )
						UI_CmdExecute( res->hovercommand );
					res->lastState = qtrue;
				}
		else {
			res->active = qfalse;
			res->lastState = qfalse;
		}
	}
}

/*
========================
UI_DrawDebugString
========================
*/
void UI_DrawDebugString(fontInfo_t *font, float x, float y, const char *s) {
	int len;
	int end;

	len = uii.R_Text_Width(font,s,0,qfalse);

	// ensure that string will be drawn entirely on screen
	end = len + x;
	end += 8; // some extra offset
	if(end > 640) {
		x += 640 - end;
	}
	uii.R_Text_Paint(font,x,y,1,0,s,0,0,qfalse,qtrue);
}
/*
========================
UI_ShowInfo

su44: an utility tool for debuging menus
========================
*/
void UI_ShowInfo() {
	uiMenu_t *menu;
	int i;
	uiResource_t *res;
	const char *s;
	int x, y;

	// region test the active menu items
	menu = uis.stack[uis.MSP];

	for (i=0; i<=menu->resPtr;i++) {
		res = &menu->resources[i];
		if (res->type != UI_RES_BUTTON
			&& res->type != UI_RES_CHECKBOX
			&& res->type != UI_RES_FIELD
			&& res->type != UI_RES_PULLDOWN
			&& res->type != UI_RES_LANSERVERLIST)
			continue;
		if (uis.cursorx >= res->rect[0]
			&& uis.cursorx <= res->rect[2]+res->rect[0]
			&& uis.cursory >= res->rect[1]
			&& uis.cursory <= res->rect[3]+res->rect[1] ) {
			// su44: print some informations about this menu
			x = uis.cursorx;
			y = uis.cursory;
			y += 28;
			if((y+16) >= 480) {
				y -= 64; // dont draw outside screen
			}
			uii.R_SetColor( 0 );
			s = va("Menu \"%s\", resource \"%s\"",menu->name,res->name);
			UI_DrawDebugString(res->font,x,y,s);
			y += 18;
			s = va("Shader \"%s\"",uii.R_GetShaderName(res->shader));
			UI_DrawDebugString(res->font,x,y,s);	
			return; // show info only about one menu at time
		}
	}
}

/*
========================
UI_DrawMenu

Logic to bring the menu resources to screen
========================
*/
void UI_DrawMenu( uiMenu_t *menu, qboolean foreground ) {
	int				j;
	int				k;
//	char			text[256];
	int				frac;

	uiResource_t	*res;
	qhandle_t		cvarshader;

	for (j=0; j<=menu->resPtr; j++ ) {
		res = &menu->resources[j];
		// su44: dont show "Back To Game" / "Disconnect" button 
		// in main menu if we're not connected to a server
		if(!strcmp(res->name,"backtogame") || !strcmp(res->name,"disconnect") ) {
			if( !com_cl_running->integer ) {
				continue;
			}
		}
		if( !strcmp(res->name,"continuebutton")|| !strcmp(res->name,"loadingflasher"))
			continue;
		if ( foreground == qtrue ) //foreground menu
			UI_SetColor( res->fgcolor );
		else UI_SetColor( res->bgcolor );

		switch ( res->type ) {
			int i;
			int source;

			case UI_RES_LABEL:
				if ( res->enablewithcvar == qtrue )
					if (!res->enabledcvar->integer)
						break;
				if ( res->rendermodel == qtrue ) {
					UI_RenderModel(res);
				}
				else if ( res->title ) {
					if ( res->linkcvarname && res->rendermodel == qfalse ) {
						if ( res->selentries > 0 ) {
							for (k=0;k < res->selentries;k++) {
								if ( res->linkcvar->latchedString && !Q_strncmp( res->linkstring1[k], res->linkcvar->latchedString, UI_MAX_NAME ))
									uii.R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,res->linkstring2[k],1,0,qtrue,qtrue);
							}
						}
						else
							uii.R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,res->linkcvar->string,1,0,qtrue,qtrue);
					} else if ( res->statvar==qfalse ) {
						if (res->borderstyle != UI_BORDER_NONE )
							UI_DrawBox( res->rect[0],res->rect[1], res->rect[2],res->rect[3],qfalse, menu->size[0], menu->size[1] );
						switch ( res->align ) {
							case UI_ALIGN_LEFT:
							uii.R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,res->title,1,0,qtrue,qtrue);
							break;
							case UI_ALIGN_NONE:
							case UI_ALIGN_CENTER:
							uii.R_Text_Paint( res->font,res->rect[0]+ (res->rect[2] - uii.R_Text_Width( res->font, res->title, 0,qtrue ))/2,res->rect[1]+ (res->rect[3] -uii.R_Text_Height( res->font, res->title, 0,qtrue ))/2,1,0,res->title,1,0,qtrue,qtrue);
							break;
							case UI_ALIGN_RIGHT:
							uii.R_Text_Paint( res->font,res->rect[0]+ res->rect[2] - uii.R_Text_Width( res->font, res->title, 0,qtrue ),res->rect[1],1,0,res->title,1,0,qtrue,qtrue);
							break;
						}
					}
				}
				else if ( res->linkcvartoshader ) {
					cvarshader=uii.Rend_RegisterMaterial(res->linkcvar->string);
					if ( cvarshader )
						UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], cvarshader );
				}
				else if ( res->statbar != STATBAR_NONE ) {
					switch ( res->statbar ) {
						case STATBAR_COMPASS:
							UI_SetColor( res->fgcolor );
							//UI_RotatedPic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->statbarshader, playerState->viewangles[YAW]/360 );
							break;
						case STATBAR_HORIZONTAL:
							UI_SetColor( res->fgcolor );
							if ( res->statbartileshader && (res->statbarRange[1]-res->statbarRange[0])!=0) {
								UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2]*(res->linkcvar->value-res->statbarRange[0])/(res->statbarRange[1]-res->statbarRange[0]), res->rect[3], res->statbartileshader );
							}
							break;
						case STATBAR_VERTICAL:
							UI_SetColor( res->bgcolor );
							if ( res->statbarshader && playerState->stats[res->maxplayerstat]!=0) {
								frac = res->rect[3] * playerState->stats[res->playerstat] / playerState->stats[res->maxplayerstat];
								UI_DrawHandlePic( res->rect[0], res->rect[1]+res->rect[3] - frac, res->rect[2], frac, res->statbarshader );
							} else if( res->statbartileshader ) {
								float picH, baseOfs;
								int c, i;

								// avoid dividing by zero.... playerState->stats[res->maxplayerstat] is 0
								// if player is using stationary MG42, available on mohdm6 (Stalingrad)
								if(playerState->stats[res->maxplayerstat] != 0) {
									picH = res->rect[3] / playerState->stats[res->maxplayerstat];
									c = playerState->stats[res->playerstat];
									baseOfs = res->rect[1] + res->rect[3] - picH;
									for(i = 0; i < c; i++) {
										float ofs = baseOfs - i * picH;
										UI_DrawHandlePic( res->rect[0], ofs, res->rect[2], picH, res->statbartileshader );
									}
								}
							}
							break;
						// su44: stagger statbars for bullet icons (see "ui/hud_ammo_***.urc")
						case STATBAR_VERTICAL_STAGGER_ODD:
							UI_SetColor( res->bgcolor );
							if ( playerState->stats[res->maxplayerstat]!=0) {
								float picH, baseOfs;
								int c, i;

								picH = res->rect[3] / playerState->stats[res->maxplayerstat];
								c = playerState->stats[res->playerstat];
								baseOfs = res->rect[1] + res->rect[3]- picH;
								for(i = 1; i < c; i+=2) {
									float ofs = baseOfs - i * picH;
									UI_DrawHandlePic( res->rect[0], ofs, res->rect[2], picH*2, res->statbartileshader );
								}
							}
							break;
						case STATBAR_VERTICAL_STAGGER_EVEN:
							UI_SetColor( res->bgcolor );
							if ( playerState->stats[res->maxplayerstat]!=0) {
								float picH, baseOfs;
								int c, i;
								
								picH = res->rect[3] / playerState->stats[res->maxplayerstat];
								c = playerState->stats[res->playerstat];
								baseOfs = res->rect[1] + res->rect[3] - picH*2;
								for(i = 0; i < c; i+=2) {
									float ofs = baseOfs - i * picH;
									UI_DrawHandlePic( res->rect[0], ofs, res->rect[2], picH*2, res->statbartileshader );
								}
							}
							break;	
					}
				}
				else if ( res->statvar == qtrue ) {
					if ( !res->itemstat ) {
						uii.R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,va("%i",playerState->stats[res->playerstat]),1,0,qtrue,qtrue);
					} else {
						// su44: see ui/hud_ammo_bar.tik.
						if(res->itemstat == 1) {
							switch ( res->align ) {
								case UI_ALIGN_LEFT:
								uii.R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,uis.currentViewModelWeaponName,1,0,qtrue,qtrue);
								break;
								case UI_ALIGN_NONE:
								case UI_ALIGN_CENTER:
								uii.R_Text_Paint( res->font,res->rect[0]+ (res->rect[2] - uii.R_Text_Width( res->font, uis.currentViewModelWeaponName, 0,qtrue ))/2,res->rect[1]+ (res->rect[3] -uii.R_Text_Height( res->font, uis.currentViewModelWeaponName, 0,qtrue ))/2,1,0,uis.currentViewModelWeaponName,1,0,qtrue,qtrue);
								break;
								case UI_ALIGN_RIGHT:
								uii.R_Text_Paint( res->font,res->rect[0]+ res->rect[2] - uii.R_Text_Width( res->font, uis.currentViewModelWeaponName, 0,qtrue ),res->rect[1],1,0,uis.currentViewModelWeaponName,1,0,qtrue,qtrue);
								break;
							}
						}
					}
				}
				else if ( res->linkcvarname && res->rendermodel == qfalse ) {
						switch ( res->align ) {
							case UI_ALIGN_LEFT:
							uii.R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,res->linkcvar->string,1,0,qtrue,qtrue);
							break;
							case UI_ALIGN_NONE:
							case UI_ALIGN_CENTER:
							uii.R_Text_Paint( res->font,res->rect[0]+ (res->rect[2]-uii.R_Text_Width(res->font,res->linkcvar->string,64,qfalse))/2,res->rect[1],1,0,res->linkcvar->string,1,0,qtrue,qtrue);
							break;
							case UI_ALIGN_RIGHT:
								uii.R_Text_Paint( res->font,res->rect[0]+res->rect[2]-uii.R_Text_Width(res->font,res->linkcvar->string,64,qfalse),res->rect[1],1,0,res->linkcvar->string,1,0,qtrue,qtrue);
							break;
						}
					}
				else {
					if ( res->shader )
						UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->shader );
					else
						UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], uis.blackShader );
				}
				break;

			case UI_RES_BUTTON:
				if (res->hoverDraw && res->active)
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->hoverShader );
				else if ( res->shader )
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->shader );
				break;
			case UI_RES_FIELD:
				if ( res->active )
					UI_SetColor(color_yellow);
				uii.R_Text_Paint( res->font,res->rect[0],res->rect[1],1,0,res->linkcvar->string,1,0,qtrue,qtrue);
				UI_SetColor( NULL );
				break;
			case UI_RES_CHECKBOX:
				if (res->linkcvar->integer)
					UI_DrawHandlePic( res->rect[0], res->rect[1], UI_CHECKBOX_SIZE, UI_CHECKBOX_SIZE, res->checked_shader );
				else UI_DrawHandlePic( res->rect[0], res->rect[1], UI_CHECKBOX_SIZE, UI_CHECKBOX_SIZE, res->unchecked_shader );
				break;
			case UI_RES_PULLDOWN:
				if (res->pressed == qtrue) {
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->selmenushader );
					UI_DrawBox( res->rect[0]+res->rect[2], res->rect[1], 128, 16 * res->selentries +3, qfalse, menu->size[0], menu->size[1] );

					// draw selected with diff color
					if ( uis.cursorx >= res->rect[0]+res->rect[2] && uis.cursorx <= res->rect[0]+res->rect[2]+128 ) {
						if ( uis.cursory >= res->rect[1] && uis.cursory <= res->rect[1]+ 16*res->selentries ) {
							for (k=0;k<res->selentries;k++) {
								if (k == (uis.cursory-res->rect[1])/16) {
									UI_SetColor( colorYellow );
									uii.R_Text_Paint( res->font, res->rect[0]+res->rect[2]+3,3+res->rect[1]+(k*16),1,1,res->linkstring1[k],0,0,qtrue,qtrue );
									UI_SetColor( NULL );
								}
								else
									uii.R_Text_Paint( res->font, res->rect[0]+res->rect[2]+3,3+res->rect[1]+(k*16),1,1,res->linkstring1[k],0,0,qtrue,qtrue );
							}
						}
					}
					else
						for (k=0;k<res->selentries;k++)
							uii.R_Text_Paint( res->font, res->rect[0]+res->rect[2]+3,3+res->rect[1]+(k*16),1,1,res->linkstring1[k],0,0,qtrue,qtrue );
				}
				else {
					UI_DrawHandlePic( res->rect[0], res->rect[1], res->rect[2], res->rect[3], res->menushader );
				}
				break;
			case UI_RES_SERVERLIST:
				source = AS_GLOBAL;
			case UI_RES_LANSERVERLIST:
				if ( res->type == UI_RES_LANSERVERLIST )
					source = AS_LOCAL;
				UI_DrawBox( res->rect[0], res->rect[1], res->rect[2], res->rect[3], qfalse, menu->size[0], menu->size[1] );
				UI_SetColor( colorGreen );
				uii.R_Text_Paint( res->font, res->rect[0],res->rect[1],1,1,"Server Name",0,0,qfalse,qtrue );
				uii.R_Text_Paint( res->font, res->rect[0]+128,res->rect[1],1,1,"Map",0,0,qfalse,qtrue );
				uii.R_Text_Paint( res->font, res->rect[0]+256,res->rect[1],1,1,"Players",0,0,qfalse,qtrue );
				uii.R_Text_Paint( res->font, res->rect[0]+384,res->rect[1],1,1,"GameType",0,0,qfalse,qtrue );
				uii.R_Text_Paint( res->font, res->rect[0]+512,res->rect[1],1,1,"Ping",0,0,qfalse,qtrue );
				UI_SetColor( colorYellow );

				for (i=0;i<uii.LAN_GetServerCount( source );i++) {
					char buf[512];
					int ping;

					uii.LAN_MarkServerVisible( source,i,qtrue );
					uii.LAN_UpdateVisiblePings(source);
					ping = uii.LAN_GetServerPing( source,i  );
					uii.LAN_GetServerInfo(source,i,buf,sizeof(buf));

					if ( uis.cursorx >= res->rect[0] && uis.cursorx <= res->rect[0]+res->rect[2]
						&& uis.cursory >= res->rect[1]+16 && uis.cursory <= res->rect[1]+16+ 16*uii.LAN_GetServerCount(source) ) {
							if (i+1 == (uis.cursory-res->rect[1])/14) {
								res->hoverListEntry = i;
								UI_SetColor( NULL );
								uii.R_Text_Paint( res->font, res->rect[0],res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"hostname"),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+128,res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"mapname"),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+256,res->rect[1]+14+i*14,1,1,va("%s / %s",Info_ValueForKey(buf,"clients"),Info_ValueForKey(buf,"sv_maxclients")),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+384,res->rect[1]+14+i*14,1,1,gtstrings[atoi(Info_ValueForKey(buf,"gametype"))],0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+512,res->rect[1]+14+i*14,1,1,va("%i",ping),0,0,qfalse,qtrue );
								UI_SetColor( colorYellow );
							}
							else if (i==res->selectedListEntry) {
								UI_SetColor( color_orange );
								uii.R_Text_Paint( res->font, res->rect[0],res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"hostname"),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+128,res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"mapname"),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+256,res->rect[1]+14+i*14,1,1,va("%s / %s",Info_ValueForKey(buf,"clients"),Info_ValueForKey(buf,"sv_maxclients")),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+384,res->rect[1]+14+i*14,1,1,gtstrings[atoi(Info_ValueForKey(buf,"gametype"))],0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+512,res->rect[1]+14+i*14,1,1,va("%i",ping),0,0,qfalse,qtrue );
								UI_SetColor( colorYellow );
							}
							else {
								uii.R_Text_Paint( res->font, res->rect[0],res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"hostname"),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+128,res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"mapname"),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+256,res->rect[1]+14+i*14,1,1,va("%s / %s",Info_ValueForKey(buf,"clients"),Info_ValueForKey(buf,"sv_maxclients")),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+384,res->rect[1]+14+i*14,1,1,gtstrings[atoi(Info_ValueForKey(buf,"gametype"))],0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+512,res->rect[1]+14+i*14,1,1,va("%i",ping),0,0,qfalse,qtrue );
							}
					}
					else if (i==res->selectedListEntry) {
								UI_SetColor( color_orange );
								uii.R_Text_Paint( res->font, res->rect[0],res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"hostname"),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+128,res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"mapname"),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+256,res->rect[1]+14+i*14,1,1,va("%s / %s",Info_ValueForKey(buf,"clients"),Info_ValueForKey(buf,"sv_maxclients")),0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+384,res->rect[1]+14+i*14,1,1,gtstrings[atoi(Info_ValueForKey(buf,"gametype"))],0,0,qfalse,qtrue );
								uii.R_Text_Paint( res->font, res->rect[0]+512,res->rect[1]+14+i*14,1,1,va("%i",ping),0,0,qfalse,qtrue );
								UI_SetColor( colorYellow );
							}
					else {
						res->hoverListEntry = -1;
						uii.R_Text_Paint( res->font, res->rect[0],res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"hostname"),0,0,qfalse,qtrue );
						uii.R_Text_Paint( res->font, res->rect[0]+128,res->rect[1]+14+i*14,1,1,Info_ValueForKey(buf,"mapname"),0,0,qfalse,qtrue );
						uii.R_Text_Paint( res->font, res->rect[0]+256,res->rect[1]+14+i*14,1,1,va("%s / %s",Info_ValueForKey(buf,"clients"),Info_ValueForKey(buf,"sv_maxclients")),0,0,qfalse,qtrue );
						uii.R_Text_Paint( res->font, res->rect[0]+384,res->rect[1]+14+i*14,1,1,gtstrings[atoi(Info_ValueForKey(buf,"gametype"))],0,0,qfalse,qtrue );
						uii.R_Text_Paint( res->font, res->rect[0]+512,res->rect[1]+14+i*14,1,1,va("%i",ping),0,0,qfalse,qtrue );
					}
				}
				UI_SetColor( NULL );
				break;
		}
	}
}

/*
=================
UI_DrawHUD
=================
*/
void UI_DrawHUD( playerState_t *ps ) {
	int gameType;
	char path[MAX_QPATH];
	cvar_t *g_gametype;
//for (i=0;i<STAT_LAST_STAT;i++){
//	Com_Printf( "%i val: %i\n", i, stats[i] );
//}
	playerState = ps;

	uii.GetConfigString(CS_WEAPONS+ps->activeItems[ITEM_WEAPON],uis.currentViewModelWeaponName,sizeof(uis.currentViewModelWeaponName));

	//Com_Printf("Current ui weapon: %s\n",uis.currentViewModelWeaponName);
	sprintf(path,"hud_ammo_%s",uis.currentViewModelWeaponName);

	// check if player weapon has changed
	if(Q_stricmp(uis.currentViewModelWeaponURC,path)) {
		strcpy(uis.currentViewModelWeaponURC,path);
		UI_LoadURC(path,&uis.hudMenus[HUD_AMMO]);
	}
	
	// su44: check if server gameType has changed
	//gameType = uii.Cvar_VariableIntegerValue("g_gameType");
	g_gametype = uii.Cvar_Get( "g_gametype", "0", 0 );
	gameType = g_gametype->integer;

	if(gameType != uis.gameType) {
		uis.gameType = ( gametype_t )gameType;
		if(gameType == GT_OBJECTIVE || gameType == GT_TEAM_ROUNDS) {
			UI_LoadURC( "obj_scoreboard", &uis.scoreboard );
		} else {
			UI_LoadURC( "dm_scoreboard", &uis.scoreboard );
		}
	}

	UI_DrawMenu( &uis.hudMenus[HUD_AMMO], qtrue );
	UI_DrawMenu( &uis.hudMenus[HUD_COMPASS], qfalse );
//	UI_DrawMenu( &uis.hudMenus[HUD_FRAGLIMIT], qtrue );
	UI_DrawMenu( &uis.hudMenus[HUD_HEALTH], qtrue );
//	UI_DrawMenu( &uis.hudMenus[HUD_ITEMS], qtrue );
	UI_DrawMenu( &uis.hudMenus[HUD_SCORE], qtrue );
//	UI_DrawMenu( &uis.hudMenus[HUD_STOPWATCH], qtrue );
//	UI_DrawMenu( &uis.hudMenus[HUD_TIMELIMIT], qfalse );
//	UI_DrawMenu( &uis.hudMenus[HUD_WEAPONS], qtrue );

	if ( !playerState->stats[STAT_INZOOM] )
		UI_DrawMenu( &uis.crosshair, qtrue );

//	if ( ui_showscores->integer == 1 ) {
	//	UI_DrawMenu( &uis.scoreboard, qtrue );
//	}
}
/*
=================
UI_Refresh
=================
*/
void UI_Refresh( int realtime )
{
	static qboolean	first=qtrue;
	int				i;
//	int				j;
//	int				k;
//	uiMenu_t		*menu;
//	uiResource_t	*res;
//	qhandle_t		cvarshader;

	uis.frametime = realtime - uis.realtime;
	uis.realtime  = realtime;

	if ( !( uii.Key_GetCatcher() & KEYCATCH_UI ) ) {
		return;
	}

	for (i=0;i<uis.MSP;i++) {
		UI_DrawMenu( uis.stack[i], qfalse );
	}
	// last one is foreground menu
	UI_DrawMenu( uis.stack[uis.MSP], qtrue );

	// draw cursor
	UI_SetColor( NULL );
	UI_DrawHandlePic( uis.cursorx, uis.cursory, 32, 32, uis.cursor);

	// su44: draw info about menu which is under cursor
	//if(ui_showInfo->integer) {
	//	UI_ShowInfo();
	//}

	if ( first == qtrue ) {
		// play main theme
//		uii.S_StartLocalSound( uis.main_theme, CHAN_LOCAL_SOUND );
		first=qfalse;
	}
}

qboolean UI_IsFullscreen( void ) {
	uiMenu_t *menu;

	if ( uis.MSP!= -1 && ( uii.Key_GetCatcher() & KEYCATCH_UI ) ) {
		menu = uis.stack[uis.MSP];
		
		return menu->fullscreen;
	}

	return qfalse;
}

void UI_SetActiveMenu( uiMenuCommand_t menu ) {
	uis.activemenu = menu;

	switch ( menu ) {
		case UIMENU_NONE:
			break;
		case UIMENU_MAIN:
			if ( uis.MSP == -1 ) {
				UI_PushMenu( "main " );
				//uii.S_StartLocalSound( uis.main_theme, CHAN_LOCAL_SOUND );
			}
			else uii.Key_SetCatcher( KEYCATCH_UI );
			break;
		case UIMENU_INGAME:
			UI_PushMenu( "dm_main" );
			break;
	}
}

char *UI_Argv( int arg ) {
	return uii.Argv( arg );
}

int	UI_Argc( void ) {

	return uii.Argc( );
}

/*
=================
UI_WidgetCommand
=================
*/
void UI_WidgetCommand( const char *widget, const char *command ) {
	uiMenu_t		*menu;
	uiResource_t	*res;
	int i;

	if (uis.MSP == -1) {
		return;
	}
	menu = uis.stack[uis.MSP];

	for (i=0; i<=menu->resPtr; i++ ) {
		res = &menu->resources[i];

		if(!strcmp(res->name,widget) ) {
			switch ( res->type ) {
				char buf[64];

				case UI_RES_LANSERVERLIST:
				if(!strcmp(command,"joingame")&&res->selectedListEntry !=-1 ) {
					uii.LAN_GetServerAddressString(AS_LOCAL,res->selectedListEntry,buf,sizeof(buf));
					uii.Cmd_ExecuteText( EXEC_APPEND, va("connect %s\n", buf) );
				}
				else
					uii.Sys_Printf( "UI_WidgetCommand: command %s not found for widget %s.\n", command, widget );
				break;
				case UI_RES_SERVERLIST:
				if(!strcmp(command,"refreshserverlist")) {
					uii.Cmd_ExecuteText( EXEC_APPEND, "refreshserverlist\n" );
				}
				else if(!strcmp(command,"connect")) {
					uii.LAN_GetServerAddressString(AS_GLOBAL,res->selectedListEntry,buf,sizeof(buf));
					uii.Cmd_ExecuteText( EXEC_APPEND, va("connect %s\n", buf) );
				}
				else if(!strcmp(command,"cancelrefresh") ) {
					uii.Sys_Printf( "cancelrefresh\n" );
				}
				else if(!strcmp(command,"updateserver") ) {
					uii.Sys_Printf( "updateserver\n" );
				}
				else
					uii.Sys_Printf( "UI_WidgetCommand: command %s not found for widget %s.\n", command, widget );
				break;
			}
			return;
		}
	}
	uii.Sys_Printf( "UI_WidgetCommand: widget %s not found.\n", widget );
}

/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	char	*cmd;
	char	arg1[64];
	char	arg2[64];

	cmd = UI_Argv( 0 );


	if ( Q_stricmp (cmd, "ui_connect") == 0 ) {
		uii.Cmd_ExecuteText( EXEC_APPEND, "connect 63.210.148.39:12203\n" ); // STONER SERVER
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "ui_test") == 0 ) {
		Com_Printf( "openmohaa menu up and running!\n uis taking %i KB\n", sizeof(uiStatic_t)/1024 );
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "pushmenu") == 0 ) {
		if ( UI_Argc() == 2 )
			UI_PushMenu( UI_Argv(1) );
		else Com_Printf( "Usage: pushmenu <menuname>\n" );
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "popmenu") == 0 ) {
		UI_PopMenu();
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "pushmenu_teamselect") == 0 ) {
		UI_PushMenu( "dm_teamselect" );
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "pushmenu_weaponselect") == 0 ) {
		UI_PushMenu( "dm_primaryselect" );
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "ui_checkrestart") == 0 ) {
		uii.Cmd_ExecuteText( EXEC_APPEND, "vid_restart" );
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "ui_resetcvars") == 0 ) {
		// STUB
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "widgetcommand") == 0 ) {
		if ( UI_Argc() == 3 ) {
			Q_strncpyz( arg1, UI_Argv(1), sizeof(arg1) );
			Q_strncpyz( arg2, UI_Argv(2), sizeof(arg2) );
			UI_WidgetCommand( arg1, arg2 );
		}
		else Com_Printf( "Usage: widgetcommand <widgetname> <command>\n" );
		return qtrue;
	}
	else if ( Q_stricmp (cmd, "ui_startdmmap") == 0 ) {
		char mapname[64];
		char maxClients[8];
		char dedicated[8];

		strcpy( mapname, uii.Cvar_Get( "ui_dmmap", "", 0 )->string );
		strcpy( dedicated, uii.Cvar_Get( "ui_dedicated", "", 0 )->string );
		strcpy( maxClients, uii.Cvar_Get( "ui_maxclients", "", 0 )->string );

		Com_Printf("ui_startdmmap: map %s, maxClients %s, dedicated %s\n",mapname,maxClients,dedicated);

		if(atoi(maxClients) > MAX_CLIENTS)
			sprintf(maxClients,"%i",MAX_CLIENTS);
		
		uii.Cmd_ExecuteText(EXEC_APPEND,va("set sv_maxclients %s; set dedicated %s; wait; map %s",
			maxClients,dedicated,mapname));

		return qtrue;
	}
	else if ( Q_stricmp (cmd, "menuconnect") == 0 ) {
		char ip[32];

		strcpy( ip, uii.Cvar_Get( "ui_connectip", "", 0 )->string );
		
		uii.Cmd_ExecuteText(EXEC_APPEND,va("connect %s",ip));

		return qtrue;		
	}

	return qfalse;
}

/*
========================
UI_DrawConnectScreen

This will also be overlaid on the cgame info screen during loading
to prevent it from blinking away too rapidly on local or lan games.
========================
*/
void UI_DrawConnectScreen( qboolean overlay ) {
	uiClientState_t		cstate;
	char					info[MAX_INFO_VALUE];
	const char				*s;
	char					*loadname;
	static char			oldload[MAX_QPATH];

	
	uii.GetClientState( &cstate );


	info[0] = '\0';
	if( uii.GetConfigString( CS_SERVERINFO, info, sizeof(info) ) ) {
		loadname = Info_ValueForKey(info,"mapname");
	} else loadname = "loading_default";
	if ( Q_strncmp(oldload,loadname,sizeof(oldload)) ) {
		memset( &uis.loading, 0, sizeof(uis.loading) );
		UI_LoadURC( loadname, &uis.loading );
		Q_strncpyz( oldload, loadname, sizeof(oldload) );
	}

	switch ( cstate.connState ) {
		case CA_CONNECTING:
			UI_DrawMenu( &uis.connecting, qtrue );
			s = va("Awaiting challenge...%i", cstate.connectPacketCount);
			break;
		case CA_CHALLENGING:
			UI_DrawMenu( &uis.connecting, qtrue );
			s = va("Awaiting connection...%i", cstate.connectPacketCount);
			break;
		case CA_CONNECTED: //{
	//		char downloadName[MAX_INFO_VALUE];
//
	//			uii.Cvar_VariableStringBuffer( "cl_downloadName", downloadName, sizeof(downloadName) );
		//		if (*downloadName) {
//					UI_DisplayDownloadInfo( downloadName );
//					return;
//				}
//			}
			UI_DrawMenu( &uis.connecting, qtrue );
			s = "Awaiting gamestate...";
			break;
		case CA_LOADING:
			UI_DrawMenu( &uis.loading, qtrue );
			return;
		case CA_PRIMED:
			return;
		default:
			return;
	}
	if ( cstate.connState < CA_CONNECTED ) {
		uii.R_Text_Paint( &uis.menuFont, 320,240,1,1,cstate.messageString,0,0,qtrue,qtrue );
	}
	uii.R_Text_Paint( &uis.menuFont, 250,270,1,1,s,0,0,qtrue,qtrue );
	// draw cursor
	UI_SetColor( NULL );
	UI_DrawHandlePic( uis.cursorx, uis.cursory, 32, 32, uis.cursor);
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
	*x = *x * uis.xscale + uis.bias;
	*y *= uis.yscale;
	*w *= uis.xscale;
	*h *= uis.yscale;
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	UI_AdjustFrom640( &x, &y, &w, &h );
	uii.R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

void UI_RotatedPic( float x, float y, float w, float h, qhandle_t hShader, float angle ) {
	float	s0;
	float	s1;
	float	t0;
	float	t1;

	if( w < 0 ) {	// flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	}
	else {
		s0 = 0;
		s1 = 1;
	}

	if( h < 0 ) {	// flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	}
	else {
		t0 = 0;
		t1 = 1;
	}
	
	UI_AdjustFrom640( &x, &y, &w, &h );
	uii.R_RotatedPic( x, y, w, h, s0, t0, s1, t1, hShader, angle );
}

void UI_SetColor( const float *rgba ) {
	uii.R_SetColor( rgba );
}

void UI_CmdExecute( const char *text ){
	uii.Cmd_ExecuteText( EXEC_APPEND, text );
}

// Wombat
/*
=================
UI_DrawBox
ctrCoord == qtrue: draw box with center at x,y
ctrCoord == qfalse: box with upper left corner at x,y
=================
*/
#define LINE_THICKNESS	1
void	UI_DrawBox( int x, int y, int w, int h, qboolean ctrCoord, int refx, int refy ) {

	if ( ctrCoord ) {
		x -= w/2;
		y -= h/2;
	}

	uii.R_SetColor( color_gray );
	uii.R_DrawStretchPic( x*uis.xscale, y*uis.yscale, w*uis.xscale, h*uis.yscale, 0, 0, 16, 16, uis.blackShader );
	uii.R_SetColor( color_white );
	uii.R_DrawStretchPic( x*uis.xscale, y*uis.yscale, w*uis.xscale, LINE_THICKNESS*uis.yscale, 0, 0, 32, 32, uis.blackShader );
	uii.R_DrawStretchPic( x*uis.xscale, y*uis.yscale, LINE_THICKNESS*uis.xscale, h*uis.yscale, 0, 0, 32, 32, uis.blackShader );
	uii.R_DrawStretchPic( x*uis.xscale, (y+h)*uis.yscale, w*uis.xscale, LINE_THICKNESS*uis.yscale, 0, 0, 32, 32, uis.blackShader );
	uii.R_DrawStretchPic( (x+w)*uis.xscale, y*uis.yscale, LINE_THICKNESS*uis.xscale, (h+LINE_THICKNESS)*uis.yscale, 0, 0, 32, 32, uis.blackShader );
	uii.R_SetColor( NULL );
}

