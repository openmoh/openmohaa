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

#ifndef __USIGNAL_H__
#define __USIGNAL_H__

class UConnection {
	class Event m_inevent;
	class Event m_outevent;
	Container<SafePtr<Listener> > m_listeners;
	Container<Event *> m_events;

public:
	bool		TypeIs( Event& ev );
	bool		AddListener( Listener *object, Event& ev );
	bool		RemoveListener( Listener *object );
	bool		SendEvent( Listener *object, Event& ev );
};

class USignal : public Listener {
	Container<UConnection *> m_connections; 

public:
	CLASS_PROTOTYPE( USignal );

	bool	SendSignal( Event& ev );
	bool	Connect( Listener *object, Event& inevent, Event& outevent );
	bool	Disconnect( Listener *object, Event& ev );
	bool	Disconnect( Listener *object );
};

#endif /* __USIGNAL_H__ */
