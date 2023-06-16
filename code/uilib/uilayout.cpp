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
#include "../script/scriptexception.h"

Event EV_Layout_Menu
	(
	"menu",
	EV_DEFAULT,
	"sffsF",
	"name width height direction motion_time",
	"Sets up the layout of the menu."
	);

CLASS_DECLARATION( Listener, UILayout, NULL )
{
	{ &EV_Layout_Menu,			&UILayout::CreateWidgetContainer },
	{ NULL, NULL }
};

UILayout::UILayout()
{
	BogusFunction();
	m_script = NULL;
	m_filename = NULL;
}

UILayout::UILayout
	(
	const char *filename
	)

{
	BogusFunction();
	m_script = NULL;
	m_filename = filename;

	if( filename )
		Load( filename, true );
}

void UILayout::BogusFunction
	(
	void
	)

{
}

void UILayout::CreateWidgetContainer
	(
	Event *ev
	)

{
	str name;
	str direction;
	float w, h;

	name = ev->GetString( 1 );

	if( !name.length() )
		return;

	w = ev->GetFloat( 2 );
	h = ev->GetFloat( 3 );
	direction = ev->GetString( 4 );

	m_currentcontainer = new UIWidgetContainer;
	m_currentcontainer->setName( name );

	if( !stricmp( direction, "from_bottom" ) )
	{
		m_currentcontainer->setDirection( D_FROM_BOTTOM );
	}
	else if( !stricmp( direction, "from_top" ) )
	{
		m_currentcontainer->setDirection( D_FROM_TOP );
	}
	else if( !stricmp( direction, "from_left" ) )
	{
		m_currentcontainer->setDirection( D_FROM_LEFT );
	}
	else if( !stricmp( direction, "from_right" ) )
	{
		m_currentcontainer->setDirection( D_FROM_RIGHT );
	}
	else
	{
		m_currentcontainer->setDirection(D_NONE);
	}

	if( ev->NumArgs() > 4 )
	{
		m_currentcontainer->setMotionTime( ev->GetFloat( 5 ) );
	}

	m_currentcontainer->setAlwaysOnBottom( true );
	m_currentcontainer->InitFrame( NULL, 0, 0, w, h );
	m_currentcontainer->m_layout = this;
	m_currentwidget = m_currentcontainer;
}

void UILayout::ProcessCommands
	(
	bool bFullLoad
	)

{
	const char		*token;
	Event			*ev;
	UIWidget		*widget;
	str				postinclude;

	try
	{
		while( m_script->TokenAvailable( true ) )
		{
			token = m_script->GetToken( true );

			if( !strcmp( token, "end." ) )
			{
				break;
			}
			else if( !strcmp( token, "include" ) )
			{
				if( m_script->TokenAvailable( false ) )
				{
					m_scriptstack.Push( m_script );
					Load( m_script->GetToken( false ), bFullLoad );
					m_script = m_scriptstack.Pop();
				}
			}
			else if( !strcmp( token, "postinclude" ) )
			{
				if( m_script->TokenAvailable( false ) )
				{
					token = m_script->GetToken( false );
					postinclude = token;
				}
			}
			else if( !strcmp( token, "resource" ) )
			{
				ClassDef *cls;
				str cl;
				const char *newname;

				token = m_script->GetToken( true );

				// find the class resource
				newname = uii.Client_TranslateWidgetName( token );
				if( !newname ) newname = token;

				cl = "UI";
				cl += newname;
				cls = getClass( cl );
				if( !cls )
				{
					Com_Printf( "Failed to find resource type '%s'\n", token );
					break;
				}

				widget = ( UIWidget * )cls->newInstance();

				token = m_script->GetToken( true );
				if( *token == '{' )
				{
					widget->InitFrame( m_currentcontainer, 0, 0, 128, 64, -1, "verdana-12" );

					if( m_currentcontainer->m_bVirtual )
					{
						ev = new Event( EV_Layout_VirtualRes );
						ev->AddInteger( 1 );
						widget->ProcessEvent( ev );
					}

					token = m_script->GetToken( true );
					while( 1 )
					{
						if( !strcmp( token, "}" ) )
						{
							break;
						}

						if( widget->ValidEvent( token ) )
						{
							ev = new Event( token );

							while( m_script->TokenAvailable( false ) )
							{
								token = m_script->GetToken( false );
								ev->AddToken( token );
							}

							widget->ProcessEvent( ev );
						}

						if( !m_script->TokenAvailable( true ) )
							break;

						token = m_script->GetToken( true );
					}
				}
			}
			else
			{
				if( ValidEvent( token ) )
				{
					ev = new Event( token );

					while( m_script->TokenAvailable( false ) )
					{
						token = m_script->GetToken( false );
						ev->AddToken( token );
					}

					ProcessEvent( ev );
				}
				else if( m_currentwidget && m_currentwidget->ValidEvent( token ) )
				{
					ev = new Event( token );

					while( m_script->TokenAvailable( false ) )
					{
						token = m_script->GetToken( false );
						ev->AddToken( token );
					}

					m_currentwidget->ProcessEvent( ev );
				}
			}
		}
	}
	catch( ScriptException& exc )
	{
		uii.Sys_Printf( "UI EXCEPTION : %s\n", exc.string.c_str() );
	}

	// process post-includes
	if( postinclude.length() )
	{
		m_scriptstack.Push( m_script );
		Load( postinclude, bFullLoad );
		m_script = m_scriptstack.Pop();
	}
}

void UILayout::Load
	(
	const char *filename,
	bool bFullLoad
	)

{
	m_bLoaded = true;

	m_script = new Script;
	m_script->LoadFile( filename );

	ProcessCommands( bFullLoad );

	m_bLoaded = bFullLoad;

	delete m_script;
	m_script = NULL;
}

int UILayout::ForceLoad
	(
	void
	)

{
	if( m_bLoaded )
		return 0;

	Load( m_filename, true );

	return true;
}
