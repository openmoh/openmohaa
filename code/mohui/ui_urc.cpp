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

#include <glb_local.h>
#include "ui_local.h"

#define PARSE_PTR	COM_Parse(ptr)

#define FILE_HASH_SIZE		64
uiMenulist_t*				hashTable[ FILE_HASH_SIZE ];

static long generateHashValue( const char *fname ) {
	int		i;
	long	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (fname[i] != '\0') {
		letter = tolower(fname[i]);
		hash+=(long)(letter)*(i+119);
		i++;
	}
	hash &= (FILE_HASH_SIZE-1);
	return hash;
}

void UI_ClearHashTable( void )
{
	memset( hashTable, 0, sizeof( hashTable ) );
}

void UI_ParseMenuResource( const char *token, char **ptr, uiResource_t *res, int *offset ) {
	const char	*var;
	int			i;
	
	if ( !Q_strncmp( token, "name", 4 ) ) {
		Q_strncpyz( res->name, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "rect", 4 ) ) {
		res->rect[0] = offset[0] + atoi( PARSE_PTR );
		res->rect[1] = offset[1] + atoi( PARSE_PTR );

		for (i=2;i<4;i++)
			res->rect[i] = atoi( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "fgcolor", 7 ) ) {
		for (i=0;i<4;i++)
			res->fgcolor[i] = atof( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "bgcolor", 7 ) ) {
		for (i=0;i<4;i++)
			res->bgcolor[i] = atof( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "borderstyle", 11 ) ) {
		var = PARSE_PTR;
		if ( !Q_strncmp(var,"RAISED",6) )
			res->borderstyle = UI_BORDER_RAISED;
		else if ( !Q_strncmp(var,"INDENT_BORDER",13) )
			res->borderstyle = UI_BORDER_INDENT;
		else if ( !Q_strncmp(var,"3D_BORDER",9) )
			res->borderstyle = UI_BORDER_3D;
		else res->borderstyle = UI_BORDER_NONE;
	}
	else if ( !Q_strncmp( token, "shader", 6 ) ) {
		res->shader = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "enabledcvar", 11 ) ) {
		res->enablewithcvar = qtrue;
		res->enabledcvar = uii.Cvar_Get( PARSE_PTR, "", 0 );
	}
	else if ( !Q_strncmp( token, "stuffcommand", 12 ) ) {
		res->stuffcommand = ( char * )UI_Alloc( UI_MAX_NAME );
		Com_sprintf( res->stuffcommand, UI_MAX_NAME, "%s\n", PARSE_PTR );
		
	}
	else if ( !Q_strncmp( token, "hovershader", 11 ) ) {
		res->hoverDraw = qtrue;
		res->hoverShader = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "clicksound", 10 ) ) {
		res->clicksound = ( char * )UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->clicksound, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "hovercommand", 12 ) ) {
		res->hovercommand = ( char * )UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->hovercommand, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "linkcvartoshader", 16 ) ) {
		res->linkcvartoshader = qtrue;
	}
	else if ( !Q_strncmp( token, "linkcvar", 8 ) ) {
		res->linkcvarname = (char*)UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->linkcvarname, PARSE_PTR, UI_MAX_NAME );
		res->linkcvar = uii.Cvar_Get( res->linkcvarname, "", 0 );
	}
	else if ( !Q_strncmp( token, "font", 4 ) ) {
//		uii.R_RegisterFont( PARSE_PTR, 0, &res->font );
		PARSE_PTR;
		res->font = &uis.menuFont;
	}
	else if ( !Q_strncmp( token, "title", 5 ) ) {
		res->title = ( char * )UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->title, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "textalign", 9 )||!Q_strncmp( token, "align", 5 ) ) {
		var = PARSE_PTR;
		if ( !Q_strncmp(var,"left",4) )
			res->align = UI_ALIGN_LEFT;
		else if ( !Q_strncmp(var,"center",128) )
			res->align = UI_ALIGN_CENTER;
		else if ( !Q_strncmp(var,"right",5) )
			res->align = UI_ALIGN_RIGHT;
	}
	else if ( !Q_strncmp( token, "checked_shader", 14 ) ) {
		res->checked_shader = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "unchecked_shader", 16 ) ) {
		res->unchecked_shader = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "menushader", 10 ) ) {
		PARSE_PTR;
		res->menushader = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "selmenushader", 13 ) ) {
		PARSE_PTR;
		res->selmenushader = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "linkstring", 10 ) ) {
		if ( res->selentries >= UI_MAX_SELECT ) {
			Com_Printf( "Too many select entries %i\n", res->selentries );
			PARSE_PTR;
			PARSE_PTR;
			return;
		}
		res->linkstring1[res->selentries] = ( char * )UI_Alloc( UI_MAX_NAME );
		res->linkstring2[res->selentries] = ( char * )UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->linkstring1[res->selentries], PARSE_PTR, UI_MAX_NAME );
		Q_strncpyz( res->linkstring2[res->selentries], PARSE_PTR, UI_MAX_NAME );
		res->selentries++;
	}
	else if ( !Q_strncmp( token, "addpopup", 8 ) ) {
		PARSE_PTR;
		if ( res->selentries >= UI_MAX_SELECT ) {
			Com_Printf( "Too many select entries %i\n", res->selentries );
			PARSE_PTR;
			PARSE_PTR;
			PARSE_PTR;
			return;
		}
		res->linkstring1[ res->selentries ] = ( char * )UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->linkstring1[res->selentries], PARSE_PTR, UI_MAX_NAME );
		PARSE_PTR;
		res->linkstring2[ res->selentries ] = ( char * )UI_Alloc( UI_MAX_NAME );
		Com_sprintf( res->linkstring2[res->selentries], UI_MAX_NAME, "%s\n", PARSE_PTR );
		res->selentries++;
	}
	else if ( !Q_strncmp( token, "ordernumber", 11 ) ) {
		res->ordernumber = atoi(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "slidertype", 10 ) ) {
		res->ordernumber = atoi(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "setrange", 8 ) ) {
		res->flRange[0] = atof(PARSE_PTR);
		res->flRange[1] = atof(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "stepsize", 8 ) ) {
		res->flStepsize = atof(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "rendermodel", 11 ) ) {
		res->rendermodel = (qboolean)atoi(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "modeloffset", 128 ) ) {
		sscanf(PARSE_PTR,"%f %f %f",&res->modeloffset[0],&res->modeloffset[1],&res->modeloffset[2]);
	}
	else if ( !Q_strncmp( token, "modelrotateoffset", 17 ) ) {
		sscanf(PARSE_PTR,"%f %f %f",&res->modelrotateoffset[0],&res->modelrotateoffset[1],&res->modelrotateoffset[2]);
	}
	else if ( !Q_strncmp( token, "modelangles", 11 ) ) {
		sscanf(PARSE_PTR,"%f %f %f",&res->modelangles[0],&res->modelangles[1],&res->modelangles[2]);
	}
	else if ( !Q_strncmp( token, "modelscale", 10 ) ) {
		res->modelscale = atof(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "modelanim", 9 ) ) {
		res->modelanim = ( char * )UI_Alloc( UI_MAX_NAME );
		Q_strncpyz( res->modelanim, PARSE_PTR, UI_MAX_NAME );
	}
	else if ( !Q_strncmp( token, "fadein", 6 ) ) {
		res->fadein = atof( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "statbar_shader_flash", 20 ) ) {
		res->statbarshaderflash = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "statbar_shader", 14 ) ) {
		res->statbarshader = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "statbar_tileshader", 14 ) ) {
		res->statbartileshader = uii.Rend_RegisterMaterial( str( PARSE_PTR ) );
	}
	else if ( !Q_strncmp( token, "statbar_endangles", 17 ) ) {
		res->statbarendangles[0] = atoi( PARSE_PTR );
		res->statbarendangles[1] = atoi( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "statbar_rotatorsize", 19 ) ) {
		res->rotatorsize[0] = atof( PARSE_PTR );
		res->rotatorsize[1] = atof( PARSE_PTR );
	}
	else if ( !Q_strncmp( token, "statbar", 7 ) ) {
		var = PARSE_PTR;
		if ( !Q_strncmp(var,"compass",7) )
			res->statbar = STATBAR_COMPASS;
		else if ( !Q_strncmp(var,"horizontal",10) ) {
			res->statbar = STATBAR_HORIZONTAL;
			res->statbarRange[0] = atof(PARSE_PTR);
			res->statbarRange[1] = atof(PARSE_PTR);
		}
		else if ( !Q_strncmp(var,"vertical_stagger_even",21) )
			res->statbar = STATBAR_VERTICAL_STAGGER_EVEN;
		else if ( !Q_strncmp(var,"vertical_stagger_odd",20) )
			res->statbar = STATBAR_VERTICAL_STAGGER_ODD;
		else if ( !Q_strncmp(var,"vertical",8) )
			res->statbar = STATBAR_VERTICAL;
		else if ( !Q_strncmp(var,"headingspinner",14) ) {
			res->statbar = STATBAR_HEADING_SPINNER;
			res->statbarRange[0] = atof(PARSE_PTR);
			res->statbarRange[1] = atof(PARSE_PTR);
		}
		else if ( !Q_strncmp(var,"rotator",7) ) {
			res->statbar = STATBAR_ROTATOR;
			res->statbarRange[0] = atof(PARSE_PTR);
			res->statbarRange[1] = atof(PARSE_PTR);
		}
		else if ( !Q_strncmp(var,"spinner",7) ) {
			res->statbar = STATBAR_SPINNER;
			res->statbarRange[0] = atof(PARSE_PTR);
			res->statbarRange[1] = atof(PARSE_PTR);
		}
		else Com_Printf( "UI_ParseMenuResource: unknown statbar %s\n", var );
	}
	else if ( !Q_strncmp( token, "maxplayerstat", 128 ) ) {
		res->maxplayerstat = (statIndex_t)atoi(PARSE_PTR);
		res->statvar = qtrue;
	}
	else if ( !Q_strncmp( token, "playerstat", 10 ) ) {
		res->playerstat = (statIndex_t)atoi(PARSE_PTR);
		res->statvar = qtrue;
	}
	else if ( !Q_strncmp( token, "itemstat", 8 ) ) {
		res->itemstat = (statIndex_t)atoi(PARSE_PTR);
		res->statvar = qtrue;
	}
	else if ( !Q_strncmp( token, "invmodelhand", 12 ) ) {
		res->invmodelhand = atoi(PARSE_PTR);
	}
	else if ( !Q_strncmp( token, "resource", 8 ) ) {
		Com_Printf( "UI_ParseMenuResource: new resource not expected: forgot }?\n" );
	}
	else {
		Com_Printf( "UI_ParseMenuResource: unknown token %s\n", token );
	}
}

