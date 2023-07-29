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
#include "../qcommon/localization.h"

Event W_Button_Pressed
(
	"button_pressed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Signal that the button was pressed"
);

Event W_Button_HoverSound
(
	"hoversound",
	EV_DEFAULT,
	"s",
	"soundName",
	"Set the sound to play when this button is hovered over"
);

Event W_Button_HoverCommand
(
	"hovercommand",
	EV_DEFAULT,
	"s",
	"string",
	"Set the command to execute when this button is hovered over"
);

Event W_Button_MouseAwayCommand
(
    "mouseawaycommand",
    EV_DEFAULT,
    "s",
    "string",
    "Set the command to execute when the mouse exits this button"
);

CLASS_DECLARATION( UIWidget, UIButtonBase, NULL )
{
	{ &W_LeftMouseDown,				&UIButtonBase::Pressed },
	{ &W_LeftMouseUp,				&UIButtonBase::Released },
	{ &W_LeftMouseDragged,			&UIButtonBase::Dragged },
	{ &W_MouseEntered,				&UIButtonBase::MouseEntered },
	{ &W_MouseExited,				&UIButtonBase::MouseExited },
	{ &W_Button_HoverSound,			&UIButtonBase::SetHoverSound },
	{ &W_Button_HoverCommand,		&UIButtonBase::SetHoverCommand },
	{ &W_Button_MouseAwayCommand,	&UIButtonBase::SetMouseExitedCommand },
	{ NULL, NULL }
};

UIButtonBase::UIButtonBase()
{
	m_mouseState = M_NONE;
	AllowActivate(true);
}

void UIButtonBase::Pressed
	(
	Event *ev
	)

{
	m_mouseState = M_DRAGGING;
	m_hovermaterial_active = qtrue;
	m_pressedmaterial_active = qtrue;
	uWinMan.setFirstResponder(this);
}

void UIButtonBase::Released
	(
	Event *ev
	)

{
	if (m_mouseState != M_DRAGGING)
	{
		m_mouseState = M_NONE;
		return;
	}

	if (uWinMan.getFirstResponder() == this) {
		// make sure to not respond multiple times
		uWinMan.setFirstResponder(NULL);
	}

	m_mouseState = M_NONE;

	if (ev)
	{
		if (!getClientFrame().contains(MouseEventToClientPoint(ev)))
		{
			// not in the frame anymore
			m_hovermaterial_active = false;
			m_pressedmaterial_active = false;
			return;
		}
	}

	Action();

	if (ev)
	{
		if (getClientFrame().contains(MouseEventToClientPoint(ev)))
		{
			// handle the button click
			SendSignal(W_Button_Pressed);
		}
	}
}

void UIButtonBase::MouseEntered
	(
	Event *ev
	)

{
	m_hovermaterial_active = qtrue;
	uWinMan.ActivateControl(this);

	if (m_hoverSound.length()) {
		uii.Snd_PlaySound(m_hoverSound.c_str());
	}

	if (m_hoverCommand.length()) {
		str stuff = m_hoverCommand + "\n";
		uii.Cmd_Stuff(stuff.c_str());
	}
}

void UIButtonBase::MouseExited
	(
	Event *ev
	)

{
	SetHovermaterialActive(false);

	if (m_mouseExitedCommand.length()) {
        str stuff = m_mouseExitedCommand + "\n";
        uii.Cmd_Stuff(stuff.c_str());
	}
}

void UIButtonBase::Dragged
	(
	Event *ev
	)

{
	SetHovermaterialActive(getClientFrame().contains(MouseEventToClientPoint(ev)));
}

void UIButtonBase::SetHoverSound
	(
	Event *ev
	)

{
	m_hoverSound = ev->GetString(1);
}

void UIButtonBase::SetHoverCommand
	(
	Event *ev
	)

{
	m_hoverCommand = ev->GetString(1);
}

void UIButtonBase::SetMouseExitedCommand(Event* ev)
{
	m_mouseExitedCommand = ev->GetString(1);
}

void UIButtonBase::Action
	(
	void
	)

