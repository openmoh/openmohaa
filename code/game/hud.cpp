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

// hud.cpp: New HUD handler for MoHAA
//


#include "glb_local.h"
#include "hud.h"

#ifdef CGAME_DLL
#include "cgamex86.h"
#include "cgame/cg_hud.h"
#include "script/centity.h"
#include "archive.h"
#endif

#ifdef GAME_DLL
#include "../game/player.h"
#endif

#include "game.h"
#include "level.h"

Container< Hud * > hudElements;
int lastNumberRemoved = -1;

Event EV_HUD_GetAlignX
(
	"alignx",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the horizontal alignment for the HUD. Specified by 'left', 'center', or 'right'",
	EV_GETTER
);

Event EV_HUD_GetAlignY
(
	"aligny",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the vertical alignment for the HUD",
	EV_GETTER
);

Event EV_HUD_GetAlpha
(
	"alpha",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the HUD alpha",
	EV_GETTER
);

Event EV_HUD_GetColor
(
	"color",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the HUD colors",
	EV_GETTER
);

Event EV_HUD_GetFont
(
	"font",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the HUD font",
	EV_GETTER
);

Event EV_HUD_GetRectX
(
	"x",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the HUD horizontal position",
	EV_GETTER
);

Event EV_HUD_GetRectY
(
	"y",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the HUD vertical position",
	EV_GETTER
);

Event EV_HUD_GetTime
(
	"gettime",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the HUD  time",
	EV_RETURN
);

Event EV_HUD_GetHeight
(
	"getheight",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the HUD height",
	EV_RETURN
);

Event EV_HUD_GetWidth
(
	"getwidth",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the HUD width",
	EV_RETURN
);

Event EV_HUD_Set3D
(
	"setdimension",
	EV_DEFAULT,
	"vbbE",
	"vector_or_offset always_show depth entity",
	"Sets this huddraw element to be a 3D world icon and can specify if this icon is always shown on-screen even if the player isn't looking at.\n"
	"It uses xy pos from huddraw_rect.\n"
	"If entity is specified, the vector will be an offset relative to the entity.\n"
	"depth specify if the icon is shown through walls.",
	EV_NORMAL
);

Event EV_HUD_SetNon3D
(
	"resetdimension",
	EV_DEFAULT,
	NULL,
	NULL,
	"Bring the HUD back to normal dimension (non-3D)",
	EV_NORMAL
);

Event EV_HUD_SetAlignX
(
	"alignx",
	EV_DEFAULT,
	"s",
	"align",
	"Sets the horizontal alignment for the HUD. Specified by 'left', 'center', or 'right'",
	EV_SETTER
);

Event EV_HUD_SetAlignY
(
	"aligny",
	EV_DEFAULT,
	"s",
	"align",
	"Sets the vertical alignment for the HUD. Specified by 'top', 'center', or 'bottom'",
	EV_SETTER
);

Event EV_HUD_SetAlpha
(
	"alpha",
	EV_DEFAULT,
	"f",
	"alpha",
	"Sets the HUD alpha",
	EV_SETTER
);

Event EV_HUD_SetColor
(
	"color",
	EV_DEFAULT,
	"v",
	"color",
	"Sets the HUD colors",
	EV_SETTER
);

Event EV_HUD_SetFont
(
	"font",
	EV_DEFAULT,
	"s",
	"font",
	"Sets the HUD font",
	EV_SETTER
);

Event EV_HUD_SetPlayer
(
	"setplayer",
	EV_DEFAULT,
	"eB",
	"entity clears",
	"Sets to which player this HUD is shown and possibly clears the HUD to the previous player(s) it was shown. A NULL/NIL entity means the HUD will be shown to all players\n"
	"Note : you will need to call in HUD functions again as it won't refresh to the player(s)",
	EV_NORMAL
);

Event EV_HUD_SetRectX
(
	"x",
	EV_DEFAULT,
	"i",
	"x",
	"Sets the HUD horizontal position",
	EV_SETTER
);

Event EV_HUD_SetRectY
(
	"y",
	EV_DEFAULT,
	"i",
	"y",
	"Sets the HUD vertical position",
	EV_SETTER
);

Event EV_HUD_SetShader
(
	"setshader",
	EV_DEFAULT,
	"sii",
	"shader width height",
	"Sets the HUD shader",
	EV_NORMAL
);

Event EV_HUD_SetText
(
	"settext",
	EV_DEFAULT,
	"s",
	"text",
	"Sets the HUD text",
	EV_NORMAL
);

Event EV_HUD_SetTimer
(
	"settimer",
	EV_DEFAULT,
	"fF",
	"time fade_at_time",
	"Sets a timer to count down an optionally fade at the specified time",
	EV_NORMAL
);

Event EV_HUD_SetTimerUp
(
	"settimerup",
	EV_DEFAULT,
	"fF",
	"time fade_at_time",
	"Sets a timer to count up an optionally fade at the specified time",
	EV_NORMAL
);

Event EV_HUD_SetVirtualSize
(
	"setvirtualsize",
	EV_DEFAULT,
	"b",
	"virtual",
	"Sets if the HUD should use virtual screen resolution for positioning and size",
	EV_NORMAL
);

Event EV_HUD_FadeOverTime
(
	"fadeovertime",
	EV_DEFAULT,
	"f",
	"time",
	"Sets the HUD to transition in color (or alpha) over time",
	EV_NORMAL
);

Event EV_HUD_MoveOverTime
(
	"moveovertime",
	EV_DEFAULT,
	"f",
	"time",
	"Sets the HUD to move over time",
	EV_NORMAL
);

Event EV_HUD_ScaleOverTime
(
	"scaleovertime",
	EV_DEFAULT,
	"fii",
	"time width height",
	"Sets the HUD to scale over time",
	EV_NORMAL
);

