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

// item.h: Base class for respawnable, carryable objects.
// 

#ifndef __ITEM_H__
#define __ITEM_H__

#include "g_local.h"
#include "entity.h"
#include "trigger.h"
#include "sentient.h"
#include <gamescript.h>

extern Event EV_Item_Pickup;
extern Event EV_Item_DropToFloor;
extern Event EV_Item_Respawn;
extern Event EV_Item_SetAmount;
extern Event EV_Item_SetMaxAmount;
extern Event EV_Item_RespawnSound;
extern Event EV_Item_DialogNeeded;
extern Event EV_Item_PickupDone;

#define DROPPED_ITEM					0x00008000
#define DROPPED_PLAYER_ITEM		0x00010000

class Item : public Trigger
{
protected:
	SentientPtr			owner;
	qboolean			respawnable;
	qboolean			playrespawn;
	float				respawntime;
	str					dialog_needed;
	int					item_index;
	str					item_name;
	int					maximum_amount;
	int					amount;
	str					sPickupSound;

	qboolean			no_remove;

	void					ItemTouch( Event *ev );

public:
	str					m_sVMprefix;
	qboolean			m_bMOHPrefix;

	CLASS_PROTOTYPE( Item );

	Item();
	~Item();
	virtual void		PlaceItem( void );
	virtual void		SetOwner( Sentient *ent );
	virtual Sentient*	GetOwner( void );
	virtual void		Delete( void );
	void				RemoveFromOwner( void );
	void					SetNoRemove( Event *ev );
	virtual void		DropToFloor( Event *ev );
	virtual Item      *ItemPickup( Entity *other, qboolean add_to_inventory = qtrue );
	virtual void		Respawn( Event *ev );
	virtual void		setRespawn( qboolean flag );
	void					setRespawn( Event *ev );
	virtual qboolean	Respawnable( void );
	virtual void		setRespawnTime( float time );
	void					setRespawnTime( Event *ev );
	virtual float		RespawnTime( void );
	void					RespawnDone( Event *ev );
	void					PickupDone( Event *ev );
	virtual int       GetItemIndex( void ) { return item_index; };
	virtual int       getAmount( void );
	virtual void      setAmount( int startamount );

	virtual int       MaxAmount( void );
	virtual qboolean  Pickupable( Entity *other );

	virtual void      setName( const char *i );
	virtual str       getName( void );
	virtual int       getIndex( void );
	virtual void      SetAmountEvent( Event *ev );
	virtual void      SetMaxAmount( Event *ev );
	virtual void      SetItemName( Event *ev );

	virtual void      SetMax( int maxamount );
	virtual void		Add( int num );
	virtual void      Remove( int num );
	virtual qboolean	Use( int amount );
	virtual qboolean	Removable( void );
	virtual void      Pickup( Event *ev );
	virtual qboolean	Drop( void );
	virtual void      RespawnSound( Event *ev );
	virtual void      DialogNeeded( Event *ev );
	virtual str       GetDialogNeeded( void );
	void					Landed( Event *ev );
	void					CoolItemEvent( Event *ev );
	void					ForceCoolItemEvent( Event *ev );
	qboolean				IsItemCool( str * dialog, str * anim, qboolean *force );
	void				   SetCoolItem( qboolean cool, str &dialog, str &anim );

	void			SetPickupSound( Event *ev );

	void			EventViewModelPrefix( Event *ev );
	void			updatePrefix( Event *ev );

	virtual void      Archive( Archiver &arc );
};

inline void Item::Archive
(
Archiver &arc
)
{
	Trigger::Archive( arc );

	arc.ArchiveSafePointer( &owner );
	arc.ArchiveBoolean( &respawnable );
	arc.ArchiveBoolean( &playrespawn );
	arc.ArchiveFloat( &respawntime );
	arc.ArchiveString( &dialog_needed );
	arc.ArchiveInteger( &item_index );
	arc.ArchiveString( &item_name );
	if( arc.Loading() )
	{
		setName( item_name.c_str() );
	}
	arc.ArchiveInteger( &maximum_amount );
	arc.ArchiveInteger( &amount );
	arc.ArchiveBoolean( &no_remove );
	arc.ArchiveString( &sPickupSound );
}

const char *GetItemName( const char *prefix, qboolean *mohprefix = NULL );
const char *GetItemPrefix( const char *name, qboolean *mohprefix = NULL );

#endif /* item.h */
