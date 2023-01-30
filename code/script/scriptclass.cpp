#include "scriptclass.h"
#include "scriptmaster.h"
#include "scriptthread.h"

//====================
// ScriptClass
//====================

MEM_BlockAlloc<ScriptClass> ScriptClass_allocator;

CLASS_DECLARATION(Listener, ScriptClass, NULL)
{
	{ NULL, NULL }
};

/*
====================
new ScriptClass
====================
*/
void* ScriptClass::operator new(size_t size)
{
	return ScriptClass_allocator.Alloc();
}

/*
====================
delete ptr
====================
*/
void ScriptClass::operator delete(void* ptr)
{
	ScriptClass_allocator.Free(ptr);
}

/*
====================
ScriptClass
====================
*/
ScriptClass::ScriptClass(GameScript* gameScript, Listener* self)
{
	m_Self = self;
	m_Script = gameScript;
	m_Threads = NULL;
}

/*
====================
ScriptClass
====================
*/
ScriptClass::ScriptClass()
{
	m_Self = NULL;
	m_Script = NULL;
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

	if (!m_Script->m_Filename)
	{
		// This is a temporary gamescript
		delete m_Script;
	}
}

/*
====================
Archive
====================
*/
void ScriptClass::Archive(Archiver& arc)
{
}

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
void ScriptClass::ArchiveScript(Archiver& arc, ScriptClass** obj)
{
	ScriptClass* scr;
	ScriptVM* m_current;
	ScriptThread* m_thread;
	int num;
	int i;

	if (arc.Saving())
	{
		scr = *obj;
		scr->ArchiveInternal(arc);

		num = 0;
		for (m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next)
			num++;

		arc.ArchiveInteger(&num);

		for (m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next)
			m_current->m_Thread->ArchiveInternal(arc);
	}
	else
	{
		scr = new ScriptClass();
		scr->ArchiveInternal(arc);

		arc.ArchiveInteger(&num);

		for (i = 0; i < num; i++)
		{
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
void ScriptClass::ArchiveCodePos(Archiver& arc, unsigned char** codePos)
{
	m_Script->ArchiveCodePos(arc, codePos);
}

/*
====================
CreateThreadInternal
====================
*/
ScriptThread* ScriptClass::CreateThreadInternal(const ScriptVariable& label)
{
	GameScript* scr;
	ScriptThread* thread = NULL;

	if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING)
	{
		ScriptClass* scriptClass = Director.CurrentScriptClass();
		scr = scriptClass->GetScript();

		if (label.GetType() == VARIABLE_CONSTSTRING)
			thread = Director.CreateScriptThread(scr, m_Self, label.constStringValue());
		else
			thread = Director.CreateScriptThread(scr, m_Self, label.stringValue());
	}
	else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1)
	{
		ScriptVariable* script = label[1];
		ScriptVariable* labelname = label[2];

		if (script->GetType() == VARIABLE_CONSTSTRING)
			scr = Director.GetGameScript(script->constStringValue());
		else
			scr = Director.GetGameScript(script->stringValue());

		if (labelname->GetType() == VARIABLE_CONSTSTRING)
			thread = Director.CreateScriptThread(scr, m_Self, labelname->constStringValue());
		else
			thread = Director.CreateScriptThread(scr, m_Self, labelname->stringValue());
	}
	else
	{
		ScriptError("ScriptClass::CreateThreadInternal: bad argument format");
	}

	return thread;
}

/*
====================
CreateScriptInternal
====================
*/
ScriptThread* ScriptClass::CreateScriptInternal(const ScriptVariable& label)
{
	GameScript* scr;
	ScriptThread* thread = NULL;

	if (label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING)
	{
		if (label.GetType() == VARIABLE_CONSTSTRING)
			thread = Director.CreateScriptThread(Director.GetGameScript(label.stringValue()), m_Self, "");
		else
			thread = Director.CreateScriptThread(Director.GetGameScript(label.constStringValue()), m_Self, "");
	}
	else if (label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1)
	{
		ScriptVariable* script = label[1];
		ScriptVariable* labelname = label[2];

		if (script->GetType() == VARIABLE_CONSTSTRING)
			scr = Director.GetGameScript(script->constStringValue());
		else
			scr = Director.GetGameScript(script->stringValue());

		if (labelname->GetType() == VARIABLE_CONSTSTRING)
			thread = Director.CreateScriptThread(scr, m_Self, labelname->constStringValue());
		else
			thread = Director.CreateScriptThread(scr, m_Self, labelname->stringValue());
	}
	else
	{
		ScriptError("ScriptClass::CreateScriptInternal: bad label type '%s'", label.GetTypeName());
	}

	return thread;
}

/*
====================
AddThread
====================
*/
void ScriptClass::AddThread(ScriptVM* m_ScriptVM)
{
	m_ScriptVM->next = m_Threads;
	m_Threads = m_ScriptVM;
}

/*
====================
KillThreads
====================
*/
void ScriptClass::KillThreads()
{
	if (!m_Threads) {
		return;
	}

	ScriptVM* m_current;
	ScriptVM* m_next;

	m_current = m_Threads;

	do
	{
		m_current->m_ScriptClass = NULL;

		m_next = m_current->next;
		delete m_current->m_Thread;

	} while (m_current = m_next);

	m_Threads = NULL;
}

/*
====================
RemoveThread
====================
*/
void ScriptClass::RemoveThread(ScriptVM* m_ScriptVM)
{
	if (m_Threads == m_ScriptVM)
	{
		m_Threads = m_ScriptVM->next;

		if (m_Threads == NULL) {
			delete this;
		}
	}
	else
	{
		ScriptVM* m_current = m_Threads;
		ScriptVM* i;

		for (i = m_Threads->next; i != m_ScriptVM; i = i->next) {
			m_current = i;
		}

		m_current->next = i->next;
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
unsigned char* ScriptClass::FindLabel(str label)
{
	return m_Script->m_State.FindLabel(label);
}

/*
====================
FindLabel
====================
*/
unsigned char* ScriptClass::FindLabel(const_str label)
{
	return m_Script->m_State.FindLabel(label);
}

/*
====================
NearestLabel
====================
*/
const_str ScriptClass::NearestLabel(unsigned char* pos)
{
	return m_Script->m_State.NearestLabel(pos);
}

/*
====================
GetCatchStateScript
====================
*/
StateScript* ScriptClass::GetCatchStateScript(unsigned char* in, unsigned char*& out)
{
	return m_Script->GetCatchStateScript(in, out);
}

/*
====================
GetScript
====================
*/
GameScript* ScriptClass::GetScript()
{
	return m_Script;
}

/*
====================
GetSelf
====================
*/
Listener* ScriptClass::GetSelf()
{
	return static_cast<Listener*>(m_Self.Pointer());
}