Event EV_HUD_Refresh
(
	"refresh",
	EV_DEFAULT,
	NULL,
	NULL,
	"Refresh the HUD",
	EV_NORMAL
);

Hud *Hud::Find( int index )
{
	for( int i = 0; i < hudElements.NumObjects(); i++ )
	{
		Hud *hud = hudElements[ i ];

		if( hud->number == index ) {
			return hud;
		}
	}

	return NULL;
}

Hud *Hud::FindOrCreate( int index )
{
	Hud *hud = Hud::Find( index );

	if( hud == NULL ) {
		hud = new Hud( index );
	}

	return hud;
}

void Hud::ProcessThink()
{
#ifdef CGAME_DLL
	if( !ui_hud->integer && !cg_hud->integer ) {
		return;
	}
#endif

	for( int i = 0; i < hudElements.NumObjects(); i++ ) {
		hudElements[ i ]->Think();
	}
}

int Hud::Sort( const void *elem1, const void *elem2 )
{
	const Hud *hud1 = *( Hud ** )elem1;
	const Hud *hud2 = *( Hud ** )elem2;

	// Sort HUDs by their number
	if ( hud1->number < hud2->number ) {
		return -1;
	} else if ( hud1->number > hud2->number ) {
		return 1;
	} else {
		return 0;
	}
}

int Hud::GetFreeNumber()
{
	for( int i = 0; i < hudElements.NumObjects(); i++ )
	{
		Hud *hud = hudElements[ i ];

		if( hud->number != i ) {
			return i;
		}
	}

	return hudElements.NumObjects();
}

#ifdef GAME_DLL
Hud::Hud( int client )
#else
Hud::Hud( int index )
#endif
{
#ifdef GAME_DLL
	if( client < -1 || client > game.maxclients )
	{
		delete this;
		ScriptError( "Invalid client number %d !\n", client );

		return;
	}
#endif

#ifdef CGAME_DLL
	if( index == -1 )
	{
#endif
		number = GetFreeNumber();
	/*if( lastNumberRemoved != -1 )
	{
		number = GetFreeNumber();
		lastNumberRemoved = -1;
	} else {
		number = hudElements.NumObjects();
	}*/
#ifdef CGAME_DLL
	} else {
		number = index;
	}
#endif

#ifdef GAME_DLL
	clientnum = client;
#endif

	alignX = HUD_CENTER;
	alignY = HUD_CENTER;

	color = Vector( 1.f, 1.f, 1.f );
	alpha = 0.0f;

	x = 0;
	y = 0;

	width = 8;
	height = 8;

#ifdef CGAME_DLL
	font = NULL;
	shaderHandle = NULL;
#endif

	fade_alpha = false;
	fade_move = false;
	fade_scale = false;
	fade_timer_flags = 0;

	fade_alpha_first = false;
	fade_move_x_first = false;
	fade_move_y_first = false;

	fade_alpha_current = 0.f;
	fade_move_current = 0.f;
	fade_scale_current = 0.f;
	fade_time_current = 0.f;

	isDimensional = false;
	org = Vector( 0, 0, 0 );
	lastOrg = org;
	always_show = false;
	depth = false;
	enttarget = -1;

	virtualSize = false;

	fontName = "";
	shader = "";
	text = "";

	hudElements.AddObject( this );

	hudElements.Sort( Hud::Sort );
}

Hud::~Hud()
{
	lastNumberRemoved = number;

	SetAlpha( 0.f );

	if( isDimensional ) {
		SetNon3D();
	}

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_DELETE );
		WriteNumber();
	gi.MSG_EndCGM();
#endif

	hudElements.RemoveObject( this );
}

#ifdef CGAME_DLL
qboolean CG_WorldToScreen( Vector pt, float *x, float *y, qboolean bOptionalSeen, float shader_width, float shader_height )
{
	float	xzi, yzi;
	Vector	local, transformed;
	Vector	vfwd, vright, vup;
	qboolean bNotShown = false;

	AngleVectors( cg->refdefViewAngles, vfwd, vright, vup );

	//VectorSub( pt, cg->refdef.viewOrg, local );

	local = pt - cg->refdef.viewOrg;

	transformed[ 0 ] = DotProduct( local, vright );
	transformed[ 1 ] = DotProduct( local, vup );
	transformed[ 2 ] = DotProduct( local, vfwd );

	if( transformed[ 2 ] < 0.01f )
	{
		if( !bOptionalSeen ) {
			return false;
		}

		bNotShown = true;
	}

	float centerX = ( ( float )cgs->glConfig.vidWidth / 2 );
	float centerY = ( ( float )cgs->glConfig.vidHeight / 2 );
	xzi = centerX / transformed[ 2 ] * ( 100.0f / cg->refdef.fovX );
	yzi = centerY / transformed[ 2 ] * ( 100.0f / cg->refdef.fovY );
	*x = centerX + ( xzi * transformed[ 0 ] ) - ( shader_width / 2.0f );
	*y = centerY - ( yzi * transformed[ 1 ] ) - ( shader_height / 2.0f );

	if( !bOptionalSeen ) {
		return true;
	}

	float limitX = ( ( float )cgs->glConfig.vidWidth - shader_width );
	float limitY = ( ( float )cgs->glConfig.vidHeight - shader_height );

	if( bNotShown )
	{
		if( *x < limitX ) {
			*x = limitX;
		} else if( *x > 0 ) {
			*x = 0;
		} else if( *y < limitY ) {
			*y = limitY;
		} else if( *y > 0 ) {
			*y = 0;
		}
	}

	if( *x > limitX ) {
		*x = limitX;
	} else if( *x < 0 ) {
		*x = 0;
	}

	if( *y > limitY ) {
		*y = limitY;
	} else if( *y < 0 ) {
		*y = 0;
	}

	return true;
}