qboolean UI_ParseMenuToken( const char *token, char **ptr, uiMenu_t *menu ) {
	const char		*var;
	int				i;
	uiResource_t	*res;

	if ( !Q_strncmp( token, "menu", 4 ) ) {
		Q_strncpyz( menu->name, PARSE_PTR, sizeof(menu->name));
		menu->size[0] = atoi( PARSE_PTR );
		menu->size[1] = atoi( PARSE_PTR );
		var = COM_Parse( ptr );
		if ( !Q_strncmp( var, "FROM_TOP", 8 ))
			menu->motion = UI_FROM_TOP;
		else if ( !Q_strncmp( var, "FROM_TOP", 8 ))
			menu->motion = UI_FROM_BOTTOM;
		else if ( !Q_strncmp( var, "FROM_BOTTOM", 8 ))
			menu->motion = UI_FROM_LEFT;
		else if ( !Q_strncmp( var, "FROM_LEFT", 8 ))
			menu->motion = UI_FROM_RIGHT;
		else if ( !Q_strncmp( var, "NONE", 8 ))
			menu->motion = UI_NONE;
		menu->time = atoi( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "end.", 4 ) ) {
		return qtrue;
	}
	else if ( !Q_strncmp( token, "include", 7 ) ) {
		Q_strncpyz( menu->include, PARSE_PTR, UI_MAX_NAME );
		UI_LoadINC( menu->include, menu, qfalse );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "postinclude", 11 ) ) {
		Q_strncpyz( menu->postinclude, PARSE_PTR, UI_MAX_NAME );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "align", 5 ) ) {
		var = PARSE_PTR;
		if ( !Q_strncmp( var, "left", 128 ) ) {
			menu->align[0] = UI_ALIGN_LEFT;
			menu->offset[0] = 0;
			}
		else if ( !Q_strncmp( var, "centerx", 128 ) ) {
			menu->align[0] = UI_ALIGN_CENTER;
			menu->offset[0] = 320 - menu->size[0]/2;
		}
		else if ( !Q_strncmp( var, "right", 128 ) ) {
			menu->align[0] = UI_ALIGN_RIGHT;
			menu->offset[0] = 640 - menu->size[0];
		}
		var = PARSE_PTR;
		if ( !Q_strncmp( var, "top", 128 ) ) {
			menu->align[1] = UI_ALIGN_LEFT;
			menu->offset[1] = 0;
		}
		else if ( !Q_strncmp( var, "centery", 128 ) ) {
			menu->align[1] = UI_ALIGN_CENTER;
			menu->offset[1] = 240 - menu->size[1]/2;;
		}
		else if ( !Q_strncmp( var, "bottom", 128 ) ) {
			menu->align[1] = UI_ALIGN_RIGHT;
			menu->offset[1] = 480 - menu->size[1];
		}
		return qfalse;
	}
	else if ( !Q_strncmp( token, "vidmode", 7 ) ) {
		menu->vidmode = atoi( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "fadein", 7 ) ) {
		menu->fadein = atof( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "fgcolor", 7 ) ) {
		for ( i=0;i<4;i++ )
			menu->fgcolor[i] = atof( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "bgcolor", 7 ) ) {
		for ( i=0;i<4;i++ )
			menu->bgcolor[i] = atof( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "borderstyle", 11 ) ) {
		var = PARSE_PTR;
		if ( !Q_strncmp(var,"RAISED",6) )
			menu->borderstyle = UI_BORDER_RAISED;
		else menu->borderstyle = UI_BORDER_NONE;
		return qfalse;
	}
	else if ( !Q_strncmp( token, "bgfill", 6 ) ) {
		for ( i=0;i<4;i++ )
			menu->bgfill[i] = atof( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "fullscreen", 10 ) ) {
		menu->fullscreen = (qboolean)atoi( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "virtualres", 10 ) ) {
		menu->virtualres = (qboolean)atoi( PARSE_PTR );
		return qfalse;
	}
	else if ( !Q_strncmp( token, "resource", 8 ) ) {
		menu->resPtr++;
		if ( menu->resPtr >= UI_MAX_RESOURCES ) {
			Com_Printf( "UI_ParseMenuToken: more than %i resources!\n", UI_MAX_RESOURCES );
			menu->resPtr--;
			return qfalse;
		}
		res = &menu->resources[menu->resPtr];
		res->font = &uis.menuFont;
//		uii.R_RegisterFont( "facfont-20", 0, &res->font ); //load standard font for resource

		var = PARSE_PTR;
		if ( !Q_strncmp( var, "Label", 5 ) )
			res->type = UI_RES_LABEL;
		else if ( !Q_strncmp( var, "Button", 6 ) )
			res->type = UI_RES_BUTTON;
		else if ( !Q_strncmp( var, "Field", 5 ) )
			res->type = UI_RES_FIELD;
		else if ( !Q_strncmp( var, "CheckBox", 8 ) )
			res->type = UI_RES_CHECKBOX;
		else if ( !Q_strncmp( var, "PulldownMenuContainer", 21 ) )
			res->type = UI_RES_PULLDOWN;
		else if ( !Q_strncmp( var, "Slider", 6 ) )
			res->type = UI_RES_SLIDER;
		else if ( !Q_strncmp( var, "FAKKServerList", 14 ) ) {
			res->type = UI_RES_SERVERLIST;
			res->selectedListEntry = -1;
		}
		else if ( !Q_strncmp( var, "FAKKBindList", 12 ) )
			// su44: that's used in ui/controls.urc
			res->type = UI_RES_BINDLIST; 
		else if ( !Q_strncmp( var, "LANGameClass", 12 ) ) {
			res->type = UI_RES_LANSERVERLIST;
			res->selectedListEntry = -1;
		}
		else
			Com_Printf( "UI_ParseMenuToken: unknown menu resource type %s\n", var );

		var = PARSE_PTR ;
		if ( *var == '{' ) {
			var = PARSE_PTR;
			while (*var != '}') {
				UI_ParseMenuResource( var, ptr, res, menu->offset );
				var = PARSE_PTR;
				if (*var ==0) {
					Com_Memset( res, 0, sizeof(uiResource_t) );
					menu->resPtr--;
					return qfalse;
				}
			}
			// "startnew" and "changemap" buttons are equal. we skip changemap
			// sigh sigh sigh
			if (!Q_strncmp(res->name,"changemap",UI_MAX_NAME)) {
				Com_Memset( res, 0, sizeof(uiResource_t) );
				menu->resPtr--;
			}
			return qfalse;
		}
		Com_Printf( "UI_ParseMenuToken: error loading resource\n" );
		return qfalse;
	}
	else {
		Com_Printf( "UI_ParseMenuToken: unknown token %s\n", token );
		return qfalse;
	}
}

