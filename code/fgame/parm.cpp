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

// parm.cpp : Event parameters

#include "glb_local.h"
#include "parm.h"
#include "scriptmaster.h"
#include "scriptthread.h"
#include "scriptvm.h"

Parm parm;

Event EV_Parm_GetOther
(
	"other",
	EV_DEFAULT,
	NULL,
	NULL,
	"other",
	EV_GETTER
);

Event EV_Parm_GetOwner
(
	"owner",
	EV_DEFAULT,
	NULL,
	NULL,
	"owner",
	EV_GETTER
);

Event EV_Parm_GetPreviousThread
(
	"previousthread",
	EV_DEFAULT,
	NULL,
	NULL,
	"previousthread",
	EV_GETTER
);

void Parm::Archive( Archiver& arc )
{
	Listener::Archive( arc );

	arc.ArchiveSafePointer( &other );
	arc.ArchiveSafePointer( &owner );
}

void Parm::GetOther( Event *ev )
{
	ev->AddListener( other );
}

void Parm::GetOwner( Event *ev )
{
	ev->AddListener( owner );
}

void Parm::GetPreviousThread( Event *ev )
{
	ev->AddListener( Director.PreviousThread() );
}

CLASS_DECLARATION( Listener, Parm, NULL )
{
	{ &EV_Parm_GetOther,				&Parm::GetOther },
	{ &EV_Parm_GetOwner,				&Parm::GetOwner },
	{ &EV_Parm_GetPreviousThread,		&Parm::GetPreviousThread },
	{ NULL, NULL }
};
