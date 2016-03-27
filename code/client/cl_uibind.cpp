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

#include "cl_ui.h"

bind_t::bind_t()
{
	// FIXME: stub
}

bind_t::~bind_t()
{
	Clear();
}

void bind_t::Clear
	(
	void
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( Listener, bindlistener, NULL )
{
	{ NULL, NULL }
};

bindlistener::bindlistener()
{
}

bindlistener::bindlistener
	(
	bind_t *b
	)

{
	bind = b;
}

bool bindlistener::Load
	(
	Script& script
	)

{
	// FIXME: stub
	return false;
}

void bindlistener::Header
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::Width
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::FillWidth
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::Height
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::CommandWidth
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::CommandHeight
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::PrimaryKeyWidth
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::PrimaryKeyHeight
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::AlternateKeyWidth
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::AlternateKeyHeight
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::NewItem
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::Align
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::TitleForegroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::TitleBackgroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::InactiveForegroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::InactiveBackgroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::ActiveForegroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::ActiveBackgroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::ActiveBorderColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::HighlightForegroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::HighlightBackgroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::SelectForegroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::SelectBackgroundColor
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::ChangeSound
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::ActiveSound
	(
	Event *ev
	)

{
	// FIXME: stub
}

void bindlistener::EnterSound
	(
	Event *ev
	)

{
	// FIXME: stub
}