void Hud::Draw3D()
{
	float tmpX = 0, tmpY = 0;
	Vector loc;

	alignX = HUD_LEFT;
	alignY = HUD_TOP;
	virtualSize = false;

	if( enttarget != -1 )
	{
		centity_t *ce = CG_GetEntity( enttarget );

		if( ce != NULL )
		{
			loc = ce->currentState.origin + org;
			lastOrg = loc;
		} else {
			loc = lastOrg;
		}
	} else {
		loc = org;
	}

	if( !shaderHandle && text[ 0 ] != '\0' )
	{
		height = 5;
		width = re.GetFontStringWidth( font == NULL ? facfont20 : font, text );
	}

	if( !CG_WorldToScreen( loc, &tmpX, &tmpY, always_show, width, height ) ) {
		return;
	}

	x = tmpX;
	y = tmpY;
}

#endif

void Hud::FadeThink()
{
#ifdef CGAME_DLL
	float frametime = ( float )cg->frametime;
#else
	float frametime = level.frametime * 1000.0f;
#endif

	fade_alpha_current += frametime;

	float ratio = fade_alpha_current / fade_alpha_time;

	if( ratio >= 1.0f )
	{
		fade_alpha = false;
		fade_alpha_current = 0.f;
		fade_alpha_first = false;
		alpha = fade_alpha_target;

		return;
	}

	alpha = fade_alpha_start + ( fade_alpha_target - fade_alpha_start ) * ratio;
}

void Hud::MoveThink()
{
#ifdef CGAME_DLL
	float frametime = ( float )cg->frametime;
#else
	float frametime = level.frametime * 1000.0f;
#endif

	fade_move_current += frametime;

	float ratio = fade_move_current / fade_move_time;

	if( ratio >= 1.0f )
	{
		fade_move = false;
		fade_move_current = 0.f;
		fade_move_x_first = false;
		fade_move_y_first = false;

		x = fade_move_x_target;
		y = fade_move_y_target;

		return;
	}

	x = ( short )( fade_move_x_start + ( fade_move_x_target - fade_move_x_start ) * ratio );
	y = ( short )( fade_move_y_start + ( fade_move_y_target - fade_move_y_start ) * ratio );
}

void Hud::ScaleThink()
{
#ifdef CGAME_DLL
	float frametime = ( float )cg->frametime;
#else
	float frametime = level.frametime * 1000.0f;
#endif

	fade_scale_current += frametime;

	float ratio = fade_scale_current / fade_scale_time;

	if( ratio >= 1.0f )
	{
		fade_scale = false;
		fade_scale_current = 0.f;
		width = fade_scale_w_target;
		height = fade_scale_h_target;

		return;
	}

	width = ( short )( fade_scale_w_start + ( fade_scale_w_target - fade_scale_w_start ) * ratio );
	height = ( short )( fade_scale_h_start + ( fade_scale_h_target - fade_scale_h_start ) * ratio );
}

