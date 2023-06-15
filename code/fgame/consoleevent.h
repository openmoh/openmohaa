#pragma once

#include <listener.h>
#include <mem_blockalloc.h>
#include "g_public.h"

class ConsoleEvent : public Event
{
private:
	gentity_t* m_consoleedict;

public:
	CLASS_PROTOTYPE(ConsoleEvent);

	void* operator new(size_t size);
	void	operator delete(void* ptr);

	ConsoleEvent();
	ConsoleEvent(str name) : Event(name) { m_consoleedict = NULL; }

	void			SetConsoleEdict(gentity_t* edict);
	gentity_t* GetConsoleEdict(void);

	void ErrorInternal(Listener* l, str text) override;
};

extern MEM_BlockAlloc<ConsoleEvent> ConsoleEvent_allocator;
