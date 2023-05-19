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
#include <localization.h>

Event W_Destroyed
	(
	"widget_destroyed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Specifies that the widget has been destroyed." 
	);

Event W_SizeChanged
	(
	"size_changed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Specifies that the size of the widget has changed." 
	);

Event W_FrameChanged
	(
	"frame_changed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Specifies that the frame has changed." 
	);

Event W_OriginChanged
	(
	"origin_changed",
	EV_DEFAULT,
	NULL,
	NULL,
	"Specifies that the origin f the widget has changed." 
	);

Event W_Activated
	(
	"activated",
	EV_DEFAULT,
	NULL,
	NULL,
	"Signals that the widget has been activated." 
	);

Event EV_Widget_Activate
	(
	"activate",
	EV_DEFAULT,
	NULL,
	NULL,
	"Activate this widget." 
	);

Event W_Deactivated
	(
	"deactivated",
	EV_DEFAULT,
	NULL,
	NULL,
	"Signals that the widget has been deactivated." 
	);

Event EV_Widget_Hide
	(
	"hide",
	EV_DEFAULT,
	NULL,
	NULL,
	"Hides the widget." 
	);

Event EV_Widget_Enable
	(
	"enable",
	EV_DEFAULT,
	NULL,
	NULL,
	"enable the widget." 
	);

Event EV_Widget_Disable
	(
	"disable",
	EV_DEFAULT,
	NULL,
	NULL,
	"enable the widget." 
	);

Event W_RealignWidget
	(
	"realign_widget",
	EV_DEFAULT,
	NULL,
	NULL,
	"Specifies that the widget has been realigned." 
	);

Event W_Draw
	(
	"draw_widget",
	EV_DEFAULT,
	NULL,
	NULL,
	"Draw the widget." 
	);

Event EV_Layout_Size
	(
	"size",
	EV_DEFAULT,
	"ffff",
	"x y width height",
	"Set the size of the widget" 
	);

Event EV_Layout_Rect
	(
	"rect",
	EV_DEFAULT,
	"ffff",
	"x y width height",
	"Set the rect of the widget" 
	);

Event EV_Layout_Name
	(
	"name",
	EV_DEFAULT,
	"s",
	"name",
	"Set the name of the widget"
	);

Event EV_Layout_Title
	(
	"title",
	EV_DEFAULT,
	"s",
	"title",
	"Set the title of the widget to be displayed in the center of it"
	);

Event EV_Layout_Font
	(
	"font",
	EV_DEFAULT,
	"s",
	"title",
	"Set the font of the widget"
	);

Event EV_Layout_BGColor
	(
	"bgcolor",
	EV_DEFAULT,
	"ffff",
	"red green blue alpha",
	"Set the background color of the widget"
	);

Event EV_Layout_FGColor
	(
	"fgcolor",
	EV_DEFAULT,
	"ffff",
	"red green blue alpha",
	"Set the foreground color of the widget"
	);

Event EV_Layout_Borderstyle
	(
	"borderstyle",
	EV_DEFAULT,
	"s",
	"style",
	"Set the borderstyle of the widget(NONE, 3D_BORDER, INDENT_BORDER) "
	);

Event EV_Layout_BorderColor
	(
	"bordercolor",
	EV_DEFAULT,
	"ffff",
	"red green blue alpha",
	"Set the border color of the widget"
	);

Event EV_Layout_Shader
	(
	"shader",
	EV_DEFAULT,
	"s",
	"shaderName",
	"Set the shader to draw on the background of the widget"
	);

Event EV_Layout_TileShader
	(
	"tileshader",
	EV_DEFAULT,
	"s",
	"shaderName",
	"Set the shader to draw tiled on the background of the widget"
	);

Event EV_Layout_HoverShader
	(
	"hovershader",
	EV_DEFAULT,
	"s",
	"shaderName",
	"Set the shader to on the background of the widget when the mouse is over the widget"
	);

Event EV_Layout_PressedShader
	(
	"pressedshader",
	EV_DEFAULT,
	"s",
	"shaderName",
	"Set the shader to on the background of the widget when it is pressed in"
	);

Event EV_Layout_FadeIn
	(
	"fadein",
	EV_DEFAULT,
	"f",
	"fadetime",
	"Set the amount of time it takes for the widget to fade in"
	);

Event EV_Layout_StopSound
	(
	"stopsound",
	EV_DEFAULT,
	"s",
	"soundName",
	"Set the sound to be played when the widget stops moving"
	);

Event EV_Layout_ClickSound
	(
	"clicksound",
	EV_DEFAULT,
	"s",
	"soundName",
	"Set the sound to be played when the widget is clicked"
	);

Event EV_Layout_Stretch
	(
	"stretch",
	EV_DEFAULT,
	"s",
	"stretchdir",
	"Set the widget to be stretched horizontally or vertically across the screen"
	);

Event EV_Layout_VirtualRes
	(
	"virtualres",
	EV_DEFAULT,
	"i",
	"usevirtualres",
	"Set the widget to be modified horizontally or vertically across the screen to account for resolutions != 640x480"
	);

Event EV_Layout_InitData
	(
	"initdata",
	EV_DEFAULT,
	"s",
	"datastring",
	"Set the linkcvar to be initialized to this value"
	);

Event EV_Layout_Direction
	(
	"direction",
	EV_DEFAULT,
	"sF",
	"direction time",
	"Set the direction the widget will appear from on the screen\n(from_top, from_bottom, from_left, or from_right)\nTime is specified to determine how fast the widget will scroll in\n"
	);

Event EV_Layout_Align
	(
	"align",
	EV_DEFAULT,
	"s",
	"alignment",
	"Set the side of the screen to align the widget(left right top bottom centerx centery)"
	);

Event EV_Layout_StuffCommand
	(
	"stuffcommand",
	EV_DEFAULT,
	"s",
	"command",
	"Set the command to be stuffed when the widget is clicked"
	);

Event EV_Layout_LinkCvar
	(
	"linkcvar",
	EV_DEFAULT,
	"s",
	"cvarname",
	"Set the cvar that will be updated when the widget is changed"
	);

Event EV_Layout_NoParentClipping
	(
	"noparentclip",
	EV_DEFAULT,
	NULL,
	NULL,
	"Set the widget to not be clipped by the parent widget"
	);

Event EV_Layout_NoParentAdjustment
	(
	"noparentadjust",
	EV_DEFAULT,
	NULL,
	NULL,
	"Set the widget to not be adjusted by the parent widget"
	);

Event EV_Widget_HideCommand
	(
	"hidecommand",
	EV_DEFAULT,
	"s",
	"command",
	"Set a command to be executed when the widget is hidden"
	);

Event EV_Widget_ShowCommand
	(
	"showcommand",
	EV_DEFAULT,
	"s",
	"command",
	"Set a command to be executed when the widget is shown"
	);

Event EV_Widget_OrderNumber
	(
	"ordernumber",
	EV_DEFAULT,
	"d",
	"num",
	"Set the order the widget should be activated in"
	);

Event EV_Widget_TextAlignment
	(
	"textalign",
	EV_DEFAULT,
	"s",
	"alignment",
	"Sets the alignment of text for this widget"
	);

Event EV_Widget_EnabledCvar
	(
	"enabledcvar",
	EV_DEFAULT,
	"s",
	"cvar_name",
	"Set the cvar to check to see if this button is enabled or not"
	);

Event EV_Layout_AliasCache
	(
	"aliascache",
	EV_DEFAULT,
	"ssSSSSSS",
	"alias realPath arg1 arg2 arg3 arg4 arg5 arg6",
	"Create an alias to the specified path and cache the resource"
	);

Event EV_Layout_BGFill
	(
	"bgfill",
	EV_DEFAULT,
	"ffff",
	"red green blue alpha",
	"Set a color to fill the entire background"
	);

Event EV_Layout_Fullscreen
	(
	"fullscreen",
	EV_DEFAULT,
	"b",
	"fullscreen",
	"Set container to be fullscreen.  This will disable the game rendering if it's active"
	);

Event EV_Layout_VidMode
	(
	"vidmode",
	EV_DEFAULT,
	"i",
	"vidmode",
	"Set container's vidmode to change to when it's active"
	);