void Hud::TimerThink()
{
	int minutes;
	float seconds;
	const char *string;
	char buffer[ 128 ];

#ifdef CGAME_DLL
	float frametime = ( float )cg->frametime;
#else
	float frametime = level.frametime * 1000.0f;
#endif

	if( fade_timer_flags & TIMER_UP ) {
		fade_time_current += 0.001f * frametime;
	} else {
		fade_time_current -= 0.001f * frametime;
	}

	if( fade_time_current <= 0.0f )
	{
		if( fade_out_time > 0.0f ) {
			alpha = 0.0f;
		}

		fade_time_current = 0.0f;
	}

	if( fade_time_current >= 60.0f )
	{
		seconds = ( float )( ( int )fade_time_current % 60 );
		minutes = ( int )( fade_time_current / 60.0f );
	}
	else
	{
		if( fade_time_current >= 30.0f ) {
			seconds = ( float )( int )fade_time_current;
		} else {
			seconds = fade_time_current;
		}

		minutes = 0;
	}

	if( fade_time_current >= 30.0f )
	{
		string = "%d:%02.0f";
	}
	else
	{
		if( fade_time_current >= 10.0f ) {
			string = "%d:%02.1f";
		} else {
			string = "%d:0%.1f";
		}
	}

	if( ( !( fade_timer_flags & TIMER_UP ) && fade_time_current * 1000.0f < fade_out_time ) ||
		( ( fade_timer_flags & TIMER_UP ) && fade_time_current * 1000.0f > fade_out_time && fade_out_time >= 0.0f ) )
	{
		alpha -= ( 2.0f - fade_time_alpha_start ) / fade_out_time * frametime;

		if( alpha < 0.0f ) {
			alpha = 0.0f;
		}
	} else {
		fade_time_alpha_start = alpha;
	}

#ifdef CGAME_DLL
	if( !shaderHandle ) {
#else
	if( !*shader ) {
#endif
		sprintf( buffer, string, minutes, seconds );

		SetText( buffer );
	} else {
		SetText( "" );
	}
}

void Hud::Think( void )
{
	if( !text.c_str() && !shader.c_str() ) {
		return;
	}

	if( fade_alpha ) {
		FadeThink();
	}

	if( fade_move ) {
		MoveThink();
	}

	if( fade_scale ) {
		ScaleThink();
	}

	if( fade_timer_flags & TIMER_ACTIVE ) {
		TimerThink();
	}

	if( alpha <= 0.0f ) {
		return;
	}

#ifdef CGAME_DLL
	if( isDimensional ) {
		Draw3D();
	}

	int vidWidth = virtualSize ? 640 : cgs->glConfig.vidWidth;
	int vidHeight = virtualSize ? 480 : cgs->glConfig.vidHeight;

	float hudX = x;
	float hudY = y;
	float hudW = width;
	float hudH = height;

	if( alignX == HUD_CENTER ) {
		hudX += vidWidth * 0.5f - hudW * 0.5f;
	} else if( alignX == HUD_RIGHT ) {
		hudX += vidWidth;
	}

	if( alignY == HUD_CENTER ) {
		hudY += vidHeight * 0.5f - hudH * 0.5f;
	} else if( alignY == HUD_BOTTOM ) {
		hudY += vidHeight;
	}

	vec4_t col = { color[ 0 ], color[ 1 ], color[ 2 ], alpha };

	cgi.R_SetColor( col );

	if( shaderHandle == NULL )
	{
		const char *Localized = cgi.LV_ConvertString( text );
		fontHeader_t **f = ( fontHeader_t ** )0x302B7D28;

		if( font == NULL && *f == NULL ) {
			return;
		}

		cgi.R_DrawString( font == NULL ? *f : font, Localized, hudX, hudY, -1, virtualSize );
	}
	else
	{
		if( virtualSize )
		{
			hudX *= cgs->screenXScale;
			hudY *= cgs->screenYScale;
			hudW *= cgs->screenXScale;
			hudH *= cgs->screenYScale;
		}

		cgi.R_DrawStretchPic( hudX, hudY, hudW, hudH, 0.0f, 0.0f, 1.0f, 1.0f, shaderHandle );
	}
#endif
}

void Hud::Archive( Archiver &arc )
{
	Listener::Archive( arc );
#ifdef CGAME_DLL
	arc.ArchiveInteger( ( int * )&number );

	arc.ArchiveInteger( ( int * )&alignX );
	arc.ArchiveInteger( ( int * )&alignY );

	arc.ArchiveFloat( &x );
	arc.ArchiveFloat( &y );
	arc.ArchiveFloat( &width );
	arc.ArchiveFloat( &height );

	arc.ArchiveVector( &color );
	arc.ArchiveFloat( &alpha );
	arc.ArchiveString( &fontName );
	arc.ArchiveString( &shader );
	arc.ArchiveString( &text );

	arc.ArchiveBool( &virtualSize );

	arc.ArchiveBool( &fade_alpha );
	arc.ArchiveBool( &fade_move );
	arc.ArchiveBool( &fade_scale );

	arc.ArchiveFloat( &fade_alpha_current );
	arc.ArchiveFloat( &fade_move_current );
	arc.ArchiveFloat( &fade_scale_current );

	arc.ArchiveFloat( &fade_alpha_time );
	arc.ArchiveFloat( &fade_move_time );
	arc.ArchiveFloat( &fade_scale_time );

	arc.ArchiveFloat( &fade_alpha_start );
	arc.ArchiveFloat( &fade_move_x_start );
	arc.ArchiveFloat( &fade_move_y_start );
	arc.ArchiveFloat( &fade_scale_w_start );
	arc.ArchiveFloat( &fade_scale_h_start );

	arc.ArchiveFloat( &fade_alpha_target );
	arc.ArchiveFloat( &fade_move_x_target );
	arc.ArchiveFloat( &fade_move_y_target );
	arc.ArchiveFloat( &fade_scale_w_target );
	arc.ArchiveFloat( &fade_scale_h_target );

	if( arc.Loading() )
	{
		if( fontName[ 0 ] != '\0' ) {
			SetFont( fontName );
		}

		if( shader[ 0 ] != '\0' ) {
			SetShader( shader, width, height );
		}

		hudElements.AddObject( this );

		hudElements.Sort( Hud::Sort );
	}
#endif
}

#ifdef GAME_DLL

int Hud::GetClient( void )
{
	return clientnum;
}

#endif

void Hud::FadeOverTime( float time )
{
	if( time <= 0.0f ) {
		return;
	}

	fade_alpha = true;
	fade_alpha_first = true;

	fade_alpha_time = time * 1000.0f;
	fade_alpha_current = 0.f;

	fade_alpha_start = alpha;
	fade_alpha_target = alpha;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_FADE );
		WriteNumber();
		gi.MSG_WriteFloat( time );
	gi.MSG_EndCGM();
#endif
}

void Hud::MoveOverTime( float time )
{
	if( time <= 0.0f ) {
		return;
	}

	fade_move = true;
	fade_move_x_first = true;
	fade_move_y_first = true;

	fade_move_time = time * 1000.0f;
	fade_move_current = 0.f;

	fade_move_x_start = x;
	fade_move_y_start = y;

	fade_move_x_target = x;
	fade_move_y_target = y;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_MOVE );
		WriteNumber();
		gi.MSG_WriteFloat( time );
	gi.MSG_EndCGM();
#endif
}

