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

CLASS_DECLARATION( UIWidget, View3D, NULL )
{
	{ NULL, NULL }
};

View3D::View3D()
{
	// FIXME: stub
}

void View3D::Draw
	(
	void
	)

{
	if (cls.state != CA_DISCONNECTED) {
		SCR_DrawScreenField();
	}

	set2D();

	re.SavePerformanceCounters();

	Draw2D();
}

void View3D::DrawLetterbox
	(
	void
	)

{
	// FIXME: stub
}

void View3D::DrawFades
	(
	void
	)

{
	// FIXME: stub
}

void View3D::Draw2D
	(
	void
	)

{
	if (!cls.no_menus) {
		DrawFades();
	}

	DrawLetterbox();

	if ((cl_debuggraph->integer || cl_timegraph->integer) && !cls.no_menus)
	{
		SCR_DrawDebugGraph();
	}
	else if (!cls.no_menus)
	{
		if (cge) {
			cge->CG_Draw2D();
		}

		if (m_locationprint) {
			LocationPrint();
		} else {
			CenterPrint();
		}

		if (!cls.no_menus)
		{
			DrawSoundOverlay();
			DrawSubtitleOverlay();
		}
	}

	if (fps->integer && !cls.no_menus)
	{
		DrawFPS();
		DrawProf();
	}
}

void View3D::DrawFPS
	(
	void
	)

{
	// FIXME: stub
}

void View3D::DrawProf
	(
	void
	)

{
	// FIXME: stub
}

void View3D::PrintSound
	(
	int channel,
	const char *name,
	float vol,
	int rvol,
	float pitch,
	float base,
	int& line
	)

{
	// FIXME: stub
}

void View3D::DrawSoundOverlay
	(
	void
	)

{
	// FIXME: stub
}

void View3D::CenterPrint
	(
	void
	)

{
	// FIXME: stub
}

void View3D::LocationPrint
	(
	void
	)

{
	// FIXME: stub
}

void View3D::OnActivate
	(
	Event *ev
	)

{
	UIWidget* wid;
    UList<UIWidget*> widgets;

    UI_CloseInventory();
    cls.keyCatchers &= ~KEYCATCH_UI;

	for (wid = getParent()->getFirstChild(); wid; wid = getParent()->getNextChild(wid))
	{
		if (wid->getAlwaysOnBottom() && wid != this) {
			widgets.AddTail(wid);
		}
	}

	widgets.IterateFromHead();
	while (widgets.getCurrent())
	{
		widgets.getCurrent()->BringToFrontPropogated();
		widgets.IterateNext();
	}
}

void View3D::OnDeactivate
	(
	Event *ev
	)

{
	cls.keyCatchers |= KEYCATCH_UI;
}

void View3D::DrawSubtitleOverlay
	(
	void
	)

{
	// FIXME: stub
}

void View3D::ClearCenterPrint
	(
	void
	)

{
	// FIXME: stub
}

void View3D::UpdateCenterPrint
	(
	const char *s,
	float alpha
	)

{
	// FIXME: stub
}

void View3D::UpdateLocationPrint
	(
	int x,
	int y,
	const char *s,
	float alpha
	)

{
	// FIXME: stub
}

qboolean View3D::LetterboxActive
	(
	void
	)

{
	return m_letterbox_active;
}

void View3D::InitSubtitle
	(
	void
	)

{
	// FIXME: stub
}

void View3D::FrameInitialized
	(
	void
	)

{
	// FIXME: stub
}

void View3D::Pressed
	(
	Event *ev
	)

{
	IN_MouseOff();
	OnActivate(ev);
}

CLASS_DECLARATION( UIWidget, ConsoleView, NULL )
{
	{ NULL, NULL }
};

void ConsoleView::Draw
	(
	void
	)

{
	// FIXME: stub
}

