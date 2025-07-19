/*
===========================================================================
Copyright (C) 2025 the OpenMoHAA team

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

// container.h: C++ Container

#pragma once

#if defined(GAME_DLL)
//
// game dll specific defines
//
#    include "../fgame/g_local.h"

#    define CONTAINER_Error          gi.Error
#    define CONTAINER_DPrintf        gi.DPrintf
#    define CONTAINER_WDPrintf(text) gi.DPrintf(text)
#    define CONTAINER_Alloc          gi.Malloc
#    define CONTAINER_Free           gi.Free

#elif defined(CGAME_DLL)
//
// cgame dll specific defines
//
#    include "../cgame/cg_local.h"

#    define CONTAINER_Error          cgi.Error
#    define CONTAINER_DPrintf        cgi.DPrintf
#    define CONTAINER_WDPrintf(text) cgi.DPrintf(text)
#    define CONTAINER_Alloc          cgi.Malloc
#    define CONTAINER_Free           cgi.Free

#elif defined(REF_DLL)

#    include "../renderercommon/tr_common.h"

//
// client specific defines
//
#    define CONTAINER_Error          Com_Error
#    define CONTAINER_DPrintf        Com_DPrintf
#    define CONTAINER_WDPrintf(text) Com_DPrintf(text)
#    define CONTAINER_Alloc          ri.Malloc
#    define CONTAINER_Free           ri.Free

#else

#    include "qcommon.h"

//
// client specific defines
//
#    define CONTAINER_Error          Com_Error
#    define CONTAINER_DPrintf        Com_DPrintf
#    define CONTAINER_WDPrintf(text) Com_DPrintf(text)
#    define CONTAINER_Alloc          Z_Malloc
#    define CONTAINER_Free           Z_Free
#endif

#include <utility>
#include <new>

class Archiver;

template<class Type>
class Container
{
private:
    Type *objlist;
    int   numobjects;
    int   maxobjects;

private:
    void Copy(const Container<Type>& container);

public:
    Container();

    Container(const Container<Type>& container);
    Container<Type>& operator=(const Container<Type>& container);

    Container(Container<Type>&& container);
    Container<Type>& operator=(Container<Type>&& container);

    ~Container();

    void Archive(Archiver& arc);
    void Archive(Archiver& arc, void (*ArchiveFunc)(Archiver& arc, Type *obj));

    int   AddObject(const Type& obj);
    int   AddUniqueObject(const Type& obj);
    void  AddObjectAt(int index, const Type& obj);
    Type *AddressOfObjectAt(int index);
    void  ClearObjectList(void);
    void  FreeObjectList(void);
    int   IndexOfObject(const Type& obj);
    void  InsertObjectAt(int index, const Type& obj);
    int   MaxObjects(void) const;
    int   NumObjects(void) const;
    Type& ObjectAt(const size_t index) const;
    bool  ObjectInList(const Type& obj);
    void  RemoveObjectAt(int index);
    void  RemoveObject(const Type& obj);
    void  Reset(void);
    void  Resize(int maxelements);
    void  SetObjectAt(int index, const Type& obj);
    void  Sort(int (*compare)(const void *elem1, const void *elem2));
    Type& operator[](const uintptr_t index) const;
};

template<class Type>
Container<Type>::Container()
{
    objlist    = NULL;
    numobjects = 0;
    maxobjects = 0;
}

template<class Type>
Container<Type>::Container(const Container<Type>& container)
{
    objlist = NULL;

    Copy(container);
}

template<class Type>
Container<Type>& Container<Type>::operator=(const Container<Type>& container)
{
    Copy(container);

    return *this;
}

template<class Type>
Container<Type>::Container(Container<Type>&& container)
{
    objlist              = container.objlist;
    numobjects           = container.numobjects;
    maxobjects           = container.maxobjects;
    container.objlist    = NULL;
    container.numobjects = 0;
    container.maxobjects = 0;
}

template<class Type>
Container<Type>& Container<Type>::operator=(Container<Type>&& container)
{
    FreeObjectList();

    objlist              = container.objlist;
    numobjects           = container.numobjects;
    maxobjects           = container.maxobjects;
    container.objlist    = NULL;
    container.numobjects = 0;
    container.maxobjects = 0;

    return *this;
}

template<class Type>
Container<Type>::~Container()
{
    FreeObjectList();
}

template<class Type>
int Container<Type>::AddObject(const Type& obj)
{
    if (!objlist) {
        Resize(10);
    }

    if (numobjects >= maxobjects) {
        Resize(numobjects * 2);
    }

    new (objlist + numobjects) Type(obj);
    numobjects++;

    return numobjects;
}

template<class Type>
int Container<Type>::AddUniqueObject(const Type& obj)
{
    int index;

    index = IndexOfObject(obj);

    if (!index) {
        index = AddObject(obj);
    }

    return index;
}

template<class Type>
void Container<Type>::AddObjectAt(int index, const Type& obj)
{
    int i;

    if (index > maxobjects) {
        Resize(index);
    }

    if (index > numobjects) {
        for (i = numobjects; i < index; i++) {
            new (objlist + i) Type();
        }

        numobjects = index;
    }

    SetObjectAt(index, obj);
}

template<class Type>
Type *Container<Type>::AddressOfObjectAt(int index)
{
    if (index > maxobjects) {
        CONTAINER_Error(ERR_DROP, "Container::AddressOfObjectAt : index is greater than maxobjects");
    }

    if (index > numobjects) {
        numobjects = index;
    }

    return &objlist[index - 1];
}

template<class Type>
void Container<Type>::ClearObjectList(void)
{
    size_t i;

    if (objlist && numobjects) {
        for (i = 0; i < numobjects; ++i) {
            objlist[i].~Type();
        }

        // don't free the object list, it can be reused later
        numobjects = 0;
    }
}

template<class Type>
void Container<Type>::FreeObjectList(void)
{
    size_t i;

    if (objlist) {
        for (i = 0; i < numobjects; ++i) {
            objlist[i].~Type();
        }

        CONTAINER_Free(objlist);
    }

    objlist    = NULL;
    numobjects = 0;
    maxobjects = 0;
}

template<class Type>
int Container<Type>::IndexOfObject(const Type& obj)
{
    int i;

    if (!objlist) {
        return 0;
    }

    for (i = 0; i < numobjects; i++) {
        if (objlist[i] == obj) {
            return i + 1;
        }
    }

    return 0;
}

template<class Type>
void Container<Type>::InsertObjectAt(int index, const Type& obj)
{
    size_t i;

    if ((index <= 0) || (index > numobjects + 1)) {
        CONTAINER_Error(ERR_DROP, "Container::InsertObjectAt : index out of range");
        return;
    }

    numobjects++;
    intptr_t arrayIndex = index - 1;

    if (numobjects > maxobjects) {
        maxobjects = numobjects;
        if (!objlist) {
            objlist = (Type *)CONTAINER_Alloc(sizeof(Type) * maxobjects);

            for (i = 0; i < arrayIndex; ++i) {
                new (objlist + i) Type();
            }

            new (objlist + arrayIndex) Type(obj);
        } else {
            Type *temp = objlist;
            if (maxobjects < numobjects) {
                maxobjects = numobjects;
            }

            objlist = (Type *)CONTAINER_Alloc(sizeof(Type) * maxobjects);

            for (i = 0; i < arrayIndex; ++i) {
                new (objlist + i) Type(std::move(temp[i]));
            }

            new (objlist + arrayIndex) Type(obj);
            for (i = arrayIndex; i < numobjects - 1; ++i) {
                new (objlist + i + 1) Type(std::move(temp[i]));
            }

            CONTAINER_Free(temp);
        }
    } else {
        for (i = numobjects - 1; i > arrayIndex; i--) {
            objlist[i] = std::move(objlist[i - 1]);
        }
        objlist[arrayIndex] = obj;
    }
}

template<class Type>
int Container<Type>::MaxObjects(void) const
{
    return maxobjects;
}

template<class Type>
int Container<Type>::NumObjects(void) const
{
    return numobjects;
}

template<class Type>
Type& Container<Type>::ObjectAt(const size_t index) const
{
    if ((index <= 0) || (index > numobjects)) {
        CONTAINER_Error(ERR_DROP, "Container::ObjectAt : index out of range");
    }

    return objlist[index - 1];
}

template<class Type>
bool Container<Type>::ObjectInList(const Type& obj)
{
    if (!IndexOfObject(obj)) {
        return false;
    }

    return true;
}

template<class Type>
void Container<Type>::RemoveObjectAt(int index)
{
    int i;

    if (!objlist) {
        return;
    }

    if ((index <= 0) || (index > numobjects)) {
        return;
    }

    i = index - 1;
    numobjects--;

    for (i = index - 1; i < numobjects; i++) {
        objlist[i] = std::move(objlist[i + 1]);
    }

    // Destroy the last object as it's now useless
    objlist[numobjects].~Type();
}

template<class Type>
void Container<Type>::RemoveObject(const Type& obj)
{
    int index;

    index = IndexOfObject(obj);

    assert(index);
    if (!index) {
        CONTAINER_WDPrintf("Container::RemoveObject : Object not in list\n");
        return;
    }

    RemoveObjectAt(index);
}

template<class Type>
void Container<Type>::Reset()
{
    objlist    = NULL;
    numobjects = 0;
    maxobjects = 0;
}

template<class Type>
void Container<Type>::Resize(int maxelements)
{
    Type  *temp;
    size_t i;

    if (maxelements <= 0) {
        FreeObjectList();
        return;
    }

    if (!objlist) {
        maxobjects = maxelements;
        objlist    = (Type *)CONTAINER_Alloc(sizeof(Type) * maxobjects);
    } else {
        temp = objlist;

        maxobjects = maxelements;

        if (maxobjects < numobjects) {
            maxobjects = numobjects;
        }

        objlist = (Type *)CONTAINER_Alloc(sizeof(Type) * maxobjects);

        for (i = 0; i < numobjects; i++) {
            // move the older type
            new (objlist + i) Type(std::move(temp[i]));

            // destruct the older type
            temp[i].~Type();
        }

        CONTAINER_Free(temp);
    }
}

template<class Type>
void Container<Type>::SetObjectAt(int index, const Type& obj)
{
    if (!objlist) {
        return;
    }

    if ((index <= 0) || (index > numobjects)) {
        CONTAINER_Error(ERR_DROP, "Container::SetObjectAt : index out of range");
    }

    objlist[index - 1] = obj;
}

template<class Type>
void Container<Type>::Sort(int (*compare)(const void *elem1, const void *elem2))
{
    if (!objlist) {
        return;
    }

    qsort((void *)objlist, (size_t)numobjects, sizeof(Type), compare);
}

template<class Type>
Type& Container<Type>::operator[](const uintptr_t index) const
{
    return ObjectAt(index + 1);
}

template<class Type>
void Container<Type>::Copy(const Container<Type>& container)
{
    int i;

    if (&container == this) {
        return;
    }

    FreeObjectList();

    numobjects = container.numobjects;
    maxobjects = container.maxobjects;
    objlist    = NULL;

    if (container.objlist == NULL || !container.maxobjects) {
        return;
    }

    Resize(maxobjects);

    if (!container.numobjects) {
        return;
    }

    for (i = 0; i < container.numobjects; i++) {
        new (objlist + i) Type(container.objlist[i]);
    }

    return;
}

template<typename T>
void *operator new(size_t count, Container<T>& container)
{
    (void)count;

    assert(count == sizeof(T));
    return &container.ObjectAt(container.AddObject());
}

template<typename T>
void operator delete(void *ptr, Container<T>& container)
{
    container.RemoveObject((T *)ptr);
}