void Hud::Refresh( int clientNumber )
{
#ifdef GAME_DLL
	SetBroadcast( clientNumber );

	gi.MSG_StartCGM( CGM_HUDDRAW_ALIGN );
		WriteNumber();
		gi.MSG_WriteBits( alignX, 2 );
		gi.MSG_WriteBits( alignY, 2 );
	gi.MSG_EndCGM();

	SetBroadcast( clientNumber );

	gi.MSG_StartCGM( CGM_HUDDRAW_ALPHA );
		WriteNumber();
		gi.MSG_WriteByte( ( uchar )( alpha * 255.0f ) );
	gi.MSG_EndCGM();

	SetBroadcast( clientNumber );

	gi.MSG_StartCGM( CGM_HUDDRAW_COLOR );
		WriteNumber();
		gi.MSG_WriteByte( ( uchar )( color.x * 255.0f ) );
		gi.MSG_WriteByte( ( uchar )( color.y * 255.0f ) );
		gi.MSG_WriteByte( ( uchar )( color.z * 255.0f ) );
	gi.MSG_EndCGM();

	SetBroadcast( clientNumber );

	gi.MSG_StartCGM( CGM_HUDDRAW_FONT );
		WriteNumber();
		gi.MSG_WriteString( fontName );
	gi.MSG_EndCGM();

	SetBroadcast( clientNumber );

	/*if( sv_reborn->integer )
	{
		gi.MSG_StartCGM( CGM_HUDDRAW_RECTX );
			WriteNumber();
			gi.MSG_WriteShort( ( short )x );
		gi.MSG_EndCGM();

		gi.MSG_StartCGM( CGM_HUDDRAW_RECTY );
			WriteNumber();
			gi.MSG_WriteShort( ( short )y );
		gi.MSG_EndCGM();

		gi.MSG_StartCGM( CGM_HUDDRAW_RECTWH );
			WriteNumber();
			gi.MSG_WriteShort( ( short )width );
			gi.MSG_WriteShort( ( short )height );
		gi.MSG_EndCGM();
	}
	else
	{
		gi.MSG_StartCGM( CGM_HUDDRAW_RECT );
			WriteNumber();
			gi.MSG_WriteShort( ( short )x );
			gi.MSG_WriteShort( ( short )y );
			gi.MSG_WriteShort( ( short )width );
			gi.MSG_WriteShort( ( short )height );
		gi.MSG_EndCGM();
	}*/

	gi.MSG_StartCGM( CGM_HUDDRAW_RECT );
		WriteNumber();
		gi.MSG_WriteShort( ( short )x );
		gi.MSG_WriteShort( ( short )y );
		gi.MSG_WriteShort( ( short )width );
		gi.MSG_WriteShort( ( short )height );
	gi.MSG_EndCGM();

	SetBroadcast( clientNumber );

	gi.MSG_StartCGM( CGM_HUDDRAW_SHADER );
		WriteNumber();
		gi.MSG_WriteString( shader );
	gi.MSG_EndCGM();

	SetBroadcast( clientNumber );

	gi.MSG_StartCGM( CGM_HUDDRAW_STRING );
		WriteNumber();
		gi.MSG_WriteString( text );
	gi.MSG_EndCGM();

	SetBroadcast( clientNumber );

	gi.MSG_StartCGM( CGM_HUDDRAW_VIRTUALSIZE );
		WriteNumber();
		gi.MSG_WriteBits( virtualSize, 1 );
	gi.MSG_EndCGM();

	if( sv_reborn->integer )
	{
		if( isDimensional )
		{
			SetBroadcast();

			gi.MSG_StartCGM( CGM_HUDDRAW_3D );
				WriteNumber();
				gi.MSG_WriteCoord( org[ 0 ] );
				gi.MSG_WriteCoord( org[ 1 ] );
				gi.MSG_WriteCoord( org[ 2 ] );

				gi.MSG_WriteShort( enttarget );

				gi.MSG_WriteBits( !!always_show, 1 );
				gi.MSG_WriteBits( !!depth, 1 );
			gi.MSG_EndCGM();
		}

		if( fade_alpha )
		{
			SetBroadcast();

			gi.MSG_StartCGM( CGM_HUDDRAW_FADE );
				WriteNumber();
				gi.MSG_WriteFloat( fade_alpha_current );
			gi.MSG_EndCGM();

			SetBroadcast();

			gi.MSG_StartCGM( CGM_HUDDRAW_ALPHA );
				WriteNumber();
				gi.MSG_WriteByte( ( uchar )( fade_alpha_target * 255.0f ) );
			gi.MSG_EndCGM();
		}

		if( fade_move )
		{
			SetBroadcast();

			gi.MSG_StartCGM( CGM_HUDDRAW_MOVE );
				WriteNumber();
				gi.MSG_WriteFloat( fade_move_current );
			gi.MSG_EndCGM();

			SetBroadcast();

			gi.MSG_StartCGM( CGM_HUDDRAW_RECT );
				WriteNumber();
				gi.MSG_WriteShort( ( short )fade_move_x_target );
				gi.MSG_WriteShort( ( short )fade_move_y_target );
			gi.MSG_EndCGM();
		}

		if( fade_timer_flags & TIMER_ACTIVE )
		{
			SetBroadcast();

			gi.MSG_StartCGM( CGM_HUDDRAW_TIMER );
				WriteNumber();
				gi.MSG_WriteFloat( fade_time_current );
				gi.MSG_WriteFloat( fade_out_time );

				if( fade_timer_flags & TIMER_UP ) {
					gi.MSG_WriteBits( 1, 1 );
				} else {
					gi.MSG_WriteBits( 0, 1 );
				}

			gi.MSG_EndCGM();
		}
	}
#endif
}

void Hud::ScaleOverTime( float time, short w, short h )
{
	if( time <= 0.0f )
	{
		width = w;
		height = h;

		return;
	}

	fade_scale = true;

	fade_scale_time = time * 1000.0f;
	fade_scale_current = 0.f;

	fade_scale_w_start = width;
	fade_scale_h_start = height;

	fade_scale_w_target = w;
	fade_scale_h_target = h;

#ifdef GAME_DLL
	width = w;
	height = h;

	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_SCALE );
		WriteNumber();
		gi.MSG_WriteFloat( time );
		gi.MSG_WriteShort( w );
		gi.MSG_WriteShort( h );
	gi.MSG_EndCGM();
#endif
}

#ifdef GAME_DLL
void Hud::SetBroadcast( int clientNumber )
{
	if( clientNumber == -1 ) {
		clientNumber = clientnum;
	}

	if( clientnum == -1 ) {
		gi.SetBroadcastAll();
	} else {
		gi.MSG_SetClient( clientnum );
	}
}
#endif

#ifdef GAME_DLL

