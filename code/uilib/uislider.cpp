/*
===========================================================================
Copyright (C) 2015-2023 the OpenMoHAA team

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
#include <string.h>

Event EV_AutoIncrement
(
	"autoinc",
	EV_DEFAULT,
	NULL,
	NULL,
	"Used to auto increment the slider"
);

Event EV_AutoDecrement
(
	"autodec",
	EV_DEFAULT,
	NULL,
	NULL,
	"Used to auto decrement the slider"
);

Event EV_Layout_Range
(
	"setrange",
	EV_DEFAULT,
	"ff",
	"min max",
	"Used to set the range of a slider"
);

Event EV_Layout_SliderType
(
	"slidertype",
	EV_DEFAULT,
	"s",
	"sliderType(float|integer)",
	"Set the slider type to be floating point or integral"
);

Event EV_Layout_SliderStep
(
	"stepsize",
	EV_DEFAULT,
	"f",
	"stepsize",
	"Set the step size of the slider"
);

Event EV_Layout_SliderThumbShader
(
	"thumbshader",
	EV_DEFAULT,
	"sS",
	"shader depressed_shader",
	"Set the shader of the slider's thumb"
);

Event EV_Layout_SliderLeftShader
(
	"leftshader",
	EV_DEFAULT,
	"sS",
	"shader depressed_shader",
	"Set the shader of the slider's left arrow"
);

Event EV_Layout_SliderRightShader
(
	"rightshader",
	EV_DEFAULT,
	"sS",
	"shader depressed_shader",
	"Set the shader of the slider's right arrow"
);

CLASS_DECLARATION( UIWidget, UISlider, NULL )
{
	{&W_LeftMouseDown,				&UISlider::Pressed		    		},
	{&W_LeftMouseDragged,			&UISlider::MouseDragged	    		},
	{&W_LeftMouseUp,				&UISlider::Released		    		},
	{&W_MouseExited,				&UISlider::MouseExited	    		},
	{&W_MouseEntered,				&UISlider::MouseEntered	    		},
	{&EV_AutoIncrement,				&UISlider::AutoIncrement    		},
	{&EV_AutoDecrement,				&UISlider::AutoDecrement    		},
	{&EV_Layout_Range,				&UISlider::LayoutSetRange			},
	{&EV_Layout_SliderType,			&UISlider::LayoutSetType    		},
	{&EV_Layout_SliderStep,			&UISlider::LayoutSetStep    		},
	// possible FAKK2 remnants below, not used currently
	{&EV_Layout_SliderThumbShader,	&UISlider::LayoutSliderThumbShader	},
	{&EV_Layout_SliderLeftShader,	&UISlider::LayoutSliderLeftShader	},
	{&EV_Layout_SliderRightShader,	&UISlider::LayoutSliderRightShader	},
	{NULL,							NULL								}
};

UISlider::UISlider()
{
	m_sliderregion = {};
	m_prev_arrow_region = {};
	m_next_arrow_region = {};
	m_minvalue = 0.0f;
	m_maxvalue = 100.0f;
	m_value = 0.0f;
	m_oldvalue = 0.0f;
	m_sliderwidth = 10.0f;
	m_stepsize = 1.0f;
	m_slidertype = SLIDER_INTEGER;
	m_arrow_width = 15;
	m_prev_arrow_depressed = qfalse;
	m_next_arrow_depressed = qfalse;
	m_thumb_depressed = qfalse;
	m_initialized = qfalse;

	m_prev_arrow_material = uWinMan.RegisterShader("textures/menu/arrow_left");
	m_prev_arrow_material_pressed = uWinMan.RegisterShader("textures/menu/arrow_left_sel");
	m_next_arrow_material = uWinMan.RegisterShader("textures/menu/arrow_right");
	m_next_arrow_material_pressed = uWinMan.RegisterShader("textures/menu/arrow_right_sel");
	m_thumbmaterial = uWinMan.RegisterShader("textures/menu/slider_thumb");
	m_thumbmaterial_pressed = uWinMan.RegisterShader("textures/menu/slider_thumb_sel");

	// these are not initialized in the original game
	m_bar_material = NULL; // seems to be unused
	m_maxx = 0;
	m_minx = 0;
}

void UISlider::setThumb
	(
	void
	)

{
	// not present in disassembly
}

void UISlider::Pressed
	(
	Event *ev
	)

{
	float xpos = ev->GetFloat(1);
	float ypos = ev->GetFloat(2);

	if (m_prev_arrow_region.contains(xpos, ypos)) // click left arrow
	{
		Decrement();
		m_prev_arrow_depressed = qtrue;
		CancelEventsOfType(&EV_AutoDecrement);
		Event* newEv = new Event(EV_AutoDecrement);
		PostEvent(newEv, 0.2f);
		return;
	}
	else if (m_next_arrow_region.contains(xpos, ypos)) // click right arrow
	{
		Increment();
		m_next_arrow_depressed = qtrue;
		CancelEventsOfType(&EV_AutoIncrement);
		Event* newEv = new Event(EV_AutoIncrement);
		PostEvent(newEv, 0.2f);
		return;
	}

	// click thumb
	m_oldvalue = m_value;
	m_thumb_depressed = qtrue;

	if (xpos <= m_minx)
	{
		m_value = m_minvalue;
	}
	else if (xpos >= m_maxx)
	{
		m_value = m_maxvalue;
	}
	else
	{
		// calculate value based on thumb position
		m_value = (xpos - m_minx) / (m_maxx - m_minx) * (m_maxvalue - m_minvalue) + m_minvalue;
	}

	UpdateData();
}

void UISlider::Released
	(
	Event *ev
	)

{
	m_prev_arrow_depressed = qfalse;
	m_next_arrow_depressed = qfalse;
	m_thumb_depressed = qfalse;
}

void UISlider::Increment
	(
	void
	)

{
	m_value = Q_clamp_float(m_value + m_stepsize, m_minvalue, m_maxvalue);
	UpdateData();
}

void UISlider::Decrement
	(
	void
	)

{
	m_value = Q_clamp_float(m_value - m_stepsize, m_minvalue, m_maxvalue);
	UpdateData();
}

void UISlider::AutoIncrement
	(
	Event *ev
	)

{
	if (m_next_arrow_depressed)
	{
		if (m_next_arrow_region.contains(uid.mouseX, uid.mouseY))
		{
			Increment();
			Event *event = new Event(EV_AutoIncrement);
			PostEvent(event, 0.1f);
		}
		else
		{
			Released(NULL);
		}
	}
}

void UISlider::AutoDecrement
	(
	Event *ev
	)

{
	if (m_prev_arrow_depressed)
	{
		if (m_prev_arrow_region.contains(uid.mouseX, uid.mouseY))
		{
			Decrement();
			Event *event = new Event(EV_AutoDecrement);
			PostEvent(event, 0.1f);
		}
		else
		{
			Released(NULL);
		}
	}
}

void UISlider::MouseExited
	(
	Event *ev
	)

{
	if (m_thumb_depressed)
	{
		m_value = m_oldvalue;
		UpdateData();
	}

	m_thumb_depressed = qfalse;
	m_prev_arrow_depressed = qfalse;
	m_next_arrow_depressed = qfalse;
}

void UISlider::MouseDragged
	(
	Event *ev
	)

{
	float xpos = ev->GetFloat(1);
	float ypos = ev->GetFloat(2);

	if (m_thumb_depressed)
	{
		if (xpos <= m_minx)
		{
			m_value = m_minvalue;
		}
		else if (xpos >= m_maxx)
		{
			m_value = m_maxvalue;
		}
		else
		{
			// calculate value based on position
			m_value = (xpos - m_minx) / (m_maxx - m_minx) * (m_maxvalue - m_minvalue) + m_minvalue;
		}

		UpdateData();
		return;
	}

	if ((!m_prev_arrow_depressed && m_prev_arrow_region.contains(xpos, ypos))
	 || (!m_next_arrow_depressed && m_next_arrow_region.contains(xpos, ypos)))
	{
		Pressed(ev);
	}
}

void UISlider::MouseEntered
	(
	Event *ev
	)

{
	int buttons = ev->GetInteger(3);
	uWinMan.ActivateControl(this);
	if (buttons & 1) // left mouse button
	{
		Pressed(ev);
	}
}

void UISlider::UpdateData
	(
	void
	)

{
	if (m_cvarname.length())
	{
		if (m_slidertype == SLIDER_INTEGER)
		{
			UI_SetCvarInt(m_cvarname, m_value);
		}
		else
		{
			UI_SetCvarFloat(m_cvarname, m_value);
		}
	}

	if (m_command.length())
	{
		char text[32];
		if (m_slidertype == SLIDER_INTEGER)
		{
			Q_snprintf(text, 32, "%d", (int)m_value);
		}
		else
		{
			Q_snprintf(text, 32, "%.3f", m_value);
		}

		m_command.append(' ');
		m_command.append(text);
		m_command.append('\n');
		Cbuf_AddText(m_command);
	}
}

void UISlider::UpdateUIElement
	(
	void
	)

{
	m_value = UI_GetCvarFloat(m_cvarname, 0.0f);
}

void UISlider::LayoutSetRange
	(
	Event *ev
	)

{
	float min = ev->GetFloat(1);
	float max = ev->GetFloat(2);
	setRange(min, max);
}

void UISlider::LayoutSetType
	(
	Event *ev
	)

{
	str type = ev->GetString(1);
	if (!Q_stricmp(type, "float"))
	{
		setType(SLIDER_FLOAT);
	}
	else if (!Q_stricmp(type, "integer"))
	{
		setType(SLIDER_INTEGER);
	}
}

void UISlider::LayoutSetStep
	(
	Event *ev
	)

{
	m_stepsize = ev->GetFloat(1);
}

void UISlider::LayoutSliderBGShader
	(
	Event *ev
	)

{
	// not present in disassembly
}

void UISlider::LayoutSliderThumbShader
	(
	Event *ev
	)

{
	str shader = ev->GetString(1);
	m_thumbmaterial = uWinMan.RegisterShader(shader);

	if (ev->NumArgs() <= 1)
	{
		m_thumbmaterial_pressed = m_thumbmaterial;
	}
	else
	{
		str pressedShader = ev->GetString(2);
		m_thumbmaterial_pressed = uWinMan.RegisterShader(pressedShader);
	}

	m_thumbmaterial->ReregisterMaterial();
	m_sliderwidth = uii.Rend_GetShaderWidth(m_thumbmaterial->GetMaterial());
	m_initialized = qfalse;
}

void UISlider::LayoutSliderLeftShader
	(
	Event *ev
	)

{
	str shader = ev->GetString(1);
	m_prev_arrow_material = uWinMan.RegisterShader(shader);

	if (ev->NumArgs() <= 1)
	{
		m_prev_arrow_material_pressed = m_prev_arrow_material;
	}
	else
	{
		str pressedShader = ev->GetString(2);
		m_prev_arrow_material_pressed = uWinMan.RegisterShader(pressedShader);
	}

	m_prev_arrow_material->ReregisterMaterial();
	m_arrow_width = uii.Rend_GetShaderWidth(m_prev_arrow_material->GetMaterial());
	m_initialized = qfalse;
}

void UISlider::LayoutSliderRightShader
	(
	Event *ev
	)

{
	str shader = ev->GetString(1);
	m_next_arrow_material = uWinMan.RegisterShader(shader);

	if (ev->NumArgs() <= 1)
	{
		m_next_arrow_material_pressed = m_next_arrow_material;
	}
	else
	{
		str pressedShader = ev->GetString(2);
		m_next_arrow_material_pressed = uWinMan.RegisterShader(pressedShader);
	}

	m_next_arrow_material->ReregisterMaterial();
	m_arrow_width = uii.Rend_GetShaderWidth(m_next_arrow_material->GetMaterial());
	m_initialized = qfalse;
}

void UISlider::FrameInitialized
	(
	void
	)

{
	float cframeX = m_clippedframe.pos.x;
	float cframeY = m_clippedframe.pos.y;
	float cframeWidth = m_clippedframe.size.width;
	float frameHeight = m_frame.size.height;
	float scaledArrowWidth = m_arrow_width * m_vVirtualScale[0];
	float scaledThumbWidth = m_sliderwidth * m_vVirtualScale[0];

	m_prev_arrow_region.pos.x = cframeX;
	m_prev_arrow_region.pos.y = cframeY;
	m_prev_arrow_region.size.height = frameHeight;
	m_prev_arrow_region.size.width = scaledArrowWidth;

	m_next_arrow_region.pos.x = cframeX + cframeWidth - scaledArrowWidth;
	m_next_arrow_region.pos.y = cframeY;
	m_next_arrow_region.size.height = frameHeight;
	m_next_arrow_region.size.width = scaledArrowWidth;

	m_sliderregion.pos.x = cframeX + scaledArrowWidth;
	m_sliderregion.pos.y = cframeY;
	m_sliderregion.size.height = frameHeight;
	// Bug in original game, width calculation was wrong:
	//m_sliderregion.size.width = scaledThumbWidth - scaledArrowWidth;
	m_sliderregion.size.width = cframeWidth - scaledArrowWidth * 2;

	// width of the "dead zone" of the slider where the thumb cannot reach
	float unreachableEdgeWidth = scaledArrowWidth + scaledThumbWidth * 0.5f;

	m_minx = cframeX + unreachableEdgeWidth;
	m_maxx = cframeX + m_frame.size.width - unreachableEdgeWidth;
}

void UISlider::Draw
	(
	void
	)

{
	if (!m_initialized)
	{
		m_initialized = qtrue;
		FrameInitialized();
	}

	UpdateUIElement();

	// current horizontal position of the thumb
	float sliderpos = (m_value - m_minvalue) / (m_maxvalue - m_minvalue);
	sliderpos = Q_clamp_float(sliderpos, 0.0f, 1.0f);
	float scaledArrowWidth = m_arrow_width * m_vVirtualScale[0];
	float scaledThumbWidth = m_sliderwidth * m_vVirtualScale[0];
	float x = (m_maxx - m_minx) * sliderpos + scaledArrowWidth;

	// draw the thumb
	if (m_thumbmaterial)
	{
		UIReggedMaterial* material = m_thumb_depressed ? m_thumbmaterial_pressed : m_thumbmaterial;
		material->ReregisterMaterial();
		uii.Rend_DrawPicStretched(x, 0.0f, scaledThumbWidth, m_frame.size.height, 0.0f, 0.0f, 1.0f, 1.0f, material->GetMaterial());
	}
	else
	{
		Draw3DBox(x, 0.0f, scaledThumbWidth, m_frame.size.height, qfalse, m_border_color, m_local_alpha);
	}

	if (m_prev_arrow_material == m_prev_arrow_material_pressed)
	{
		Draw3DBox(0.0f, 0.0f, scaledArrowWidth, m_frame.size.height, m_prev_arrow_depressed, m_border_color, m_local_alpha);

		m_prev_arrow_material->ReregisterMaterial();
		uii.Rend_DrawPicStretched(
			2.0f,
			2.0f,
			scaledArrowWidth - 4.0f,
			m_frame.size.height - 4.0f,
			0.0f,
			0.0f,
			1.0f,
			1.0f,
			m_prev_arrow_material->GetMaterial()
		);
	}
	else
	{
		uihandle_t material;
		if (m_prev_arrow_depressed)
		{
			m_prev_arrow_material_pressed->ReregisterMaterial();
			material = m_prev_arrow_material_pressed->GetMaterial();
		}
		else
		{
			m_prev_arrow_material->ReregisterMaterial();
			material = m_prev_arrow_material->GetMaterial();
		}

		uii.Rend_DrawPicStretched(0.0f, 0.0f, scaledArrowWidth, m_frame.size.height, 0.0f, 0.0f, 1.0f, 1.0f, material);
	}

	if (m_next_arrow_material == m_next_arrow_material_pressed)
	{
		Draw3DBox(m_frame.size.width - scaledArrowWidth, 0.0f, scaledArrowWidth, m_frame.size.height, m_next_arrow_depressed, m_border_color, m_local_alpha);

		m_next_arrow_material->ReregisterMaterial();
		uii.Rend_DrawPicStretched(
			m_frame.size.width - scaledArrowWidth + 2.0f,
			2.0f,
			scaledArrowWidth - 4.0f,
			m_frame.size.height - 4.0f,
			0.0f,
			0.0f,
			1.0f,
			1.0f,
			m_next_arrow_material->GetMaterial()
		);
	}
	else
	{
		uihandle_t material;
		if (m_next_arrow_depressed)
		{
			m_next_arrow_material_pressed->ReregisterMaterial();
			material = m_next_arrow_material_pressed->GetMaterial();
		}
		else
		{
			m_next_arrow_material->ReregisterMaterial();
			material = m_next_arrow_material->GetMaterial();
		}

		uii.Rend_DrawPicStretched(
			m_frame.size.width - scaledArrowWidth,
			0.0f,
			scaledArrowWidth,
			m_frame.size.height,
			0.0f,
			0.0f,
			1.0f,
			1.0f,
			material
		);
	}
}

void UISlider::setType
	(
	slidertype_t type
	)

{
	m_slidertype = type;
}

qboolean UISlider::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	if (key == K_RIGHTARROW)
	{
		Increment();
	}
	else if (key == K_LEFTARROW)
	{
		Decrement();
	}
	else
	{
		return qfalse;
	}

	return qtrue;
}

void UISlider::setRange
	(
	float min,
	float max
	)

{
	m_minvalue = min;
	m_maxvalue = max;
}

void UISlider::setStep
	(
	float value
	)

{
	m_stepsize = value;
}