{
	if (m_clicksound.length()) {
		uii.Snd_PlaySound(m_clicksound.c_str());
	}

	if (m_command.length()) {
		str stuff = m_command + "\n";
		uii.Cmd_Stuff(stuff.c_str());
	}
}

CLASS_DECLARATION( UIButtonBase, UIButton, NULL )
{
	{ NULL, NULL }
};

UIButton::UIButton()
{
	setBorderStyle(border_none);
}

void UIButton::Draw
	(
	void
	)

{
	if (m_mouseState == M_DRAGGING && m_screenframe.contains(uid.mouseX, uid.mouseY)) {
		DrawPressed();
	} else {
		DrawUnpressed();
	}
}

void UIButton::DrawPressed
	(
	void
	)

{
	UIRect2D clientFrame = getClientFrame();

	if (m_borderStyle != border_none) {
		Draw3DBox(clientFrame, true, m_border_color, m_local_alpha);
	}

	m_font->setColor(m_foreground_color);

	clientFrame.pos.x += 1;
	clientFrame.pos.y += 1;

	m_font->PrintJustified(
		clientFrame,
		m_iFontAlignmentHorizontal,
		m_iFontAlignmentVertical,
		Sys_LV_CL_ConvertString(m_title.c_str()),
		m_bVirtual ? m_vVirtualScale : NULL
	);
}

void UIButton::DrawUnpressed
	(
	void
	)

{
	m_font->setColor(m_foreground_color);
	m_font->setAlpha(m_local_alpha);

	UIRect2D clientFrame = getClientFrame();

	m_font->PrintJustified(
		clientFrame,
		m_iFontAlignmentHorizontal,
		m_iFontAlignmentVertical,
		Sys_LV_CL_ConvertString(m_title.c_str()),
		m_bVirtual ? m_vVirtualScale : NULL
	);
}

qboolean UIButton::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	if (key != K_ENTER && key != K_KP_ENTER) {
		return false;
	}

	UIButtonBase::Action();
	return true;
}

CLASS_DECLARATION( USignal, ToggleCVar, NULL )
{
	{ NULL, NULL }
};

ToggleCVar::ToggleCVar()
{
	m_button = 0;
}

ToggleCVar::ToggleCVar
	(
	UIButton *button,
	const char *cvar
	) : ToggleCVar()

{
	setButton(button);

	if (cvar) {
		setCVar(cvar);
	}
}

void ToggleCVar::Press
	(
	Event *ev
	)

{
	if (m_cvarname.length())
	{
		UI_SetCvarInt(m_cvarname.c_str(), UI_GetCvarInt(m_cvarname.c_str(), 0));
	}
}

void ToggleCVar::setCVar
	(
	const char *cvar
	)

{
	m_cvarname = cvar;
}

void ToggleCVar::setButton
	(
	UIButton *button
	)

{
	if (m_button) {
		m_button->Disconnect(this, W_Button_Pressed);
	}

	m_button = button;
	if (m_button) {
		m_button->Connect(this, W_Button_Pressed, W_Button_Pressed);
	}
}

CLASS_DECLARATION( USignal, ExecCmd, NULL )
{
	{ NULL, NULL }
};

ExecCmd::ExecCmd()
{
	m_button = NULL;
}

ExecCmd::ExecCmd
	(
	UIButton *button,
	const char *cmd
	) : ExecCmd()

{
	setButton(button);
	setCommand(cmd);
}

void ExecCmd::Press
	(
	Event *ev
	)

{
	if (m_cmd.length()) {
		str stuff = m_cmd + "\n";
		uii.Cmd_Stuff(stuff.c_str());
	}
}

void ExecCmd::setCommand
	(
	const char *cmd
	)

{
	m_cmd = cmd;
}

void ExecCmd::setButton
	(
	UIButton *button
	)

{
	if (m_button) {
		m_button->Disconnect(this, W_Button_Pressed);
	}

	m_button = button;
	if (m_button) {
		m_button->Connect(this, W_Button_Pressed, W_Button_Pressed);
	}
}
