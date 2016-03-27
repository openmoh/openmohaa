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

CLASS_DECLARATION( UIMultiLineEdit, UINotepadEdit, NULL )
{
	{ NULL, NULL }
};

bool UI_LoadNotepadFile( const char *filename )
{
	return false;
}

UINotepadEdit::UINotepadEdit()
{
	// FIXME: stub
}

void UINotepadEdit::CharEvent
	(
	int ch
	)

{
	// FIXME: stub
}

void UINotepadEdit::setNotepad
	(
	UINotepad *notepad
	)

{
	// FIXME: stub
}

bool UINotepadEdit::GotoLine
	(
	int line
	)

{
	// FIXME: stub
	return false;
}

bool UINotepadEdit::FindText
	(
	const char *text,
	int offsetFromSel
	)

{
	// FIXME: stub
	return false;
}

void UINotepadEdit::MousePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

CLASS_DECLARATION( UIFloatingWindow, UINotepad, NULL )
{
	{ NULL, NULL }
};


UINotepad::UINotepad()
{
	// FIXME: stub
}

void UINotepad::TimeMessage
	(
	const char *message,
	int howlong
	)

{
	// FIXME: stub
}

bool UINotepad::OpenFile
	(
	const char *filename
	)

{
	// FIXME: stub
	return false;
}

bool UINotepad::Create
	(
	UIWidget *parent,
	UIRect2D& rect,
	const char *filename
	)

{
	// FIXME: stub
	return false;
}

void UINotepad::ChildSizeChanged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::SaveAs
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::Save
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::Open
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::OpenFile
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::ClosePressed
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::OnFind
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::OnGoto
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::OnCopy
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::OnPaste
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UINotepad::OnCut
	(
	Event *ev
	)

{
	// FIXME: stub
}

bool UINotepad::ProcessControlEvents
	(
	int ch
	)

{
	// FIXME: stub
	return false;
}

bool UINotepad::ProcessCharEvent
	(
	int ch
	)

{
	// FIXME: stub
	return false;
}

void UINotepad::Draw
	(
	void
	)

{
	// FIXME: stub
}

void UINotepad::setFileName
	(
	const char *filename
	)

{
	// FIXME: stub
}
