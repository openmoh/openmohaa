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

MenuManager menuManager;

Event EV_PushMenu
	(
	"pushmenu",
	EV_DEFAULT,
	"s",
	"menuname",
	"Pushes the menu on the stack"
	);

Event EV_LockMenus
	(
	"lock",
	EV_DEFAULT,
	NULL,
	NULL,
	"Lock out the menu from receiving input"
	);

Event EV_UnlockMenus
	(
	"unlock",
	EV_DEFAULT,
	NULL,
	NULL,
	"Unlock the menu from receiving input"
	);

Event EV_ShowMenu
	(
	"showmenu",
	EV_DEFAULT,
	"B",
	"activate",
	"Shows the menu."
	);

Event EV_HideMenu
	(
	"hidemenu",
	EV_DEFAULT,
	NULL,
	NULL,
	"Hides the menu."
	);

CLASS_DECLARATION( Listener, Menu, NULL )
{
	{ &EV_HideMenu,			&Menu::HideMenu },
	{ &EV_ShowMenu,			&Menu::ShowMenu },
	{ NULL, NULL }
};

Menu::Menu()
{
	menuManager.AddMenu( this );
	m_fullscreen = qfalse;
}

Menu::Menu
	(
	str name
	)

{
	setName( name );
	m_fullscreen = qfalse;
	menuManager.AddMenu( this );
}

void Menu::AddMenuItem
	(
	UIWidget *item
	)

{
	m_itemlist.AddObject( item );

	// add all item's children
	for( int i = 1; i <= item->m_children.NumObjects(); i++ )
	{
		m_itemlist.AddObject( item->m_children.ObjectAt( i ) );
	}
}

void Menu::DeleteMenuItem
	(
	UIWidget *item
	)

{
	// remove all item's children
	for( int i = item->m_children.NumObjects(); i > 0; i-- )
	{
		m_itemlist.RemoveObject( item->m_children.ObjectAt( i ) );
	}

	m_itemlist.RemoveObject( item );
}

void Menu::setName
	(
	str name
	)

{
	m_name = name;
}

void Menu::ShowMenu
	(
	Event *ev
	)

{
	qboolean activate;
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );
		UIWidgetContainer *widcon = ( UIWidgetContainer * )wid;

		if( wid->isSubclassOf( UIWidgetContainer ) && widcon->m_layout )
		{
			widcon->m_layout->ForceLoad();
		}

		if( wid->isEnabled() )
		{
			wid->setShow( true );
			wid->ResetMotion( MOTION_IN );
			wid->UpdateUIElement();
			wid->UpdateData();
			wid->ExecuteShowCommands();
		}
	}

	if( ev && ev->NumArgs() > 0 ) {
		activate = ev->GetBoolean( 1 );
	} else {
		activate = qtrue;
	}

	if( n && activate )
	{
		UIWidgetContainer *widcon = ( UIWidgetContainer * )m_itemlist.ObjectAt( 1 );
		uWinMan.ActivateControl( widcon );
		widcon->SetLastActiveWidgetOrderNum();
	}

	uWinMan.setFirstResponder( NULL );
}

void Menu::HideMenu
	(
	Event *ev
	)

{
	int i, n;
	bool force = false;
	float maxtime;

	maxtime = GetMaxMotionTime();

	if( ev->NumArgs() > 0 )
		force = ev->GetBoolean( 1 );

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );

		if( wid->getMotionType() != MOTION_OUT )
		{
			wid->ResetMotion( MOTION_OUT );
			if( !force )
			{
				Event *event = new Event( "hide" );
				wid->PostEvent( event, maxtime );
			}
			else {
				wid->setShow(false);
			}
		}
	}
}

void Menu::ForceShow
	(
	void
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );
		UIWidgetContainer *widcon = ( UIWidgetContainer * )wid;

		if( wid->isSubclassOf( UIWidgetContainer ) && widcon->m_layout )
		{
			widcon->m_layout->ForceLoad();
		}

		if( wid->isEnabled() )
		{
			wid->setShow( true );
		}
	}
}

void Menu::ForceHide
	(
	void
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );

		wid->ExecuteHideCommands();
		wid->setShow( false );
	}
}

UIWidget *Menu::GetContainerWidget
	(
	void
	)

{
	if( m_itemlist.NumObjects() > 0 ) {
		return m_itemlist.ObjectAt( 1 );
	} else {
		return NULL;
	}
}

