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

// scriptvariable.cpp : Dynamic variables for scripts

#include "scriptvariable.h"
#include "scriptexception.h"
#include "../qcommon/str.h"
#include "../qcommon/con_set.h"

#ifdef GAME_DLL
#    include "../fgame/archive.h"
#    include "../fgame/g_local.h"
#    include "../fgame/navigate.h"
#endif

#ifdef WITH_SCRIPT_ENGINE
#    include "../fgame/worldspawn.h"
#    include "../fgame/scriptmaster.h"
#    include "../fgame/simpleentity.h"
#endif

#include <utility>

template<>
int HashCode<ScriptVariable>(const ScriptVariable& key)
{
#if defined(GAME_DLL)
    Entity *e;
#endif

    switch (key.GetType()) {
    case VARIABLE_STRING:
    case VARIABLE_CONSTSTRING:
        return HashCode<str>(key.stringValue());

    case VARIABLE_INTEGER:
        return key.m_data.intValue;

#if defined(GAME_DLL)
    case VARIABLE_LISTENER:
        e = (Entity *)key.listenerValue();

        if (checkInheritance(&Entity::ClassInfo, e->classinfo())) {
            return e->entnum;
        }
#endif

    default:
        throw ScriptException("Bad hash code value: %s", key.stringValue().c_str());
    }
}

#if defined(ARCHIVE_SUPPORTED)
template<>
void con_set<ScriptVariable, ScriptVariable>::Entry::Archive(Archiver& arc)
{
    key.ArchiveInternal(arc);
    value.ArchiveInternal(arc);
}

void ScriptArrayHolder::Archive(Archiver& arc)
{
    arc.ArchiveUnsigned(&refCount);
    arrayValue.Archive(arc);
}

void ScriptArrayHolder::Archive(Archiver& arc, ScriptArrayHolder *& arrayValue)
{
    qboolean newRef;

    if (arc.Loading()) {
        arc.ArchiveBoolean(&newRef);
    } else {
        newRef = !arc.ObjectPositionExists(arrayValue);
        arc.ArchiveBoolean(&newRef);
    }

    if (newRef) {
        if (arc.Loading()) {
            arrayValue = new ScriptArrayHolder();
        }

        arc.ArchiveObjectPosition(arrayValue);
        arrayValue->Archive(arc);
        return;
    } else {
        arc.ArchiveObjectPointer((Class **)&arrayValue);
    }
}

void ScriptConstArrayHolder::Archive(Archiver& arc)
{
    arc.ArchiveUnsigned(&refCount);
    arc.ArchiveUnsigned(&size);

    if (arc.Loading()) {
        constArrayValue = new ScriptVariable[size + 1] - 1;
    }

    for (unsigned int i = 1; i <= size; i++) {
        constArrayValue[i].ArchiveInternal(arc);
    }
}

void ScriptConstArrayHolder::Archive(Archiver& arc, ScriptConstArrayHolder *& constArrayValue)
{
    qboolean newRef;

    if (arc.Loading()) {
        arc.ArchiveBoolean(&newRef);
    } else {
        newRef = !arc.ObjectPositionExists(constArrayValue);
        arc.ArchiveBoolean(&newRef);
    }

    if (newRef) {
        if (arc.Loading()) {
            constArrayValue = new ScriptConstArrayHolder();
        }

        arc.ArchiveObjectPosition(constArrayValue);
        constArrayValue->Archive(arc);
        return;
    } else {
        arc.ArchiveObjectPointer((Class **)&constArrayValue);
    }
}

void ScriptPointer::Archive(Archiver& arc)
{
    list.Archive(arc, ScriptVariable::Archive);
}

void ScriptPointer::Archive(Archiver& arc, ScriptPointer *& pointerValue)
{
    qboolean newRef;

    if (arc.Loading()) {
        arc.ArchiveBoolean(&newRef);
    } else {
        newRef = !arc.ObjectPositionExists(pointerValue);
        arc.ArchiveBoolean(&newRef);
    }

    if (newRef) {
        if (arc.Loading()) {
            pointerValue = new ScriptPointer();
        }

        arc.ArchiveObjectPosition(pointerValue);
        pointerValue->Archive(arc);
        return;
    } else {
        arc.ArchiveObjectPointer((Class **)&pointerValue);
    }
}

void ScriptVariable::Archive(Archiver& arc)
{
#    ifdef WITH_SCRIPT_ENGINE
    const_str s;

    if (arc.Loading()) {
        Director.ArchiveString(arc, s);
        key = s;
    } else {
        s = key;
        Director.ArchiveString(arc, s);
    }
#    endif

    ArchiveInternal(arc);
}

void ScriptVariable::Archive(Archiver& arc, ScriptVariable **obj)
{
    arc.ArchiveObjectPointer((Class **)obj);
}

void ScriptVariable::ArchiveInternal(Archiver& arc)
{
    arc.ArchiveObjectPosition(this);

    arc.ArchiveByte(&type);
    switch (type) {
    case VARIABLE_STRING:
        if (arc.Loading()) {
            m_data.stringValue = new str;
        }

        arc.ArchiveString(m_data.stringValue);
        break;

    case VARIABLE_INTEGER:
        arc.ArchiveInteger(&m_data.intValue);
        break;

    case VARIABLE_FLOAT:
        arc.ArchiveFloat(&m_data.floatValue);
        break;

    case VARIABLE_CHAR:
        arc.ArchiveChar(&m_data.charValue);
        break;

#    ifdef WITH_SCRIPT_ENGINE
    case VARIABLE_CONSTSTRING:
        if (arc.Loading()) {
            str s;
            arc.ArchiveString(&s);
            m_data.intValue = Director.AddString(s);
        } else {
            str s = Director.GetString(m_data.intValue);
            arc.ArchiveString(&s);
        }
        break;
#    endif

    case VARIABLE_LISTENER:
        if (arc.Loading()) {
            m_data.listenerValue = new SafePtr<Listener>;
        }

        arc.ArchiveSafePointer(m_data.listenerValue);
        break;

    case VARIABLE_ARRAY:
        ScriptArrayHolder::Archive(arc, m_data.arrayValue);
        break;

    case VARIABLE_CONSTARRAY:
        ScriptConstArrayHolder::Archive(arc, m_data.constArrayValue);
        break;

    case VARIABLE_REF:
    case VARIABLE_CONTAINER:
        arc.ArchiveObjectPointer((Class **)&m_data.refValue);
        break;

    case VARIABLE_SAFECONTAINER:
        if (arc.Loading()) {
            m_data.safeContainerValue = new SafePtr<ConList>;
        }

        arc.ArchiveSafePointer(m_data.safeContainerValue);
        break;

    case VARIABLE_POINTER:
        ScriptPointer::Archive(arc, m_data.pointerValue);
        break;

    case VARIABLE_VECTOR:
        if (arc.Loading()) {
            m_data.vectorValue = new float[3];
        }

        arc.ArchiveVec3(m_data.vectorValue);
        break;

    default:
        break;
    }
}
#endif

ScriptArrayHolder::ScriptArrayHolder()
    : refCount(0)
{}

ScriptConstArrayHolder::ScriptConstArrayHolder(ScriptVariable *pVar, unsigned int size)
{
    refCount   = 0;
    this->size = size;

    constArrayValue = new ScriptVariable[size + 1] - 1;

    for (unsigned int i = 1; i <= size; i++) {
        constArrayValue[i] = pVar[i];
    }
}

ScriptConstArrayHolder::ScriptConstArrayHolder(unsigned int size)
{
    refCount   = 0;
    this->size = size;

    constArrayValue = new ScriptVariable[size + 1] - 1;
}

ScriptConstArrayHolder::ScriptConstArrayHolder()
{
    refCount        = 0;
    size            = 0;
    constArrayValue = NULL;
}

ScriptConstArrayHolder::~ScriptConstArrayHolder()
{
    if (constArrayValue) {
        ScriptVariable *const offset = constArrayValue + 1;
        delete[] offset;
    }
}

void ScriptPointer::Clear()
{
    for (int i = 0; i < list.NumObjects(); i++) {
        ScriptVariable *variable = list[i];

        variable->type = 0;
    }

    delete this;
}

