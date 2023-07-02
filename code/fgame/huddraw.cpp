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

// huddraw.cpp :	This is a HudDraw Functions Code section with huddraw functions reversed for 
//					altering clients HUD

#include "g_local.h"
#include "hud.h"

void HudWriteNumber( int num )
{
	if( sv_reborn->integer ) {
		gi.MSG_WriteShort( num );
	} else {
		gi.MSG_WriteByte( num );
	}
}

void HudDrawShader( int info, const char *name )
{
	gi.SetBroadcastAll();
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_SHADER));
	HudWriteNumber( info );		// c = info
	gi.MSG_WriteString(name);		// s = name (shader_name)
	gi.MSG_EndCGM();
}

void HudDrawAlign( int info, int horizontalAlign, int verticalAlign )
{
	gi.SetBroadcastAll();
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_ALIGN));
	HudWriteNumber( info );					// c = probably "info"
	gi.MSG_WriteBits(horizontalAlign, 2);	// value = 0,1,2	bits = 2
											// 0 - left		\
											// 1 - center	 - horizontalAlign
											// 2 - right	/
	
	gi.MSG_WriteBits(verticalAlign, 2);		// value = 0,1,2	bits = 2
											// 0 - top		\
											// 1 - center	 - verticalAlign
											// 2 - bottom	/
	
	gi.MSG_EndCGM();
	
}

void HudDrawRect(int info, int x, int y, int width, int height)
{
	gi.SetBroadcastAll();
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_RECT));
	HudWriteNumber( info );			// c = probably "info"
	gi.MSG_WriteShort(x);			// c = probably "x"
	gi.MSG_WriteShort(y);			// c = probably "y"
	gi.MSG_WriteShort(width);		// c = probably "width"
	gi.MSG_WriteShort(height);		// c = probably "height"
	gi.MSG_EndCGM();
	
}

void HudDrawVirtualSize(int info, int virtualScreen)
{
	gi.SetBroadcastAll();
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_VIRTUALSIZE));
	HudWriteNumber( info );					// c = info
	
	/*__asm
	{
		pushad
		pushfd
		mov eax, virtualScreen
		NEG eax
		SBB eax, eax
		NEG eax
		mov virtualScreen, eax
		popfd
		popad
	}*/
	
	gi.MSG_WriteBits(!!virtualScreen, 1);	// value = ?	bits = 1
											// value = esi
											// esi = virtualScreen
											// NEG esi
											// SBB esi, esi
											// NEG esi
											// call
	
	gi.MSG_EndCGM();
	
}

void HudDrawColor(int info, float *color)
{
	long int temp[3];
	temp[0] = (long int)(color[0]*255.0f);
	temp[1] = (long int)(color[1]*255.0f);
	temp[2] = (long int)(color[2]*255.0f);
	
	
	gi.SetBroadcastAll();
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_COLOR));
	HudWriteNumber( info );			// c = info
	gi.MSG_WriteByte(temp[0]);		// c = color[2]		
	gi.MSG_WriteByte(temp[1]);		// c = color[1]		 - Values can be messed up. To be tested.
	gi.MSG_WriteByte(temp[2]);		// c = color[3]		/
	gi.MSG_EndCGM();

	// Note: Each float value is multiplied by 255.0 and converted to long using ftol function, thats why it's using WriteByte
}

void HudDrawAlpha(int info, float alpha)
{
	long int temp;
	temp = (long int)(alpha*255.0f);
	
	gi.SetBroadcastAll();
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_ALPHA));
	HudWriteNumber( info );		// c = info
	gi.MSG_WriteByte(temp);		// c = alpha
	gi.MSG_EndCGM();
	
	// Note: alpha is multiplied by 255.0 and converted to long using ftol function
	
}

void HudDrawString(int info, const char *string)
{
	gi.SetBroadcastAll();
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_STRING));
	HudWriteNumber( info );		// c = info
	gi.MSG_WriteString(string);	// s = string (to show)
	gi.MSG_EndCGM();

}

void HudDrawFont(int info, const char *fontName)
{
	gi.SetBroadcastAll();
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_FONT));
	HudWriteNumber( info );			// c = info
	gi.MSG_WriteString(fontName);	// s = fontName (to use)
	gi.MSG_EndCGM();
	
}

void HudDraw3d( int index, vec3_t vector, int ent_num, qboolean bAlwaysShow, qboolean depth )
{
	// FIXME...
	/*
	gi.SetBroadcastAll();

	gi.MSG_StartCGM( BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_3D ));
		HudWriteNumber( index );
		gi.MSG_WriteCoord( vector[0] );
		gi.MSG_WriteCoord( vector[1] );
		gi.MSG_WriteCoord( vector[2] );

		gi.MSG_WriteShort( ent_num );

		gi.MSG_WriteByte( bAlwaysShow );
		gi.MSG_WriteByte( depth );
	gi.MSG_EndCGM();
	*/
}

void HudDrawTimer( int index, float duration, float fade_out_time )
{
/*
	gi.SetBroadcastAll();

	gi.MSG_StartCGM( BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_TIMER ));
		HudWriteNumber( index );
		gi.MSG_WriteFloat( duration );
		gi.MSG_WriteFloat( fade_out_time );
	gi.MSG_EndCGM();
*/
}