CLASS_DECLARATION( USignal, UIWidget, NULL )
{
	{ &EV_Widget_Hide,					&UIWidget::Hide },
	{ &EV_Widget_Activate,				&UIWidget::Activate },
	{ &EV_Layout_Size,					&UIWidget::LayoutSize },
	{ &EV_Layout_Rect,					&UIWidget::LayoutRect },
	{ &EV_Layout_Name,					&UIWidget::LayoutName },
	{ &EV_Layout_Title,					&UIWidget::LayoutTitle },
	{ &EV_Layout_Font,					&UIWidget::LayoutFont },
	{ &EV_Layout_BGColor,				&UIWidget::LayoutBgColor },
	{ &EV_Layout_FGColor,				&UIWidget::LayoutFgColor },
	{ &EV_Layout_Borderstyle,			&UIWidget::LayoutBorderStyle },
	{ &EV_Layout_BorderColor,			&UIWidget::LayoutBorderColor },
	{ &EV_Layout_Shader,				&UIWidget::LayoutShader },
	{ &EV_Layout_TileShader,			&UIWidget::LayoutTileShader },
	{ &EV_Layout_HoverShader,			&UIWidget::LayoutHoverShader },
	{ &EV_Layout_PressedShader,			&UIWidget::LayoutPressedShader },
	{ &EV_Layout_FadeIn,				&UIWidget::LayoutFadeIn },
	{ &EV_Layout_StopSound,				&UIWidget::LayoutStopSound },
	{ &EV_Layout_ClickSound,			&UIWidget::LayoutClickSound },
	{ &EV_Layout_Stretch,				&UIWidget::LayoutStretch },
	{ &EV_Layout_VirtualRes,			&UIWidget::LayoutVirtualRes },
	{ &EV_Layout_InitData,				&UIWidget::LayoutInitData },
	{ &EV_Layout_Direction,				&UIWidget::LayoutDirection },
	{ &EV_Layout_Align,					&UIWidget::LayoutAlign },
	{ &EV_Layout_NoParentClipping,		&UIWidget::LayoutNoParentClipping },
	{ &EV_Layout_NoParentAdjustment,	&UIWidget::LayoutNoParentAdjustment },
	{ &EV_Layout_StuffCommand,			&UIWidget::LayoutStuffCommand },
	{ &EV_Layout_LinkCvar,				&UIWidget::LayoutLinkCvar },
	{ &EV_Layout_AliasCache,			&UIWidget::LayoutAliasCache },
	{ &EV_Widget_ShowCommand,			&UIWidget::ShowCommand },
	{ &EV_Widget_HideCommand,			&UIWidget::HideCommand },
	{ &EV_Widget_OrderNumber,			&UIWidget::LayoutOrderNumber },
	{ &EV_Widget_TextAlignment,			&UIWidget::TextAlignment },
	{ &EV_Widget_Enable,				&UIWidget::EnableEvent },
	{ &EV_Widget_Disable,				&UIWidget::DisableEvent },
	{ &EV_Widget_EnabledCvar,			&UIWidget::SetEnabledCvar },

	{ NULL, NULL }
};

void SetColor( const UColor& color, float alpha )
{
	const vec4_t col = {
		color.r,
		color.g,
		color.b,
		color.a * alpha
	};

	uii.Rend_SetColor( col );
}

void DrawBox( const UIRect2D& rect, const UColor& color, float alpha )
{
	DrawBox( rect.pos.x, rect.pos.y, rect.size.width, rect.size.height, color, alpha );
}

void Draw3DBox( const UIRect2D& rect, bool indent, const UBorderColor& color, float alpha )
{
	Draw3DBox( rect.pos.x, rect.pos.y, rect.size.width, rect.size.height, indent, color, alpha );
}

void DrawBox( float x, float y, float width, float height, const UColor &color, float alpha )
{
	SetColor( color, alpha );
	uii.Rend_DrawBox( x, y, width, height );
}

void Draw3DBox( float x, float y, float width, float height, bool indent, const UBorderColor& color, float alpha )
{
	if( indent )
	{
		DrawBox( x, y, 1.0, height - 1.0, color.dark, alpha );
		DrawBox( x + 1.0, y + 1.0, 1.0, height - 3.0, color.reallydark, alpha );
		DrawBox( x, y, width - 1.0, 1.0, color.dark, alpha );
		DrawBox( x + 1.0, y + 1.0, width - 1.0, 1.0, color.reallydark, alpha );
		DrawBox( x + width - 1.0, y, 1.0, height, color.light, alpha );
		DrawBox( x, y + height - 1.0, width, 1.0, color.light, alpha );
	}
	else
	{
		DrawBox( x + width - 2.0, y + 1.0, 1.0, height - 2.0, color.dark, alpha );
		DrawBox( x + width - 1.0, y, 1.0, height, color.reallydark, alpha );
		DrawBox( x + 1.0, y + height - 2.0, width - 2.0, 1.0, color.dark, alpha );
		DrawBox( x, y + height - 1.0, width, 1.0, color.reallydark, alpha );
		DrawBox( x, y, 1.0, height - 1.0, color.light, alpha );
		DrawBox( x, y, width - 1.0, 1.0, color.light, alpha );
	}
}

void DrawMac3DBox( const UIRect2D& rect, bool indent, const UBorderColor& color, int inset, float alpha )
{
	float x, y;
	float width, height;

	x = rect.pos.x + inset;
	y = rect.pos.y + inset;
	width = rect.size.width - inset * 2;
	height = rect.size.height - inset * 2;

	if( indent )
	{
		DrawBox( x + 1.0, y + 1.0, width - 2.0, height - 2.0, color.dark, alpha );
		DrawBox( x, y, width, 1.0, color.reallydark, alpha );
		DrawBox( x, y + 1.0, 1.0, width - 2.0, color.reallydark, alpha );
		DrawBox( x + width - 1.0, y + 1.0, 1.0, height - 1.0, color.light, alpha );
		DrawBox( x, y + height - 1.0, width - 1.0, 1.0, color.light, alpha );
	}
	else
	{
		DrawBox( x, y, width, 1.0, color.light, alpha );
		DrawBox( x, y + 1.0, 1.0, height - 2.0, color.light, alpha );
		DrawBox( x + width - 1.0, y + 1.0, 1.0, height - 1.0, color.dark, alpha );
		DrawBox( x, y + height - 1.0, width - 1.0, 1.0, color.dark, alpha );
	}
}

void DrawBoxWithSolidBorder( const UIRect2D& rect, const UColor& inside, const UColor& outside, int size, int flags, float alpha )
{
	if( flags & 1 )
	{
		DrawBox( rect.pos.x + size, rect.pos.y + size, rect.size.width - size * 2, rect.size.height - size * 2, inside, alpha );
	}

	if( size && flags & 2 )
	{
		DrawBox( rect.pos.x, rect.pos.y, size, rect.size.height - size, outside, alpha );
		DrawBox( rect.pos.x + size, rect.pos.y, rect.size.width - size, size, outside, alpha );
		DrawBox( rect.pos.x + rect.size.width - size, rect.pos.y + size, size, rect.size.height - size, outside, alpha );
		DrawBox( rect.pos.x, rect.pos.y + rect.size.height - size, rect.size.width - size, size, outside, alpha );
	}
}

UIReggedMaterial::UIReggedMaterial()
{
	hMat = 0;
	isSet = true;
	isGot = false;
}

uihandle_t UIReggedMaterial::GetMaterial()
{
	return hMat;
}

void UIReggedMaterial::ReregisterMaterial
	(
	void
	)

{
	if( !isGot )
	{
		isGot = true;
		this->hMat = uii.Rend_RegisterMaterial( name.c_str() );
	}
}

void UIReggedMaterial::CleanupMaterial
	(
	void
	)

{
	isGot = false;
}

void UIReggedMaterial::SetMaterial
	(
	const str& name
	)

{
	this->name = name;
}

void UIReggedMaterial::RefreshMaterial
	(
	void
	)

{
	if( !isGot )
	{
		isGot = true;
		this->hMat = uii.Rend_RefreshMaterial( name.c_str() );
	}
}

str UIReggedMaterial::GetName
	(
	void
	)

{
	return name;
}

UIWidget::UIWidget()
{
	VectorSet2D( m_vVirtualScale, 1.0f, 1.0f );
	m_parent = NULL;
	m_background_color.set( 0.075f, 0.097f, 0.139f, 1.0f );
	m_foreground_color = UColor(UHudColor);
	m_border_color = UColor(0.15f, 0.195f, 0.278f, 1.0f);
	m_indent = 10.0f;
	m_bottomindent = 5.0f;
	m_borderStyle = border_indent;
	m_visible = true;
	m_enabled = true;
	m_canactivate = false;
	m_font = NULL;
	m_material = NULL;
	m_hovermaterial = NULL;
	m_pressedmaterial = NULL;
	m_hovermaterial_active = false;
	m_pressedmaterial_active = false;
	m_direction = D_NONE;
	m_fadetime = 0.0f;
	m_alpha = 1.0f;
	m_motiontime = 0.0f;
	m_starttime = 0.0f;
	m_direction_orig = D_NONE;
	m_motiontype = MOTION_IN;
	m_align = 0;
	m_flags = 0;
	m_dying = false;
	m_ordernum = 0;
	m_configstring_index = -1;
	m_iFontAlignmentHorizontal = FONT_JUSTHORZ_CENTER;
	m_iFontAlignmentVertical = FONT_JUSTVERT_CENTER;
	m_bVirtual = false;
	m_enabledCvar = "";

	UIRect2D frame( 6.0f, 6.0f, 100.0f, 13.0f );
	setFrame( frame );
}