void ScriptPointer::add(ScriptVariable *var)
{
    list.AddObject(var);
}

void ScriptPointer::remove(ScriptVariable *var)
{
    list.RemoveObject(var);

    if (list.NumObjects() == 0) {
        delete this;
    }
}

void ScriptPointer::setValue(const ScriptVariable& var)
{
    int             i;
    ScriptVariable *pVar;

    if (var.type == VARIABLE_POINTER) {
        for (i = list.NumObjects(); i > 0; i--) {
            pVar = list.ObjectAt(i);

            pVar->m_data.pointerValue = var.m_data.pointerValue;
            var.m_data.pointerValue->add(pVar);
        }
    } else {
        for (i = list.NumObjects(); i > 0; i--) {
            pVar = list.ObjectAt(i);

            pVar->type = 0;
            *pVar      = var;
        }
    }

    delete this;
}

ScriptVariable::ScriptVariable()
{
#if defined(GAME_DLL)
    key = 0;
#endif
    type                = 0;
    m_data.pointerValue = NULL;
}

ScriptVariable::ScriptVariable(const ScriptVariable& variable)
{
    type                = 0;
    m_data.pointerValue = NULL;

    *this = variable;
}

ScriptVariable::ScriptVariable(ScriptVariable&& variable)
{
#if defined(GAME_DLL)
    key          = variable.GetKey();
    variable.key = 0;
#endif
    type          = variable.GetType();
    m_data        = variable.m_data;
    variable.type = VARIABLE_NONE;
}

ScriptVariable::~ScriptVariable()
{
    ClearInternal();
}

void ScriptVariable::CastBoolean(void)
{
    int newvalue = booleanValue();

    ClearInternal();

    type            = VARIABLE_INTEGER;
    m_data.intValue = newvalue;
}

void ScriptVariable::CastConstArrayValue(void)
{
    con_map_enum<ScriptVariable, ScriptVariable> en;
    ScriptConstArrayHolder                      *constArrayValue;
    ScriptVariable                              *value;
    int                                          i;
    ConList                                     *listeners;

    switch (GetType()) {
    case VARIABLE_POINTER:
        ClearPointerInternal();
    case VARIABLE_NONE:
        throw ScriptException("cannot cast NIL to an array");

    case VARIABLE_CONSTARRAY:
        return;

    case VARIABLE_ARRAY:
        constArrayValue = new ScriptConstArrayHolder(m_data.arrayValue->arrayValue.size());

        en = m_data.arrayValue->arrayValue;

        i = 0;

        for (value = en.NextValue(); value != NULL; value = en.NextValue()) {
            i++;
            constArrayValue->constArrayValue[i] = *value;
        }

        break;

    case VARIABLE_CONTAINER:
        constArrayValue = new ScriptConstArrayHolder(m_data.containerValue->NumObjects());

        for (int i = m_data.containerValue->NumObjects(); i > 0; i--) {
            constArrayValue->constArrayValue[i].setListenerValue(m_data.containerValue->ObjectAt(i));
        }
        break;

    case VARIABLE_SAFECONTAINER:
        listeners = *m_data.safeContainerValue;

        if (listeners) {
            constArrayValue = new ScriptConstArrayHolder(listeners->NumObjects());

            for (int i = listeners->NumObjects(); i > 0; i--) {
                constArrayValue->constArrayValue[i].setListenerValue(listeners->ObjectAt(i));
            }
        } else {
            constArrayValue = new ScriptConstArrayHolder(0);
        }
        break;

    default:
        constArrayValue                     = new ScriptConstArrayHolder(1);
        constArrayValue->constArrayValue[1] = *this;

        break;
    }

    ClearInternal();
    type                   = VARIABLE_CONSTARRAY;
    m_data.constArrayValue = constArrayValue;
}

void ScriptVariable::CastEntity(void)
{
    setListenerValue((Listener *)entityValue());
}

void ScriptVariable::CastFloat(void)
{
    setFloatValue(floatValue());
}

void ScriptVariable::CastInteger(void)
{
    setIntValue(intValue());
}

void ScriptVariable::CastString(void)
{
    setStringValue(stringValue());
}

void ScriptVariable::Clear()
{
    ClearInternal();
    type = 0;
}

void ScriptVariable::ClearInternal()
{
    switch (GetType()) {
    case VARIABLE_STRING:
        if (m_data.stringValue) {
            delete m_data.stringValue;
            m_data.stringValue = NULL;
        }

        break;

    case VARIABLE_ARRAY:
        if (m_data.arrayValue->refCount) {
            m_data.arrayValue->refCount--;
        } else {
            delete m_data.arrayValue;
        }

        m_data.arrayValue = NULL;
        break;

    case VARIABLE_CONSTARRAY:
        if (m_data.constArrayValue->refCount) {
            m_data.constArrayValue->refCount--;
        } else {
            delete m_data.constArrayValue;
        }

        m_data.constArrayValue = NULL;
        break;

    case VARIABLE_LISTENER:
        if (m_data.listenerValue) {
            delete m_data.listenerValue;
            m_data.listenerValue = NULL;
        }

        break;

    case VARIABLE_SAFECONTAINER:
        if (m_data.safeContainerValue) {
            delete m_data.safeContainerValue;
            m_data.safeContainerValue = NULL;
        }

        break;

    case VARIABLE_POINTER:
        m_data.pointerValue->remove(this);
        m_data.pointerValue = NULL;
        break;

    case VARIABLE_VECTOR:
        delete[] m_data.vectorValue;
        m_data.vectorValue = NULL;
        break;

    default:
        break;
    }
}

void ScriptVariable::ClearPointer()
{
    if (type == VARIABLE_POINTER) {
        return ClearPointerInternal();
    }
}

void ScriptVariable::ClearPointerInternal() const
{
    m_data.pointerValue->Clear();
}

const char *ScriptVariable::GetTypeName() const
{
    return typenames[type];
}

variabletype ScriptVariable::GetType() const
{
    return (variabletype)type;
}

qboolean ScriptVariable::IsEntity(void)
{
    if (type == VARIABLE_LISTENER) {
        if (!m_data.listenerValue->Pointer()
#if defined(GAME_DLL)
            || checkInheritance(Entity::classinfostatic(), m_data.listenerValue->Pointer()->classinfo())
#endif
        ) {
            return true;
        }
    }

    return false;
}

qboolean ScriptVariable::IsListener(void)
{
    return type == VARIABLE_LISTENER;
}

qboolean ScriptVariable::IsNumeric(void)
{
    return type == VARIABLE_INTEGER || type == VARIABLE_FLOAT;
}

qboolean ScriptVariable::IsConstArray() const
{
    return type == VARIABLE_CONSTARRAY || type == VARIABLE_CONTAINER || type == VARIABLE_SAFECONTAINER;
}

#ifdef WITH_SCRIPT_ENGINE

qboolean ScriptVariable::IsSimpleEntity(void)
{
    if (type == VARIABLE_LISTENER) {
        if (!m_data.listenerValue->Pointer()
            || checkInheritance(&SimpleEntity::ClassInfo, m_data.listenerValue->Pointer()->classinfo())) {
            return true;
        }
    }

    return false;
}

#endif

qboolean ScriptVariable::IsString(void)
{
    return (type == VARIABLE_STRING || type == VARIABLE_CONSTSTRING);
}

qboolean ScriptVariable::IsVector(void)
{
    return type == VARIABLE_VECTOR;
}

void ScriptVariable::PrintValue(void)
{
    switch (GetType()) {
    case VARIABLE_NONE:
        printf("None");
        break;

#ifdef WITH_SCRIPT_ENGINE
    case VARIABLE_CONSTSTRING:
        printf("%s", Director.GetString(m_data.intValue).c_str());
        break;
#endif

    case VARIABLE_STRING:
        printf("%s", m_data.stringValue->c_str());
        break;

    case VARIABLE_INTEGER:
        printf("%d", m_data.intValue);
        break;

    case VARIABLE_FLOAT:
        printf("%f", m_data.floatValue);
        break;

    case VARIABLE_CHAR:
        printf("%c", m_data.charValue);
        break;

    case VARIABLE_LISTENER:
        printf("<Listener>%p", m_data.listenerValue->Pointer());
        break;

    case VARIABLE_REF:
    case VARIABLE_ARRAY:
    case VARIABLE_CONSTARRAY:
    case VARIABLE_CONTAINER:
    case VARIABLE_SAFECONTAINER:
    case VARIABLE_POINTER:
        printf("type: %s", GetTypeName());
        break;

    case VARIABLE_VECTOR:
        printf("( %f %f %f )", m_data.vectorValue[0], m_data.vectorValue[1], m_data.vectorValue[2]);
        break;
    default:
        printf("unknown");
        break;
    }
}

