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

// scripttimer.cpp: Scripted Timer & Fader
//

#ifndef __SCRIPTTIMER_H__
#define __SCRIPTTIMER_H__

#include "listener.h"

typedef enum timertype_s {
	TIMER_NORMAL,
	TIMER_GLIDE
} timertype_e;

class ScriptTimer : public Listener
{
private:
	float			targetTime;
	float			currentTime;
	float			realTime;

	float			glideRatio;

	timertype_e		timerType;

	bool			bEnabled;

private:
	void		GlideRefresh();

public:
	CLASS_PROTOTYPE( ScriptTimer );

	ScriptTimer( timertype_e type = TIMER_NORMAL );
	virtual ~ScriptTimer();

	virtual void	Archive( Archiver &arc );

	void		Think( Event *ev );

	void		Disable();
	void		Enable();

	qboolean	Done();

	float		GetCurrentTime();
	float		GetRatio();
	float		GetTime();

	float		LerpValue( float start, float end );
	Vector		LerpValue( Vector start, Vector end );

	void		Reset();
	void		SetCurrentTime( float time );
	void		SetPhase( float phase );
	void		SetTime( float time );

	bool		isEnabled();
	void		setType( timertype_e type );
};

#endif /* __SCRIPTTIMER_H__ */
