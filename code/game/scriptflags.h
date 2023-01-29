#pragma once

#include "listener.h"

class Flag
{
public:
	char flagName[MAX_QPATH];
	qboolean bSignaled;

private:
	Container< ScriptVM* > m_WaitList;

public:
	Flag();
	~Flag();

	void Reset(void);
	void Set(void);
	void Wait(ScriptThread* Thread);
};

class FlagList
{
	friend class Flag;

private:
	void AddFlag(Flag* flag);
	void RemoveFlag(Flag* flag);

public:
	Container< Flag* > m_Flags;

	Flag* FindFlag(const char* name);
};

extern FlagList flags;
