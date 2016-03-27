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

#include "ui_local.h"

CLASS_DECLARATION( UIWidget, UIButtonBase, NULL )
{
	{ NULL, NULL }
};

UIButtonBase::UIButtonBase()
{
	// FIXME: stub
}

void UIButtonBase::Pressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIButtonBase::Released
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIButtonBase::MouseEntered
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIButtonBase::MouseExited
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIButtonBase::Dragged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIButtonBase::SetHoverSound
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIButtonBase::SetHoverCommand
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIButtonBase::Action
	(
	void
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UIButtonBase, UIButton, NULL )
{
	{ NULL, NULL }
};

UIButton::UIButton()
{
	// FIXME: stub
}

void UIButton::Draw
	(
	void
	)

{
	// FIXME: stub
}

void UIButton::DrawPressed
	(
	void
	)

{
	// FIXME: stub
}

void UIButton::DrawUnpressed
	(
	void
	)

{
	// FIXME: stub
}

qboolean UIButton::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: stub
	return qfalse;
}

CLASS_DECLARATION( USignal, ToggleCVar, NULL )
{
	{ NULL, NULL }
};

ToggleCVar::ToggleCVar()
{
	// FIXME: stub
}

ToggleCVar::ToggleCVar
	(
	UIButton *button,
	const char *cvar
	)

{
	// FIXME: stub
}

void ToggleCVar::Press
	(
	Event *ev
	)

{
	// FIXME: stub
}

void ToggleCVar::setCVar
	(
	const char *cvar
	)

{
	// FIXME: stub
}

void ToggleCVar::setButton
	(
	UIButton *button
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( USignal, ExecCmd, NULL )
{
	{ NULL, NULL }
};

ExecCmd::ExecCmd()
{
	// FIXME: stub
}

ExecCmd::ExecCmd
	(
	UIButton *button,
	const char *cmd
	)

{
	// FIXME: stub
}

void ExecCmd::Press
	(
	Event *ev
	)

{
	// FIXME: stub
}

void ExecCmd::setCommand
	(
	const char *cmd
	)

{
	// FIXME: stub
}

void ExecCmd::setButton
	(
	UIButton *button
	)

{
	// FIXME: stub
}