void ScriptVariable::SetFalse(void)
{
    setIntValue(0);
}

void ScriptVariable::SetTrue(void)
{
    setIntValue(1);
}

int ScriptVariable::arraysize(void) const
{
    switch (GetType()) {
    case VARIABLE_NONE:
        return -1;

    case VARIABLE_ARRAY:
        return m_data.arrayValue->arrayValue.size();

    case VARIABLE_CONSTARRAY:
        return m_data.constArrayValue->size;

    case VARIABLE_CONTAINER:
        return m_data.containerValue->NumObjects();

    case VARIABLE_SAFECONTAINER:
        if (*m_data.safeContainerValue) {
            return (*m_data.safeContainerValue)->NumObjects();
        } else {
            return 0;
        }

    case VARIABLE_POINTER:
        ClearPointerInternal();
        return -1;

    default:
        return 1;
    }

    return 0;
}

size_t ScriptVariable::size(void) const
{
    switch (GetType()) {
    case VARIABLE_NONE:
        return -1;

    case VARIABLE_CONSTSTRING:
    case VARIABLE_STRING:
        return stringValue().length();

    case VARIABLE_LISTENER:
        return *m_data.listenerValue != NULL;

    case VARIABLE_ARRAY:
        return m_data.arrayValue->arrayValue.size();

    case VARIABLE_CONSTARRAY:
        return m_data.constArrayValue->size;

    case VARIABLE_CONTAINER:
        return m_data.containerValue->NumObjects();

    case VARIABLE_SAFECONTAINER:
        if (*m_data.safeContainerValue) {
            return (*m_data.safeContainerValue)->NumObjects();
        } else {
            return 0;
        }

    case VARIABLE_POINTER:
        ClearPointerInternal();
        return -1;

    default:
        return 1;
    }

    return 0;
}

bool ScriptVariable::booleanNumericValue(void)
{
    str value;

    switch (GetType()) {
    case VARIABLE_STRING:
    case VARIABLE_CONSTSTRING:
        value = stringValue();

        return atoi(value.c_str()) ? true : false;

    case VARIABLE_INTEGER:
        return m_data.intValue != 0;

    case VARIABLE_FLOAT:
        return fabs(m_data.floatValue) >= 0.00009999999747378752;

    case VARIABLE_LISTENER:
        return (*m_data.listenerValue) != NULL;

    default:
        throw ScriptException("Cannot cast '%s' to boolean numeric", GetTypeName());
    }

    return true;
}

bool ScriptVariable::booleanValue(void) const
{
    switch (GetType()) {
    case VARIABLE_NONE:
        return false;

    case VARIABLE_STRING:
        if (m_data.stringValue) {
            return m_data.stringValue->length() != 0;
        }

        return false;

    case VARIABLE_INTEGER:
        return m_data.intValue != 0;

    case VARIABLE_FLOAT:
        return fabs(m_data.floatValue) >= 0.00009999999747378752;

    case VARIABLE_CONSTSTRING:
        return m_data.intValue != STRING_EMPTY;

    case VARIABLE_LISTENER:
        return (*m_data.listenerValue) != NULL;

    default:
        return true;
    }
}

char ScriptVariable::charValue(void) const
{
    str value;

    switch (GetType()) {
    case VARIABLE_CHAR:
        return m_data.charValue;

    case VARIABLE_CONSTSTRING:
    case VARIABLE_STRING:
        value = stringValue();

        if (value.length() != 1) {
            throw ScriptException("Cannot cast string not of length 1 to char");
        }

        return *value;

    default:
        throw ScriptException("Cannot cast '%s' to char", GetTypeName());
    }

    return 0;
}

ScriptVariable *ScriptVariable::constArrayValue(void)
{
    return m_data.constArrayValue->constArrayValue;
}

#ifdef WITH_SCRIPT_ENGINE

str getname_null = "";

str& ScriptVariable::getName(void)
{
    return Director.GetString(GetKey());
}

short3& ScriptVariable::GetKey()
{
    return key;
}

void ScriptVariable::SetKey(const short3& key)
{
    this->key = key;
}

#endif

Entity *ScriptVariable::entityValue(void)
{
#if defined(GAME_DLL)
    return (Entity *)listenerValue();
#else
    return NULL;
#endif
}

void ScriptVariable::evalArrayAt(ScriptVariable& var)
{
    unsigned int    index;
    str             string;
    ScriptVariable *array;

    switch (GetType()) {
    case VARIABLE_VECTOR:
        index = var.intValue();

        if (index > 2) {
            Clear();
            throw ScriptException("Vector index '%d' out of range", index);
        }

        return setFloatValue(m_data.vectorValue[index]);

    case VARIABLE_NONE:
        break;

    case VARIABLE_CONSTSTRING:
    case VARIABLE_STRING:
        index  = var.intValue();
        string = stringValue();

        if (index >= string.length()) {
            Clear();
            throw ScriptException("String index %d out of range", index);
        }

        return setCharValue(string[index]);

    case VARIABLE_LISTENER:
        index = var.intValue();

        if (index != 1) {
            Clear();
            throw ScriptException("array index %d out of range", index);
        }

        break;

    case VARIABLE_ARRAY:
        array = m_data.arrayValue->arrayValue.find(var);

        if (array) {
            *this = *array;
        } else {
            Clear();
        }

        break;

    case VARIABLE_CONSTARRAY:
        index = var.intValue();

        if (!index || index > m_data.constArrayValue->size) {
            throw ScriptException("array index %d out of range", index);
        }

        *this = m_data.constArrayValue->constArrayValue[index];
        break;

    case VARIABLE_CONTAINER:
        index = var.intValue();

        if (!index || index > m_data.containerValue->NumObjects()) {
            throw ScriptException("array index %d out of range", index);
        }

        setListenerValue(m_data.containerValue->ObjectAt(index));
        break;

    case VARIABLE_SAFECONTAINER:
        index = var.intValue();

        if (!*m_data.safeContainerValue || !index || index > m_data.constArrayValue->size) {
            throw ScriptException("array index %d out of range", index);
        }

        setListenerValue((*m_data.safeContainerValue)->ObjectAt(index));
        break;

    default:
        Clear();
        throw ScriptException("[] applied to invalid type '%s'", typenames[GetType()]);
        break;
    }
}

float ScriptVariable::floatValue(void) const
{
    const char *string;
    float       val;

    switch (type) {
    case VARIABLE_FLOAT:
        return m_data.floatValue;

    case VARIABLE_INTEGER:
        return (float)m_data.intValue;

    /* Transform the string into an integer if possible */
    case VARIABLE_STRING:
    case VARIABLE_CONSTSTRING:
        string = stringValue();
        val    = atof((const char *)string);

        return val;

    default:
        throw ScriptException("Cannot cast '%s' to float", typenames[type]);
    }
}

int ScriptVariable::intValue(void) const
{
    str string;
    int val;

    switch (type) {
    case VARIABLE_INTEGER:
        return m_data.intValue;

    case VARIABLE_FLOAT:
        return (int)m_data.floatValue;

    case VARIABLE_STRING:
    case VARIABLE_CONSTSTRING:
        string = stringValue();
        val    = atoi(string);

        return val;

    default:
        throw ScriptException("Cannot cast '%s' to int", typenames[type]);
    }
}

Listener *ScriptVariable::listenerValue(void) const
{
    switch (type) {
#ifdef WITH_SCRIPT_ENGINE
    case VARIABLE_CONSTSTRING:
        return world->GetScriptTarget(Director.GetString(m_data.intValue));

    case VARIABLE_STRING:
        return world->GetScriptTarget(stringValue());
#endif

    case VARIABLE_LISTENER:
        return (Listener *)m_data.listenerValue->Pointer();

    default:
        throw ScriptException("Cannot cast '%s' to listener", typenames[type]);
    }

    return NULL;
}