void	UI_LoadINC( const char *name, uiMenu_t *menu, qboolean post ) {
	fileHandle_t	f;
	int				len;
	char			filename[MAX_QPATH];
	char			buffer[UI_MAX_URCSIZE];
	const char		*token;
	char			*ptr;
	uiResource_t	*res;
	const char		*var;

	Q_strncpyz( filename, name, sizeof( filename ) );
	if ( post == qfalse )
		COM_DefaultExtension( filename, sizeof( filename ), ".inc" );

	len = uii.FS_FOpenFile( filename, &f, FS_READ);
	if (!f) {
		Com_Printf( "couldn't load include file %s\n", name );
		return;
	}
	if ( len > UI_MAX_URCSIZE ) {
		Com_Printf( "Include file too large, %i KB. Max size is %i KB\n", len/1024, UI_MAX_URCSIZE/1024 );
		return;
	}

	uii.FS_Read( buffer, len, f );
	buffer[len] = 0;

	ptr = buffer;
	token = COM_Parse( &ptr );

	if ( post == qtrue ) {
		if (!Q_strncmp(token, "menu",4)) {
			COM_Parse( &ptr );
			token = COM_Parse(&ptr);
		}
	}

	while (*token) {
		if ( !Q_strncmp( token, "resource", 8 ) ) {
			menu->resPtr++;
			if ( menu->resPtr >= UI_MAX_RESOURCES ) {
				Com_Printf( "UI_ParseMenuToken: more than %i resources!\n", UI_MAX_RESOURCES );
				menu->resPtr--;
				return;
			}
			res = &menu->resources[menu->resPtr];
			res->font = &uis.menuFont;
//			uii.R_RegisterFont( "facfont-20", 0, &res->font ); //load standard font for resource

			var = COM_Parse( &ptr );
			if ( !Q_strncmp( var, "Label", 5 ) )
				res->type = UI_RES_LABEL;
			else if ( !Q_strncmp( var, "Button", 6 ) )
				res->type = UI_RES_BUTTON;
			else if ( !Q_strncmp( var, "Field", 5 ) )
				res->type = UI_RES_FIELD;
			else if ( !Q_strncmp( var, "CheckBox", 8 ) )
				res->type = UI_RES_CHECKBOX;
			else if ( !Q_strncmp( var, "PulldownMenuContainer", 21 ) )
				res->type = UI_RES_PULLDOWN;
			else if ( !Q_strncmp( var, "Slider", 6 ) )
				res->type = UI_RES_SLIDER;
			else Com_Printf( "UI_ParseMenuToken: unknown menu resource type %s\n", var );

			var = COM_Parse( &ptr ) ;
			if ( *var == '{' ) {
				var = COM_Parse( &ptr );
				while (*var != '}') {
					UI_ParseMenuResource( var, &ptr, res, menu->offset );
					var = COM_Parse( &ptr );
					if (*var ==0) {
						Com_Memset( res, 0, sizeof(uiResource_t) );
						menu->resPtr--;
						return;
					}
				}
			}
		}

		token = COM_Parse( &ptr );
	}

	uii.FS_FCloseFile( f );
}

