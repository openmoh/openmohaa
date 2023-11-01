#include "scriptclass.h"
#include "../fgame/scriptmaster.h"
#include "../fgame/scriptthread.h"
#include "scriptexception.h"

//====================
// ScriptClass
//====================

MEM_BlockAlloc<ScriptClass> ScriptClass_allocator;

CLASS_DECLARATION(Listener, ScriptClass, NULL) {
    {NULL, NULL}
};

/*
====================
new ScriptClass
====================
*/
void *ScriptClass::operator new(size_t size)
{
    return ScriptClass_allocator.Alloc();
}

/*
====================
delete ptr
====================
*/
void ScriptClass::operator delete(void *ptr)
{
    ScriptClass_allocator.Free(ptr);
}

/*
====================
ScriptClass
====================
*/
ScriptClass::ScriptClass(GameScript *gameScript, Listener *self)
{
    m_Self    = self;
    m_Script  = gameScript;
    m_Threads = NULL;
}

/*
====================
ScriptClass
====================
*/
ScriptClass::ScriptClass()
{
    m_Self    = NULL;
    m_Script  = NULL;
    m_Threads = NULL;
}

/*
====================
~ScriptClass
====================
*/
ScriptClass::~ScriptClass()
{
    if (m_Script == NULL) {
        throw ScriptException("Attempting to delete dead class.");
    }

    KillThreads();

    if (!m_Script->m_Filename) {
        // This is a temporary gamescript
        delete m_Script;
    }
}

/*
====================
StoppedNotify
====================
*/
void ScriptClass::StoppedNotify()
{
    delete this;
}

/*
====================
Archive
====================
*/
void ScriptClass::Archive(Archiver& arc) {}

/*
====================
ArchiveInternal
====================
*/
void ScriptClass::ArchiveInternal(Archiver& arc)
{
    Listener::Archive(arc);

    arc.ArchiveObjectPosition(this);
    arc.ArchiveSafePointer(&m_Self);
    GameScript::Archive(arc, m_Script);
}

/*
====================
ArchiveScript
====================
*/
void ScriptClass::ArchiveScript(Archiver& arc, ScriptClass **obj)
{
    ScriptClass  *scr;
    ScriptVM     *m_current;
    ScriptThread *m_thread;
    int           num;
    int           i;

    if (arc.Saving()) {
        scr = *obj;
        scr->ArchiveInternal(arc);

        num = 0;
        for (m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next) {
            num++;
        }

        arc.ArchiveInteger(&num);

        for (m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next) {
            m_current->m_Thread->ArchiveInternal(arc);
        }
    } else {
        scr = new ScriptClass();
        scr->ArchiveInternal(arc);

        arc.ArchiveInteger(&num);

        for (i = 0; i < num; i++) {
            m_thread = new ScriptThread(scr, NULL);
            m_thread->ArchiveInternal(arc);
        }

        *obj = scr;
    }
}

/*
====================
ArchiveCodePos
====================
*/
void ScriptClass::ArchiveCodePos(Archiver& arc, unsigned char **codePos)
{
    m_Script->ArchiveCodePos(arc, codePos);
}

/*
====================
CreateThreadInternal
====================
*/
ScriptThread *ScriptClass::CreateThreadInternal(const ScriptVariable& label)
{
    GameScript   *scr;
    ScriptThread *thread = NULL;

    if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING) {
        if (label.GetType() == VARIABLE_CONSTSTRING) {
            thread = Director.CreateScriptThread(this, label.constStringValue());
        } else {
            thread = Director.CreateScriptThread(this, label.stringValue());
        }
    } else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1) {
        ScriptVariable *script    = label[1];
        ScriptVariable *labelname = label[2];

        if (script->GetType() == VARIABLE_CONSTSTRING) {
            scr = Director.GetGameScript(script->constStringValue());
        } else {
            scr = Director.GetGameScript(script->stringValue());
        }

        if (labelname->GetType() == VARIABLE_CONSTSTRING) {
            thread = Director.CreateScriptThread(scr, GetSelf(), labelname->constStringValue());
        } else {
            thread = Director.CreateScriptThread(scr, GetSelf(), labelname->stringValue());
        }
    } else {
        ScriptError("ScriptClass::CreateThreadInternal: bad argument format");
    }

    return thread;
}