Listener *ScriptVariable::listenerAt(uintptr_t index) const
{
    switch (type) {
    case VARIABLE_CONSTARRAY:
        return m_data.constArrayValue->constArrayValue[index].listenerValue();

    case VARIABLE_CONTAINER:
        return m_data.containerValue->ObjectAt(index);

    case VARIABLE_SAFECONTAINER:
        assert(*m_data.safeContainerValue);
        return (*m_data.safeContainerValue)->ObjectAt(index);

    default:
        throw ScriptException("Cannot cast '%s' to listener", typenames[type]);
    }
}

void ScriptVariable::newPointer(void)
{
    type = VARIABLE_POINTER;

    m_data.pointerValue = new ScriptPointer();
    m_data.pointerValue->add(this);
}

#ifdef WITH_SCRIPT_ENGINE

SimpleEntity *ScriptVariable::simpleEntityValue(void) const
{
    return (SimpleEntity *)listenerValue();
}

#endif

str ScriptVariable::stringValue() const
{
    str string;

    switch (GetType()) {
    case VARIABLE_NONE:
        return "NIL";

#ifdef WITH_SCRIPT_ENGINE
    case VARIABLE_CONSTSTRING:
        return Director.GetString(m_data.intValue);
#endif

    case VARIABLE_STRING:
        return *m_data.stringValue;

    case VARIABLE_INTEGER:
        return str(m_data.intValue);

    case VARIABLE_FLOAT:
        return str(m_data.floatValue);

    case VARIABLE_CHAR:
        return str(m_data.charValue);

    case VARIABLE_LISTENER:
        if (m_data.listenerValue->Pointer()) {
#ifdef WITH_SCRIPT_ENGINE
            if (m_data.listenerValue->Pointer()->isSubclassOf(SimpleEntity)) {
                SimpleEntity *s = (SimpleEntity *)m_data.listenerValue->Pointer();
                return s->targetname;
            } else {
                string = "class '" + str(m_data.listenerValue->Pointer()->getClassname()) + "'";
                return string;
            }
#else
            string = "class '" + str(m_data.listenerValue->Pointer()->getClassname()) + "'";
            return string;
#endif
        } else {
            return "NULL";
        }

    case VARIABLE_VECTOR:
        return str("( ") + str(m_data.vectorValue[0]) + str(" ") + str(m_data.vectorValue[1]) + str(" ")
             + str(m_data.vectorValue[2]) + str(" )");

    default:
        throw ScriptException("Cannot cast '%s' to string", typenames[GetType()]);
        break;
    }

    return "";
}

Vector ScriptVariable::vectorValue(void) const
{
    const char *string;
    float       x = 0.f, y = 0.f, z = 0.f;

    switch (type) {
    case VARIABLE_VECTOR:
        return Vector(m_data.vectorValue);

    case VARIABLE_CONSTSTRING:
    case VARIABLE_STRING:
        string = stringValue();

        if (strcmp(string, "") == 0) {
            throw ScriptException("cannot cast empty string to vector");
        }

        if (*string == '(') {
            if (sscanf(string, "(%f %f %f)", &x, &y, &z) != 3) {
                if (sscanf(string, "(%f, %f, %f)", &x, &y, &z) != 3) {
                    throw ScriptException("Couldn't convert string to vector - malformed string '%s'", string);
                }
            }
        } else {
            if (sscanf(string, "%f %f %f", &x, &y, &z) != 3) {
                if (sscanf(string, "%f, %f, %f", &x, &y, &z) != 3) {
                    throw ScriptException("Couldn't convert string to vector - malformed string '%s'", string);
                }
            }
        }

        return Vector(x, y, z);
    case VARIABLE_LISTENER:
        {
            if (!m_data.listenerValue->Pointer()) {
                throw ScriptException("Cannot cast NULL to vector");
            }

#ifdef WITH_SCRIPT_ENGINE
            if (!checkInheritance(&SimpleEntity::ClassInfo, m_data.listenerValue->Pointer()->classinfo())) {
                throw ScriptException("Cannot cast '%s' to vector", GetTypeName());
            }

            SimpleEntity *ent = (SimpleEntity *)m_data.listenerValue->Pointer();

            return Vector(ent->origin[0], ent->origin[1], ent->origin[2]);
#else
            throw ScriptException("Cannot cast '%s' to vector", GetTypeName());
#endif
        }

    default:
        throw ScriptException("Cannot cast '%s' to vector", GetTypeName());
    }
}

#ifdef GAME_DLL

PathNode *ScriptVariable::pathNodeValue(void) const
{
    Listener *node = listenerValue();

    if (!node) {
        throw ScriptException("listener is NULL");
    }

    if (!node->isSubclassOf(PathNode)) {
        throw ScriptException("listener is not a path node");
    }

    return (PathNode *)node;
}

Waypoint *ScriptVariable::waypointValue(void) const
{
    Listener *node = listenerValue();

    if (!node) {
        throw ScriptException("listener is NULL");
    }

    if (!node->isSubclassOf(Waypoint)) {
        throw ScriptException("listener is not a way point");
    }

    return (Waypoint *)node;
}

#endif

void ScriptVariable::setArrayAt(ScriptVariable& index, ScriptVariable& value)
{
    return m_data.refValue->setArrayAtRef(index, value);
}

void ScriptVariable::setArrayAtRef(ScriptVariable& index, ScriptVariable& value)
{
    unsigned int intValue;
    str          string;

    switch (type) {
    case VARIABLE_VECTOR:
        intValue = index.intValue();

        if (intValue > 2) {
            throw ScriptException("Vector index '%d' out of range", intValue);
        }

        m_data.vectorValue[intValue] = value.floatValue();
        break;

    case VARIABLE_REF:
        return;

    case VARIABLE_NONE:
        type = VARIABLE_ARRAY;

        m_data.arrayValue = new ScriptArrayHolder;

        if (value.GetType() != VARIABLE_NONE) {
            m_data.arrayValue->arrayValue[index] = value;
        }

        break;

    case VARIABLE_ARRAY:
        if (value.GetType() == VARIABLE_NONE) {
            m_data.arrayValue->arrayValue.remove(index);
        } else {
            m_data.arrayValue->arrayValue[index] = value;
        }
        break;

    case VARIABLE_STRING:
    case VARIABLE_CONSTSTRING:
        intValue = index.intValue();
        string   = stringValue();

        if (intValue >= strlen(string)) {
            throw ScriptException("String index '%d' out of range", intValue);
        }

        string[intValue] = value.charValue();

        setStringValue(string);

        break;

    case VARIABLE_CONSTARRAY:
        intValue = index.intValue();

        if (!intValue || intValue > m_data.constArrayValue->size) {
            throw ScriptException("array index %d out of range", intValue);
        }

        if (value.GetType()) {
            m_data.constArrayValue->constArrayValue[intValue] = value;
        } else {
            m_data.constArrayValue->constArrayValue[intValue].Clear();
        }

        break;

    default:
        throw ScriptException("[] applied to invalid type '%s'\n", typenames[GetType()]);
        break;
    }
}

void ScriptVariable::setArrayRefValue(ScriptVariable& var)
{
    setRefValue(&(*m_data.refValue)[var]);
}

void ScriptVariable::setCharValue(char newvalue)
{
    ClearInternal();

    type             = VARIABLE_CHAR;
    m_data.charValue = newvalue;
}

void ScriptVariable::setContainerValue(Container<SafePtr<Listener>> *newvalue)
{
    ClearInternal();

    type                  = VARIABLE_CONTAINER;
    m_data.containerValue = newvalue;
}

void ScriptVariable::setSafeContainerValue(ConList *newvalue)
{
    ClearInternal();

    if (newvalue) {
        type                      = VARIABLE_SAFECONTAINER;
        m_data.safeContainerValue = new SafePtr<ConList>(newvalue);
    } else {
        type = VARIABLE_NONE;
    }
}