void	UI_LoadURC( const char *name, uiMenu_t *menu ) {
	fileHandle_t	f;
	int				len;
	char			filename[MAX_QPATH];
	char			buffer[UI_MAX_URCSIZE];
	const char		*token;
	char			*ptr;
	qboolean		end;

	uiMenulist_t*	menulist;
	long			hash;
	qboolean		found;

	Q_strncpyz( filename, "ui/", MAX_QPATH );
	hash = generateHashValue(name);
	found = qfalse;
	for (menulist=hashTable[hash] ; menulist ; menulist=menulist->next) {
		if (!Q_stricmp(name, menulist->name)) {
			found = qtrue;
			break;
		}
	}

	if ( found == qtrue )
		Q_strncpyz( filename + 3, menulist->file, MAX_QPATH-3 );
	else {
		Q_strncpyz( filename + 3, name, MAX_QPATH-3 );
	}
	COM_DefaultExtension( filename, MAX_QPATH, ".urc" );

	len = uii.FS_FOpenFile( filename, &f, FS_READ );
	if (!f) {
		Com_Printf( "couldn't load URC menu %s. file not found.\n", name );
		return;
	}
	if ( len >= UI_MAX_URCSIZE ) {
		Com_Printf( "URC file too large, %i KB. Max size is %i KB\n", len/1024, UI_MAX_URCSIZE/1024 );
		return;
	}

	memset(menu,0,sizeof(uiMenu_t)); // su44 was here

	uii.FS_Read( buffer, len, f );
	buffer[len] = 0;

	ptr = buffer;
	token = COM_Parse( &ptr );

	menu->resPtr = -1;
	while (*token) {
		end = UI_ParseMenuToken( token, &ptr, menu );
		token = COM_Parse( &ptr );
	}
	if (!end)
		Com_Printf( "UI_LoadURC hit end of file without end statement\n" );

	uii.FS_FCloseFile( f );
	if ( menu->postinclude[0] )
		UI_LoadINC( menu->postinclude, menu, qtrue );
}

