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

// hud.h: New HUD handler for MoHAA
//

#ifndef __HUD_H__
#define __HUD_H__

#include "listener.h"
#include "container.h"

#define TIMER_ACTIVE			(1<<0)
#define TIMER_UP				(1<<1)

typedef enum hudAlign_s
{
	HUD_LEFT		= 0,
	HUD_CENTER		= 1,
	HUD_RIGHT		= 2,
	HUD_TOP			= 0,
	HUD_BOTTOM		= 2,
	HUD_INVALID		= 3
} hudAlign_t;

class Hud : public Listener
{
private:
	unsigned int	number;					// auto-assigned number
#ifdef GAME_DLL
	char			clientnum;				// assigned client number
#else
	fontHeader_t	*font;
	qhandle_t		shaderHandle;
#endif
	hudAlign_t		alignX, alignY;
	float			x, y;
	float			width, height;
	Vector			color;
	float			alpha;
	str				fontName;
	str				shader;
	str				text;
	bool			virtualSize;
	bool			isDimensional;

	bool			fade_alpha;
	bool			fade_move;
	bool			fade_scale;
	int				fade_timer_flags;

	bool			fade_alpha_first;
	int				fade_move_x_first;
	int				fade_move_y_first;

	float			fade_alpha_current;
	float			fade_move_current;
	float			fade_scale_current;
	float			fade_time_current;

	float			fade_alpha_time;
	float			fade_move_time;
	float			fade_scale_time;
	float			fade_time;
	float			fade_out_time;

	float			fade_alpha_start;
	float			fade_move_x_start;
	float			fade_move_y_start;
	float			fade_scale_w_start;
	float			fade_scale_h_start;
	float			fade_time_alpha_start;

	float			fade_alpha_target;
	float			fade_move_x_target;
	float			fade_move_y_target;
	float			fade_scale_w_target;
	float			fade_scale_h_target;
	float			fade_time_target;

	Vector			org;
	Vector			lastOrg;
	qboolean		always_show;
	qboolean		depth;
	int				enttarget;

#ifdef GAME_DLL
	void		SetBroadcast( int clientNumber = -1 );	// Broadcast to someone or everyone
#endif

private:
#ifdef GAME_DLL
	void			WriteNumber();
#endif

public:
	CLASS_PROTOTYPE( Hud );

	void Archive( Archiver &arc ) override;

	static Hud		*Find( int index );
	static Hud		*FindOrCreate( int index );
	static int		GetFreeNumber( void );
	static void		ProcessThink( void );
	static int		Sort( const void *elem1, const void *elem2 );

#ifdef CGAME_DLL
	static void		ArchiveFunction( Archiver &arc );
#endif

#ifdef GAME_DLL
	Hud( int client = -1 );
#else
	Hud( int index = -1 );
#endif
	~Hud( void );

#ifdef CGAME_DLL
	void		Draw3D( void );
#endif

	void		FadeThink( void );
	void		MoveThink( void );
	void		ScaleThink( void );
	void		TimerThink( void );

	void		Think( void );

	void		FadeOverTime( float time );
	void		MoveOverTime( float time );
	void		ScaleOverTime( float time, short width, short height );

	void		Refresh( int clientNumber = -1 );

#ifdef GAME_DLL
	int			GetClient( void );
#endif

	void		Set3D( Vector vector_or_offset, qboolean always_show, qboolean depth, int entnum = -1 );
	void		SetNon3D( void );
	void		SetAlignX( hudAlign_t align );
	void		SetAlignY( hudAlign_t align );
	void		SetAlpha( float alpha );
	void		SetClient( int clientnum, qboolean clears = false );
	void		SetColor( Vector color );
	void		SetFont( const char * font );
	void		SetRectX( short x );
	void		SetRectY( short y );
	void		SetRectHeight( short height );
	void		SetRectWidth( short height );
	void		SetShader( const char * shader, float width, float height );
	void		SetText( const char * text );
	void		SetTimer( float time, float fade_at_time = -1.0f );
	void		SetTimerUp( float time, float fade_at_time = -1.0f );
	void		SetVirtualSize( qboolean virtualSize );

	// Events
	void		EventGetAlignX( Event * ev );
	void		EventGetAlignY( Event * ev );
	void		EventGetAlpha( Event * ev );
	void		EventGetColor( Event * ev );
	void		EventGetFont( Event * ev );
	void		EventGetHeight( Event *ev );
	void		EventGetRectX( Event * ev );
	void		EventGetRectY( Event * ev );
	void		EventGetTime( Event *ev );
	void		EventGetWidth( Event *ev );
	void		EventFadeDone( Event * ev );
	void		EventFadeOverTime( Event * ev );
	void		EventMoveDone( Event * ev );
	void		EventMoveOverTime( Event * ev );
	void		EventRefresh( Event * ev );
	void		EventScaleOverTime( Event * ev );
	void		EventSet3D( Event *ev );
	void		EventSetNon3D( Event *ev );
	void		EventSetAlignX( Event * ev );
	void		EventSetAlignY( Event * ev );
	void		EventSetAlpha( Event * ev );
	void		EventSetColor( Event * ev );
	void		EventSetFont( Event * ev );
	void		EventSetPlayer( Event * ev );
	void		EventSetRectX( Event * ev );
	void		EventSetRectY( Event * ev );
	void		EventSetShader( Event * ev );
	void		EventSetText( Event * ev );
	void		EventSetTimer( Event *ev );
	void		EventSetTimerUp( Event *ev );
	void		EventSetVirtualSize( Event * ev );
};

extern Container< Hud * > hudElements;

#endif /* __HUD_H__  */
