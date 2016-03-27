/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

#include "ui_local.h"

uidef_t uid;
uiexport_t uie;
uiimport_t uii;
uiGlobals_t ui;

void UIE_AddFileToList( const char *name )
{
	ui.fileList.AddTail( name );
}

void UIE_ResolutionChanged( void )
{
	UIRect2D frame( 0, 0, uid.vidWidth, uid.vidHeight );
	uWinMan.setFrame( frame );
	uWinMan.RefreshShadersFromList();
}

void UI_GetMouseState( int *mouseX, int *mouseY, int *flags )
{
	if( mouseX )
	{
		*mouseX = uid.mouseX;
	}

	if( mouseY )
	{
		*mouseY = uid.mouseY;
	}

	if( flags )
	{
		*flags = uid.mouseFlags;
	}
}

int UI_GetCvarInt( const char *name, int def )
{
	cvar_t *cvar = uii.Cvar_Find( name );

	if( !cvar )
	{
		return def;
	}

	return cvar->integer;
}

float UI_GetCvarFloat( const char *name, float def )
{
	cvar_t *cvar = uii.Cvar_Find( name );

	if( !cvar )
	{
		return def;
	}

	return cvar->value;
}

const char *UI_GetCvarString( const char *cvar, char *def )
{
	return uii.Cvar_GetString( cvar, def );
}

cvar_t *UI_FindCvar( const char *cvar )
{
	return uii.Cvar_Find( cvar );
}

void UI_SetCvarInt( const char *cvar, int value )
{
	char s[ 16 ];

	sprintf( s, "%d", value );
	uii.Cvar_Set( cvar, s );
}

void UI_SetCvarFloat( const char *cvar, float value )
{
	char s[ 16 ];

	sprintf( s, "%f", value );
	uii.Cvar_Set( cvar, s );
}

void UI_ListFiles( const char *filespec )
{
	ui.fileList.RemoveAllItems();
	uii.File_ListFiles( filespec );
}

const char *UI_ConfigString( int index )
{
	return uii.GetConfigstring( index );
}

void UI_Init( void )
{
	UIRect2D frame( 0, 0, uid.vidWidth, uid.vidHeight );

	ui.globalFont = new UIFont;
	uWinMan.Init( frame, "verdana-12" );

	UColor color( 0, 0, 0, 0 );
	uWinMan.setBackgroundColor( color, true );
}

void UI_Shutdown( void )
{
	if( ui.globalFont )
	{
		delete ui.globalFont;
		ui.globalFont = NULL;
	}

	uWinMan.Shutdown();
}

void UI_InitExports()
{
	uie.AddFileToList		= UIE_AddFileToList;
	uie.ResolutionChange	= UIE_ResolutionChanged;
	uie.Init				= UI_Init;
	uie.Shutdown			= UI_Shutdown;
	uie.FontStringWidth		= UI_FontStringWidth;

	//
	// OLD implementation
	//
	/*
	uie.KeyEvent			= UI_KeyEvent;
	uie.MouseEvent			= UI_MouseEvent;
	uie.Refresh				= UI_Refresh;
	uie.DrawHUD				= UI_DrawHUD;
	uie.IsFullscreen		= UI_IsFullscreen;
	uie.SetActiveMenu		= UI_SetActiveMenu;
	uie.ConsoleCommand		= UI_ConsoleCommand;
	uie.DrawConnectScreen	= UI_DrawConnectScreen;
	*/
}