void	UI_PushMenu( const char *name ) {
	uiMenu_t		*menu;

	uis.MSP++;
	if ( uis.MSP >= UI_MAX_MENUS ) {
		Com_Printf( "UI_PushMenu: Menu Stack Overflow (%i).\n", UI_MAX_MENUS );
		uis.MSP--;
		return;
	}

	if ( !Q_strncmp( name, "main", 4 ) )
		uis.stack[uis.MSP] = &uis.main;
	else if ( !Q_strncmp( name, "connecting", 10 ) )
		uis.stack[uis.MSP] = &uis.connecting;
	else {
		uis.CP++;
		if ( uis.CP >= UI_MAX_MENUS ) {
			Com_Printf( "UI_PushMenu: Menu Cache Overflow (%i).\n", UI_MAX_MENUS );
			uis.CP--;
			return;
		}
		menu = &uis.cache[uis.CP];
		Com_Memset( menu, 0, sizeof(uiMenu_t) );

		UI_LoadURC( name, menu );

		menu->standard = qfalse;
		uis.stack[uis.MSP] = menu;
	}

//	if ( !(uii.Key_GetCatcher() & KEYCATCH_UI) )
		uii.Key_SetCatcher( KEYCATCH_UI );
}

void	UI_PopMenu( void ) {

	if ( uis.MSP >= 0 ) {

		if ( uis.stack[uis.MSP]->standard == qfalse )
			uis.CP--;
		uis.MSP--;
	}
	if ( uis.MSP == -1 ) {
		uii.Key_SetCatcher( uii.Key_GetCatcher() & ~KEYCATCH_UI );
		uii.Key_ClearStates();
	}
}

