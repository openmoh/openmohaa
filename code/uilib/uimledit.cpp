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

CLASS_DECLARATION( UIWidget, UIMultiLineEdit, NULL )
{
	{ NULL, NULL }
};

UIMultiLineEdit::UIMultiLineEdit()
{
	// FIXME: stub
}

void UIMultiLineEdit::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::PointToSelectionPoint
	(
	UIPoint2D& p,
	selectionpoint_t& sel
	)

{
	// FIXME: stub
}

static str str_null = "";

str& UIMultiLineEdit::LineFromLineNumber
	(
	int num,
	bool resetpos
	)

{
	// FIXME: stub
	return str_null;
}

void UIMultiLineEdit::EnsureSelectionPointVisible
	(
	selectionpoint_t& point
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::BoundSelectionPoint
	(
	selectionpoint_t& point
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::SortSelection
	(
	selectionpoint_t **topsel,
	selectionpoint_t **botsel
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::UpdateCvarEvent
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::SetEdit
	(
	Event *ev
	)

{
	// FIXME: stub
}

bool UIMultiLineEdit::IsSelectionEmpty
	(
	void
	)

{
	// FIXME: stub
	return false;
}

void UIMultiLineEdit::Draw
	(
	void
	)

{
	// FIXME: stub
}

qboolean UIMultiLineEdit::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	// FIXME: stub
	return qfalse;
}

void UIMultiLineEdit::CharEvent
	(
	int ch
	)

{
	// FIXME: stub
}

UIPoint2D UIMultiLineEdit::getEndSelPoint
	(
	void
	)

{
	// FIXME: stub
	return UIPoint2D();
}

void UIMultiLineEdit::MouseDown
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::MouseUp
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::MouseDragged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::Scroll
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::DragTimer
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::SizeChanged
	(
	Event *ev
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::setData
	(
	const char *data
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::getData
	(
	str& data
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::Empty
	(
	void
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::CopySelection
	(
	void
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::PasteSelection
	(
	void
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::DeleteSelection
	(
	void
	)

{
	// FIXME: stub
}

void UIMultiLineEdit::setChanged
	(
	bool b
	)

{
	// FIXME: stub
}

bool UIMultiLineEdit::IsChanged
	(
	void
	)

{
	// FIXME: stub
	return false;
}

