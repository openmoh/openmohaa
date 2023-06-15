#include "scriptflags.h"

#include "scriptthread.h"
#include "scriptvm.h"

FlagList flags;

Flag* FlagList::FindFlag(const char* name)
{
	for (int i = 0; i < m_Flags.NumObjects(); i++)
	{
		Flag* index = m_Flags[i];

		// found the flag
		if (strcmp(index->flagName, name) == 0) {
			return index;
		}
	}

	return NULL;
}

void FlagList::AddFlag(Flag* flag)
{
	m_Flags.AddObject(flag);
}

void FlagList::RemoveFlag(Flag* flag)
{
	m_Flags.RemoveObject(flag);
}

Flag::Flag()
{
	flags.AddFlag(this);
}

Flag::~Flag()
{
	flags.RemoveFlag(this);

	m_WaitList.FreeObjectList();
}

void Flag::Reset()
{
	bSignaled = false;
}

void Flag::Set()
{
	// Don't signal again
	if (bSignaled) {
		return;
	}

	bSignaled = true;

	for (int i = 0; i < m_WaitList.NumObjects(); i++)
	{
		ScriptVM* Thread = m_WaitList[i];

		if (Thread->state != STATE_DESTROYED && Thread->m_Thread != NULL) {
			Thread->m_Thread->StoppedWaitFor(STRING_EMPTY, false);
		}
	}

	// Clear the list
	m_WaitList.FreeObjectList();
}

void Flag::Wait(ScriptThread* Thread)
{
	// Don't wait if it's signaled
	if (bSignaled) {
		return;
	}

	Thread->StartedWaitFor();

	m_WaitList.AddObject(Thread->m_ScriptVM);
}
