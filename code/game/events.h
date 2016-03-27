//-----------------------------------------------------------------------------
//
//  $Logfile:: /fakk2_code/fakk2_new/fgame/events.h                           $
// $Revision:: 1                                                              $
//   $Author:: Jimdose                                                        $
//     $Date:: 9/10/99 10:53a                                                 $
//
// Copyright (C) 1997 by Ritual Entertainment, Inc.
// All rights reserved.
//
// This source is may not be distributed and/or modified without
// expressly written permission by Ritual Entertainment, Inc.
//
// $Log:: /fakk2_code/fakk2_new/fgame/events.h                                $
// 
// 1     9/10/99 10:53a Jimdose
// 
// 1     9/08/99 3:15p Aldie
// 
// DESCRIPTION:
// Event definitions for communication between Sin and DLL's
//

#ifndef __EVENTS_H__
#define __EVENTS_H__

#if 1
typedef int event_t;

enum
	{
	EVENT_DEFAULT_MINEVENT = 1,

	// Player events
	EVENT_CLIENTCONNECT = EVENT_DEFAULT_MINEVENT,
	EVENT_CLIENTDISCONNECT,
	EVENT_CLIENTKILL,
	EVENT_CLIENTMOVE,
	EVENT_CLIENTENDFRAME,
	EVENT_SETNEWPARMS,
	EVENT_SETCHANGEPARMS,
	EVENT_PRETHINK,
	EVENT_POSTTHINK,

	// Generic entity events
	EVENT_SPAWN,
	EVENT_REMOVE,
	EVENT_PRECACHE,
	EVENT_THINK,
	EVENT_ACTIVATE,
	EVENT_USE,
    EVENT_FOOTSTEP,

	// Physics events
	EVENT_MOVEDONE,
	EVENT_TOUCH,
	EVENT_BLOCKED,

   // Animation events
   EVENT_ANIM_FRAME,
   EVENT_ANIM_NEWANIM,
   EVENT_ANIM_LASTFRAME,

	// For subclass events
	EVENT_DEFAULT_MAXEVENT
	};

#endif

#endif /* events.h */