UIWidget::~UIWidget()
{
	//int i;

	m_dying = true;

	if( IsActive() )
		uWinMan.DeactivateCurrentSmart();

	//for( i = m_children.NumObjects(); i > 0; i-- ) {
	//
	//}

	// remove this child from our parent
	if( m_parent )
		m_parent->removeChild( this );

	// delete font
	if (m_font)
	{
		delete m_font;
		m_font = NULL;
	}

	SendSignal( W_Destroyed );
}

bool UIWidget::addChild
	(
	UIWidget *widget
	)

{
	if (m_children.ObjectInList(widget))
	{
		return false;
	}

	m_children.AddObject(widget);
	if (m_children.NumObjects() != 1)
	{
		UIWidget* child = m_children.ObjectAt(m_children.NumObjects() - 1);
		if ((child->m_flags & (WF_ALWAYS_TOP | WF_ALWAYS_BOTTOM)) != (widget->m_flags & (WF_ALWAYS_TOP | WF_ALWAYS_BOTTOM)))
		{
			ArrangeWidgetList(m_children);
		}
	}

	return true;
}

bool UIWidget::removeChild
	(
	UIWidget *widget
	)

{
	if( m_children.ObjectInList( widget ) )
	{
		m_children.RemoveObject( widget );
		return true;
	}

	return false;
}

void UIWidget::PropogateCoordinateSystem
	(
	void
	)

{
	int i;
	int n;
	UIWidget *subview;

	if( !m_parent || (m_flags & WF_NOPARENTADJUST) )
	{
		m_screenorigin = m_origin;
		m_clippedorigin = m_origin;
		m_screenframe = m_frame;
		m_clippedframe = m_frame;
	}
	else
	{
		m_screenframe.pos.x = m_frame.pos.x + m_parent->m_screenorigin.x;
		m_screenframe.pos.y = m_frame.pos.y + m_parent->m_screenorigin.y;
		m_screenframe.size = m_frame.size;
		m_screenorigin.x = m_screenframe.pos.x - m_origin.x;
		m_screenorigin.y = m_screenframe.pos.y - m_origin.y;
		m_clippedframe = m_screenframe;
		m_clippedorigin.x = -m_origin.x;
		m_clippedorigin.y = -m_origin.y;
	}

	if( m_parent && !( m_flags & WF_DIRECTED ) )
	{
		if( m_parent->m_clippedframe.pos.x > m_clippedframe.pos.x )
		{
			m_clippedframe.size.width -= ( m_parent->m_clippedframe.pos.x - m_clippedframe.pos.x );
			
			if( m_clippedframe.size.width < 0.0 )
				m_clippedframe.size.width = 0.0;

			m_clippedframe.pos.x = m_parent->m_clippedframe.pos.x;
		}

		if( m_parent->m_clippedframe.pos.y > m_clippedframe.pos.y )
		{
			m_clippedframe.size.height -= ( m_parent->m_clippedframe.pos.y - m_clippedframe.pos.y );

			if( m_clippedframe.size.height < 0.0 )
				m_clippedframe.size.height = 0.0;

			m_clippedframe.pos.y = m_parent->m_clippedframe.pos.y;
		}

		if( m_parent->m_clippedframe.getMaxX() < m_clippedframe.getMaxX() )
		{
			m_clippedframe.size.width = m_parent->m_clippedframe.getMaxX() - m_clippedframe.pos.x;

			if( m_clippedframe.size.width < 0.0 )
				m_clippedframe.size.width = 0.0;
		}

		if( m_parent->m_clippedframe.getMaxY() < m_clippedframe.getMaxY() )
		{
			m_clippedframe.size.height = m_parent->m_clippedframe.getMaxY() - m_clippedframe.pos.y;

			if( m_clippedframe.size.height < 0.0 )
				m_clippedframe.size.height = 0.0;
		}
	}

	n = m_children.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		subview = m_children.ObjectAt( i );
		subview->PropogateCoordinateSystem();
	}
}

void UIWidget::set2D
	(
	void
	)

{
	uii.Rend_Set2D( m_screenframe.pos.x,
		uid.vidHeight - (int)(m_screenframe.size.height + m_screenframe.pos.y),
		m_screenframe.size.width,
		m_screenframe.size.height,
		m_clippedorigin.x,
		m_clippedorigin.x + m_screenframe.size.width,
		m_clippedorigin.y + m_screenframe.size.height,
		m_clippedorigin.y,
		-1.0, 1.0 );

	uii.Rend_Scissor( m_clippedframe.pos.x,
		uid.vidHeight - (int)(m_clippedframe.size.height + m_clippedframe.pos.y),
		m_clippedframe.size.width,
		m_clippedframe.size.height );
}

void UIWidget::Draw
	(
	void
	)

{
}

void UIWidget::FrameInitialized
	(
	void
	)

{
}

void UIWidget::DrawTitle
	(
	float x,
	float y
	)

{
	m_font->setColor( m_foreground_color );
	m_font->Print( x, y, Sys_LV_CL_ConvertString( m_title ), -1, m_bVirtual );
}

void UIWidget::Motion
	(
	void
	)

{
	float t;
	float frac;
	int i;
	float nx, ny;

	t = uid.time / 1000.0;

	if( m_fadetime >= 0.0 )
	{
		frac = ( t - m_starttime ) / m_fadetime;

		m_alpha = frac;
		if( m_alpha > 1.0 )
			m_alpha = 1.0;

		if( m_motiontype == MOTION_OUT )
			m_alpha = 1.0 - m_alpha;
	}

	if( m_direction )
	{
		frac = ( t - m_starttime ) / m_motiontime;

		if( m_motiontype == MOTION_OUT )
		{
			frac = 1.0 - frac;
			if (frac <= 0.0)
			{
				switch (m_direction)
				{
				case D_FROM_BOTTOM:
					setFrame(UIRect2D(m_frame.pos.x, uid.vidHeight, m_frame.size.width, m_frame.size.height));
					break;
				case D_FROM_TOP:
					setFrame(UIRect2D(m_frame.pos.x, -m_frame.size.height - m_parent->m_screenorigin.y, m_frame.size.width, m_frame.size.height));
					break;
				case D_FROM_LEFT:
					setFrame(UIRect2D(-m_frame.size.width - m_parent->m_screenorigin.x, m_frame.pos.y, m_frame.size.width, m_frame.size.height));
					break;
				case D_FROM_RIGHT:
					setFrame(UIRect2D(uid.vidWidth, m_frame.pos.y, m_frame.size.width, m_frame.size.height));
					break;
				default:
					setFrame(UIRect2D(m_frame.pos.x, m_frame.pos.y, m_frame.size.width, m_frame.size.height));
					break;
				}

				m_direction = D_NONE;
			}
		}
		else
		{
			if (frac >= 1.0)
			{
				// done moving
				setFrame(UIRect2D(m_startingpos, m_frame.size));
				FrameInitialized();

				for (i = 1; i <= m_children.NumObjects(); i++)
				{
					m_children.ObjectAt(i)->FrameInitialized();
				}

				m_direction = D_NONE;
			}
		}
	}

	switch( m_direction )
	{
	case D_FROM_LEFT:
		nx = 0;
		if( m_parent )
			nx = m_frame.size.width + m_parent->m_screenorigin.x;
		nx = ( m_startingpos.x + nx ) * frac - nx;
		ny = m_frame.pos.y;

		setFrame(UIRect2D(nx, ny, m_frame.size.width, m_frame.size.height));
		break;
	case D_FROM_RIGHT:
		nx = uid.vidWidth - ( uid.vidWidth - m_startingpos.x ) * frac;
		ny = m_frame.pos.y;

		setFrame(UIRect2D(nx, ny, m_frame.size.width, m_frame.size.height));
		break;
	case D_FROM_BOTTOM:
		nx = m_frame.pos.x;
		ny = uid.vidHeight - ( uid.vidHeight - m_startingpos.y ) * frac;

		setFrame(UIRect2D(nx, ny, m_frame.size.width, m_frame.size.height));
		break;
	case D_FROM_TOP:
		nx = m_frame.pos.y;
		ny = 0;
		if( m_parent )
			ny = m_frame.size.height + m_parent->m_screenorigin.y;
		ny = (m_startingpos.y + ny) * frac - ny;

		setFrame(UIRect2D(nx, ny, m_frame.size.width, m_frame.size.height));
		break;
	default:
		break;
	}

	if (m_direction == D_NONE)
	{
		//
		// play a sound to indicate the motion has finished
		if (m_stopsound.length()) {
			uii.Snd_PlaySound(m_stopsound);
		}
	}
}

