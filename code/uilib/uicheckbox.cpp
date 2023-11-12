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

Event EV_UICheckBox_SetCheckedCommand
(
	"checked_command",
	EV_DEFAULT,
	"s",
	"command",
	"Command to execute when checkbox is checked"
);

Event EV_UICheckBox_SetUncheckedCommand
(
	"unchecked_command",
	EV_DEFAULT,
	"s",
	"command",
	"Command to execute when checkbox is unchecked" // text not present in original executable
);

Event EV_UICheckBox_SetCheckedShader
(
	"checked_shader",
	EV_DEFAULT,
	"s",
	"shader",
	"Set the shader of the small button when it is checked"
);

Event EV_UICheckBox_SetUncheckedShader
(
	"unchecked_shader",
	EV_DEFAULT,
	"s",
	"shader",
	"Set the shader of the small button when it is unchecked"
);

CLASS_DECLARATION( UIWidget, UICheckBox, NULL )
{
	{&W_LeftMouseDown,						&UICheckBox::Pressed			},
	{&W_LeftMouseUp,						&UICheckBox::Released			},
	{&W_MouseEntered,						&UICheckBox::MouseEntered		},
	{&W_MouseExited,						&UICheckBox::MouseExited		},
	{&EV_UICheckBox_SetCheckedCommand,		&UICheckBox::SetCheckedCommand	},
	{&EV_UICheckBox_SetUncheckedCommand,	&UICheckBox::SetUncheckedCommand},
	{&EV_UICheckBox_SetCheckedShader,		&UICheckBox::SetCheckedShader	},
	{&EV_UICheckBox_SetUncheckedShader,		&UICheckBox::SetUncheckedShader	},
	{NULL,									NULL							}
};

UICheckBox::UICheckBox()
{
	AllowActivate(qtrue);
	m_check_height = 14.0;
	m_check_width = 14.0;
	m_indent = 8.0;
	m_depressed = qfalse;
	m_checked_material = NULL;
	m_unchecked_material = NULL;
}

void UICheckBox::Draw()
{
	UpdateData();

	if (!m_depressed)
	{
		Draw3DBox(0.0f, 0.0f, m_frame.size.width, m_frame.size.height, 0, m_border_color, m_local_alpha);
	}

	SetColor(m_foreground_color, m_local_alpha);

	UColor color = m_checked ? UBlue : UBlack;
	UIReggedMaterial* material = m_checked ? m_checked_material : m_unchecked_material;

	float width = m_check_width * m_vVirtualScale[0];
	float height = m_check_height * m_vVirtualScale[1];
	float x = m_vVirtualScale[0] * m_indent;
	float y = (m_frame.size.height - height) * 0.5f;
	if (material)
	{
		material->ReregisterMaterial();
		uii.Rend_DrawPicStretched(x, y, width, height, 0.0f, 0.0f, 1.0f, 1.0f, material->GetMaterial());
	}
	else
	{
		DrawBox(x, y, width, height, color, m_local_alpha);
	}

	float titleX = m_indent * 1.5f + m_check_width;
	float titleY = (m_frame.size.height / m_vVirtualScale[1] - getTitleHeight()) * 0.5f - 1.0f;
	return DrawTitle(titleX, titleY);
}

void UICheckBox::CharEvent(int ch)
{
	// Support ticking checkboxes with spacebar
	if (ch == ' ')
	{
		m_checked = !m_checked;
		UpdateCvar();
	}
}

void UICheckBox::Pressed(Event *ev)
{
	m_depressed = qtrue;
}

void UICheckBox::Released(Event *ev)
{
	m_checked = !m_checked;

	// Handle additional commands:
	// this is for checkboxes that either change multiple cvars,
	// or that set a cvar with a non-bool value.
	// e.g.: the Invert Mouse checkbox originally flips cvar m_invert_pitch,
	// but also does ("set m_pitch -0.022" : "set m_pitch 0.022") commands using this
	str command = m_checked ? m_checked_command : m_unchecked_command;
	Cbuf_AddText(command.c_str());

	m_depressed = qfalse;
	UpdateCvar();
}

void UICheckBox::UpdateCvar()
{
	if (m_cvarname.length())
	{
		UI_SetCvarInt(m_cvarname, m_checked);
	}

	if (m_command.length()) // always seems to be NULL, could be remnant code
	{
		const char* text_to_append = m_checked ? " 1\n" : " 0\n";
		m_command.append(text_to_append);
		Cbuf_AddText(m_command.c_str());
	}

	if (m_commandhandler) // always seems to be NULL, could be remnant code
	{
		const char *checkedInt = va("%d", m_checked);
		m_commandhandler(checkedInt, NULL);
	}
}

void UICheckBox::MouseEntered(Event *ev)
{
	auto num = ev->GetInteger(3);
	uWinMan.ActivateControl(this);
	if (num)
	{
		m_depressed = qtrue;
	}
}

void UICheckBox::MouseExited(Event *ev)
{
	m_depressed = qfalse;
}

void UICheckBox::SetCheckedCommand(Event *ev)
{
	m_checked_command = ev->GetString(1);
}

void UICheckBox::SetUncheckedCommand(Event *ev)
{
	m_unchecked_command = ev->GetString(1);
}

void UICheckBox::SetCheckedShader(Event *ev)
{
	str shader = ev->GetString(1);
	m_checked_material = uWinMan.RegisterShader(shader);
}

void UICheckBox::SetUncheckedShader(Event *ev)
{
	str shader = ev->GetString(1);
	m_unchecked_material = uWinMan.RegisterShader(shader);
}

void UICheckBox::UpdateData(void)
{
	if (m_cvarname.length())
	{
		m_checked = UI_GetCvarInt(m_cvarname.c_str(), 0);
	}
}

bool UICheckBox::isChecked()
{
	return m_checked;
}
