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

#include "animationevent.h"
#include "../qcommon/mem_blockalloc.h"

MEM_BlockAlloc<AnimationEvent> AnimationEvent_allocator;

CLASS_DECLARATION(Event, AnimationEvent, NULL) {
    {NULL, NULL}
};

AnimationEvent::AnimationEvent(str command, int numArgs)
    : Event(command, numArgs)
{
    anim_number = 0;
    anim_frame  = 0;
}

AnimationEvent::AnimationEvent(const Event& ev)
    : Event(ev)
{
    anim_number = 0;
    anim_frame  = 0;
}

AnimationEvent::AnimationEvent() {}

void *AnimationEvent::operator new(size_t size)
{
    return AnimationEvent_allocator.Alloc();
}

void AnimationEvent::operator delete(void *ptr)
{
    return AnimationEvent_allocator.Free(ptr);
}

int AnimationEvent::GetAnimationFrame() const
{
    return anim_frame;
}

int AnimationEvent::GetAnimationNumber() const
{
    return anim_number;
}

void AnimationEvent::SetAnimationFrame(int frame)
{
    anim_frame = frame;
};

void AnimationEvent::SetAnimationNumber(int num)
{
    anim_number = num;
};