void UIWidget::AlignPosition
	(
	void
	)

{
	if (m_bVirtual)
	{
		vec2_t vNewVirtualScale;

		vNewVirtualScale[0] = uid.vidWidth / 640.0;
		vNewVirtualScale[1] = uid.vidHeight / 480.0;

		if (!VectorCompare2D(m_vVirtualScale, vNewVirtualScale))
		{
			m_frame.pos.x = m_frame.pos.x / m_vVirtualScale[0] * vNewVirtualScale[0];
			m_frame.pos.y = m_frame.pos.y / m_vVirtualScale[1] * vNewVirtualScale[1];
			m_frame.size.width = m_frame.size.width / m_vVirtualScale[0] * vNewVirtualScale[0];
			m_frame.size.height = m_frame.size.height / m_vVirtualScale[1] * vNewVirtualScale[1];
			m_vVirtualScale[0] = vNewVirtualScale[0];
			m_vVirtualScale[1] = vNewVirtualScale[1];
		}
	}

	if (m_flags & WF_STRETCH_VERTICAL)
	{
		m_frame.pos.y = 0;
		m_frame.size.height = uid.vidHeight;
	}
	if (m_flags & WF_STRETCH_HORIZONTAL)
	{
		m_frame.pos.x = 0;
		m_frame.size.width = uid.vidWidth;
	}

	if (m_align & WA_CENTERX)
	{
		m_frame.pos.x = uid.vidWidth * 0.5 - m_frame.size.width * 0.5;
	}

	if (m_align & WA_RIGHT)
	{
		m_frame.pos.x = uid.vidWidth - m_frame.size.width;
	}
	else if (m_align & WA_LEFT)
	{
		m_frame.pos.x = 0;
	}

	if (m_align & WA_CENTERY)
	{
		m_frame.pos.y = uid.vidHeight * 0.5 - m_frame.size.height * 0.5;
	}

	if (m_align & WA_BOTTOM)
	{
		m_frame.pos.y = uid.vidHeight - m_frame.size.height;
	}
	else if (m_align & WA_TOP)
	{
		m_frame.pos.y = 0;
	}

	setFrame(m_frame);
}

void UIWidget::Hide
	(
	Event *ev
	)

{
	setShow( false );
}

void UIWidget::Activate
	(
	Event *ev
	)

{
	uWinMan.ActivateControl( this );
}

void UIWidget::BringToFront
	(
	void
	)

{
	UIWidget *me;

	if( m_parent->m_children.NumObjects() > 1 )
	{
		me = m_parent->m_children.ObjectAt( m_parent->m_children.NumObjects() );
		if( me != this )
		{
			m_parent->m_children.RemoveObject( this );
			m_parent->m_children.AddObject( this );

			if( ( m_parent->m_children.ObjectAt( m_parent->m_children.NumObjects() - 1 )->m_flags & 0x30 ) != ( m_flags & 0x30 ) )
			{
				ArrangeWidgetList( m_parent->m_children );
			}
		}
	}
}

void UIWidget::SplitWidgets
	(
	Container<UIWidget*>& src,
	Container<UIWidget*>& bottom,
	Container<UIWidget*>& normal,
	Container<UIWidget*>& top
	)

{
	int i;

	bottom.FreeObjectList();
	normal.FreeObjectList();
	top.FreeObjectList();

	for( i = 1; i <= src.NumObjects(); i++ )
	{
		UIWidget *wid = src.ObjectAt( i );

		if( wid->m_flags & 0x10 )
		{
			bottom.AddObject( wid );
		}
		else if( wid->m_flags & 0x20 )
		{
			top.AddObject( wid );
		}
		else
		{
			normal.AddObject( wid );
		}
	}
}

void UIWidget::CombineWidgets
	(
	Container<UIWidget*>& dst,
	Container<UIWidget*>& bottom,
	Container<UIWidget*>& normal,
	Container<UIWidget*>& top
	)

{
	Container< UIWidget * > *lists[ 3 ];
	int list;
	int i;

	dst.ClearObjectList();

	lists[ 0 ] = &bottom;
	lists[ 1 ] = &normal;
	lists[ 2 ] = &top;

	for( list = 0; list < 3; list++ )
	{
		for( i = 1; i <= lists[ list ]->NumObjects(); i++ )
		{
			dst.AddObject( lists[ list ]->ObjectAt( i ) );
		}
	}
}

void UIWidget::ArrangeWidgetList
	(
	Container<UIWidget*>& list
	)

{
	Container<UIWidget *> bottom;
	Container<UIWidget *> normal;
	Container<UIWidget *> top;

	SplitWidgets( list, bottom, normal, top );
	CombineWidgets( list, bottom, normal, top );
}

void UIWidget::Shutdown
	(
	void
	)

{
	int i;

	if( IsActive() )
		uWinMan.DeactivateCurrentSmart();

	// notify childs
	for( i = m_children.NumObjects(); i > 0; i-- )
	{
		UIWidget *wid = m_children.ObjectAt( i );

		wid->Shutdown();
		delete wid;
	}

	if( m_font )
	{
		delete m_font;
		m_font = NULL;
	}
}

void UIWidget::InitFrame
	(
	UIWidget *parentview,
	float x,
	float y,
	float width,
	float height,
	int border,
	const char *fontname
	)

{
	if( border != -1 )
	{
		setBorderStyle( ( borderstyle_t )border );
	}

	setFrame( UIRect2D( x, y, width, height ) );

	if( parentview )
	{
		setParent( parentview );
	}
	else if( this != &uWinMan )
	{
		setParent( &uWinMan );
	}

	m_startingpos = m_frame.pos;

	setFont( fontname );

	FrameInitialized();
}

void UIWidget::InitFrame
	(
	UIWidget *parentview,
	const UIRect2D& r,
	int border,
	const char *fontname
	)

{
	InitFrame( parentview, r.pos.x, r.pos.y, r.size.width, r.size.height, border, fontname );
}

void UIWidget::LayoutSize
	(
	Event *ev
	)

{
	LayoutRect( ev );
}

void UIWidget::LayoutRect
	(
	Event *ev
	)

{
	UIPoint2D pos;
	UISize2D size;

	if( ev->NumArgs() == 4 )
	{
		pos.x = ev->GetFloat( 1 );
		pos.y = ev->GetFloat( 2 );
		size.width = ev->GetFloat( 3 );
		size.height = ev->GetFloat( 4 );

		setFrame( UIRect2D( pos, size ) );

		m_startingpos = pos;
	}
}

void UIWidget::LayoutName
	(
	Event *ev
	)

{
	setName( ev->GetString( 1 ) );
}

void UIWidget::LayoutTitle
	(
	Event *ev
	)

{
	setTitle( ev->GetString( 1 ) );
}

void UIWidget::LayoutFont
	(
	Event *ev
	)

{
	setFont( ev->GetString( 1 ) );
}

void UIWidget::LayoutBgColor
	(
	Event *ev
	)

{
	setBackgroundColor( UColor( ev->GetFloat( 1 ), ev->GetFloat( 2 ), ev->GetFloat( 3 ), ev->GetFloat( 4 ) ), true );
}

void UIWidget::LayoutFgColor
	(
	Event *ev
	)

{
	setForegroundColor( UColor( ev->GetFloat( 1 ), ev->GetFloat( 2 ), ev->GetFloat( 3 ), ev->GetFloat( 4 ) ) );
}

void UIWidget::LayoutBorderStyle
	(
	Event *ev
	)

{
	str style = ev->GetString( 1 );

	if( !stricmp( style, "NONE" ) )
	{
		setBorderStyle( border_none );
	}
	else if( !stricmp( style, "3D_BORDER" ) )
	{
		setBorderStyle( border_3D );
	}
	else if( !stricmp( style, "INDENT_BORDER" ) )
	{
		setBorderStyle( border_indent );
	}
}

void UIWidget::LayoutBorderColor
	(
	Event *ev
	)

{
	setBorderColor( UColor( ev->GetFloat( 1 ), ev->GetFloat( 2 ), ev->GetFloat( 3 ), ev->GetFloat( 4 ) ) );
}

void UIWidget::LayoutShader
	(
	Event *ev
	)

{
	setMaterial( uWinMan.RegisterShader( ev->GetString( 1 ) ) );
	m_flags &= ~WF_TILESHADER;
}

void UIWidget::LayoutTileShader
	(
	Event *ev
	)

{
	setMaterial( uWinMan.RegisterShader( ev->GetString( 1 ) ) );
	m_flags |= WF_TILESHADER;
}