void Hud::WriteNumber()
{
	if( sv_reborn->integer ) {
		gi.MSG_WriteShort( number );
	} else {
		gi.MSG_WriteByte( number );
	}
}

#endif

void Hud::Set3D( Vector vector_or_offset, qboolean alwaysOnScreen, qboolean hasDepth, int entnum )
{
	org = vector_or_offset;
	lastOrg = vector_or_offset;
	always_show = alwaysOnScreen;
	depth = hasDepth;
	enttarget = entnum;

	isDimensional = true;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_3D );
		WriteNumber();
		gi.MSG_WriteCoord( vector_or_offset[ 0 ] );
		gi.MSG_WriteCoord( vector_or_offset[ 1 ] );
		gi.MSG_WriteCoord( vector_or_offset[ 2 ] );

		gi.MSG_WriteShort( entnum );

		gi.MSG_WriteBits( !!alwaysOnScreen, 1 );
		gi.MSG_WriteBits( !!hasDepth, 1 );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetNon3D()
{
	isDimensional = false;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_BREAK3D );
		WriteNumber();
	gi.MSG_EndCGM();
#endif
}

void Hud::SetAlignX( hudAlign_t align )
{
	if( align >= HUD_INVALID ) {
		ScriptError( "Wrong %d X align!", align );
	}

	alignX = align;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_ALIGN );
		WriteNumber();
		gi.MSG_WriteBits( alignX, 2 );
		gi.MSG_WriteBits( alignY, 2 );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetAlignY( hudAlign_t align )
{
	if( align >= HUD_INVALID ) {
		ScriptError( "Wrong %d Y align!", align );
	}

	alignY = align;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_ALIGN );
		WriteNumber();
		gi.MSG_WriteBits( alignX, 2 );
		gi.MSG_WriteBits( alignY, 2 );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetAlpha( float value )
{
//#ifdef CGAME_DLL
	if( fade_alpha_first )
	{
		fade_alpha_target = value;
		fade_alpha_first = false;
	}
	else
	{
		alpha = value;
		fade_alpha = false;
	}
//#else
//	alpha = value;
//#endif

#ifdef GAME_DLL
	long int ucharvalue;
	ucharvalue = ( long int )( value * 255.0f );

	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_ALPHA );
		WriteNumber();
		gi.MSG_WriteByte( ucharvalue );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetColor( Vector c )
{
	color = c;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_COLOR );
		WriteNumber();
		gi.MSG_WriteByte( ( uchar )( c.x * 255.0f ) );
		gi.MSG_WriteByte( ( uchar )( c.y * 255.0f ) );
		gi.MSG_WriteByte( ( uchar )( c.z * 255.0f ) );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetClient( int c, qboolean clears )
{
#ifdef GAME_DLL
	if( c == -1 ) {
		clears = false;
	}

	if( clears ) {
#ifdef GAME_DLL
		SetBroadcast();

		gi.MSG_StartCGM( CGM_HUDDRAW_ALPHA );
			WriteNumber();
			gi.MSG_WriteByte( 0 );
		gi.MSG_EndCGM();
#endif
	}

	clientnum = c;
#endif
}

void Hud::SetFont( const char * f )
{
	if( f != NULL && strlen( f ) > 0 ) {
		fontName = f;
	} else {
		fontName = "";
	}

#ifdef CGAME_DLL
	if( fontName[ 0 ] != '\0' ) {
		font = cgi.R_LoadFont( fontName );
	} else {
		font = NULL;
	}

#endif

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_FONT );
		WriteNumber();
		gi.MSG_WriteString( f );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetRectX( short value )
{
//#ifdef CGAME_DLL
	if( fade_move_x_first )
	{
		fade_move_x_target = value;
		fade_move_x_first = false;
	}
	else
	{
		x = value;
		fade_move = false;
	}
//#else
//	x = value;
//#endif

#ifdef GAME_DLL
	SetBroadcast();

	if( sv_reborn->integer )
	{
		gi.MSG_StartCGM( CGM_HUDDRAW_RECTX );
			WriteNumber();
			gi.MSG_WriteShort( ( short )value );
		gi.MSG_EndCGM();
	}
	else
	{
		gi.MSG_StartCGM( CGM_HUDDRAW_RECT );
			WriteNumber();
			gi.MSG_WriteShort( ( short )fade_move_x_target );
			gi.MSG_WriteShort( ( short )fade_move_y_target );
			gi.MSG_WriteShort( ( short )width );
			gi.MSG_WriteShort( ( short )height );
		gi.MSG_EndCGM();
	}
#endif
}

void Hud::SetRectY( short value )
{
//#ifdef CGAME_DLL
	if( fade_move_y_first )
	{
		fade_move_y_target = value;
		fade_move_y_first = false;
	}
	else
	{
		y = value;
		fade_move = false;
	}
//#else
//	y = value;
//#endif

#ifdef GAME_DLL
	SetBroadcast();

	if( sv_reborn->integer )
	{
		gi.MSG_StartCGM( CGM_HUDDRAW_RECTY );
			WriteNumber();
			gi.MSG_WriteShort( ( short )value );
		gi.MSG_EndCGM();
	}
	else
	{
		gi.MSG_StartCGM( CGM_HUDDRAW_RECT );
			WriteNumber();
			gi.MSG_WriteShort( ( short )fade_move_x_target );
			gi.MSG_WriteShort( ( short )fade_move_y_target );
			gi.MSG_WriteShort( ( short )width );
			gi.MSG_WriteShort( ( short )height );
		gi.MSG_EndCGM();
	}
#endif
}

void Hud::SetRectHeight( short h )
{
	height = h;
}

void Hud::SetRectWidth( short w )
{
	width = w;
}

