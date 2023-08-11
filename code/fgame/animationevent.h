/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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
// animate.h -- Animation event

#include "../qcommon/listener.h"

class AnimationEvent : public Event
{
private:
    short anim_number;
    short anim_frame;

public:
    CLASS_PROTOTYPE(AnimationEvent);

    void *operator new(size_t size);
    void  operator delete(void *ptr);

    AnimationEvent(str command);
    AnimationEvent(const Event& ev);
    AnimationEvent();

    int  GetAnimationFrame() const;
    int  GetAnimationNumber() const;
    void SetAnimationFrame(int frame);
    void SetAnimationNumber(int num);
};

extern MEM_BlockAlloc<AnimationEvent> AnimationEvent_allocator;
