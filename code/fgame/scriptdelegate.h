/*
===========================================================================
Copyright (C) 2026 the OpenMoHAA team

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

// scriptdelegate -- manages function delegate

#pragma once

#include "../corepp/listener.h"
#include "../corepp/delegate.h"
#include "gamescript.h"

class ScriptRegisteredDelegate
{
public:
    void Execute(Listener *object, const Event& ev);
};

/**
 * Registered delegate, for scripts.
 * It contains a ScriptThreadLabel with the game script and the label to execute.
 */
class ScriptRegisteredDelegate_Script : public ScriptRegisteredDelegate
{
public:
    ScriptRegisteredDelegate_Script(const ScriptThreadLabel& inLabel);

    ScriptVariable Execute(Listener *object, const Event& ev);

    bool operator==(const ScriptRegisteredDelegate_Script& registeredDelegate) const;

private:
    ScriptThreadLabel label;
};

/**
 * Registered delegate, for code use.
 * It contains the function to execute.
 */
class ScriptRegisteredDelegate_Code : public ScriptRegisteredDelegate
{
public:
    using DelegateResponse = void (*)(Listener *object, const Event& ev);

public:
    ScriptRegisteredDelegate_Code(DelegateResponse inResponse);

    void Execute(Listener *object, const Event& ev);

    bool operator==(const ScriptRegisteredDelegate_Code& registeredDelegate) const;

private:
    DelegateResponse response;
};

/**
 * Registered delegate, for code use.
 * It contains the object along the member function to execute.
 * The function will not be executed if the object is NULL.
 */
class ScriptRegisteredDelegate_CodeMember : public ScriptRegisteredDelegate
{
public:
    using DelegateClassResponse = void (Class::*)(Listener *object, const Event& ev);

public:
    ScriptRegisteredDelegate_CodeMember(Class *inObject, DelegateClassResponse inResponse);

    void Execute(Listener *object, const Event& ev);

    bool operator==(const ScriptRegisteredDelegate_CodeMember& registeredDelegate) const;

private:
    SafePtr<Class>        object;
    DelegateClassResponse response;
};

/**
 * A script delegate provides a way for code to subscribe for events.
 * Scripts and code can register for a delegate and have their function executed
 * when the delegate gets triggered.
 */
class ScriptDelegate
{
public:
    ScriptDelegate(const char *name, const char *description);
    ~ScriptDelegate();

    static const ScriptDelegate *GetRoot();
    const ScriptDelegate        *GetNext() const;

    /**
     * Register a script label.
     *
     * @param label The label to be executed
     */
    void Register(const ScriptThreadLabel& label);

    /**
     * Unregistered the label.
     *
     * @param label The label to unregister
     */
    void Unregister(const ScriptThreadLabel& label);

    /**
     * Register a function.
     *
     * @param response The function to be executed
     */
    void Register(ScriptRegisteredDelegate_Code::DelegateResponse response);

    /**
     * Unregistered the function.
     *
     * @param response the function to unregister
     */
    void Unregister(ScriptRegisteredDelegate_Code::DelegateResponse response);

    /**
     * Register with an object and a member function.
     *
     * @param object The object to notify
     * @param response The member function of the object to be executed
     */
    void Register(Class *object, ScriptRegisteredDelegate_CodeMember::DelegateClassResponse response);

    /**
     * Unregistered the member function.
     *
     * @param object The object where the member function is
     * @param response The member function to unregister
     */
    void Unregister(Class *object, ScriptRegisteredDelegate_CodeMember::DelegateClassResponse response);

    /**
     * Executes all registered delegates with the specified event.
     *
     * @param ev Parameter list
     */
    ScriptVariable Trigger(const Event& ev = Event()) const;

    /**
     * Executes all registered delegates with the specified event.
     *
     * @param ev Parameter list
     */
    ScriptVariable Trigger(Listener *object, const Event& ev = Event()) const;

    /**
     * Reset the delegate, unregister callbacks.
     */
    void Reset();

    /**
     * Search and return the specified script delegate by name.
     *
     * @param name The name to search for
     */
    static ScriptDelegate *GetScriptDelegate(const char *name);
    static void            ResetAllDelegates();

    // non-movable and non-copyable
    ScriptDelegate(ScriptDelegate&& other)                 = delete;
    ScriptDelegate& operator=(ScriptDelegate&& other)      = delete;
    ScriptDelegate(const ScriptDelegate& other)            = delete;
    ScriptDelegate& operator=(const ScriptDelegate& other) = delete;

private:
    // Linked-list
    ScriptDelegate        *next;
    ScriptDelegate        *prev;
    static ScriptDelegate *root;
    const char            *name;
    const char            *description;

    Container<ScriptRegisteredDelegate_Script>     list_script;
    Container<ScriptRegisteredDelegate_Code>       list_code;
    Container<ScriptRegisteredDelegate_CodeMember> list_codeMember;
};
