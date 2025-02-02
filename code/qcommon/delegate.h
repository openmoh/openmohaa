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

#pragma once

#include <functional>
#include "container.h"

template<typename T>
using Delegate = std::function<T>;

struct DelegateHandle {
public:
    DelegateHandle();

    bool operator==(const DelegateHandle& other) const;
    bool operator!=(const DelegateHandle& other) const;

private:
    static uint64_t GenerateDelegateID();
    static uint64_t currentHandle;

    uint64_t handle;
};

template<typename T>
class DelegateMultiElement
{
public:
    DelegateMultiElement(Delegate<T>&& inFunction);

    template<typename... Args>
    void Execute(Args&&...args) const;

    DelegateHandle GetHandle() const;

private:
    DelegateHandle handle;
    Delegate<T>    func;
};

template<typename T>
DelegateMultiElement<T>::DelegateMultiElement(Delegate<T>&& inFunction)
    : func(inFunction)
{}

template<typename T>
template<typename... Args>
void DelegateMultiElement<T>::Execute(Args&&...args) const
{
    func(std::move(args)...);
}

template<typename T>
DelegateHandle DelegateMultiElement<T>::GetHandle() const
{
    return handle;
}

template<typename T>
class MulticastDelegate
{
public:
    DelegateHandle Add(Delegate<T>&& function);
    void           Remove(DelegateHandle handle);

    template<typename... Args>
    void Execute(Args&&...args);

private:
    Container<DelegateMultiElement<T>> delegates;
};

template<typename T>
DelegateHandle MulticastDelegate<T>::Add(Delegate<T>&& function)
{
    int index = delegates.AddObject(DelegateMultiElement<T>(std::move(function)));

    return delegates.ObjectAt(index).GetHandle();
}

template<typename T>
void MulticastDelegate<T>::Remove(DelegateHandle handle)
{
    int i;

    for (i = delegates.NumObjects(); i > 0; i--) {
        const DelegateMultiElement<T>& elem = delegates.ObjectAt(i);

        if (elem.GetHandle() == handle) {
            delegates.RemoveObjectAt(i);
            break;
        }
    }
}

template<typename T>
template<typename... Args>
void MulticastDelegate<T>::Execute(Args&&...args)
{
    size_t i;

    for (i = 1; i <= delegates.NumObjects(); i++) {
        const DelegateMultiElement<T>& element = delegates.ObjectAt(i);

        element.Execute(std::move(args)...);
    }
}