void UIWidget::LayoutHoverShader
	(
	Event *ev
	)

{
	setHoverMaterial( uWinMan.RegisterShader( ev->GetString( 1 ) ) );
}

void UIWidget::LayoutPressedShader
	(
	Event *ev
	)

{
	setPressedMaterial( uWinMan.RegisterShader( ev->GetString( 1 ) ) );
}

void UIWidget::LayoutFadeIn
	(
	Event *ev
	)

{
	m_fadetime = ev->GetFloat( 1 );
	if( m_motiontime != 0.0 )
	{
		setMotionTime( m_fadetime );
	}
}

void UIWidget::LayoutStopSound
	(
	Event *ev
	)

{
	m_stopsound = ev->GetString( 1 );
	uii.Snd_RegisterSound( m_stopsound, false );
}

void UIWidget::LayoutClickSound
	(
	Event *ev
	)

{
	m_clicksound = ev->GetString( 1 );
	uii.Snd_RegisterSound( m_clicksound, false );
}

void UIWidget::LayoutStretch
	(
	Event *ev
	)

{
	str stretch = ev->GetString( 1 );

	if( !stricmp( stretch, "vertical" ) )
	{
		m_flags |= WF_STRETCH_VERTICAL;
	}
	else if( !stricmp( stretch, "horizontal" ) )
	{
		m_flags |= WF_STRETCH_HORIZONTAL;
	}
	else if( !stricmp( stretch, "both" ) )
	{
		m_flags |= WF_STRETCH_VERTICAL | WF_STRETCH_HORIZONTAL;
	}

	m_flags |= WF_NOPARENTADJUST | WF_DIRECTED;
}

void UIWidget::LayoutVirtualRes
	(
	Event *ev
	)

{
	if( ev->GetInteger( 1 ) )
	{
		m_bVirtual = true;
	}
}

void UIWidget::LayoutInitData
	(
	Event *ev
	)

{
	if( m_cvarname.length() )
	{
		uii.Cvar_Set( m_cvarname, ev->GetString( 1 ) );
	}
}

void UIWidget::LayoutDirection
	(
	Event *ev
	)

{
	str direction = ev->GetString( 1 );

	if( !stricmp( direction, "from_bottom" ) )
	{
		setDirection( D_FROM_BOTTOM );
	}
	else if( !stricmp( direction, "from_top" ) )
	{
		setDirection( D_FROM_TOP );
	}
	else if( !stricmp( direction, "from_left" ) )
	{
		setDirection( D_FROM_LEFT );
	}
	else if( !stricmp( direction, "from_right" ) )
	{
		setDirection( D_FROM_RIGHT );
	}
	else
	{
		setDirection( D_NONE );
	}

	if( ev->NumArgs() > 1 )
	{
		setMotionTime( ev->GetFloat( 2 ) );
	}

	m_flags |= WF_DIRECTED;
}

void UIWidget::LayoutAlign
	(
	Event *ev
	)

{
	for( int i = 1; i <= ev->NumArgs(); i++ )
	{
		str align = ev->GetString( i );

		if( !stricmp( align, "left" ) )
		{
			m_align |= WA_LEFT;
		}
		else if( !stricmp( align, "right" ) )
		{
			m_align |= WA_RIGHT;
		}
		else if( !stricmp( align, "top" ) )
		{
			m_align |= WA_TOP;
		}
		else if( !stricmp( align, "bottom" ) )
		{
			m_align |= WA_BOTTOM;
		}
		else if( !stricmp( align, "centerx" ) )
		{
			m_align |= WA_CENTERX;
		}
		else if( !stricmp( align, "centery" ) )
		{
			m_align |= WA_CENTERY;
		}
	}

	if( !isSubclassOf( UIWidgetContainer ) )
	{
		m_flags |= WF_NOPARENTADJUST | WF_DIRECTED;
	}
}

void UIWidget::LayoutStuffCommand
	(
	Event *ev
	)

{
	LinkCommand( ev->GetString( 1 ) );
}

void UIWidget::LayoutLinkCvar
	(
	Event *ev
	)

{
	LinkCvar( ev->GetString( 1 ) );
}

void UIWidget::LayoutNoParentClipping
	(
	Event *ev
	)

{
	m_flags |= WF_DIRECTED;
}

void UIWidget::LayoutNoParentAdjustment
	(
	Event *ev
	)

{
	m_flags |= WF_NOPARENTADJUST;
}

void UIWidget::LayoutOrderNumber
	(
	Event *ev
	)

{
	m_ordernum = ev->GetInteger( 1 );
}

void UIWidget::TextAlignment
	(
	Event *ev
	)

{
	for( int i = 1; i <= ev->NumArgs(); i++ )
	{
		str align = ev->GetString( 1 );

		if( !stricmp( align, "left" ) )
		{
			setFontHorizontalAlignment( FONT_JUSTHORZ_LEFT );
		}
		else if( !stricmp( align, "right" ) )
		{
			setFontHorizontalAlignment( FONT_JUSTHORZ_RIGHT );
		}
		else if( !stricmp( align, "top" ) )
		{
			setFontVerticalAlignment( FONT_JUSTVERT_TOP );
		}
		else if( !stricmp( align, "bottom" ) )
		{
			setFontVerticalAlignment( FONT_JUSTVERT_BOTTOM );
		}
		else if( !stricmp( align, "centerx" ) )
		{
			setFontHorizontalAlignment( FONT_JUSTHORZ_CENTER );
		}
		else if( !stricmp( align, "centery" ) )
		{
			setFontVerticalAlignment( FONT_JUSTVERT_CENTER );
		}
	}
}

void UIWidget::LayoutAliasCache
	(
	Event *ev
	)

{
	int i;
	str parms;
	str realSound;

	if( ev->NumArgs() <= 1 )
		return;

	for( i = 3; i <= ev->NumArgs(); i++ )
	{
		parms += ev->GetToken( i );
		parms += " ";
	}

	realSound = ev->GetString( 2 );

	uii.Alias_Add( parms, realSound, parms );

	if( strstr( realSound, ".wav" ) || strstr( realSound, ".mp3" ) )
	{
		uii.Snd_RegisterSound( realSound, false );
	}
}

void UIWidget::SetEnabledCvar
	(
	Event *ev
	)

{
	m_enabledCvar = ev->GetString( 1 );
}

void UIWidget::setParent
	(
	UIWidget *parent
	)

{
	if( m_parent )
		m_parent->removeChild( this );

	m_parent = parent;

	if( m_parent )
		m_parent->addChild( this );

	PropogateCoordinateSystem();
}

UIWidget *UIWidget::getParent
	(
	void
	)

{
	return m_parent;
}

UIWidget *UIWidget::getFirstChild
	(
	void
	)

{
	if( m_children.NumObjects() > 0 )
		return m_children.ObjectAt( 1 );

	return NULL;
}

UIWidget *UIWidget::getNextSibling
	(
	void
	)

{
	if( m_parent )
		return m_parent->getNextChild( this );

	return NULL;
}

UIWidget *UIWidget::getNextChild
	(
	UIWidget *child
	)

{
	int index = m_children.IndexOfObject( child );

	if( index && m_children.NumObjects() != index )
	{
		return m_children.ObjectAt( index + 1 );
	}

	return NULL;
}

UIWidget *UIWidget::getPrevChild
	(
	UIWidget *child
	)

{
	int index;

	if( child )
	{
		index = m_children.IndexOfObject( child );
	}
	else
	{
		index = m_children.NumObjects();
	}

	if( index > 1 )
	{
		return m_children.ObjectAt( index - 1 );
	}

	return NULL;
}

UIWidget *UIWidget::getPrevSibling
	(
	void
	)

{
	if( m_parent )
		return m_parent->getPrevChild( this );

	return NULL;
}

UIWidget *UIWidget::getPrevSibling
	(
	UIWidget *curr
	)

{
	if( m_parent )
		return m_parent->getPrevChild( curr );

	return NULL;
}

UIWidget *UIWidget::getLastSibling
	(
	void
	)

{
	if( m_parent && m_children.NumObjects() )
	{
		UIWidget *wid = m_parent->m_children.ObjectAt( m_parent->m_children.NumObjects() );
		if( wid == this )
		{
			return getPrevSibling();
		}
		else
		{
			return wid;
		}
	}
	else
	{
		return NULL;
	}
}

UIWidget *UIWidget::findSibling
	(
	str name
	)

{
	UIWidget *wid;

	for( wid = getLastSibling(); wid != NULL; wid = getPrevSibling( wid ) )
	{
		if( wid->getName() == name )
			return wid;
	}

	return NULL;
}

void UIWidget::Enable
	(
	void
	)

