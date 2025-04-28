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

// containerclass.h: C++ Class Container for use with SafePtr

#pragma once

#include "class.h"
#include "container.h"

template<class Type>
class ContainerClass : public Class
{
    Container<Type> value;

public:
    ContainerClass();
    ContainerClass(const Container<Type>& container);
    ContainerClass<Type>& operator=(const Container<Type>& container);
    ContainerClass(Container<Type>&& container);
    ContainerClass<Type>& operator=(Container<Type>&& container);

    ~ContainerClass() override;

    void Archive(Archiver& arc) override;

    int   AddObject(const Type& obj);
    int   AddUniqueObject(const Type& obj);
    void  AddObjectAt(int index, const Type& obj);
    Type *AddressOfObjectAt(int index);
    void  ClearObjectList(void);
    void  FreeObjectList(void);
    int   IndexOfObject(const Type& obj);
    void  InsertObjectAt(int index, const Type& obj);
    int   NumObjects(void) const;
    Type& ObjectAt(const size_t index) const;
    bool  ObjectInList(const Type& obj);
    void  RemoveObjectAt(int index);
    void  RemoveObject(const Type& obj);
    void  Reset(void);
    void  Resize(int maxelements);
    void  SetObjectAt(int index, const Type& obj);
    void  Sort(int (*compare)(const void *elem1, const void *elem2));
    Type& operator[](const int index) const;
};

template<typename Type>
ContainerClass<Type>::ContainerClass()
{}

template<typename Type>
ContainerClass<Type>::ContainerClass(const Container<Type>& container)
    : value(container)
{}

template<typename Type>
ContainerClass<Type>& ContainerClass<Type>::operator=(const Container<Type>& container)
{
    value = container;

    return *this;
}

template<typename Type>
ContainerClass<Type>::ContainerClass(Container<Type>&& container)
    : value(std::move(container))
{}

template<typename Type>
ContainerClass<Type>& ContainerClass<Type>::operator=(Container<Type>&& container)
{
    value = std::move(container);

    return *this;
}

template<typename Type>
ContainerClass<Type>::~ContainerClass()
{
    value.FreeObjectList();
}

template<typename Type>
int ContainerClass<Type>::AddObject(const Type& obj)
{
    return value.AddObject(obj);
}

template<typename Type>
int ContainerClass<Type>::AddUniqueObject(const Type& obj)
{
    return value.AddUniqueObject(obj);
}

template<typename Type>
void ContainerClass<Type>::AddObjectAt(int index, const Type& obj)
{
    return value.AddObjectAt(index, obj);
}

template<typename Type>
Type *ContainerClass<Type>::AddressOfObjectAt(int index)
{
    return value.AddressOfObjectAt(index);
}

template<typename Type>
void ContainerClass<Type>::ClearObjectList(void)
{
    return value.ClearObjectList();
}

template<typename Type>
void ContainerClass<Type>::FreeObjectList(void)
{
    return value.FreeObjectList();
}

template<typename Type>
int ContainerClass<Type>::IndexOfObject(const Type& obj)
{
    return value.IndexOfObject(obj);
}

template<typename Type>
void ContainerClass<Type>::InsertObjectAt(int index, const Type& obj)
{
    return value.InsertObjectAt(index, obj);
}

template<typename Type>
int ContainerClass<Type>::NumObjects(void) const
{
    return value.NumObjects();
}

template<typename Type>
Type& ContainerClass<Type>::ObjectAt(const size_t index) const
{
    return value.ObjectAt(index);
}

template<typename Type>
bool ContainerClass<Type>::ObjectInList(const Type& obj)
{
    return value.ObjectInList(obj);
}

template<typename Type>
void ContainerClass<Type>::RemoveObjectAt(int index)
{
    return value.RemoveObjectAt(index);
}

template<typename Type>
void ContainerClass<Type>::RemoveObject(const Type& obj)
{
    return value.RemoveObject(obj);
}

template<typename Type>
void ContainerClass<Type>::Reset(void)
{
    return value.Reset();
}

template<typename Type>
void ContainerClass<Type>::Resize(int maxelements)
{
    return value.Resize(maxelements);
}

template<typename Type>
void ContainerClass<Type>::SetObjectAt(int index, const Type& obj)
{
    return value.SetObjectAt(index, obj);
}

template<typename Type>
void ContainerClass<Type>::Sort(int (*compare)(const void *elem1, const void *elem2))
{
    return value.Sort(compare);
}

template<typename Type>
Type& ContainerClass<Type>::operator[](const int index) const
{
    return value[index];
}