/*
====================
CreateScriptInternal
====================
*/
ScriptThread *ScriptClass::CreateScriptInternal(const ScriptVariable& label)
{
    GameScript   *scr;
    ScriptThread *thread = NULL;

    if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING) {
        if (label.GetType() == VARIABLE_CONSTSTRING) {
            scr = Director.GetGameScript(label.constStringValue());
        } else {
            scr = Director.GetGameScript(label.stringValue());
        }
        thread = Director.CreateScriptThread(scr, GetSelf(), "");
    } else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1) {
        ScriptVariable *script    = label[1];
        ScriptVariable *labelname = label[2];

        if (script->GetType() == VARIABLE_CONSTSTRING) {
            scr = Director.GetGameScript(script->constStringValue());
        } else {
            scr = Director.GetGameScript(script->stringValue());
        }

        if (labelname->GetType() == VARIABLE_CONSTSTRING) {
            thread = Director.CreateScriptThread(scr, GetSelf(), labelname->constStringValue());
        } else {
            thread = Director.CreateScriptThread(scr, GetSelf(), labelname->stringValue());
        }
    } else {
        ScriptError("ScriptClass::CreateScriptInternal: bad label type '%s'", label.GetTypeName());
    }

    return thread;
}

/*
====================
AddThread
====================
*/
void ScriptClass::AddThread(ScriptVM *thread)
{
    thread->next = m_Threads;
    m_Threads    = thread;
}

/*
====================
KillThreads
====================
*/
void ScriptClass::KillThreads()
{
    ScriptVM *thread;
    ScriptVM *next;

    if (!m_Threads) {
        return;
    }

    for (thread = m_Threads; thread; thread = next) {
        if (g_scripttrace->integer && thread->CanScriptTracePrint()) {
            gi.DPrintf2("---KILLTHREADS THREAD: %p\n", thread);
        }

        thread->m_ScriptClass = NULL;
        next                  = thread->next;
        delete thread->m_Thread;
    }

    m_Threads = NULL;
}

/*
====================
RemoveThread
====================
*/
void ScriptClass::RemoveThread(ScriptVM *thread)
{
    ScriptVM *current;
    ScriptVM *next;

    if (m_Threads == thread) {
        m_Threads = thread->next;

        if (!m_Threads) {
            delete this;
        }
    } else {
        next = m_Threads;
        for (current = m_Threads->next; current != thread; current = current->next) {
            next = current;
        }

        next->next = current->next;
    }
}

/*
====================
Filename
====================
*/
str ScriptClass::Filename()
{
    return m_Script->Filename();
}

/*
====================
FindLabel
====================
*/
unsigned char *ScriptClass::FindLabel(str label)
{
    return m_Script->m_State.FindLabel(label);
}

/*
====================
FindLabel
====================
*/
unsigned char *ScriptClass::FindLabel(const_str label)
{
    return m_Script->m_State.FindLabel(label);
}

/*
====================
NearestLabel
====================
*/
const_str ScriptClass::NearestLabel(unsigned char *pos)
{
    return m_Script->m_State.NearestLabel(pos);
}

/*
====================
GetCatchStateScript
====================
*/
StateScript *ScriptClass::GetCatchStateScript(unsigned char *in, unsigned char *& out)
{
    return m_Script->GetCatchStateScript(in, out);
}

/*
====================
GetScript
====================
*/
GameScript *ScriptClass::GetScript()
{
    return m_Script;
}

/*
====================
GetSelf
====================
*/
Listener *ScriptClass::GetSelf()
{
    return static_cast<Listener *>(m_Self.Pointer());
}
