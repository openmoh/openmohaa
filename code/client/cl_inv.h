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

#ifndef __CL_INV_H__
#define __CL_INV_H__

typedef enum { INV_MOVE_NONE, INV_MOVE_BOB, INV_MOVE_SPIN } inv_move_type;
typedef enum { INV_CASCADE_LEFT, INV_CASCADE_RIGHT } inv_cascade_type;
typedef enum { INV_HUDANGLES_BASE, INV_HUDANGLES_COMPASS, INV_HUDANGLES_COMPASS_NEEDLE } inv_hudangles_type;

class item_properties_t : public Class {
public:
	float scale;
	Vector angles;
	Vector angledeltas;
	Vector rotateoffset;
	Vector offset;
	inv_move_type move;
	str model;
	str anim;
};

class inventory_item_t : public Class {
public:
	str name;
	str ammoname;
	int equip;
	int width;
	int height;
	int barwidth;
	int barheight;
	int baroffsetY;
	int baroffsetX;
	float modelWindowX;
	float modelWindowY;
	float modelWindowWidth;
	float modelWindowHeight;
	bool selShaderOnTop;
	bool checkammo;
	str command;
	inv_hudangles_type anglesType;
	item_properties_t hudprops;
	item_properties_t invprops;
	UIReggedMaterial *bgshader;
	UIReggedMaterial *barshader;
	UIReggedMaterial *selshader;
};

class inventory_type_t {
public:
	str name;
	bool bg_tile;
	UIReggedMaterial *texture;
	UIReggedMaterial *bg;
	UIReggedMaterial *hoverTexture;
	UIReggedMaterial *selTexture;
	Container<inventory_item_t *> items;

	int IndexOfItemPtr( inventory_item_t *item );
};

inline
int inventory_type_t::IndexOfItemPtr( inventory_item_t *item )
{
	for( int i = items.NumObjects(); i > 0; i-- )
	{
		if( items.ObjectAt( i ) == item )
		{
			return i;
		}
	}

	return 0;
}

class inventory_t {
public:
	int typewidth;
	int typeheight;
	int horizoffset;
	int vertoffset;
	int align;
	inv_cascade_type cascade;
	str selectsound;
	str rejectsound;
	str changesound;
	Container<inventory_type_t *> types;

	void Clear() { types.ClearObjectList(); }
};

class invlistener : public Listener {
protected:
	inventory_t *inv;
	inventory_type_t *curtype;
	inventory_item_t *curitem;
	int defaultWidth;
	int defaultHeight;
	int defaultBarWidth;
	int defaultBarHeight;
	int defaultBarOffsetX;
	int defaultBarOffsetY;

public:
	CLASS_PROTOTYPE( invlistener );

protected:
	void	verify_curitem( void );
	void	verify_curtype( void );
	void	verify_one_arg( Event *ev );

public:
	invlistener( inventory_t *i );
	invlistener();

	bool		Load( Script& script );
	void		InvSelectSound( Event *ev );
	void		InvRejectSound( Event *ev );
	void		InvChangeSound( Event *ev );
	void		InvWidth( Event *ev );
	void		InvHeight( Event *ev );
	void		InvHorizOffset( Event *ev );
	void		InvVertOffset( Event *ev );
	void		InvAlign( Event *ev );
	void		InvCascade( Event *ev );
	void		Typedef( Event *ev );
	void		OpenBrace( Event *ev );
	void		CloseBrace( Event *ev );
	void		ButtonShader( Event *ev );
	void		HoverShader( Event *ev );
	void		SelShader( Event *ev );
	void		Background( Event *ev );
	void		BackgroundTile( Event *ev );
	void		Width( Event *ev );
	void		Height( Event *ev );
	void		BarWidth( Event *ev );
	void		BarHeight( Event *ev );
	void		BarOffsetX( Event *ev );
	void		BarOffsetY( Event *ev );
	void		Item( Event *ev );
	void		Ammo( Event *ev );
	void		Equip( Event *ev );
	void		CheckAmmo( Event *ev );
	void		Command( Event *ev );
	void		BGShader( Event *ev );
	void		BarShader( Event *ev );
	void		SelItemShader( Event *ev );
	void		SelItemShaderOnTop( Event *ev );
	void		RotateOffset( Event *ev );
	void		Offset( Event *ev );
	void		Model( Event *ev );
	void		Anim( Event *ev );
	void		Scale( Event *ev );
	void		Angles( Event *ev );
	void		AngleDeltas( Event *ev );
	void		Move( Event *ev );
	void		ModelWindow( Event *ev );
	void		HudRotateOffset( Event *ev );
	void		HudOffset( Event *ev );
	void		HudModel( Event *ev );
	void		HudAnim( Event *ev );
	void		HudScale( Event *ev );
	void		HudAngles( Event *ev );
	void		HudAngleDeltas( Event *ev );
	void		HudMove( Event *ev );
	void		HudCompassAngles( Event *ev );
	void		HudCompassNeedleAngles( Event *ev );
};

bool CL_LoadInventory( const char *filename, inventory_t *inv );
inventory_item_t *CL_GetInvItemByName( inventory_t *inv, const char *name );
qboolean CL_HasInventoryItem( const char *name );
void CL_AmmoCount( const char *name, int *ammo_count, int *max_ammo_count );

#endif // __CL_INV_H__