{
	int n;
	int i;

	setShow( true );
	m_enabled = true;

	n = m_children.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		m_children.ObjectAt( i )->Enable();
	}
}

void UIWidget::Disable
	(
	void
	)

{
	int n;
	int i;

	PostEvent( EV_Widget_Hide, 0 );
	m_enabled = false;

	n = m_children.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		m_children.ObjectAt( i )->Disable();
	}
}

bool UIWidget::isEnabled
	(
	void
	)

{
	if( !strcmp( m_enabledCvar, "" ) || UI_GetCvarInt( m_enabledCvar, 0 ) )
	{
		return m_enabled;
	}

	return false;
}

bool UIWidget::IsDying
	(
	void
	)

{
	return m_dying;
}

UIWidget *UIWidget::FindResponder
	(
	const UIPoint2D& pos
	)

{
	UIWidget *subview;
	UIWidget *responder;
	int i;

	if( !m_visible || !m_clippedframe.contains( pos ) )
	{
		return NULL;
	}

	for( i = m_children.NumObjects(); i >= 1; i-- )
	{
		subview = m_children.ObjectAt( i );
		responder = subview->FindResponder( pos );
		if( responder )
		{
			return responder;
		}
	}

	return this;
}

void UIWidget::setFont
	(
	const char *name
	)

{
	if( m_font )
		delete m_font;

	m_font = new UIFont( name );
}

void UIWidget::setFontHorizontalAlignment
	(
	fonthorzjustify_t alignment
	)

{
	m_iFontAlignmentHorizontal = alignment;
}

void UIWidget::setFontVerticalAlignment
	(
	fontvertjustify_t alignment
	)

{
	m_iFontAlignmentVertical = alignment;
}

void UIWidget::setShow
	(
	bool visible
	)

{
	m_visible = visible;

	if( !visible )
	{
		if( IsThisOrChildActive() )
			uWinMan.DeactivateCurrentSmart();
	}
}

bool UIWidget::getShow
	(
	void
	)

{
	return m_visible;
}

UColor UIWidget::getBackgroundColor
	(
	void
	)

{
	return m_background_color;
}

void UIWidget::setBackgroundColor
	(
	const UColor& color,
	bool setbordercolor
	)

{
	m_background_color = color;

	if( setbordercolor )
	{
		setBorderColor( color );
	}
}

UBorderColor UIWidget::getBorderColor
	(
	void
	)

{
	return m_border_color;
}

void UIWidget::setBorderColor
	(
	const UBorderColor& color
	)

{
	m_border_color = color;
}

UColor UIWidget::getForegroundColor
	(
	void
	)

{
	return m_foreground_color;
}

void UIWidget::setForegroundColor
	(
	const UColor& color
	)

{
	m_foreground_color = color;
}

borderstyle_t UIWidget::getBorderStyle
	(
	void
	)

{
	return m_borderStyle;
}

void UIWidget::setBorderStyle
	(
	borderstyle_t style
	)

{
	m_borderStyle = style;
}

UISize2D UIWidget::getSize
	(
	void
	)

{
	return m_frame.size;
}

void UIWidget::setSize
	(
	const UISize2D& newsize
	)

{
	if( newsize.width != m_frame.size.width
		|| newsize.height != m_frame.size.height )
	{
		m_frame.size = newsize;
		PropogateCoordinateSystem();
		SendSignal( W_SizeChanged );
	}
}

UIRect2D UIWidget::getFrame
	(
	void
	)

{
	return m_frame;
}

void UIWidget::setFrame
	(
	const UIRect2D& newframe
	)

{
	bool bNewSize;

	bNewSize = newframe.size != m_frame.size;

	m_frame = newframe;

	PropogateCoordinateSystem();
	SendSignal( W_FrameChanged );

	if( bNewSize )
	{
		SendSignal( W_SizeChanged );
	}
}

UIPoint2D UIWidget::getOrigin
	(
	void
	)

{
	return m_origin;
}

void UIWidget::setOrigin
	(
	const UIPoint2D& neworigin
	)

{
	if( neworigin != m_origin )
	{
		m_origin = neworigin;
		PropogateCoordinateSystem();
	}
}

void UIWidget::setName
	(
	str name
	)

{
	m_name = name;
}

const char *UIWidget::getName
	(
	void
	)

{
	return m_name;
}

void UIWidget::setTitle
	(
	str title
	)

{
	m_title = title;
}

const char *UIWidget::getTitle
	(
	void
	)

{
	return m_title;
}

void UIWidget::setDirection
	(
	direction_t dir
	)

{
	m_direction = dir;
	m_direction_orig = dir;
}

direction_t UIWidget::getDirection
	(
	void
	)

{
	return m_direction;
}

void UIWidget::setMotionType
	(
	motion_t type
	)

{
	m_motiontype = type;
}

motion_t UIWidget::getMotionType
	(
	void
	)

{
	return m_motiontype;
}

void UIWidget::setMotionTime
	(
	float time
	)

{
	m_motiontime = time;
}

float UIWidget::getMotionTime
	(
	void
	)

{
	return m_motiontime;
}

void UIWidget::setAlign
	(
	int align
	)

{
	m_align = align;
}

int UIWidget::getAlign
	(
	void
	)

{
	return m_align;
}

void UIWidget::setMaterial
	(
	UIReggedMaterial *mat
	)

{
	m_material = mat;
}

void UIWidget::setHoverMaterial
	(
	UIReggedMaterial *mat
	)

{
	m_hovermaterial = mat;
}

void UIWidget::setPressedMaterial
	(
	UIReggedMaterial *mat
	)

{
	m_pressedmaterial = mat;
}

UIPoint2D UIWidget::getLocalPoint
	(
	const UIPoint2D& pos
	)

{
	if( m_parent )
	{
		return UIPoint2D( pos.x - m_screenframe.pos.x, pos.y - m_screenframe.pos.y );
	}
	else
	{
		return pos;
	}
}

UIPoint2D UIWidget::getGlobalPoint
	(
	const UIPoint2D& pos
	)

{
	if( m_parent )
	{
		return UIPoint2D( pos.x + m_screenframe.pos.x, pos.y + m_screenframe.pos.y );
	}
	else
	{
		return pos;
	}
}

void UIWidget::setBackgroundAlpha
	(
	float f
	)

{
	m_alpha = f;
}

float UIWidget::getBackgroundAlpha
	(
	void
	)

{
	return m_alpha;
}

void UIWidget::Display
	(
	const UIRect2D& drawframe,
	float parent_alpha
	)

{
	int i;
	int n;
	vec4_t col;

	VectorSet( col, 1, 1, 1 );

	if( !m_enabledCvar.length() || UI_GetCvarInt( m_enabledCvar, 0 ) )
	{
		if( !m_enabledCvar.length() && !IsVisible() )
			return;

		if (m_direction || m_fadetime > 0.0) {
			Motion();
		}

		m_local_alpha = m_alpha * parent_alpha;
		set2D();

		col[ 3 ] = m_local_alpha;
		uii.Rend_SetColor( col );

		if( m_background_color.a != 0.0 )
		{
			DrawBox( 0, 0, m_frame.size.width, m_frame.size.height, m_background_color, m_background_color.a * m_local_alpha );
		}

		if( m_borderStyle == border_outline )
		{
			DrawBoxWithSolidBorder( UIRect2D( 0, 0, m_frame.size.width, m_frame.size.height ), UWhite, m_border_color.original, 2, 2, m_local_alpha );
		}
		else if( m_borderStyle )
		{
			Draw3DBox( 0, 0, m_frame.size.width, m_frame.size.height, m_borderStyle == border_indent, m_border_color, m_local_alpha );
		}

		if( m_material )
		{
			uii.Rend_SetColor( m_foreground_color );

			m_material->ReregisterMaterial();

			if( m_flags & WA_TOP )
			{
				uii.Rend_DrawPicTiled( 0, 0, m_frame.size.width, m_frame.size.height, m_material->GetMaterial() );
			}
			else
			{
				uii.Rend_DrawPicStretched( 0, 0, m_frame.size.width, m_frame.size.height, 0, 0, 1, 1, m_material->GetMaterial() );
			}
		}

		if( m_pressedmaterial_active && m_pressedmaterial != NULL )
		{
			m_pressedmaterial->ReregisterMaterial();
			uii.Rend_DrawPicStretched( 0, 0, m_frame.size.width, m_frame.size.height, 0, 0, 1, 1, m_pressedmaterial->GetMaterial() );
		}
		else if( m_hovermaterial_active && m_hovermaterial != NULL )
		{
			m_hovermaterial->ReregisterMaterial();
			uii.Rend_DrawPicStretched( 0, 0, m_frame.size.width, m_frame.size.height, 0, 0, 1, 1, m_hovermaterial->GetMaterial() );
		}

		Draw();

		n = m_children.NumObjects();
		for( i = 1; i <= m_children.NumObjects(); i++ )
		{
			m_children.ObjectAt( i )->Display( m_frame, m_local_alpha );
		}
	}
}

