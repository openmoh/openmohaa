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

CLASS_DECLARATION( UIWidget, UIConsole, NULL )
{
	{ NULL, NULL }
};

UIConsole::UIConsole()
{

}

int UIConsole::getFirstItem
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

int UIConsole::getNextItem
	(
	int prev
	)

{
	// FIXME: stub
	return 0;
}

int UIConsole::getLastItem
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

int UIConsole::AddLine
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

void UIConsole::DrawBottomLine
	(
	void
	)

{
	// FIXME: stub
}

void UIConsole::AddHistory
	(
	void
	)

{
	// FIXME: stub
}

void UIConsole::Print
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIConsole::KeyEnter
	(
	void
	)

{
	// FIXME: stub
}

void UIConsole::setConsoleHandler
	(
	consoleHandler_t handler
	)

{
	// FIXME: stub
}

void UIConsole::AddText
	(
	const char *text,
	UColor *pColor
	)

{
	// FIXME: stub
}

void UIConsole::CalcLineBreaks
	(
	item& theItem
	)

{
	// FIXME: stub
}

void UIConsole::Clear
	(
	void
	)

{
	// FIXME: stub
}

void UIConsole::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UIConsole::Draw
	(
	void
	)

{
	// FIXME: stub
}

void UIConsole::CharEvent
	(
	int ch
	)

{
	// FIXME: stub
}

qboolean UIConsole::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: stub
	return qfalse;
}

void UIConsole::OnSizeChanged
	(
	Event *ev
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UIFloatingWindow, UIFloatingConsole, NULL )
{
	{ NULL, NULL }
};

UIFloatingConsole::UIFloatingConsole()
{
	// FIXME: stub
}

void UIFloatingConsole::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UIFloatingConsole::OnChildSizeChanged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingConsole::AddText
	(
	const char *text,
	UColor *pColor
	)

{
	// FIXME: stub
}

void UIFloatingConsole::setConsoleHandler
	(
	consoleHandler_t handler
	)

{
	// FIXME: stub
}

void UIFloatingConsole::Clear
	(
	void
	)

{
	// FIXME: stub
}

void UIFloatingConsole::OnClosePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingConsole::setConsoleBackground
	(
	const UColor& color,
	float alpha
	)

{
	// FIXME: stub
}

void UIFloatingConsole::setConsoleColor
	(
	const UColor& color
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UIConsole, UIDMConsole, NULL )
{
	{ NULL, NULL }
};

UIDMConsole::UIDMConsole()
{
	// FIXME: stub
}

void UIDMConsole::KeyEnter
	(
	void
	)

{
	// FIXME: stub
}

void UIDMConsole::AddDMMessageText
	(
	const char *text,
	UColor *pColor
	)

{
	// FIXME: stub
}

void UIDMConsole::Draw
	(
	void
	)

{
	// FIXME: stub
}

qboolean UIDMConsole::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: stub
	return qfalse;
}

qboolean UIDMConsole::GetQuickMessageMode
	(
	void
	)

{
	// FIXME: stub
	return qfalse;
}

void UIDMConsole::SetQuickMessageMode
	(
	qboolean bQuickMessage
	)

{
	// FIXME: stub
}

int UIDMConsole::GetMessageMode
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

void UIDMConsole::SetMessageMode
	(
	int iMode
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UIFloatingConsole, UIFloatingDMConsole, NULL )
{
	{ NULL, NULL }
};

UIFloatingDMConsole::UIFloatingDMConsole()
{
	// FIXME: stub
}

void UIFloatingDMConsole::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UIFloatingDMConsole::OnChildSizeChanged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingDMConsole::AddText
	(
	const char *text,
	UColor *pColor
	)

{
	// FIXME: stub
}

void UIFloatingDMConsole::AddDMMessageText
	(
	const char *text,
	UColor *pColor
	)

{
	// FIXME: stub
}

void UIFloatingDMConsole::setConsoleHandler
	(
	consoleHandler_t handler
	)

{
	// FIXME: stub
}

void UIFloatingDMConsole::Clear
	(
	void
	)

{
	// FIXME: stub
}

void UIFloatingDMConsole::OnClosePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIFloatingDMConsole::setConsoleBackground
	(
	const UColor& color,
	float alpha
	)

{
	// FIXME: stub
}

void UIFloatingDMConsole::setConsoleColor
	(
	const UColor& color
	)

{
	// FIXME: stub
}

qboolean UIFloatingDMConsole::GetQuickMessageMode
	(
	void
	)

{
	// FIXME: stub
	return qfalse;
}

void UIFloatingDMConsole::SetQuickMessageMode
	(
	qboolean bQuickMessage
	)

{
	// FIXME: stub
}

int UIFloatingDMConsole::GetMessageMode
	(
	void
	)

{
	// FIXME: stub
	return 0;
}

void UIFloatingDMConsole::SetMessageMode
	(
	int iMode
	)

{
	// FIXME: stub
}
