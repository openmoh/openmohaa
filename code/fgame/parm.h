/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// parm.h: Event parameters

#ifndef __PARM_H__
#define __PARM_H__

#include "listener.h"

class Parm : public Listener {
public:
	// General trigger variables
	SafePtr< Listener > other;
	SafePtr< Listener > owner;

	// Failure variables
	qboolean	movedone;
	qboolean	movefail;
	qboolean	motionfail;
	qboolean	upperfail;
	qboolean	sayfail;

public:
	CLASS_PROTOTYPE( Parm );

	void Archive( Archiver& arc ) override;

	void				GetOther( Event *ev );
	void				GetOwner( Event *ev );
	void				GetPreviousThread( Event *ev );
};

extern Parm parm;

#endif /* __PARM_H__ */