void Hud::SetShader( const char * s, float w, float h )
{
	if( s != NULL && strlen( s ) > 0 )
	{
		shader = s;
		text = "";
	} else {
		shader = "";
	}

	fade_scale = false;

	if( w != -1 ) {
		width = w;
	}

	if( h != -1 ) {
		height = h;
	}

#ifdef CGAME_DLL
	if( shader[ 0 ] != '\0' ) {
		shaderHandle = cgi.R_RegisterShaderNoMip( shader );
	} else {
		shaderHandle = NULL;
	}
#endif

#ifdef GAME_DLL
	SetBroadcast();

	if( sv_reborn->integer )
	{
		gi.MSG_StartCGM( CGM_HUDDRAW_RECTWH );
			WriteNumber();
			gi.MSG_WriteShort( ( short )width );
			gi.MSG_WriteShort( ( short )height );
		gi.MSG_EndCGM();
	}
	else
	{
		gi.MSG_StartCGM( CGM_HUDDRAW_RECT );
			WriteNumber();
			gi.MSG_WriteShort( ( short )x );
			gi.MSG_WriteShort( ( short )y );
			gi.MSG_WriteShort( ( short )width );
			gi.MSG_WriteShort( ( short )height );
		gi.MSG_EndCGM();
	}

	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_SHADER );
		WriteNumber();
		gi.MSG_WriteString( s );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetText( const char * t )
{
	if( t != NULL && strlen( t ) > 0 )
	{
		//width = strlen( t );
		//height = 32;
		text = t;
		shader = "";
#ifdef CGAME_DLL
		shaderHandle = NULL;
#endif
	} else {
		text = "";
	}

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_STRING );
		WriteNumber();
		gi.MSG_WriteString( t );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetTimer( float time, float fade_at_time )
{
	fade_time_current = time;

	if( fade_at_time >= 0.0f ) {
		fade_out_time = fade_at_time * 1000.0f;
	} else {
		fade_out_time = -1.0f;
	}

	fade_time_target = 0.0f;

	fade_timer_flags = TIMER_ACTIVE;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_TIMER );
		WriteNumber();
		gi.MSG_WriteFloat( time );
		gi.MSG_WriteFloat( fade_at_time );
		gi.MSG_WriteBits( 0, 1 );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetTimerUp( float time, float fade_at_time )
{
	fade_time_current = 0.0f;

	if( fade_at_time >= 0.0f ) {
		fade_out_time = fade_at_time * 1000.0f;
	} else {
		fade_out_time = -1.0f;
	}

	fade_time_target = time;

	fade_timer_flags = TIMER_ACTIVE | TIMER_UP;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_TIMER );
		WriteNumber();
		gi.MSG_WriteFloat( time );
		gi.MSG_WriteFloat( fade_at_time );
		gi.MSG_WriteBits( 1, 1 );
	gi.MSG_EndCGM();
#endif
}

void Hud::SetVirtualSize( qboolean v )
{
	virtualSize = !!v;

#ifdef GAME_DLL
	SetBroadcast();

	gi.MSG_StartCGM( CGM_HUDDRAW_VIRTUALSIZE );
		WriteNumber();
		gi.MSG_WriteBits( virtualSize, 1 );
	gi.MSG_EndCGM();
#endif
}

// Events

void Hud::EventGetAlignX( Event * ev )
{
	switch (alignX)
	{
	case HUD_LEFT:
		return ev->AddString("left");

	case HUD_CENTER:
		return ev->AddString("center");

	case HUD_RIGHT:
		return ev->AddString("right");

	default:
		return ev->AddString("none");
	}
}

void Hud::EventGetAlignY( Event * ev )
{
	switch (alignY)
	{
	case HUD_TOP:
		return ev->AddString("top");

	case HUD_CENTER:
		return ev->AddString("center");

	case HUD_BOTTOM:
		return ev->AddString("bottom");

	default:
		return ev->AddString("none");
	}
}

void Hud::EventGetAlpha( Event * ev )
{
	ev->AddFloat( alpha );
}

void Hud::EventGetColor( Event * ev )
{
	ev->AddVector( color );
}

void Hud::EventGetFont( Event * ev )
{
	ev->AddString( fontName );
}

void Hud::EventGetHeight( Event * ev )
{
	ev->AddFloat( height );
}

void Hud::EventGetRectX( Event * ev )
{
	ev->AddFloat( x );
}

void Hud::EventGetRectY( Event * ev )
{
	ev->AddFloat( y );
}

void Hud::EventGetTime( Event *ev )
{
	ev->AddFloat( fade_time_current );
}

void Hud::EventGetWidth( Event * ev )
{
	ev->AddFloat( width );
}

void Hud::EventFadeOverTime( Event * ev )
{
	FadeOverTime( ev->GetFloat( 1 ) );
}

void Hud::EventMoveOverTime( Event * ev )
{
	MoveOverTime( ev->GetFloat( 1 ) );
}

void Hud::EventRefresh( Event * ev )
{
	Refresh();
}

void Hud::EventScaleOverTime( Event * ev )
{
	ScaleOverTime( ev->GetFloat( 1 ), ev->GetInteger( 2 ), ev->GetInteger( 3 ) );
}

void Hud::EventSet3D( Event *ev )
{
	Vector vector;
	int ent_num = -1;
	qboolean bAlwaysShow, bDepth;

	vector = ev->GetVector( 1 );

	bAlwaysShow = ev->GetInteger( 2 );
	bDepth = ev->GetInteger( 3 );

	if( ev->NumArgs() > 3 )
	{
		Entity *entity = ev->GetEntity( 4 );
		ent_num = entity->entnum;
	}

	Set3D( vector, bAlwaysShow, bDepth, ent_num );
}

void Hud::EventSetNon3D( Event *ev )
{
	SetNon3D();
}