void ScriptVariable::setConstArrayValue(ScriptVariable *pVar, unsigned int size)
{
    ScriptConstArrayHolder *constArray = new ScriptConstArrayHolder(pVar - 1, size);

    ClearInternal();
    type = VARIABLE_CONSTARRAY;

    m_data.constArrayValue = constArray;
}

#ifdef WITH_SCRIPT_ENGINE

const_str ScriptVariable::constStringValue(void) const
{
    if (GetType() == VARIABLE_CONSTSTRING) {
        return m_data.intValue;
    } else {
        return Director.AddString(stringValue());
    }
}

void ScriptVariable::setConstStringValue(const_str s)
{
    ClearInternal();
    type            = VARIABLE_CONSTSTRING;
    m_data.intValue = s;
}

#endif

void ScriptVariable::setFloatValue(float newvalue)
{
    ClearInternal();
    type              = VARIABLE_FLOAT;
    m_data.floatValue = newvalue;
}

void ScriptVariable::setIntValue(int newvalue)
{
    ClearInternal();
    type            = VARIABLE_INTEGER;
    m_data.intValue = newvalue;
}

void ScriptVariable::setListenerValue(Listener *newvalue)
{
    ClearInternal();

    type = VARIABLE_LISTENER;

    m_data.listenerValue = new SafePtr<Listener>(newvalue);
}

void ScriptVariable::setPointer(const ScriptVariable& newvalue)
{
    if (GetType() == VARIABLE_POINTER) {
        m_data.pointerValue->setValue(newvalue);
    }
}

void ScriptVariable::setRefValue(ScriptVariable *ref)
{
    ClearInternal();

    type            = VARIABLE_REF;
    m_data.refValue = ref;
}

void ScriptVariable::setStringValue(str newvalue)
{
    str *s;

    ClearInternal();
    type = VARIABLE_STRING;

    s = new str(newvalue);

    m_data.stringValue = s;
}

void ScriptVariable::setVectorValue(const Vector& newvector)
{
    ClearInternal();

    type               = VARIABLE_VECTOR;
    m_data.vectorValue = new float[3];
    newvector.copyTo(m_data.vectorValue);
}

void ScriptVariable::operator+=(const ScriptVariable& value)
{
    int type = GetType();

    switch (type + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '+' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) + ( int )
        m_data.intValue = m_data.intValue + value.m_data.intValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) + ( float )
        setFloatValue((float)m_data.intValue + value.m_data.floatValue);
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) + ( float )
        m_data.floatValue = m_data.floatValue + value.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) + ( int )
        m_data.floatValue = m_data.floatValue + value.m_data.intValue;
        break;

    case VARIABLE_STRING + VARIABLE_STRING *VARIABLE_MAX: // ( string )			+		( string )
    case VARIABLE_INTEGER
        + VARIABLE_STRING                 *VARIABLE_MAX: // ( int )				+		( string )
    case VARIABLE_FLOAT + VARIABLE_STRING *VARIABLE_MAX: // ( float )			+		( string )
    case VARIABLE_CHAR
        + VARIABLE_STRING *VARIABLE_MAX: // ( char )				+		( string )
    case VARIABLE_CONSTSTRING
        + VARIABLE_STRING                        *VARIABLE_MAX: // ( const string )		+		( string )
    case VARIABLE_LISTENER + VARIABLE_STRING     *VARIABLE_MAX: // ( listener )			+		( string )
    case VARIABLE_VECTOR + VARIABLE_STRING       *VARIABLE_MAX: // ( vector )			+		( string )
    case VARIABLE_STRING + VARIABLE_INTEGER      *VARIABLE_MAX: // ( string )			+		( int )
    case VARIABLE_CONSTSTRING + VARIABLE_INTEGER *VARIABLE_MAX: // ( const string )		+		( int )
    case VARIABLE_STRING + VARIABLE_FLOAT        *VARIABLE_MAX: // ( string )			+		( float )
    case VARIABLE_CONSTSTRING + VARIABLE_FLOAT   *VARIABLE_MAX: // ( const string )		+		( float )
    case VARIABLE_STRING + VARIABLE_CHAR         *VARIABLE_MAX: // ( string )			+		( char )
    case VARIABLE_CONSTSTRING + VARIABLE_CHAR    *VARIABLE_MAX: // ( const string )		+		( char )
    case VARIABLE_STRING
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( string )			+		( const string )
    case VARIABLE_INTEGER
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( int )				+		( const string )
    case VARIABLE_FLOAT
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( float )			+		( const string )
    case VARIABLE_CHAR
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( char )				+		( const string )
    case VARIABLE_CONSTSTRING
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( const string )		+		( const string )
    case VARIABLE_LISTENER
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( listener )			+		( const string )
    case VARIABLE_VECTOR
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( vector )			+		( const string )
    case VARIABLE_STRING
        + VARIABLE_LISTENER *VARIABLE_MAX: // ( string )			+		( listener )
    case VARIABLE_CONSTSTRING
        + VARIABLE_LISTENER                *VARIABLE_MAX: // ( const string )		+		( listener )
    case VARIABLE_STRING + VARIABLE_VECTOR *VARIABLE_MAX: // ( string )			+		( vector )
    case VARIABLE_CONSTSTRING
        + VARIABLE_VECTOR *VARIABLE_MAX: // ( const string )		+		( vector )
        setStringValue(stringValue() + value.stringValue());
        break;

    case VARIABLE_VECTOR + VARIABLE_VECTOR *VARIABLE_MAX:
        VectorAdd(m_data.vectorValue, value.m_data.vectorValue, m_data.vectorValue);
        break;
    }
}

void ScriptVariable::operator-=(const ScriptVariable& value)
{
    switch (GetType() + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '-' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) - ( int )
        m_data.intValue = m_data.intValue - value.m_data.intValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) - ( float )
        setFloatValue((float)m_data.intValue - value.m_data.floatValue);
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) - ( float )
        m_data.floatValue = m_data.floatValue - value.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) - ( int )
        m_data.floatValue = m_data.floatValue - value.m_data.intValue;
        break;

    case VARIABLE_VECTOR + VARIABLE_VECTOR *VARIABLE_MAX: // ( vector ) - ( vector )
        VectorSubtract(m_data.vectorValue, value.m_data.vectorValue, m_data.vectorValue);
        break;
    }
}

void ScriptVariable::operator*=(const ScriptVariable& value)
{
    switch (GetType() + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '*' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) * ( int )
        m_data.intValue = m_data.intValue * value.m_data.intValue;
        break;

    case VARIABLE_VECTOR + VARIABLE_INTEGER *VARIABLE_MAX: // ( vector ) * ( int )
        VectorScale(m_data.vectorValue, (float)value.m_data.intValue, m_data.vectorValue);
        break;

    case VARIABLE_VECTOR + VARIABLE_FLOAT *VARIABLE_MAX: // ( vector ) * ( float )
        VectorScale(m_data.vectorValue, value.m_data.floatValue, m_data.vectorValue);
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) * ( float )
        setFloatValue((float)m_data.intValue * value.m_data.floatValue);
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) * ( float )
        m_data.floatValue = m_data.floatValue * value.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) * ( int )
        m_data.floatValue = m_data.floatValue * value.m_data.intValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_VECTOR *VARIABLE_MAX: // ( int ) * ( vector )
        setVectorValue((float)m_data.intValue * Vector(value.m_data.vectorValue));
        break;

    case VARIABLE_FLOAT + VARIABLE_VECTOR *VARIABLE_MAX: // ( float ) * ( vector )
        setVectorValue(m_data.floatValue * Vector(value.m_data.vectorValue));
        break;

    case VARIABLE_VECTOR + VARIABLE_VECTOR *VARIABLE_MAX: // ( vector ) * ( vector )
        setFloatValue(DotProduct(m_data.vectorValue, value.m_data.vectorValue));
        break;
    }
}