qboolean UIWidget::KeyEvent
	(
	int key,
	unsigned int time
	)

{
	return qfalse;
}

void UIWidget::CharEvent
	(
	int ch
	)

{
}

void UIWidget::UpdateData
	(
	void
	)

{
}

void UIWidget::UpdateUIElement
	(
	void
	)

{
}

float UIWidget::getTitleWidth
	(
	void
	)

{
	return m_font->getWidth( getTitle(), -1 );
}

float UIWidget::getTitleHeight
	(
	void
	)

{
	return m_font->getHeight( m_bVirtual );
}

bool UIWidget::CanActivate
	(
	void
	)

{
	if( !m_canactivate || m_dying )
		return false;

	for( UIWidget *wid = this; wid != NULL; wid = wid->getParent() )
	{
		if( !wid->getShow() )
		{
			return false;
		}
	}

	return true;
}

void UIWidget::AllowActivate
	(
	bool canactivate
	)

{
	m_canactivate = canactivate;
}

bool UIWidget::IsActive
	(
	void
	)

{
	return uWinMan.ActiveControl() == this;
}

bool UIWidget::IsVisible
	(
	void
	)

{
	return m_visible;
}

void UIWidget::LinkCvar
	(
	str cvarname
	)

{
	m_cvarname = cvarname;
}

void UIWidget::LinkCommand
	(
	str cmd
	)

{
	m_command = cmd;
}

void UIWidget::ExecuteShowCommands
	(
	void
	)

{
	for( int i = 1; i <= m_showcommands.NumObjects(); i++ )
	{
		uii.Cmd_Stuff( m_showcommands.ObjectAt( i )->c_str() );
	}
}

void UIWidget::ExecuteHideCommands
	(
	void
	)

{
	for( int i = 1; i <= m_hidecommands.NumObjects(); i++ )
	{
		uii.Cmd_Stuff( m_hidecommands.ObjectAt( i )->c_str() );
	}
}

void UIWidget::InitializeCommandHandler
	(
	void( *fcn ) ( const char *, void * )
	)

{
	m_commandhandler = fcn;
}

UIWidget *UIWidget::FindWidget
	(
	str name
	)

{
	int i;
	int n;

	n = m_children.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_children.ObjectAt( i );
		if( wid->getName() == name )
		{
			return wid;
		}

		UIWidget *w = wid->FindWidget( name );
		if( w )
		{
			return w;
		}
	}

	return NULL;
}

void UIWidget::ResetMotion
	(
	motion_t type
	)

{
	AlignPosition();

	m_direction = m_direction_orig;
	m_motiontype = type;

	if( type == MOTION_IN )
	{
		if( m_direction == D_FROM_BOTTOM )
		{
			UIPoint2D point(m_frame.pos.x, uid.vidHeight);
			setFrame(UIRect2D(point, m_frame.size));
		}
		else if( m_direction == D_FROM_TOP )
		{
			UIPoint2D point(m_frame.pos.x, -m_frame.size.height - m_parent->m_screenorigin.y);
			setFrame(UIRect2D(point, m_frame.size));
		}
		else if( m_direction == D_FROM_LEFT )
		{
			UIPoint2D point(-m_frame.size.width - m_parent->m_screenorigin.x, m_frame.pos.y);
			setFrame(UIRect2D(point, m_frame.size));
		}
		else if( m_direction == D_FROM_RIGHT )
		{
			UIPoint2D point(uid.vidWidth, m_frame.pos.y);
			setFrame(UIRect2D(point, m_frame.size));
		}
		else
		{
			setFrame(m_frame);
			FrameInitialized();
		}
	}

	m_starttime = uid.time / 1000.0;
}

void UIWidget::Realign
	(
	void
	)

{
	if (m_bVirtual)
	{
		vec2_t vNewVirtualScale;

		vNewVirtualScale[0] = uid.vidWidth / 640.0;
		vNewVirtualScale[1] = uid.vidHeight / 480.0;

		if (!VectorCompare2D(m_vVirtualScale, vNewVirtualScale))
		{
			m_frame.pos.x = m_frame.pos.x / m_vVirtualScale[0] * vNewVirtualScale[0];
			m_frame.pos.y = m_frame.pos.y / m_vVirtualScale[1] * vNewVirtualScale[1];
			m_frame.size.width = m_frame.size.width / m_vVirtualScale[0] * vNewVirtualScale[0];
			m_frame.size.height = m_frame.size.height / m_vVirtualScale[1] * vNewVirtualScale[1];
			m_vVirtualScale[0] = vNewVirtualScale[0];
			m_vVirtualScale[1] = vNewVirtualScale[1];
		}
	}

	if (m_flags & WF_STRETCH_VERTICAL)
	{
		m_frame.pos.y = 0;
		m_frame.size.height = uid.vidHeight;
	}
	if (m_flags & WF_STRETCH_HORIZONTAL)
	{
		m_frame.pos.x = 0;
		m_frame.size.width = uid.vidWidth;
	}

	if (m_align & WA_CENTERX)
	{
		m_frame.pos.x = uid.vidWidth * 0.5 - m_frame.size.width * 0.5;
	}

	if (m_align & WA_RIGHT)
	{
		m_frame.pos.x = uid.vidWidth - m_frame.size.width;
	}
	else if (m_align & WA_LEFT)
	{
		m_frame.pos.x = 0;
	}

	if (m_align & WA_CENTERY)
	{
		m_frame.pos.y = uid.vidHeight - m_frame.size.height;
	}

	if (m_align & WA_BOTTOM)
	{
		m_frame.pos.y = uid.vidHeight - m_frame.size.height;
	}
	else if (m_align & WA_TOP)
	{
		m_frame.pos.y = 0;
	}

	if ((m_align & WA_FULL) || (m_flags & (WF_STRETCH_HORIZONTAL | WF_STRETCH_VERTICAL)) || (m_bVirtual))
	{
		setFrame(m_frame);
		m_startingpos = m_frame.pos;
	}
}

void UIWidget::BringToFrontPropogated
	(
	void
	)

{
	for( UIWidget *at = this; at->getParent() != NULL; at = at->getParent() )
	{
		at->BringToFront();
	}
}

UIWidget *UIWidget::IsThisOrChildActive
	(
	void
	)

{
	UIWidget* child = this;

	if (child->IsActive()) {
		return child;
	}

	for( int i = 1; i <= m_children.NumObjects(); i++ )
	{
		child = m_children.ObjectAt( i );
		if( child->IsThisOrChildActive() )
		{
			return child;
		}
	}

	return NULL;
}

UIPoint2D UIWidget::MouseEventToClientPoint
	(
	Event *ev
	)

{
	UIPoint2D p;

	p.x = ev->GetFloat( 1 ) - m_screenframe.pos.x;
	p.y = ev->GetFloat( 2 ) - m_screenframe.pos.y;

	return p;
}

UIRect2D UIWidget::getClientFrame
	(
	void
	)

{
	// don't return the position
	return UIRect2D(UIPoint2D(0, 0), m_frame.size);
}

void UIWidget::setAlwaysOnBottom
	(
	bool b
	)

{
	if( getAlwaysOnBottom() != b )
	{
		if( b )
		{
			m_flags &= ~WF_ALWAYS_TOP;
			m_flags |= WF_ALWAYS_BOTTOM;
		}
		else
		{
			m_flags &= ~WF_ALWAYS_BOTTOM;
		}

		if( m_parent )
			ArrangeWidgetList( m_parent->m_children );
	}
}

void UIWidget::setAlwaysOnTop
	(
	bool b
	)

{
	if( getAlwaysOnTop() != b )
	{
		if( b )
		{
			m_flags &= ~WF_ALWAYS_BOTTOM;
			m_flags |= WF_ALWAYS_TOP;
		}
		else
		{
			m_flags &= ~WF_ALWAYS_TOP;
		}

		if( m_parent )
			ArrangeWidgetList( m_parent->m_children );
	}
}

bool UIWidget::getAlwaysOnBottom
	(
	void
	)

{
	return ( m_flags & WF_ALWAYS_BOTTOM ) ? true : false;
}

bool UIWidget::getAlwaysOnTop
	(
	void
	)

{
	return ( m_flags & WF_ALWAYS_TOP ) ? true : false;
}

bool UIWidget::SendSignal
	(
	Event& event
	)

{
	if( uWinMan.IsDead() )
	{
		return false;
	}
	else
	{
		return USignal::SendSignal( event );
	}
}

void UIWidget::ShowCommand
	(
	Event *ev
	)