void Hud::EventSetAlignX( Event * ev )
{
	str name = ev->GetString( 1 );
	hudAlign_t alignment = HUD_INVALID;

	if( name == "left" ) {
		alignment = HUD_LEFT;
	}
	else if( name == "center" ) {
		alignment = HUD_CENTER;
	}
	else if( name == "right" ) {
		alignment = HUD_RIGHT;
	} else {
		ScriptError( "Invalid alignment %s!\n", name.c_str() );
	}

	SetAlignX( alignment );
}

void Hud::EventSetAlignY( Event * ev )
{
	str name = ev->GetString( 1 );
	hudAlign_t alignment = HUD_INVALID;

	if( name == "top" ) {
		alignment = HUD_TOP;
	}
	else if( name == "center" ) {
		alignment = HUD_CENTER;
	}
	else if( name == "bottom" ) {
		alignment = HUD_BOTTOM;
	} else {
		ScriptError( "Invalid alignment %s!\n", name.c_str() );
	}

	SetAlignY( alignment );
}

void Hud::EventSetAlpha( Event * ev )
{
	float a = ev->GetFloat( 1 );

	SetAlpha( a );
}

void Hud::EventSetColor( Event * ev )
{
	Vector c = ev->GetVector( 1 );

	SetColor( c );
}

void Hud::EventSetFont( Event * ev )
{
	str font = ev->GetString( 1 );

	SetFont( font );
}

void Hud::EventSetPlayer( Event * ev )
{
#ifdef GAME_DLL
	int clientNum = -1;
	qboolean clears = false;

	if( !ev->IsNilAt( 1 ) )
	{
		Player *player = ( Player * )ev->GetEntity( 1 );

		clientNum = player->edict - g_entities;

		if( ev->NumArgs() > 1 ) {
			clears = ev->GetBoolean( 2 );
		} else {
			clears = false;
		}
	}

	SetClient( clientNum, clears );
#endif
}

void Hud::EventSetRectX( Event * ev )
{
	int val = ev->GetInteger( 1 );

	SetRectX( val );
}

void Hud::EventSetRectY( Event * ev )
{
	int val = ev->GetInteger( 1 );

	SetRectY( val );
}

void Hud::EventSetShader( Event * ev )
{
	str shader = ev->GetString( 1 );
	float w = ev->GetFloat( 2 );
	float h = ev->GetFloat( 3 );

	SetShader( shader, w, h );
}

void Hud::EventSetText( Event * ev )
{
	str text = ev->GetString( 1 );

	SetText( text );
}

void Hud::EventSetTimer( Event *ev )
{
	float time = ev->GetFloat( 1 );
	float fade_at_time = -1.0f;

	if( ev->NumArgs() > 1 ) {
		fade_at_time = ev->GetFloat( 2 );
	}

	SetTimer( time, fade_at_time );
}

void Hud::EventSetTimerUp( Event *ev )
{
	float time = ev->GetFloat( 1 );
	float fade_at_time = -1.0f;

	if( ev->NumArgs() > 1 ) {
		fade_at_time = ev->GetFloat( 2 );
	}

	SetTimerUp( time, fade_at_time );
}

void Hud::EventSetVirtualSize( Event * ev )
{
	qboolean bVirtual = ev->GetBoolean( 1 );

	SetVirtualSize( bVirtual );
}

CLASS_DECLARATION( Listener, Hud, NULL )
{
	{ &EV_HUD_GetAlignX,				&Hud::EventGetAlignX },
	{ &EV_HUD_GetAlignY,				&Hud::EventGetAlignY },
	{ &EV_HUD_GetAlpha,					&Hud::EventGetAlpha },
	{ &EV_HUD_GetColor,					&Hud::EventGetColor },
	{ &EV_HUD_GetFont,					&Hud::EventGetFont },
	{ &EV_HUD_GetHeight,				&Hud::EventGetHeight },
	{ &EV_HUD_GetRectX,					&Hud::EventGetRectX },
	{ &EV_HUD_GetRectY,					&Hud::EventGetRectY },
	{ &EV_HUD_GetTime,					&Hud::EventGetTime },
	{ &EV_HUD_GetWidth,					&Hud::EventGetWidth },
	{ &EV_HUD_FadeOverTime,				&Hud::EventFadeOverTime },
	{ &EV_HUD_MoveOverTime,				&Hud::EventMoveOverTime },
	{ &EV_HUD_Refresh,					&Hud::EventRefresh },
	{ &EV_HUD_ScaleOverTime,			&Hud::EventScaleOverTime },
	{ &EV_HUD_Set3D,					&Hud::EventSet3D },
	{ &EV_HUD_SetNon3D,					&Hud::EventSetNon3D },
	{ &EV_HUD_SetAlignX,				&Hud::EventSetAlignX },
	{ &EV_HUD_SetAlignY,				&Hud::EventSetAlignY },
	{ &EV_HUD_SetAlpha,					&Hud::EventSetAlpha },
	{ &EV_HUD_SetColor,					&Hud::EventSetColor },
	{ &EV_HUD_SetFont,					&Hud::EventSetFont },
	{ &EV_HUD_SetPlayer,				&Hud::EventSetPlayer },
	{ &EV_HUD_SetRectX,					&Hud::EventSetRectX },
	{ &EV_HUD_SetRectY,					&Hud::EventSetRectY },
	{ &EV_HUD_SetShader,				&Hud::EventSetShader },
	{ &EV_HUD_SetText,					&Hud::EventSetText },
	{ &EV_HUD_SetTimer,					&Hud::EventSetTimer },
	{ &EV_HUD_SetTimerUp,				&Hud::EventSetTimerUp },
	{ &EV_HUD_SetVirtualSize,			&Hud::EventSetVirtualSize },
	{ NULL, NULL }
};