void ScriptVariable::operator/=(const ScriptVariable& value)
{
    switch (GetType() + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '/' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) / ( int )
        if (value.m_data.intValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.intValue = m_data.intValue / value.m_data.intValue;
        break;

    case VARIABLE_VECTOR + VARIABLE_INTEGER *VARIABLE_MAX: // ( vector ) / ( int )
        if (value.m_data.intValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        (Vector) m_data.vectorValue = (Vector)m_data.vectorValue / (float)value.m_data.intValue;
        break;

    case VARIABLE_VECTOR + VARIABLE_FLOAT *VARIABLE_MAX: // ( vector ) / ( float )
        if (value.m_data.floatValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.vectorValue[0] = m_data.vectorValue[0] / value.m_data.floatValue;
        m_data.vectorValue[1] = m_data.vectorValue[1] / value.m_data.floatValue;
        m_data.vectorValue[2] = m_data.vectorValue[2] / value.m_data.floatValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) / ( float )
        if (value.m_data.floatValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        setFloatValue((float)m_data.intValue / value.m_data.floatValue);
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) / ( float )
        if (value.m_data.floatValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.floatValue = m_data.floatValue / value.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) / ( int )
        if (value.m_data.intValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.floatValue = m_data.floatValue / value.m_data.intValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_VECTOR *VARIABLE_MAX: // ( int ) / ( vector )
        if (m_data.intValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        setVectorValue((float)m_data.intValue / Vector(value.m_data.vectorValue));
        break;

    case VARIABLE_FLOAT + VARIABLE_VECTOR *VARIABLE_MAX: // ( float ) / ( vector )
        if (m_data.floatValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        setVectorValue(m_data.floatValue / Vector(value.m_data.vectorValue));
        break;

    case VARIABLE_VECTOR + VARIABLE_VECTOR *VARIABLE_MAX: // ( vector ) / ( vector )
        m_data.vectorValue = vec_zero;

        if (value.m_data.vectorValue[0] != 0) {
            m_data.vectorValue[0] = m_data.vectorValue[0] / value.m_data.vectorValue[0];
        }

        if (value.m_data.vectorValue[1] != 0) {
            m_data.vectorValue[1] = m_data.vectorValue[1] / value.m_data.vectorValue[1];
        }

        if (value.m_data.vectorValue[2] != 0) {
            m_data.vectorValue[2] = m_data.vectorValue[2] / value.m_data.vectorValue[2];
        }
        break;
    }
}

void ScriptVariable::operator%=(const ScriptVariable& value)
{
    float mult = 0.0f;

    switch (GetType() + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '%%' applied to incompatible types '%s' and '%s'", typenames[GetType()], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) % ( int )
        if (value.m_data.intValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.intValue = m_data.intValue % value.m_data.intValue;
        break;

    case VARIABLE_VECTOR + VARIABLE_INTEGER *VARIABLE_MAX: // ( vector ) % ( int )
        if (value.m_data.intValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.vectorValue[0] = fmod(m_data.vectorValue[0], value.m_data.intValue);
        m_data.vectorValue[1] = fmod(m_data.vectorValue[1], value.m_data.intValue);
        m_data.vectorValue[2] = fmod(m_data.vectorValue[2], value.m_data.intValue);
        break;

    case VARIABLE_VECTOR + VARIABLE_FLOAT *VARIABLE_MAX: // ( vector ) % ( float )
        if (value.m_data.floatValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.vectorValue[0] = fmod(m_data.vectorValue[0], value.m_data.floatValue);
        m_data.vectorValue[1] = fmod(m_data.vectorValue[1], value.m_data.floatValue);
        m_data.vectorValue[2] = fmod(m_data.vectorValue[2], value.m_data.floatValue);
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) % ( float )
        if (value.m_data.floatValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        setFloatValue(fmod((float)m_data.intValue, value.m_data.floatValue));
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) % ( float )
        if (value.m_data.floatValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.floatValue = fmod(m_data.floatValue, value.m_data.floatValue);
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) % ( int )
        if (value.m_data.intValue == 0) {
            throw ScriptException("Division by zero error\n");
        }

        m_data.floatValue = fmod(m_data.floatValue, (float)value.m_data.intValue);
        break;

    case VARIABLE_INTEGER + VARIABLE_VECTOR *VARIABLE_MAX: // ( int ) % ( vector )
        mult = (float)m_data.intValue;

        if (mult == 0) {
            throw ScriptException("Division by zero error\n");
        }

        setVectorValue(vec_zero);

        m_data.vectorValue[0] = fmod(value.m_data.vectorValue[0], mult);
        m_data.vectorValue[1] = fmod(value.m_data.vectorValue[1], mult);
        m_data.vectorValue[2] = fmod(value.m_data.vectorValue[2], mult);
        break;

    case VARIABLE_FLOAT + VARIABLE_VECTOR *VARIABLE_MAX: // ( float ) % ( vector )
        mult = m_data.floatValue;

        if (mult == 0) {
            throw ScriptException("Division by zero error\n");
        }

        setVectorValue(vec_zero);

        m_data.vectorValue[0] = fmod(m_data.vectorValue[0], mult);
        m_data.vectorValue[1] = fmod(m_data.vectorValue[1], mult);
        m_data.vectorValue[2] = fmod(m_data.vectorValue[2], mult);
        break;

    case VARIABLE_VECTOR + VARIABLE_VECTOR *VARIABLE_MAX: // ( vector ) % ( vector )
        m_data.vectorValue = vec_zero;

        if (value.m_data.vectorValue[0] != 0) {
            m_data.vectorValue[0] = fmod(m_data.vectorValue[0], value.m_data.vectorValue[0]);
        }

        if (value.m_data.vectorValue[1] != 0) {
            m_data.vectorValue[1] = fmod(m_data.vectorValue[1], value.m_data.vectorValue[1]);
        }

        if (value.m_data.vectorValue[2] != 0) {
            m_data.vectorValue[2] = fmod(m_data.vectorValue[2], value.m_data.vectorValue[2]);
        }

        break;
    }
}

void ScriptVariable::operator&=(const ScriptVariable& value)
{
    int type = GetType();

    switch (type + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '&' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) &= ( int )
        m_data.intValue &= value.m_data.intValue;
        break;
    }
}

void ScriptVariable::operator^=(const ScriptVariable& value)
{
    int type = GetType();

    switch (type + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '^' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) ^= ( int )
        m_data.intValue ^= value.m_data.intValue;
        break;
    }
}

void ScriptVariable::operator|=(const ScriptVariable& value)
{
    int type = GetType();

    switch (type + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '|' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) |= ( int )
        m_data.intValue |= value.m_data.intValue;
        break;
    }
}

void ScriptVariable::operator<<=(const ScriptVariable& value)
{
    int type = GetType();

    switch (type + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '<<' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) <<= ( int )
        m_data.intValue <<= value.m_data.intValue;
        break;
    }
}

void ScriptVariable::operator>>=(const ScriptVariable& value)
{
    int type = GetType();

    switch (type + value.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '>>' applied to incompatible types '%s' and '%s'", typenames[type], typenames[value.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) >>= ( int )
        m_data.intValue >>= value.m_data.intValue;
        break;
    }
}

bool ScriptVariable::operator!=(const ScriptVariable& value)
{
    return !(*this == value);
}

bool ScriptVariable::operator==(const ScriptVariable& value)
{
    int type = GetType();

    switch (type + value.GetType() * VARIABLE_MAX) {
    default: // ( lval )	==	( nil )
             // ( nil )	==	( rval )
        Clear();
        return false;

    case VARIABLE_NONE + VARIABLE_NONE *VARIABLE_MAX: // ( nil ) == ( nil )
        return true;

    case VARIABLE_LISTENER + VARIABLE_LISTENER *VARIABLE_MAX: // ( listener ) == ( listener )
        {
            Class *lval = NULL;
            Class *rval = NULL;

            if (m_data.listenerValue) {
                lval = m_data.listenerValue->Pointer();
            }

            if (value.m_data.listenerValue) {
                rval = value.m_data.listenerValue->Pointer();
            }

            return lval == rval;
        }

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) == ( int )
        return m_data.intValue == value.m_data.intValue;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) == ( float )
        return m_data.intValue == value.m_data.floatValue;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) == ( float )
        return m_data.floatValue == value.m_data.floatValue;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) == ( int )
        return m_data.floatValue == value.m_data.intValue;

    case VARIABLE_CONSTSTRING
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( const string )		==		( const string )
        return m_data.intValue == value.m_data.intValue;

    case VARIABLE_STRING + VARIABLE_STRING *VARIABLE_MAX: // ( string )			==		( string )
    case VARIABLE_INTEGER
        + VARIABLE_STRING                 *VARIABLE_MAX: // ( int )				==		( string )
    case VARIABLE_FLOAT + VARIABLE_STRING *VARIABLE_MAX: // ( float )			==		( string )
    case VARIABLE_CHAR
        + VARIABLE_STRING *VARIABLE_MAX: // ( char )				==		( string )
    case VARIABLE_CONSTSTRING
        + VARIABLE_STRING *VARIABLE_MAX: // ( const string )		==		( string )
    case VARIABLE_LISTENER
        + VARIABLE_STRING                  *VARIABLE_MAX: // ( listener )			==		( string )
    case VARIABLE_VECTOR + VARIABLE_STRING *VARIABLE_MAX: // ( vector )			==		( string )
    case VARIABLE_STRING
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( string )			==		( const string )
    case VARIABLE_INTEGER
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( int )				==		( const string )
    case VARIABLE_FLOAT
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( float )			==		( const string )
    case VARIABLE_CHAR
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( char )				==		( const string )
    case VARIABLE_LISTENER
        + VARIABLE_CONSTSTRING *VARIABLE_MAX: // ( listener )			==		( const string )
    case VARIABLE_VECTOR
        + VARIABLE_CONSTSTRING                   *VARIABLE_MAX: // ( vector )			==		( const string )
    case VARIABLE_STRING + VARIABLE_INTEGER      *VARIABLE_MAX: // ( string )			==		( int )
    case VARIABLE_CONSTSTRING + VARIABLE_INTEGER *VARIABLE_MAX: // ( const string )		==		( int )
    case VARIABLE_STRING + VARIABLE_FLOAT        *VARIABLE_MAX: // ( string )			==		( float )
    case VARIABLE_CONSTSTRING + VARIABLE_FLOAT   *VARIABLE_MAX: // ( const string )		==		( float )
    case VARIABLE_STRING + VARIABLE_CHAR         *VARIABLE_MAX: // ( string )			==		( char )
    case VARIABLE_CONSTSTRING + VARIABLE_CHAR    *VARIABLE_MAX: // ( const string )		==		( char )
    case VARIABLE_STRING + VARIABLE_LISTENER     *VARIABLE_MAX: // ( string )			==		( listener )
    case VARIABLE_CONSTSTRING
        + VARIABLE_LISTENER                *VARIABLE_MAX: // ( const string )		==		( listener )
    case VARIABLE_STRING + VARIABLE_VECTOR *VARIABLE_MAX: // ( string )			==		( vector )
    case VARIABLE_CONSTSTRING
        + VARIABLE_VECTOR *VARIABLE_MAX: // ( const string )		==		( vector )
        {
            str lval = stringValue();
            str rval = value.stringValue();

            return (!lval.length() && !rval.length()) || (lval == rval);
        }

    case VARIABLE_CHAR + VARIABLE_CHAR *VARIABLE_MAX: // ( char ) == ( char )
        return m_data.charValue == value.m_data.charValue;

    case VARIABLE_VECTOR + VARIABLE_VECTOR *VARIABLE_MAX: // ( vector ) == ( vector )
        return VectorCompare(m_data.vectorValue, value.m_data.vectorValue) ? true : false;
    }
}

