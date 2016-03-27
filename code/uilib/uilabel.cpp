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

Event EV_UILabel_LinkString
	(
	"linkstring",
	EV_DEFAULT,
	"is",
	"value string",
	"Creates a link from the specified value to a string. "
	"Use this if you want the label to display a string different from the value of the cvar"
	);

Event EV_UILabel_LinkCvarToShader
	(
	"linkcvartoshader",
	EV_DEFAULT,
	NULL,
	NULL,
	"Links the label's cvar to its shader"
	);

CLASS_DECLARATION( UIWidget, UILabel, NULL )
{
	{ &W_MouseEntered,					&UILabel::MouseEntered },
	{ &W_MouseExited,					&UILabel::MouseExited },
	{ &EV_UILabel_LinkString,			&UILabel::LinkString },
	{ &EV_UILabel_LinkCvarToShader,		&UILabel::SetLinkCvarToShader },
	{ &EV_Layout_Shader,				&UILabel::LabelLayoutShader },
	{ &EV_Layout_TileShader,			&UILabel::LabelLayoutTileShader },
	{ NULL, NULL }
};

UILabel::UILabel()
{
	AllowActivate( false );
	m_bLinkCvarToShader = false;
}

void UILabel::MouseEntered
	(
	Event *ev
	)

{
	SetHovermaterialActive( true );
}

void UILabel::MouseExited
	(
	Event *ev
	)

{
	SetHovermaterialActive( false );
	SetPressedmaterialActive( false );
}

void UILabel::LinkString
	(
	Event *ev
	)

{
	linkstring *ls;
	str value;
	str string;

	value = ev->GetString( 1 );
	string = ev->GetString( 2 );

	ls = new linkstring( value, string );

	m_linkstrings.AddObject( ls );
}

int UILabel::FindLinkString
	(
	str val
	)

{
	int i;

	for( i = 1; i <= m_linkstrings.NumObjects(); i++ )
	{
		if( val == m_linkstrings.ObjectAt( i )->value )
			return i;
	}

	return 0;
}

void UILabel::LabelLayoutShader
	(
	Event *ev
	)

{
	m_sCurrentShaderName = ev->GetString( 1 );
	setMaterial( uWinMan.RegisterShader( m_sCurrentShaderName ) );

	m_flags &= ~WF_TILESHADER;
}

void UILabel::LabelLayoutTileShader
	(
	Event *ev
	)

{
	m_sCurrentShaderName = ev->GetString( 1 );
	setMaterial( uWinMan.RegisterShader( m_sCurrentShaderName ) );

	m_flags |= WF_TILESHADER;
}

void UILabel::SetLinkCvarToShader
	(
	Event *ev
	)

{
	m_bLinkCvarToShader = true;
}

void UILabel::SetLabel
	(
	str lab
	)

{
	label = lab;
}

void UILabel::Draw
	(
	void
	)

{
	const char *string;
	str val;

	if( m_cvarname.length() )
	{
		string = uii.Cvar_GetString( m_cvarname, "" );

		if( m_linkstrings.NumObjects() )
		{
			cvar_t *cvar = UI_FindCvar( m_cvarname );
			if( cvar )
			{
				int ret;

				if( cvar->latchedString )
					val = cvar->latchedString;
				else
					val = cvar->string;

				ret = FindLinkString( val );
				if( ret )
				{
					string = m_linkstrings.ObjectAt( ret )->value;
				}
			}
		}

		if( m_bLinkCvarToShader )
		{
			if( str::icmp( m_sCurrentShaderName, string ) )
			{
				m_sCurrentShaderName = string;

				setMaterial( uWinMan.RegisterShader( m_sCurrentShaderName ) );
				m_material->ReregisterMaterial();

				if( !m_material->GetMaterial() )
				{
					setMaterial( NULL );
				}
			}

			if( label.length() )
			{
				string = label;
			}
			else if( m_title.length() )
			{
				string = m_title;
			}
			else
			{
				string = "";
			}
		}
	}
	else
	{
		if( label.length() )
		{
			string = label;
		}
		else if( m_title.length() )
		{
			string = m_title;
		}
		else
		{
			string = "";
		}
	}

	if( *string )
	{
		float nr, ng, nb;

		nr = m_foreground_color.r;
		ng = m_foreground_color.g;
		nb = m_foreground_color.b;

		m_font->setColor( UColor( nr, ng, nb, m_foreground_color.a * m_local_alpha ) );

		// print the text
		m_font->PrintJustified( getClientFrame(),
			m_iFontAlignmentHorizontal,
			m_iFontAlignmentVertical,
			Sys_LV_CL_ConvertString( string ),
			m_bVirtual ? m_vVirtualScale : NULL );
	}
}