UIWidget *Menu::GetNamedWidget
	(
	const char *pszName
	)

{
	int i;
	UIWidget *pWidget;

	for( i = 1; i <= m_itemlist.NumObjects(); i++ )
	{
		pWidget = m_itemlist.ObjectAt( i );
		if( !stricmp( pszName, pWidget->getName() ) )
		{
			return pWidget;
		}
	}

	return pWidget;
}

void Menu::Update
	(
	void
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );
		wid->UpdateUIElement();
		wid->UpdateData();
	}
}

void Menu::RealignWidgets
	(
	void
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );
		wid->Realign();
	}
}

float Menu::GetMaxMotionTime
	(
	void
	)

{
	int i;
	int n;
	float maxtime = 0.0;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );
		if( maxtime < wid->getMotionTime() )
			maxtime = wid->getMotionTime();
	}

	return maxtime;
}

void Menu::ActivateMenu
	(
	void
	)

{
	if( m_itemlist.NumObjects() )
	{
		UIWidgetContainer *widcon = ( UIWidgetContainer * )m_itemlist.ObjectAt( 1 );
		uWinMan.ActivateControl( widcon );
		widcon->SetLastActiveWidgetOrderNum();
		uWinMan.setFirstResponder( NULL );
		widcon->BringToFrontPropogated();
	}
}

qboolean Menu::isFullscreen
	(
	void
	)

{
	return m_fullscreen;
}

void Menu::setFullscreen
	(
	qboolean bFullScreen
	)

{
	m_fullscreen = bFullScreen;
}

int Menu::getVidMode
	(
	void
	)

{
	return m_vidmode;
}

void Menu::setVidMode
	(
	int iMode
	)

{
	m_vidmode = iMode;
}

qboolean Menu::isVisible
	(
	void
	)

{
	UIWidget *wid = GetContainerWidget();
	if( wid )
		return wid->IsVisible();

	return false;
}

void Menu::SaveCVars
	(
	void
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );

		if( wid->m_cvarname.length() )
		{
			const char *ret = UI_GetCvarString( wid->m_cvarname, NULL );
			if( ret )
				wid->m_cvarvalue = ret;
			else
				wid->m_cvarvalue = "";
		}
	}
}

void Menu::RestoreCVars
	(
	void
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );

		if( wid->m_cvarname.length() )
		{
			uii.Cvar_Set( wid->m_cvarvalue, "" );
		}
	}
}

void Menu::ResetCVars
	(
	void
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );

		if( wid->m_cvarname.length() )
		{
			uii.Cvar_Reset( wid->m_cvarname );
		}
	}
}

void Menu::PassEventToWidget
	(
	str name,
	Event *ev
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );
		if( wid->getName() == name && wid->ValidEvent( ev->getName() ) )
		{
			wid->ProcessEvent( ev );
		}
	}
}

void Menu::PassEventToAllWidgets
	(
	Event& ev
	)

{
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );
		if( wid->ValidEvent( ev.getName() ) )
		{
			wid->ProcessEvent( ev );
		}
	}
}

void Menu::CheckRestart
	(
	void
	)

{
	cvar_t *cvar;
	qboolean do_restart = false;
	qboolean do_snd_restart = false;
	qboolean do_ter_restart = false;
	int i;
	int n;

	n = m_itemlist.NumObjects();
	for( i = 1; i <= n; i++ )
	{
		UIWidget *wid = m_itemlist.ObjectAt( i );

		if( wid->m_cvarname.length() )
		{
			cvar = uii.Cvar_Find( wid->m_cvarname );
			if( cvar && cvar->latchedString )
			{
				if( str::icmp( wid->m_cvarvalue, cvar->latchedString ) )
				{
					if( cvar->flags & CVAR_LATCH )
					{
						do_restart = true;
					}
					else if( cvar->flags & CVAR_SOUND_LATCH )
					{
						do_snd_restart = true;
					}
					else if( cvar->flags & CVAR_TERRAIN_LATCH )
					{
						do_ter_restart = true;
					}
				}
			}
		}
	}

	if( do_restart )
	{
		uii.Cmd_Stuff( "vid_restart\n" );
	}
	else
	{
		if( do_snd_restart ) {
			uii.Cmd_Stuff( "snd_restart\n" );
		}

		if( do_ter_restart ) {
			uii.Cmd_Stuff( "ter_restart\n" );
		}
	}
}