ScriptVariable& ScriptVariable::operator=(const ScriptVariable& variable)
{
    if (type == variable.GetType() && m_data.anyValue == variable.m_data.anyValue) {
        return *this;
    }

    if (type != variable.GetType()) {
        ClearInternal();

        type = variable.GetType();

        switch (type) {
        case VARIABLE_NONE:
            break;

        case VARIABLE_CONSTSTRING:
            m_data.intValue = variable.m_data.intValue;
            break;

        case VARIABLE_STRING:
            m_data.stringValue = new str(*variable.m_data.stringValue);
            break;

        case VARIABLE_FLOAT:
            m_data.floatValue = variable.m_data.floatValue;
            break;

        case VARIABLE_CHAR:
            m_data.charValue = variable.m_data.charValue;
            break;

        case VARIABLE_INTEGER:
            m_data.intValue = variable.m_data.intValue;
            break;

        case VARIABLE_LISTENER:
            m_data.listenerValue = new SafePtr<Listener>(*variable.m_data.listenerValue);
            break;

        case VARIABLE_ARRAY:
            m_data.arrayValue = variable.m_data.arrayValue;
            m_data.arrayValue->refCount++;
            break;

        case VARIABLE_CONSTARRAY:
            m_data.constArrayValue = variable.m_data.constArrayValue;
            m_data.constArrayValue->refCount++;
            break;

        case VARIABLE_CONTAINER:
            m_data.containerValue = variable.m_data.containerValue;
            break;

        case VARIABLE_SAFECONTAINER:
            m_data.safeContainerValue = new SafePtr<ConList>(*variable.m_data.safeContainerValue);
            break;

        case VARIABLE_POINTER:
            m_data.pointerValue = variable.m_data.pointerValue;
            m_data.pointerValue->add(this);
            break;

        case VARIABLE_VECTOR:
            m_data.vectorValue = (float *)new float[3];
            VectorCopy(variable.m_data.vectorValue, m_data.vectorValue);
            break;
        }
    } else {
        type = variable.GetType();

        switch (type) {
        case VARIABLE_NONE:
            break;

        case VARIABLE_CONSTSTRING:
            m_data.intValue = variable.m_data.intValue;
            break;

        case VARIABLE_STRING:
            *m_data.stringValue = *variable.m_data.stringValue;
            break;

        case VARIABLE_FLOAT:
            m_data.floatValue = variable.m_data.floatValue;
            break;

        case VARIABLE_CHAR:
            m_data.charValue = variable.m_data.charValue;
            break;

        case VARIABLE_INTEGER:
            m_data.intValue = variable.m_data.intValue;
            break;

        case VARIABLE_LISTENER:
            *m_data.listenerValue = *variable.m_data.listenerValue;
            break;

        case VARIABLE_ARRAY:
            ClearInternal();
            m_data.arrayValue = variable.m_data.arrayValue;
            m_data.arrayValue->refCount++;
            break;

        case VARIABLE_CONSTARRAY:
            ClearInternal();
            m_data.constArrayValue = variable.m_data.constArrayValue;
            m_data.constArrayValue->refCount++;
            break;

        case VARIABLE_CONTAINER:
            ClearInternal();
            m_data.containerValue = variable.m_data.containerValue;
            break;

        case VARIABLE_SAFECONTAINER:
            ClearInternal();
            m_data.safeContainerValue = new SafePtr<ConList>(*variable.m_data.safeContainerValue);
            break;

        case VARIABLE_POINTER:
            ClearInternal();
            m_data.pointerValue = variable.m_data.pointerValue;
            m_data.pointerValue->add(this);
            break;

        case VARIABLE_VECTOR:
            VectorCopy(variable.m_data.vectorValue, m_data.vectorValue);
            break;
        }
    }

    return *this;
}

ScriptVariable& ScriptVariable::operator=(ScriptVariable&& variable)
{
    ClearInternal();

#if defined(GAME_DLL)
    key          = variable.GetKey();
    variable.key = 0;
#endif

    type          = variable.GetType();
    m_data        = variable.m_data;
    variable.type = VARIABLE_NONE;

    if (type == VARIABLE_POINTER) {
        // if it's a pointer, make sure to properly point
        m_data.pointerValue->add(this);
        m_data.pointerValue->remove(&variable);
    }

    return *this;
}

ScriptVariable& ScriptVariable::operator[](ScriptVariable& index)
{
    int i;

    switch (GetType()) {
    case VARIABLE_NONE:
        type = VARIABLE_ARRAY;

        m_data.arrayValue = new ScriptArrayHolder;
        return m_data.arrayValue->arrayValue[index];

    case VARIABLE_ARRAY:
        return m_data.arrayValue->arrayValue[index];

    case VARIABLE_CONSTARRAY:
        i = index.intValue();

        if (i == 0 || i > m_data.constArrayValue->size) {
            throw ScriptException("array index %d out of range", i);
        }

        return m_data.constArrayValue->constArrayValue[i];

    default:
        throw ScriptException("[] applied to invalid type '%s'", typenames[GetType()]);
    }
}