{
	int i;
	str *command = new str;

	for( i = 1; i <= ev->NumArgs(); i++ )
	{
		*command += ev->GetToken( i );
	}

	m_showcommands.AddObject( command );
}

void UIWidget::HideCommand
	(
	Event *ev
	)

{
	int i;
	str *command = new str;

	for( i = 1; i <= ev->NumArgs(); i++ )
	{
		*command += ev->GetToken( i );
	}

	m_hidecommands.AddObject( command );
}

void UIWidget::AddFlag
	(
	int flag
	)

{
	m_flags |= flag;
}

void UIWidget::SetHovermaterialActive
	(
	bool a
	)

{
	m_hovermaterial_active = a;
}

void UIWidget::SetPressedmaterialActive
	(
	bool a
	)

{
	m_pressedmaterial_active = a;
}

int UIWidget::getOrderNum
	(
	void
	)

{
	return m_ordernum;
}

str UIWidget::getCommand
	(
	void
	)

{
	return m_command;
}

void UIWidget::ActivateOrder
	(
	void
	)

{
	UIWidget *selwidget;

	for( selwidget = this; selwidget != NULL; selwidget = selwidget->getParent() )
	{
		if( selwidget->getParent() == &uWinMan )
		{
			if( selwidget->isSubclassOf( UIWidgetContainer ) )
			{
				UIWidgetContainer *widcon = ( UIWidgetContainer * )selwidget;
				widcon->SetActiveWidgetOrderNum( this );
			}

			return;
		}
	}
}

void UIWidget::EnableEvent
	(
	Event *ev
	)

{
	Enable();
}

void UIWidget::DisableEvent
	(
	Event *ev
	)

{
	Disable();
}

void UIWidget::setConfigstringIndex
	(
	int cs
	)

{
	m_configstring_index = cs;
}

int UIWidget::getConfigstringIndex
	(
	void
	)

{
	return m_configstring_index;
}

bool UIWidget::PassEventToWidget
	(
	str name,
	Event *ev
	)

{
	int i;
	int n;

	if( m_enabledCvar.length() )
	{
		if( !UI_GetCvarInt( m_cvarname, NULL ) )
			return false;
	}

	if( name.length() )
	{
		n = m_children.NumObjects();
		for( i = 1; i <= n; i++ )
		{
			if( m_children.ObjectAt( i )->PassEventToWidget( name, ev ) )
				return true;
		}
	}

	ProcessEvent( ev );

	return true;
}

CLASS_DECLARATION( UIWidget, UIWidgetContainer, NULL )
{
	{ &EV_Layout_BGFill,					&UIWidgetContainer::SetBGFill },
	{ &EV_Layout_Fullscreen,				&UIWidgetContainer::SetFullscreen },
	{ &EV_Layout_VidMode,					&UIWidgetContainer::SetVidMode },
	{ NULL, NULL }
};

UIWidgetContainer::UIWidgetContainer()
{
	m_bgfill = UColor(0, 0, 0, 0);
	m_fullscreen = false;
	m_vidmode = -1;
	m_currentwidnum = 0;
	m_maxordernum = 0;
	m_layout = NULL;
	AllowActivate( false );
}

void UIWidgetContainer::AlignPosition
	(
	void
	)

{
	if (m_bVirtual)
	{
		vec2_t vNewVirtualScale;

		vNewVirtualScale[0] = uid.vidWidth / 640.0;
		vNewVirtualScale[1] = uid.vidHeight / 480.0;

		if (!VectorCompare2D(m_vVirtualScale, vNewVirtualScale))
		{
			m_frame.pos.x = m_frame.pos.x / m_vVirtualScale[0] * vNewVirtualScale[0];
			m_frame.pos.y = m_frame.pos.y / m_vVirtualScale[1] * vNewVirtualScale[1];
			m_frame.size.width = m_frame.size.width / m_vVirtualScale[0] * vNewVirtualScale[0];
			m_frame.size.height = m_frame.size.height / m_vVirtualScale[1] * vNewVirtualScale[1];
			m_vVirtualScale[0] = vNewVirtualScale[0];
			m_vVirtualScale[1] = vNewVirtualScale[1];
		}
	}

	if (m_align & WA_LEFT)
	{
		m_frame.pos.x = 0;
	}
	else if (m_align & WA_RIGHT)
	{
		m_frame.pos.x = uid.vidWidth - m_frame.size.width;
	}
	else
	{
		// default to center
		m_frame.pos.x = (uid.vidWidth - m_frame.size.width) * 0.5;
	}

	if (m_align & WA_BOTTOM)
	{
		m_frame.pos.y = uid.vidHeight - m_frame.size.height;
	}
	else if (m_align & WA_TOP)
	{
		m_frame.pos.y = 0;
	}
	else
	{
		// default to center
		m_frame.pos.y = (uid.vidHeight - m_frame.size.height) * 0.5;
	}

	setFrame(m_frame);
	m_startingpos = m_frame.pos;
}

void UIWidgetContainer::Draw
	(
	void
	)

{
	uii.Rend_Set2D( 0, 0, uid.vidWidth, uid.vidHeight, 0, uid.vidWidth, uid.vidHeight, 0, -1.0, 1.0 );
	uii.Rend_Scissor( 0, 0, uid.vidWidth, uid.vidHeight );

	if( m_bgfill.a != 0.0 )
		DrawBox( 0, 0, uid.vidWidth, uid.vidHeight, m_bgfill, m_local_alpha );

	set2D();
}

void UIWidgetContainer::Realign
	(
	void
	)

{
	AlignPosition();
}

void UIWidgetContainer::SetBGFill
	(
	Event *ev
	)

{
	m_bgfill.r = ev->GetFloat( 1 );
	m_bgfill.b = ev->GetFloat( 2 );
	m_bgfill.g = ev->GetFloat( 3 );
	m_bgfill.a = ev->GetFloat( 4 );
}

void UIWidgetContainer::SetFullscreen
	(
	Event *ev
	)

{
	m_fullscreen = ev->GetBoolean( 1 );
}

void UIWidgetContainer::SetVidMode
	(
	Event *ev
	)

{
	m_vidmode = ev->GetInteger( 1 );
}

int UIWidgetContainer::getVidMode
	(
	void
	)

{
	return m_vidmode;
}

qboolean UIWidgetContainer::isFullscreen
	(
	void
	)

{
	return m_fullscreen;
}

UIWidget *UIWidgetContainer::GetNextWidgetInOrder
	(
	void
	)

{
	int i;

	if( !m_maxordernum )
	{
		for( i = 1; i <= m_children.NumObjects(); i++ )
		{
			if( m_children.ObjectAt( i )->getOrderNum() )
				m_maxordernum++;
		}
	}

	m_currentwidnum++;

	if( m_currentwidnum > m_maxordernum )
		m_currentwidnum = m_maxordernum;

	for( i = 1; i <= m_children.NumObjects(); i++ )
	{
		UIWidget *wid = m_children.ObjectAt( i );

		if( wid->getOrderNum() == m_currentwidnum )
			return wid;
	}

	return NULL;
}

UIWidget *UIWidgetContainer::GetPrevWidgetInOrder
	(
	void
	)

{
	int i;

	if( !m_maxordernum )
	{
		for( i = 1; i <= m_children.NumObjects(); i++ )
		{
			if( m_children.ObjectAt( i )->getOrderNum() )
				m_maxordernum++;
		}
	}

	m_currentwidnum--;

	if( m_currentwidnum <= 0 )
		m_currentwidnum = m_maxordernum;

	for( i = 1; i <= m_children.NumObjects(); i++ )
	{
		UIWidget *wid = m_children.ObjectAt( i );

		if( wid->getOrderNum() == m_currentwidnum )
			return wid;
	}

	return NULL;
}

void UIWidgetContainer::SetActiveWidgetOrderNum
	(
	UIWidget *wid
	)

{
	int i;

	for( i = 1; i <= m_children.NumObjects(); i++ )
	{
		if( wid == m_children.ObjectAt( i ) )
			m_currentwidnum = wid->getOrderNum();
	}
}

void UIWidgetContainer::SetActiveWidgetOrderNum
	(
	int num
	)

{
	UIWidget *wid;
	int i;

	for( i = 1; i <= m_children.NumObjects(); i++ )
	{
		wid = m_children.ObjectAt( i );

		if( wid->getOrderNum() == num )
		{
			uWinMan.ActivateControl( wid );
		}
		else
		{
			wid->SetHovermaterialActive( false );
			wid->SetPressedmaterialActive( false );
		}
	}
}

void UIWidgetContainer::SetLastActiveWidgetOrderNum
	(
	void
	)

{
	if( !m_currentwidnum )
		m_currentwidnum = 1;

	SetActiveWidgetOrderNum( m_currentwidnum );
}
