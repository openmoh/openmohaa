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

// class.cpp: General class.

#include "../corepp/class.h"
#include "../corepp/listener.h"
#include "../qcommon/q_shared.h"

#if defined(GAME_DLL)

#    include "../fgame/g_local.h"

#    define CLASS_Printf  gi.Printf
#    define CLASS_DPrintf gi.DPrintf
#    define CLASS_Error   gi.Error

#elif defined(CGAME_DLL)

#    include "../cgame/cg_local.h"

#    define CLASS_Printf  cgi.Printf
#    define CLASS_DPrintf cgi.DPrintf
#    define CLASS_Error   cgi.Error

#else

#    define CLASS_Printf  Com_Printf
#    define CLASS_DPrintf Com_DPrintf
#    define CLASS_Error   Com_Error

#endif

#ifdef WITH_SCRIPT_ENGINE

#    include "../fgame/scriptmaster.h"

#endif

ClassDef *ClassDef::classlist;
ClassDef *ClassDef::classroot;
int       ClassDef::numclasses;

int                   ClassDef::dump_numclasses;
int                   ClassDef::dump_numevents;
Container<int>        ClassDef::sortedList;
Container<ClassDef *> ClassDef::sortedClassList;

int ClassDef::compareClasses(const void *arg1, const void *arg2)
{
    ClassDef *c1 = *(ClassDef **)arg1;
    ClassDef *c2 = *(ClassDef **)arg2;

    return Q_stricmp(c1->classname, c2->classname);
}

void ClassDef::SortClassList(Container<ClassDef *> *sortedList)
{
    ClassDef *c;

    sortedList->Resize(numclasses);

    for (c = classlist; c; c = c->next) {
        sortedList->AddObject(c);
    }

    qsort(
        (void *)sortedList->AddressOfObjectAt(1), (size_t)sortedList->NumObjects(), sizeof(ClassDef *), compareClasses
    );
}

ClassDef *getClassForID(const char *name)
{
    ClassDef *classlist = ClassDef::classlist;
    ClassDef *c;

    for (c = classlist; c; c = c->next) {
        if (c->classID && !Q_stricmp(c->classID, name)) {
            return c;
        }
    }

    return NULL;
}

ClassDef *getClass(const char *name)
{
    if (name == NULL || !*name) {
        return NULL;
    }

    ClassDef *list = ClassDef::classlist;
    ClassDef *c;

    for (c = list; c; c = c->next) {
        if (Q_stricmp(c->classname, name) == 0) {
            return c;
        }
    }

    return NULL;
}

ClassDef *getClassList(void)
{
    return ClassDef::classlist;
}

void listAllClasses(void)
{
    ClassDef *c;
    ClassDef *list = ClassDef::classlist;

    for (c = list; c; c = c->next) {
        CLASS_DPrintf("%s\n", c->classname);
    }
}

void listInheritanceOrder(const char *classname)
{
    ClassDef *cls;
    ClassDef *c;

    cls = getClass(classname);
    if (!cls) {
        CLASS_DPrintf("Unknown class: %s\n", classname);
        return;
    }
    for (c = cls; c != NULL; c = c->super) {
        CLASS_DPrintf("%s\n", c->classname);
    }
}

qboolean checkInheritance(const ClassDef *superclass, const ClassDef *subclass)
{
    const ClassDef *c;

    for (c = subclass; c != NULL; c = c->super) {
        if (c == superclass) {
            return true;
        }
    }
    return false;
}

qboolean checkInheritance(ClassDef *superclass, const char *subclass)
{
    ClassDef *c;

    c = getClass(subclass);

    if (c == NULL) {
        CLASS_DPrintf("Unknown class: %s\n", subclass);
        return false;
    }

    return checkInheritance(superclass, c);
}

qboolean checkInheritance(const char *superclass, const char *subclass)
{
    ClassDef *c1;
    ClassDef *c2;

    c1 = getClass(superclass);
    c2 = getClass(subclass);

    if (c1 == NULL) {
        CLASS_DPrintf("Unknown class: %s\n", superclass);
        return false;
    }

    if (c2 == NULL) {
        CLASS_DPrintf("Unknown class: %s\n", subclass);
        return false;
    }

    return checkInheritance(c1, c2);
}