ScriptVariable *ScriptVariable::operator[](unsigned index) const
{
    return &m_data.constArrayValue->constArrayValue[index];
}

ScriptVariable *ScriptVariable::operator*()
{
    return m_data.refValue;
}

void ScriptVariable::complement(void)
{
    if (type == VARIABLE_INTEGER) {
        m_data.intValue = ~m_data.intValue;
    } else {
        float value = floatValue();
        int   i     = ~*(int *)&value; // ley0k: evil floating point hack

        setFloatValue(*(float *)&i);
    }
}

void ScriptVariable::greaterthan(ScriptVariable& variable)
{
    switch (GetType() + variable.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '>' applied to incompatible types '%s' and '%s'",
            typenames[GetType()],
            typenames[variable.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) > ( int )
        m_data.intValue = m_data.intValue > variable.m_data.intValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) > ( float )
        m_data.intValue = m_data.intValue > variable.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) > ( float )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.floatValue > variable.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) > ( int )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.floatValue > variable.m_data.intValue;
        break;

    case VARIABLE_CHAR + VARIABLE_CHAR *VARIABLE_MAX: // ( char ) >= ( char )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.charValue >= variable.m_data.charValue;
        break;
    }
}

void ScriptVariable::greaterthanorequal(ScriptVariable& variable)
{
    switch (GetType() + variable.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '>=' applied to incompatible types '%s' and '%s'",
            typenames[GetType()],
            typenames[variable.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) >= ( int )
        m_data.intValue = m_data.intValue >= variable.m_data.intValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) >= ( float )
        m_data.intValue = m_data.intValue >= variable.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) >= ( float )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.floatValue >= variable.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) >= ( int )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.floatValue >= variable.m_data.intValue;
        break;

    case VARIABLE_CHAR + VARIABLE_CHAR *VARIABLE_MAX: // ( char ) >= ( char )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.charValue >= variable.m_data.charValue;
        break;
    }
}

void ScriptVariable::lessthan(ScriptVariable& variable)
{
    switch (GetType() + variable.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '<' applied to incompatible types '%s' and '%s'",
            typenames[GetType()],
            typenames[variable.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) < ( int )
        m_data.intValue = m_data.intValue < variable.m_data.intValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) < ( float )
        m_data.intValue = m_data.intValue < variable.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) < ( float )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.floatValue < variable.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) < ( int )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.floatValue < variable.m_data.intValue;
        break;

    case VARIABLE_CHAR + VARIABLE_CHAR *VARIABLE_MAX: // ( char ) < ( char )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.charValue < variable.m_data.charValue;
        break;
    }
}

void ScriptVariable::lessthanorequal(ScriptVariable& variable)
{
    switch (GetType() + variable.GetType() * VARIABLE_MAX) {
    default:
        Clear();

        throw ScriptException(
            "binary '<=' applied to incompatible types '%s' and '%s'",
            typenames[GetType()],
            typenames[variable.GetType()]
        );

        break;

    case VARIABLE_INTEGER + VARIABLE_INTEGER *VARIABLE_MAX: // ( int ) <= ( int )
        m_data.intValue = m_data.intValue <= variable.m_data.intValue;
        break;

    case VARIABLE_INTEGER + VARIABLE_FLOAT *VARIABLE_MAX: // ( int ) <= ( float )
        m_data.intValue = m_data.intValue <= variable.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_FLOAT *VARIABLE_MAX: // ( float ) <= ( float )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.floatValue <= variable.m_data.floatValue;
        break;

    case VARIABLE_FLOAT + VARIABLE_INTEGER *VARIABLE_MAX: // ( float ) <= ( int )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.floatValue <= variable.m_data.intValue;
        break;

    case VARIABLE_CHAR + VARIABLE_CHAR *VARIABLE_MAX: // ( char ) <= ( char )
        type            = VARIABLE_INTEGER;
        m_data.intValue = m_data.charValue <= variable.m_data.charValue;
        break;
    }
}

void ScriptVariable::minus(void)
{
    if (GetType() == VARIABLE_INTEGER) {
        m_data.intValue = -m_data.intValue;
    } else if (GetType() == VARIABLE_FLOAT) {
        m_data.floatValue = -m_data.floatValue;
    } else {
        setIntValue(-intValue());
    }
}

ScriptVariable ScriptVariable::operator++(int)
{
    switch (type) {
    case VARIABLE_NONE:
        return *this;

    case VARIABLE_INTEGER:
        setIntValue(intValue() + 1);
        break;

    case VARIABLE_POINTER:
        ClearPointerInternal();
        break;

    case VARIABLE_FLOAT:
        setFloatValue(floatValue() + 1.0f);
        break;

    default:
        int newvalue = intValue();

        ClearInternal();

        type            = VARIABLE_INTEGER;
        m_data.intValue = newvalue + 1;

        break;
    }

    return *this;
}

ScriptVariable ScriptVariable::operator--(int)
{
    switch (type) {
    case VARIABLE_NONE:
        return *this;

    case VARIABLE_INTEGER:
        setIntValue(intValue() - 1);
        break;

    case VARIABLE_POINTER:
        ClearPointerInternal();
        break;

    case VARIABLE_FLOAT:
        setFloatValue(floatValue() - 1.0f);
        break;

    default:
        int newvalue = intValue();

        ClearInternal();

        type            = VARIABLE_INTEGER;
        m_data.intValue = newvalue - 1;

        break;
    }

    return *this;
}

#ifdef WITH_SCRIPT_ENGINE

template<>
class con_set<short3, ScriptVariable>::Entry
{
    friend con_set<short3, ScriptVariable>;
    friend con_set_enum<short3, ScriptVariable>;

private:
    Entry         *next;
    ScriptVariable value;

public:
#    ifdef ARCHIVE_SUPPORTED
    void Archive(Archiver& arc) { value.Archive(arc); }
#    endif
    short3& GetKey() { return value.GetKey(); }
    void SetKey(const short3& newKey) { value.SetKey(newKey); }
};

ScriptVariableList::ScriptVariableList() {}

void ScriptVariableList::ClearList(void)
{
    list.clear();
}

ScriptVariable *ScriptVariableList::GetOrCreateVariable(str name)
{
    return GetOrCreateVariable(Director.AddString(name));
}

ScriptVariable *ScriptVariableList::GetOrCreateVariable(unsigned int name)
{
    return &list.addKeyValue(name);
}

ScriptVariable *ScriptVariableList::GetVariable(str name) const
{
    return GetVariable(Director.AddString(name));
}

ScriptVariable *ScriptVariableList::GetVariable(unsigned int name) const
{
    return list.findKeyValue(name);
}

bool ScriptVariableList::VariableExists(str name) const
{
    return GetVariable(name) != NULL;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, int value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    variable->setIntValue(value);

    return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, float value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    variable->setFloatValue(value);

    return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, const char *value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    variable->setStringValue(value);

    return variable;
}

#    if defined(GAME_DLL)

ScriptVariable *ScriptVariableList::SetVariable(const char *name, Entity *value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    variable->setListenerValue((Listener *)value);

    return variable;
}

#    endif

ScriptVariable *ScriptVariableList::SetVariable(const char *name, Listener *value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    variable->setListenerValue(value);

    return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, ScriptVariable& value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    *variable = value;

    return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(const char *name, Vector& value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    variable->setVectorValue(value);

    return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(unsigned int name, ScriptVariable& value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    *variable = value;
    variable->SetKey(name);

    return variable;
}

ScriptVariable *ScriptVariableList::SetVariable(unsigned int name, ScriptVariable&& value)
{
    ScriptVariable *variable = GetOrCreateVariable(name);

    *variable = std::move(value);
    variable->SetKey(name);

    return variable;
}

void ScriptVariableList::Archive(Archiver& arc)
{
    Class::Archive(arc);
    list.Archive(arc);
}

CLASS_DECLARATION(Class, ScriptVariableList, NULL) {
    {NULL, NULL}
};

#endif