#define MENULIST_BUFSIZE 2*MAX_STRING_CHARS
void UI_FindMenus( void ) {
	char	*filelist, *namelist;
	char	*ptr, *ptr2;
	int		i, numFiles, fnameLen, nameptr;
	char	filebuf[MAX_QPATH];

	filelist = (char*)UI_Alloc( MENULIST_BUFSIZE );
	namelist = (char*)UI_Alloc( MENULIST_BUFSIZE );

	numFiles = uii.FS_GetFileList( "ui", ".urc", filelist, MENULIST_BUFSIZE );

	ptr = filelist;
	nameptr = 0;
	for (i = 0; i < numFiles; i++, ptr += fnameLen+1) {
		uiMenulist_t *ml;
		fileHandle_t f;
		int hash;
		Q_strncpyz( filebuf, "ui/", sizeof(filebuf) );
		Q_strncpyz( filebuf + 3, ptr, sizeof( filebuf )-3 );
		uii.FS_FOpenFile( filebuf, &f, FS_READ );
		uii.FS_Read( filebuf, sizeof(filebuf), f );
		uii.FS_FCloseFile( f );
		ptr2 = filebuf;

		if ( !Q_strncmp( COM_Parse(&ptr2), "menu", MAX_QPATH ) ){
			Q_strncpyz( namelist+nameptr, COM_Parse(&ptr2), MENULIST_BUFSIZE-nameptr );
		}
		else {
			Com_Printf( "Menu name not recognized in %s\n", ptr );
			continue;
		}
		hash = generateHashValue( namelist + nameptr );
		/*ml = hashTable[ hash ];
		if ( ml ) {
			while( ml->next )²
				ml = ml->next;
			ml->next = &uis.menuList[uis.MLP];
			ml = ml->next;
		}
		else{
			ml = &uis.menuList[uis.MLP];
			hashTable[ hash ] = ml;
		}*/
		ml = &uis.menuList[ uis.MLP ];
		if( !hashTable[ hash ] )
		{
			ml->next = NULL;
		}
		else
		{
			ml->next = hashTable[ hash ];
		}
		hashTable[ hash ] = ml;
		ml->file = ptr;
		ml->name = namelist+nameptr;

		fnameLen = strlen(ptr);
		nameptr += strlen(namelist+nameptr)+1;
		if ( nameptr >= MENULIST_BUFSIZE ) {
			Com_Printf( "Too many menus in place. Namebuffer overrun!\n" );
			return;
		}
		uis.MLP++;
		if ( uis.MLP >= UI_MAX_FILES ) {
			Com_Printf( "Too many menus in place.\n" );
			return;
		}
	}
}