CLASS_DECLARATION( Listener, MenuManager, NULL )
{
	{ &EV_PushMenu,			&MenuManager::PushMenu },
	{ &EV_LockMenus,		&MenuManager::Lock },
	{ &EV_UnlockMenus,		&MenuManager::Unlock },
	{ NULL, NULL }
};

MenuManager::MenuManager()
{
	m_lock = false;
}

void MenuManager::RealignMenus
	(
	void
	)

{
	int i;
	int num;

	num = m_menulist.NumObjects();
	for (i = 1; i <= num; i++)
	{
		m_menulist.ObjectAt(i)->RealignWidgets();
	}
}

void MenuManager::AddMenu
	(
	Menu *m
	)

{
	m_menulist.AddObject( m );
}

void MenuManager::DeleteMenu
	(
	Menu *m
	)

{
	m_menulist.RemoveObject( m );
}

void MenuManager::DeleteAllMenus
	(
	void
	)

{
	for( int i = m_menulist.NumObjects(); i > 0; i-- )
	{
		Menu *menu = m_menulist.ObjectAt( i );
		delete menu;
	}
}

Menu *MenuManager::FindMenu
	(
	str name
	)

{
	int i;
	int count;

	count = m_menulist.NumObjects();

	for( i = 1; i <= count; i++ )
	{
		Menu *m = m_menulist.ObjectAt( i );

		if( !str::icmp( m->m_name, name ) )
			return m;
	}

	return NULL;
}

bool MenuManager::PushMenu
	(
	str name
	)

{
	Menu *m;
	float maxouttime = 0.0;
	float maxintime;
	Menu *head;

	if( m_lock )
	{
		return false;
	}

	m = FindMenu( name );
	if( !m )
	{
		uii.Sys_Printf( "Couldn't find menu %s\n", name.c_str() );
		return false;
	}

	// don't push the same menu
	head = m_menustack.Head();
	if( head == m )
	{
		return false;
	}

	m->SaveCVars();

	if( head )
	{
		maxouttime = m->GetMaxMotionTime();
		head->ProcessEvent( Event( "hidemenu" ) );
	}

	if( maxouttime == 0.0 )
	{
		m->ProcessEvent( Event( "showmenu" ) );
	}
	else
	{
		m->PostEvent( Event( "showmenu" ), maxouttime );
	}

	maxintime = m->GetMaxMotionTime();

	Lock( NULL );
	PostEvent( EV_UnlockMenus, maxintime + maxouttime );

	m_menustack.Push( m );

	return true;
}

bool MenuManager::ShowMenu
	(
	str name
	)

{
	Menu *m;
	float maxintime;

	if( m_lock )
	{
		return false;
	}

	m = FindMenu( name );
	if( !m )
	{
		uii.Sys_Printf( "Couldn't find menu %s\n", name.c_str() );
		return false;
	}

	m->SaveCVars();
	m->ProcessEvent( new Event( "showmenu" ) );

	maxintime = m->GetMaxMotionTime();

	Lock( NULL );
	PostEvent( EV_UnlockMenus, maxintime );

	m_showmenustack.Push( m );

	return true;
}

void MenuManager::PushMenu
	(
	Event *ev
	)

{
	if( !ui_pLoadingMenu || CurrentMenu() != ui_pLoadingMenu )
	{
		PushMenu( ev->GetString( 1 ) );
	}
}

void MenuManager::PopMenu
	(
	qboolean restore_cvars
	)

{
	Menu *top;
	Menu *head;
	float maxtime = 0.0;

	if( m_lock )
	{
		return;
	}

	top = m_showmenustack.Pop();
	if( top )
	{
		if( restore_cvars ) {
			top->RestoreCVars();
		}

		//maxtime = top->GetMaxMotionTime();
		top->ProcessEvent( Event( "hidemenu" ) );
	}
	else
	{
		top = m_menustack.Pop();
		if( top )
		{
			if( restore_cvars ) {
				top->RestoreCVars();
			}

			maxtime = top->GetMaxMotionTime();
			top->ProcessEvent( Event( "hidemenu" ) );

			head = m_menustack.Head();
			if( head )
			{
				head->PostEvent( Event( "showmenu" ), maxtime );
			}
		}

		Lock(NULL);
		PostEvent(EV_UnlockMenus, maxtime);
	}
}

Menu *MenuManager::CurrentMenu
	(
	void
	)