void CLASS_Print(FILE *class_file, const char *fmt, ...)
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    if (class_file) {
        fprintf(class_file, "%s", text);
    } else {
        CLASS_DPrintf("%s", text);
    }
}

void CLASS_Print(fileHandle_t class_file, const char *fmt, ...)
{
    va_list argptr;
    char    text[1024];
    int     len;

    va_start(argptr, fmt);
    len = Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    if (class_file) {
#if defined(CGAME_DLL)
        cgi.FS_Write(text, len, class_file);
#elif defined(GAME_DLL)
        gi.FS_Write(text, len, class_file);
#else
        FS_Write(text, len, class_file);
#endif
    } else {
        CLASS_DPrintf("%s", text);
    }
}

Class::Class()
{
    SafePtrList = NULL;
}

Class::~Class()
{
    ClearSafePointers();
}

void Class::Archive(Archiver& arc) {}

void Class::ClearSafePointers(void)
{
    while (SafePtrList != NULL) {
        SafePtrList->Clear();
    }
}

void Class::warning(const char *function, const char *format, ...) const
{
    char        buffer[MAX_STRING_CHARS];
    const char *classname;
    va_list     va;

    va_start(va, format);
    Q_vsnprintf(buffer, sizeof(buffer), format, va);

    classname = classinfo()->classname;

#ifdef GAME_DLL
    gi.DPrintf(
#elif defined CGAME_DLL
    cgi.DPrintf(
#else
    Com_DPrintf(
#endif
        "%s::%s : %s\n", classname, function, buffer
    );
}

void Class::error(const char *function, const char *fmt, ...) const
{
    va_list argptr;
    char    text[1024];

    va_start(argptr, fmt);
    Q_vsnprintf(text, sizeof(text), fmt, argptr);
    va_end(argptr);

    if (getClassID()) {
        CLASS_Error(ERR_DROP, "%s::%s : %s\n", getClassID(), function, text);
    } else {
        CLASS_Error(ERR_DROP, "%s::%s : %s\n", getClassname(), function, text);
    }
}

qboolean Class::inheritsFrom(ClassDef *c) const
{
    return checkInheritance(c, classinfo());
}

qboolean Class::inheritsFrom(const char *name) const
{
    ClassDef *c;

    c = getClass(name);

    if (c == NULL) {
        CLASS_Printf("Unknown class: %s\n", name);
        return false;
    }

    return checkInheritance(c, classinfo());
}

qboolean Class::isInheritedBy(const char *name) const
{
    ClassDef *c;

    c = getClass(name);

    if (c == NULL) {
        CLASS_DPrintf("Unknown class: %s\n", name);
        return false;
    }

    return checkInheritance(classinfo(), c);
}

qboolean Class::isInheritedBy(ClassDef *c) const
{
    return checkInheritance(classinfo(), c);
}

const char *Class::getClassname(void) const
{
    return classinfo()->classname;
}

const char *Class::getClassID(void) const
{
    return classinfo()->classID;
}

const char *Class::getSuperclass(void) const
{
    return classinfo()->superclass;
}

ClassDef::ClassDef()
{
    this->classname      = NULL;
    this->classID        = NULL;
    this->superclass     = NULL;
    this->responses      = NULL;
    this->numEvents      = 0;
    this->responseLookup = NULL;
    this->newInstance    = NULL;
    this->classSize      = 0;
    this->super          = NULL;
    this->prev           = this;
    this->next           = this;

#ifdef WITH_SCRIPT_ENGINE
    this->waitTillSet = NULL;
#endif
}

ClassDef::ClassDef(
    const char         *classname,
    const char         *classID,
    const char         *superclass,
    ResponseDef<Class> *responses,
    void *(*newInstance)(void),
    int classSize
)
{
    ClassDef *node;

    if (classlist == NULL) {
        classlist = this;
    }

    this->classname      = classname;
    this->classID        = classID;
    this->superclass     = superclass;
    this->responses      = responses;
    this->numEvents      = 0;
    this->responseLookup = NULL;
    this->newInstance    = newInstance;
    this->classSize      = classSize;
    this->super          = getClass(superclass);

#ifdef WITH_SCRIPT_ENGINE
    this->waitTillSet = NULL;
#endif

    if (!classID) {
        this->classID = "";
    }

    for (node = classlist; node; node = node->next) {
        if ((node->super == NULL) && (!Q_stricmp(node->superclass, this->classname))
            && (Q_stricmp(node->classname, "Class"))) {
            node->super = this;
        }
    }

    // Add to front of list
    LL_SafeAdd(classroot, this, next, prev);

    numclasses++;
}

ClassDef::~ClassDef()
{
    ClassDef *node;

    LL_SafeRemoveRoot(classlist, this, next, prev);

    // Check if any subclasses were initialized before their superclass
    for (node = classlist; node; node = node->next) {
        if (node->super == this) {
            node->super = NULL;
        }
    }

    if (responseLookup) {
        delete[] responseLookup;
        responseLookup = NULL;
    }
#ifdef WITH_SCRIPT_ENGINE
    if (waitTillSet) {
        delete waitTillSet;
    }
#endif
}

#ifdef WITH_SCRIPT_ENGINE

void ClassDef::AddWaitTill(str s)
{
    return AddWaitTill(Director.AddString(s));
}

void ClassDef::AddWaitTill(const_str s)
{
    if (!waitTillSet) {
        waitTillSet = new con_set<const_str, const_str>;
    }

    waitTillSet->addKeyValue(s) = s;
}

void ClassDef::RemoveWaitTill(str s)
{
    return RemoveWaitTill(Director.AddString(s));
}

void ClassDef::RemoveWaitTill(const_str s)
{
    if (waitTillSet) {
        waitTillSet->remove(s);
    }
}

bool ClassDef::WaitTillDefined(str s)
{
    return WaitTillDefined(Director.AddString(s));
}

bool ClassDef::WaitTillDefined(const_str s)
{
    if (!waitTillSet) {
        return false;
    }

    return waitTillSet->findKeyValue(s) != NULL;
}

#endif

EventDef *ClassDef::GetDef(int eventnum)
{
    ResponseDef<Class> *r = responseLookup[eventnum];

    if (r) {
        return r->def;
    } else {
        return NULL;
    }
}

EventDef *ClassDef::GetDef(Event *ev)
{
    return GetDef(ev->eventnum);
}

int ClassDef::GetFlags(Event *event)
{
    EventDef *def = GetDef(event->eventnum);

    if (def) {
        return def->flags;
    } else {
        return 0;
    }
}

void ClassDef::BuildResponseList(void)
{
    ClassDef           *c;
    ResponseDef<Class> *r;
    int                 ev;
    int                 i;
    qboolean           *set;
    int                 num;

    if (responseLookup) {
        delete[] responseLookup;
        responseLookup = NULL;
    }
    //size will be total event count, because it WAS faster to look for an event via eventnum
    //nowadays there's not much overhead in performance, TODO: change size to appropriate.
    num            = Event::NumEventCommands();
    responseLookup = (ResponseDef<Class> **)new char[sizeof(ResponseDef<Class> *) * num];
    memset(responseLookup, 0, sizeof(ResponseDef<Class> *) * num);

    set = new qboolean[num];
    memset(set, 0, sizeof(qboolean) * num);

    this->numEvents = num;

    for (c = this; c != NULL; c = c->super) {
        r = c->responses;

        if (r) {
            for (i = 0; r[i].event != NULL; i++) {
                ev       = (int)r[i].event->eventnum;
                r[i].def = r[i].event->getInfo();

                if (!set[ev]) {
                    set[ev] = true;

                    if (r[i].response) {
                        responseLookup[ev] = &r[i];
                    } else {
                        responseLookup[ev] = NULL;
                    }
                }
            }
        }
    }

    delete[] set;
}

void ClassDef::BuildEventResponses(void)
{
    ClassDef *c;
    int       amount;
    int       numclasses;

    amount     = 0;
    numclasses = 0;

    for (c = classlist; c; c = c->next) {
        c->BuildResponseList();

        amount += c->numEvents * sizeof(Response *);
        numclasses++;
    }

    CLASS_DPrintf(
        "\n------------------\nEvent system initialized: "
        "%d classes %d events %d total memory in response list\n\n",
        numclasses,
        Event::NumEventCommands(),
        amount
    );
}

#define MAX_INHERITANCE 64

void ClassEvents(const char *classname, qboolean print_to_disk)
{
    str                 class_filename;
    str                 class_title;
    fileHandle_t        class_file;
    ClassDef           *c;
    ResponseDef<Class> *r;
    int                 ev;
    int                 i, j;
    qboolean           *set;
    int                 num, orderNum;
    Event             **events;
    byte               *order;
    ClassDef           *classes[MAX_INHERITANCE];
    ClassEventPrinter   printer;

    c          = getClass(classname);
    class_file = 0;

    if (print_to_disk) {
#if defined(GAME_DLL)
        class_file = gi.FS_FOpenFileWrite(class_filename);
#elif defined(CGAME_DLL)
        class_file = cgi.FS_FOpenFileWrite(class_filename);
#else
        class_file = FS_FOpenFileWrite_HomeData(class_filename);
#endif

        if (!class_file) {
            return;
        }
    }

    num = Event::NumEventCommands();

    set = new qboolean[num];
    memset(set, 0, sizeof(qboolean) * num);

    events = new Event *[num];
    memset(events, 0, sizeof(Event *) * num);

    order = new byte[num];
    memset(order, 0, sizeof(byte) * num);

    orderNum = 0;
    for (; c != NULL; c = c->super) {
        if (orderNum < MAX_INHERITANCE) {
            classes[orderNum] = c;
        }
        r = c->responses;
        if (r) {
            for (i = 0; r[i].event != NULL; i++) {
                ev = (int)r[i].event->eventnum;
                assert(ev < num);
                if (!set[ev]) {
                    set[ev] = true;

                    if (r[i].response) {
                        events[ev] = r[i].event;
                        order[ev]  = orderNum;
                    }
                }
            }
        }
        orderNum++;
    }

    Event::SortEventList(&ClassDef::sortedList);
    Container<EventDef *> sortedDefs;

    sortedDefs.Resize(ClassDef::sortedList.NumObjects());

    c = getClass(classname);
    for (int i = 1; i <= ClassDef::sortedList.NumObjects(); i++) {
        EventDef *def = c->GetDef(ClassDef::sortedList.ObjectAt(i));
        sortedDefs.AddObject(def);
    }

    ClassDef::sortedList.FreeObjectList();

    BasicFormatter formatter;
    printer.DumpClass(c, class_file, formatter, sortedDefs);

    delete[] events;
    delete[] order;
    delete[] set;

    if (class_file != 0) {
#if defined(GAME_DLL)
        gi.FS_FCloseFile(class_file);
#elif defined(CGAME_DLL)
        cgi.FS_FCloseFile(class_file);
#else
        FS_FCloseFile(class_file);
#endif
    }
}

#define MAX_CLASSES 1024

void DumpAllClasses(void)
{
    str               class_filename;
    str               class_title;
    fileHandle_t      class_file;
    ClassEventPrinter printer;

#if defined(GAME_DLL)
    class_filename = "g_allclasses.html";
    class_title    = "Game Module";
    class_file     = gi.FS_FOpenFileWrite(class_filename);
#elif defined(CGAME_DLL)
    class_filename = "cg_allclasses.html";
    class_title    = "Client Game Module";
    class_file     = cgi.FS_FOpenFileWrite(class_filename);
#else
    class_filename = "cl_allclasses.html";
    class_title    = "Client Module";
    class_file     = FS_FOpenFileWrite_HomeData(class_filename);
#endif

    HTMLFormater htmlFormater;
    printer.DumpAllClasses(class_file, htmlFormater, class_title);

#if defined(GAME_DLL)
    gi.FS_FCloseFile(class_file);
#elif defined(CGAME_DLL)
    cgi.FS_FCloseFile(class_file);
#else
    FS_FCloseFile(class_file);
#endif
}

ClassDefHook::ClassDefHook()
{
    this->classdef = NULL;
}

ClassDef
    Class::ClassInfo("Class", NULL, NULL, (ResponseDef<Class> *)Class::Responses, Class::_newInstance, sizeof(Class));

void *Class::_newInstance(void)
{
    return new Class();
}

ClassDef *Class::classinfo(void) const
{
    return &(Class::ClassInfo);
}

ClassDef *Class::classinfostatic(void)
{
    return &(Class::ClassInfo);
}

ResponseDef<Class> Class::Responses[] = {
    {NULL, NULL}
};