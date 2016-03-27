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

#ifndef __CL_INVRENDER_H__
#define __CL_INVRENDER_H__

class FakkInventory;

typedef enum { selectsound, rejectsound, changesound } invsound_t;
class FakkItemList : public UIWidget {
protected:
	inventory_type_t *type;
	uipopup_describe m_describe[ 10 ];
	inventory_item_t *m_hoveritem;
	inventory_item_t *m_lastmenuitem;
	FakkInventory *m_parent;
	float m_hovertop;

public:
	CLASS_PROTOTYPE( FakkItemList );

	FakkItemList();
	~FakkItemList();

	void	VerifyItemUp( inventory_item_t *item, qboolean warpmouse );
	void	setType( inventory_type_t *t );
	void	Create( float x, float y, FakkInventory *parent );
	bool	HasAnyItems( void );
	void	Draw( void );
	void	OnLeftMouseDown( Event *ev );
	void	OnRightMouseDown( Event *ev );
	void	OnLeftMouseUp( Event *ev );
	void	OnRightMouseUp( Event *ev );
	void	OnMouseMove( Event *ev );
	void	OnMouseEnter( Event *ev );
	void	OnMouseLeave( Event *ev );
	void	OnMenuKilled( Event *ev );
	void	EquipItem( Event *ev );
};

class FakkInventory : public UIWidget {
protected:
	inventory_t *m_inv;
	SafePtr<FakkItemList> m_currentlist;
	inventory_type_t *m_currenttype;
	int m_currentitemnum;

public:
	CLASS_PROTOTYPE( FakkInventory );

protected:
	void		VerifyItemUp( inventory_item_t *item, qboolean warpmouse );

public:
	FakkInventory();
	~FakkInventory();

	void		setInventory( inventory_t *i );
	void		Draw( void );
	void		OnMouseMove( Event *ev );
	void		OnMouseLeave( Event *ev );
	void		OnMouseEnter( Event *ev );
	void		OnMouseDown( Event *ev );
	void		WarpTo( const char *name );
	void		WarpTo( int slotnum );
	void		NextItem( void );
	void		PrevItem( void );
	qboolean	isDying( void );
	qboolean	KeyEvent( int key, unsigned int time );
	void		PlaySound( invsound_t type );
	void		Timeout( Event *ev );
	void		ChangeItem( int sign );
	int			FindFirstItem( int itemindex, int sign );
};

void CL_Draw3DModel( float x, float y, float w, float h, qhandle_t model, vec3_t origin, vec3_t rotateoffset, vec3_t offset, vec3_t angle, vec3_t color, str anim );
qboolean UI_CloseInventory( void );

#endif // __CL_INVRENDER_H__
