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

#include "scriptdelegate.h"
#include "../script/scriptexception.h"
#include "../script/scriptvariable.h"

ScriptDelegate *ScriptDelegate::root = NULL;

ScriptRegisteredDelegate_Script::ScriptRegisteredDelegate_Script(const ScriptThreadLabel& inLabel)
    : label(inLabel)
{}

ScriptVariable ScriptRegisteredDelegate_Script::Execute(Listener *object, const Event& ev)
{
    Event newev = ev;

    label.Execute(object, newev);

    if (newev.NumArgs() > ev.NumArgs()) {
        ScriptVariable& value = newev.GetValue(newev.NumArgs());
        if (value.GetType() != VARIABLE_NONE) {
            return value;
        }
    }

    return ScriptVariable();
}

bool ScriptRegisteredDelegate_Script::operator==(const ScriptRegisteredDelegate_Script& registeredDelegate) const
{
    return label == registeredDelegate.label;
}

ScriptRegisteredDelegate_CodeMember::ScriptRegisteredDelegate_CodeMember(
    Class *inObject, DelegateClassResponse inResponse
)
    : object(inObject)
    , response(inResponse)
{}

void ScriptRegisteredDelegate_CodeMember::Execute(Listener *object, const Event& ev)
{
    if (!object) {
        return;
    }

    (object->*response)(object, ev);
}

bool ScriptRegisteredDelegate_CodeMember::operator==(const ScriptRegisteredDelegate_CodeMember& registeredDelegate
) const
{
    return object == registeredDelegate.object && response == registeredDelegate.response;
}

ScriptRegisteredDelegate_Code::ScriptRegisteredDelegate_Code(DelegateResponse inResponse)
    : response(inResponse)
{}

void ScriptRegisteredDelegate_Code::Execute(Listener *object, const Event& ev)
{
    (*response)(object, ev);
}

bool ScriptRegisteredDelegate_Code::operator==(const ScriptRegisteredDelegate_Code& registeredDelegate) const
{
    return response == registeredDelegate.response;
}

ScriptDelegate::ScriptDelegate(const char *inName, const char *inDescription)
    : name(inName)
    , description(inDescription)
{
    LL_SafeAddFirst(root, this, next, prev);
}

ScriptDelegate::~ScriptDelegate()
{
    LL_SafeRemoveRoot(root, this, next, prev);
}

const ScriptDelegate *ScriptDelegate::GetRoot()
{
    return root;
}

const ScriptDelegate *ScriptDelegate::GetNext() const
{
    return next;
}

void ScriptDelegate::Register(const ScriptThreadLabel& label)
{
    if (!label.IsSet()) {
        ScriptError("Invalid label specified for the script delegate");
    }

    list_script.AddUniqueObject(label);
}

void ScriptDelegate::Unregister(const ScriptThreadLabel& label)
{
    list_script.RemoveObject(label);
}

void ScriptDelegate::Register(ScriptRegisteredDelegate_Code::DelegateResponse response)
{
    list_code.AddUniqueObject(ScriptRegisteredDelegate_Code(response));
}

void ScriptDelegate::Unregister(ScriptRegisteredDelegate_Code::DelegateResponse response)
{
    list_code.RemoveObject(response);
}

void ScriptDelegate::Register(Class *object, ScriptRegisteredDelegate_CodeMember::DelegateClassResponse response)
{
    list_codeMember.AddUniqueObject(ScriptRegisteredDelegate_CodeMember(object, response));
}

void ScriptDelegate::Unregister(Class *object, ScriptRegisteredDelegate_CodeMember::DelegateClassResponse response)
{
    list_codeMember.RemoveObject(ScriptRegisteredDelegate_CodeMember(object, response));
}

ScriptVariable ScriptDelegate::Trigger(const Event& ev) const
{
    return Trigger(NULL, ev);
}

ScriptVariable ScriptDelegate::Trigger(Listener *object, const Event& ev) const
{
    size_t i;
    ScriptVariable lastResult;

    {
        const Container<ScriptRegisteredDelegate_Script> tmpList = list_script;
        for (i = 1; i <= tmpList.NumObjects(); i++) {
            lastResult = tmpList.ObjectAt(i).Execute(object, ev);
        }
    }

    {
        const Container<ScriptRegisteredDelegate_Code> tmpList = list_code;
        for (i = 1; i <= tmpList.NumObjects(); i++) {
            tmpList.ObjectAt(i).Execute(object, ev);
        }
    }

    {
        const Container<ScriptRegisteredDelegate_CodeMember> tmpList = list_codeMember;
        for (i = 1; i <= tmpList.NumObjects(); i++) {
            tmpList.ObjectAt(i).Execute(object, ev);
        }
    }

    return lastResult;
}

ScriptDelegate *ScriptDelegate::GetScriptDelegate(const char *name)
{
    for (ScriptDelegate *delegate = root; delegate; delegate = delegate->next) {
        if (!Q_stricmp(delegate->name, name)) {
            return delegate;
        }
    }

    return NULL;
}

void ScriptDelegate::Reset()
{
    list_script.FreeObjectList();
    list_code.FreeObjectList();
    list_codeMember.FreeObjectList();
}

void ScriptDelegate::ResetAllDelegates()
{
    for (ScriptDelegate *delegate = root; delegate; delegate = delegate->next) {
        delegate->Reset();
    }
}
