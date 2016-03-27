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

// conevent.h: TU Console.

#ifndef __CONEVENT_H__
#define __CONEVENT_H__

#include <listener.h>

class ConsoleEvent : public Listener {
public:
	CLASS_PROTOTYPE( ConsoleEvent );

	void			CMD_Help( Event *ev );
	void			CMD_Print( Event *ev );
	void			CMD_Shutdown( Event *ev );
	void			CMD_ExecScript( Event *ev );
	void			CMD_ScriptStatus( Event *ev );
	void			CMD_PrintThread( Event *ev );
};

extern ConsoleEvent g_consoleEvent;

#endif /* __CONEVENT_H__ */