void iHudDrawShader( int cl_num, int info, const char *name )
{
	gi.MSG_SetClient(cl_num);
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_SHADER));
	HudWriteNumber( info );		// c = info
	gi.MSG_WriteString(name);		// s = name (shader_name)
	gi.MSG_EndCGM();

}

void iHudDrawAlign(int cl_num, int info, int horizontalAlign, int verticalAlign )
{
	gi.MSG_SetClient(cl_num);
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_ALIGN));
	HudWriteNumber( info );					// c = probably "info"
	gi.MSG_WriteBits(horizontalAlign, 2);	// value = 0,1,2	bits = 2
											// 0 - left		\
											// 1 - center	 - horizontalAlign
											// 2 - right	/
	
	gi.MSG_WriteBits(verticalAlign, 2);		// value = 0,1,2	bits = 2
											// 0 - top		\
											// 1 - center	 - verticalAlign
											// 2 - bottom	/
	
	gi.MSG_EndCGM();
	
}

void iHudDrawRect(int cl_num, int info, int x, int y, int width, int height)
{
	gi.MSG_SetClient(cl_num);
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_RECT));
	HudWriteNumber( info );		// c = probably "info"
	gi.MSG_WriteShort(x);			// c = probably "x"
	gi.MSG_WriteShort(y);			// c = probably "y"
	gi.MSG_WriteShort(width);		// c = probably "width"
	gi.MSG_WriteShort(height);		// c = probably "height"
	gi.MSG_EndCGM();
}

void iHudDraw3d( int cl_num, int index, vec3_t vector, int ent_num, qboolean bAlwaysShow, qboolean depth )
{
	/*
	gi.MSG_SetClient( cl_num );

	gi.MSG_StartCGM( BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_3D ));
		HudWriteNumber( index );
		gi.MSG_WriteCoord( vector[0] );
		gi.MSG_WriteCoord( vector[1] );
		gi.MSG_WriteCoord( vector[2] );

		gi.MSG_WriteShort( ent_num );

		gi.MSG_WriteByte( bAlwaysShow );
		gi.MSG_WriteByte( depth );
	gi.MSG_EndCGM();
	*/
}

void iHudDrawTimer( int cl_num, int index, float duration, float fade_out_time )
{
/*
	gi.MSG_SetClient( cl_num );

	gi.MSG_StartCGM( BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_TIMER ));
		HudWriteNumber( index );
		gi.MSG_WriteFloat( duration );
		gi.MSG_WriteFloat( fade_out_time );
	gi.MSG_EndCGM();
*/
}

void iHudDrawVirtualSize(int cl_num, int info, int virtualScreen)
{
	gi.MSG_SetClient(cl_num);
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_VIRTUALSIZE));
	HudWriteNumber( info );					// c = info

	gi.MSG_WriteBits(!!virtualScreen, 1);	// value = ?	bits = 1
											// value = esi
											// esi = virtualScreen
											// NEG esi
											// SBB esi, esi
											// NEG esi
											// call
	
	gi.MSG_EndCGM();
	
}

void iHudDrawColor(int cl_num, int info, float *color)
{
	long int temp[3];

	temp[0] = (long int)(color[0]*255.0f);
	temp[1] = (long int)(color[1]*255.0f);
	temp[2] = (long int)(color[2]*255.0f);

	gi.MSG_SetClient(cl_num);
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_COLOR));
	HudWriteNumber( info );			// c = info
	gi.MSG_WriteByte(temp[0]);		// c = color[2]		
	gi.MSG_WriteByte(temp[1]);		// c = color[1]		 - Values can be messed up. To be tested.
	gi.MSG_WriteByte(temp[2]);		// c = color[3]		/
	gi.MSG_EndCGM();

	// Note: Each float value is multiplied by 255.0 and converted to long using ftol function, thats why it's using WriteByte
}

void iHudDrawAlpha(int cl_num, int info, float alpha)
{
	long int temp;
	temp = (long int)(alpha*255.0f);

	gi.MSG_SetClient(cl_num);
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_ALPHA));
	HudWriteNumber( info );			// c = info
	gi.MSG_WriteByte(temp);		// c = alpha
	gi.MSG_EndCGM();
	
	// Note: alpha is multiplied by 255.0 and converted to long using ftol function
	
}

void iHudDrawString( int cl_num, int info, const char *string )
{
	gi.MSG_SetClient(cl_num);
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_STRING));
	HudWriteNumber( info );		// c = info
	gi.MSG_WriteString(string);	// s = string (to show)
	gi.MSG_EndCGM();

}

void iHudDrawFont( int cl_num, int info, const char *fontName )
{
	gi.MSG_SetClient(cl_num);
	gi.MSG_StartCGM(BG_MapCGMToProtocol(g_protocol, CGM_HUDDRAW_FONT));
	HudWriteNumber( info );			// c = info
	gi.MSG_WriteString(fontName);	// s = fontName (to use)
	gi.MSG_EndCGM();
	
}
/* FONTS

  facfont-20
courier-16
courier-18
courier-20
handle-16 
handle-18
marlett
verdana-12
verdana-14

  */
