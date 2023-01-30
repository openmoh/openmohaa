#include "consoleevent.h"
#include "g_local.h"

//===============================
// ConsoleEvent
//===============================

MEM_BlockAlloc<ConsoleEvent> ConsoleEvent_allocator;

CLASS_DECLARATION(Event, ConsoleEvent, NULL)
{
	{ NULL, NULL }
};

/*
=======================
new ConsoleEvent
=======================
*/
void* ConsoleEvent::operator new(size_t size)
{
	return ConsoleEvent_allocator.Alloc();
}

/*
=======================
delete ptr
=======================
*/
void ConsoleEvent::operator delete(void* ptr)
{
	ConsoleEvent_allocator.Free(ptr);
}

/*
=======================
ConsoleEvent
=======================
*/
ConsoleEvent::ConsoleEvent(void)
{
	m_consoleedict = NULL;
}

/*
=======================
SetConsoleEdict
=======================
*/
void ConsoleEvent::SetConsoleEdict(gentity_t* edict)
{
	m_consoleedict = edict;
}

/*
=======================
GetConsoleEdict
=======================
*/
gentity_t* ConsoleEvent::GetConsoleEdict(void)
{
	if (m_consoleedict)
		return m_consoleedict;

	return g_entities;
}

/*
=======================
ErrorInternal
=======================
*/
void ConsoleEvent::ErrorInternal(Listener* l, str text)
{
	gentity_t* edict = GetConsoleEdict();
	str eventname = getName();

	gi.DPrintf("^~^~^ Game ( Event '%s', Client '%s' ) : %s\n",
		eventname.c_str(),
		edict->client ? edict->client->pers.netname : "",
		text.c_str());

	gi.SendServerCommand(GetConsoleEdict() - g_entities,
		"print \"Console: '%s' : %s\n\"",
		eventname.c_str(),
		text.c_str());
}