{
	Menu *head;

	head = m_showmenustack.Head();
	if( head )
	{
		return head;
	}

	head = m_menustack.Head();
	if( head )
	{
		return head;
	}

	return NULL;
}

bool MenuManager::ClearMenus
	(
	bool force
	)

{
	int i, num;
	Menu* top;
	Menu* menu;
	float maxtime;
	float newmaxtime;

	maxtime = 0.0;
	if (m_lock) {
		return false;
	}

	if (!m_showmenustack.Head() &&!m_menustack.Head()) {
		return true;
	}

	top = m_showmenustack.Head();
	while (!m_showmenustack.Empty())
	{
		menu = m_showmenustack.Pop();
		newmaxtime = top->GetMaxMotionTime();
		if (maxtime < newmaxtime) {
			maxtime = newmaxtime;
		}

		if (force)
		{
			// force the menu to hide
			menu->ForceHide();
			continue;
		}

		// otherwise send the "hidemenu" event
		// the menu will process it or not
		menu->ProcessEvent(new Event("hidemenu"));
	}

	top = m_menustack.Head();
	while (!m_menustack.Empty())
	{
		menu = m_menustack.Pop();
		newmaxtime = top->GetMaxMotionTime();
		if (maxtime < newmaxtime) {
			maxtime = newmaxtime;
		}

		if (force)
		{
			// force the menu to hide
			menu->ForceHide();
			continue;
		}

		// otherwise send the "hidemenu" event
		// the menu will process it or not
		menu->ProcessEvent(new Event("hidemenu"));
	}

	num = m_menulist.NumObjects();
	for (i = 1; i <= num; i++)
	{
		menu = m_menulist.ObjectAt(i);
		if (!menu->isVisible()) {
			continue;
		}

		newmaxtime = top->GetMaxMotionTime();
		if (maxtime < newmaxtime) {
			maxtime = newmaxtime;
		}

		if (force)
		{
			menu->ForceHide();
			continue;
		}

		menu->ProcessEvent(new Event("hidemenu"));
	}

	if (!force)
	{
		Lock(NULL);
		PostEvent(EV_UnlockMenus, maxtime);
	}

	return true;
}

void MenuManager::ListMenus
	(
	void
	)

{
	int i;
	int num;

	num = m_menulist.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		Menu *menu = m_menulist.ObjectAt( i );
		uii.Sys_Printf( "%s\n", menu->m_name.c_str() );
	}
}

void MenuManager::UpdateAllMenus
	(
	void
	)

{
	int i;
	int num;

	num = m_menulist.NumObjects();
	for( i = 1; i <= num; i++ )
	{
		Menu *menu = m_menulist.ObjectAt( i );
		menu->Update();
	}
}

void MenuManager::Lock
	(
	Event *ev
	)

{
	m_lock = true;
}

void MenuManager::Unlock
	(
	Event *ev
	)

{
	m_lock = false;
}

void MenuManager::PassEventToWidget
	(
	str name,
	Event *ev
	)

{
	Menu *head = CurrentMenu();

	if( head )
	{
		head->PassEventToWidget( name, ev );
	}
}

void MenuManager::PassEventToAllWidgets
	(
	Event& ev
	)

{
	int i;
	int iMenuCount;

	iMenuCount = m_menulist.NumObjects();
	for( i = 1; i <= iMenuCount; i++ )
	{
		Menu *pMenu = m_menulist.ObjectAt( i );
		pMenu->PassEventToAllWidgets( ev );
	}
}

void MenuManager::ResetCVars
	(
	void
	)

{
	Menu *head = CurrentMenu();

	if( head )
	{
		head->ResetCVars();
	}
}

void MenuManager::CheckRestart
	(
	void
	)

{
	Menu *head = CurrentMenu();

	if( head )
	{
		head->CheckRestart();
	}
}

bool MenuManager::ForceMenu
	(
	str name
	)

{
	Menu *m;
	Menu *head;


	m = FindMenu( name );
	if( !m )
	{
		uii.Sys_Printf( "Couldn't find menu %s\n", name.c_str() );
		return false;
	}

	// don't push the same menu
	head = m_menustack.Head();
	if( head == m )
	{
		return false;
	}

	m->SaveCVars();

	if( head )
	{
		head->ForceHide();
	}

	m->ProcessEvent( Event( "showmenu" ) );
	// add the new menu to the stack
	m_menustack.Push(m);

	return true;
}

