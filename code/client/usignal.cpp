/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "../uilib/ui_local.h"

UConnection::UConnection()
{
}

UConnection::UConnection(const Event& inevent, const Event& outevent)
	: m_inevent(inevent)
	, m_outevent(outevent)
{
}

bool UConnection::TypeIs
	(
	Event& ev
	)

{
	return ev.eventnum == m_inevent.eventnum;
}

bool UConnection::AddListener
	(
	Listener *object,
	Event& ev
	)

{
	Event *e;
	SafePtr< Listener > ptr;

	if( !object )
		return false;

	ptr = object;

	// don't add the same listener
	if (m_listeners.ObjectInList(ptr))
	{
		return false;
	}

	m_listeners.AddObject( ptr );

	e = new Event( ev );
	m_events.AddObject( e );

	return true;
}

bool UConnection::RemoveListener
	(
	Listener *object
	)

{
	int i;
	ListenerPtr ptr;

	if( !object )
		return false;

	ptr = object;

	if( !m_listeners.ObjectInList( ptr ) )
	{
		return false;
	}

	i = m_listeners.IndexOfObject( ptr );
	delete m_events.ObjectAt( i );
	m_events.RemoveObjectAt( i );
	m_listeners.RemoveObjectAt( i );

	return true;
}

bool UConnection::SendEvent
	(
	Listener *object,
	Event& ev
	)

{
	int n;
	int i;
	int j;
	bool sent;
	Listener *ptr;
	int numargs;

	sent = false;
	n = m_listeners.NumObjects();
	numargs = ev.NumArgs();

	for( i = n; i > 0; i-- )
	{
		ptr = m_listeners.ObjectAt( i );
		if( ptr )
		{
			Event *e = new Event( *m_events.ObjectAt( i ) );

			for( j = 1; j < numargs; j++ )
			{
				e->AddToken( ev.GetToken( j ) );
			}

			ptr->ProcessEvent( e );
			sent = true;
		}
		else
		{
			Event *e = m_events.ObjectAt( i );
			delete e;

			m_listeners.RemoveObjectAt( i );
			m_events.RemoveObjectAt( i );
		}
	}

	return sent;
}

CLASS_DECLARATION( Listener, USignal, NULL )
{
	{ NULL, NULL }
};

bool USignal::SendSignal
	(
	Event& ev
	)

{
	int i;
	int n;
	UConnection *c;

	n = m_connections.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		c = m_connections.ObjectAt( i );
		if( c->TypeIs( ev ) )
		{
			return c->SendEvent( this, ev );
		}
	}

	return false;
}

bool USignal::Connect
	(
	Listener *object,
	Event& inevent,
	Event& outevent
	)

{
	int i;
	int n;
	UConnection *c;

	n = m_connections.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		c = m_connections.ObjectAt( i );
		if( c->TypeIs( inevent ) )
		{
			return c->AddListener( object, outevent );
		}
	}

	c = new UConnection(inevent, outevent);
	m_connections.AddObject( c );

	return c->AddListener( object, outevent );
}

bool USignal::Disconnect
	(
	Listener *object,
	Event& ev
	)

{
	int i;
	int n;
	UConnection *c;

	n = m_connections.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		c = m_connections.ObjectAt( i );
		if( c->TypeIs( ev ) )
		{
			return c->RemoveListener( object );
		}
	}

	return false;
}

bool USignal::Disconnect
	(
	Listener *object
	)

{
	int i;
	int n;
	bool result;
	UConnection *c;

	result = false;

	n = m_connections.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		c = m_connections.ObjectAt( i );
		if( c->RemoveListener( object ) )
		{
			result = true;
		}
	}

	return result;
}
