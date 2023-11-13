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

// scriptvm.cpp : Script virtual machine, interprets and execute scripts

#include "../fgame/g_local.h"
#include "../fgame/scriptmaster.h"
#include "../fgame/scriptthread.h"
#include "scriptclass.h"
#include "scriptvm.h"
#include "scriptcompiler.h"
#include "scriptexception.h"
#include "../fgame/game.h"
#include "../fgame/level.h"
#include "../fgame/parm.h"
#include "../fgame/worldspawn.h"

#include <utility>

#ifdef CGAME_DLL

#    define VM_Printf  cgi.Printf
#    define VM_DPrintf cgi.DPrintf

#elif defined GAME_DLL

#    define VM_Printf  gi.Printf
#    define VM_DPrintf gi.DPrintf2

#else

#    define VM_Printf  printf
#    define VM_DPrintf printf

#endif

static const ScriptVM *currentScriptFile;
static unsigned int    currentScriptLine;

class ScriptCommandEvent : public Event
{
public:
    ScriptCommandEvent(unsigned int eventNum);
    ScriptCommandEvent(unsigned int eventNum, int numArgs);
};

ScriptCommandEvent::ScriptCommandEvent(unsigned int eventNum)
    : Event(eventNum)
{
    fromScript = true;
}

ScriptCommandEvent::ScriptCommandEvent(unsigned int eventNum, int numArgs)
    : Event(eventNum, numArgs)
{
    fromScript = true;
}

ScriptVMStack::ScriptVMStack()
    : localStack(nullptr)
    , stackBottom(nullptr)
    , pTop(nullptr)
{}

ScriptVMStack::ScriptVMStack(size_t stackSize)
{
    if (!stackSize) {
        stackSize = 1;
    }

    // allocate at once
    uint8_t *data = (uint8_t *)gi.Malloc((sizeof(ScriptVariable) + sizeof(ScriptVariable *)) * stackSize);
    localStack    = new (data) ScriptVariable[stackSize];
    data += sizeof(ScriptVariable) * stackSize;

    pTop        = localStack;
    stackBottom = localStack + stackSize;
}

ScriptVMStack::ScriptVMStack(ScriptVMStack&& other)
    : localStack(other.localStack)
    , stackBottom(other.stackBottom)
    , pTop(other.pTop)
{
    other.localStack = other.stackBottom = nullptr;
    other.pTop                           = nullptr;
}

ScriptVMStack& ScriptVMStack::operator=(ScriptVMStack&& other)
{
    localStack       = other.localStack;
    stackBottom      = other.stackBottom;
    pTop             = other.pTop;
    other.localStack = other.stackBottom = nullptr;
    other.pTop                           = nullptr;

    return *this;
}

ScriptVMStack::~ScriptVMStack()
{
    const size_t localStackSize = GetStackSize();
    for (uintptr_t i = 0; i < localStackSize; ++i) {
        localStack[i].~ScriptVariable();
    }

    uint8_t *const data = (uint8_t *)localStack;
    if (data) {
        gi.Free(data);
    }
}

size_t ScriptVMStack::GetStackSize() const
{
    return stackBottom - localStack;
}

ScriptVariable& ScriptVMStack::SetTop(ScriptVariable& newTop)
{
    return *(pTop = &newTop);
}

ScriptVariable& ScriptVMStack::GetTop() const
{
    return *pTop;
}

ScriptVariable& ScriptVMStack::GetTop(size_t offset) const
{
    return *(pTop + offset);
}

ScriptVariable *ScriptVMStack::GetTopPtr() const
{
    return pTop;
}

ScriptVariable *ScriptVMStack::GetTopPtr(size_t offset) const
{
    return pTop + offset;
}

ScriptVariable *ScriptVMStack::GetTopArray(size_t offset) const
{
    return pTop + offset;
}

uintptr_t ScriptVMStack::GetIndex() const
{
    return pTop - localStack;
}

ScriptVariable& ScriptVMStack::Pop()
{
    return *(pTop--);
}

ScriptVariable& ScriptVMStack::Pop(size_t offset)
{
    ScriptVariable& old = *pTop;
    pTop -= offset;
    return old;
}

ScriptVariable& ScriptVMStack::PopAndGet()
{
    return *--pTop;
}

ScriptVariable& ScriptVMStack::PopAndGet(size_t offset)
{
    pTop -= offset;
    return *pTop;
}

ScriptVariable& ScriptVMStack::Push()
{
    return *(pTop++);
}

ScriptVariable& ScriptVMStack::Push(size_t offset)
{
    ScriptVariable& old = *pTop;
    pTop += offset;
    return old;
}

ScriptVariable& ScriptVMStack::PushAndGet()
{
    return *++pTop;
}

ScriptVariable& ScriptVMStack::PushAndGet(size_t offset)
{
    pTop += offset;
    return *pTop;
}

void ScriptVMStack::MoveTop(ScriptVariable&& other)
{
    *pTop = std::move(other);
}

//====================
// ScriptVM
//====================

MEM_BlockAlloc<ScriptVM> ScriptVM_allocator;

/*
====================
new ScriptVM
====================
*/
void *ScriptVM::operator new(size_t size)
{
    return ScriptVM_allocator.Alloc();
}

/*
====================
delete ptr
====================
*/
void ScriptVM::operator delete(void *ptr)
{
    ScriptVM_allocator.Free(ptr);
}

/*
====================
ScriptVM
====================
*/
ScriptVM::ScriptVM(ScriptClass *scriptClass, unsigned char *pCodePos, ScriptThread *thread)
    : m_VMStack(scriptClass->GetScript()->GetRequiredStackSize())
{
    next = NULL;

    m_Thread      = thread;
    m_ScriptClass = scriptClass;

    m_Stack = NULL;

    m_PrevCodePos = NULL;
    m_CodePos     = pCodePos;

    state         = STATE_EXECUTION;
    m_ThreadState = THREAD_RUNNING;

    m_pOldData    = NULL;
    m_OldDataSize = 0;

    m_bMarkStack = false;
    m_StackPos   = NULL;

    m_ScriptClass->AddThread(this);

    if (g_scripttrace->integer && CanScriptTracePrint()) {
        gi.DPrintf2("+++THREAD: %p %p\n", this, m_ScriptClass);
    }
}

/*
====================
~ScriptVM
====================
*/
ScriptVM::~ScriptVM()
{
    fastEvent.data     = m_pOldData;
    fastEvent.dataSize = m_OldDataSize;

    // clean-up the call stack
    while (callStack.NumObjects()) {
        LeaveFunction();
    }
}

/*
====================
Archive
====================
*/
void ScriptVM::Archive(Archiver& arc)
{
    int stack = 0;

    if (arc.Saving()) {
        if (m_Stack) {
            stack = m_Stack->m_Count;
        }

        arc.ArchiveInteger(&stack);
    } else {
        arc.ArchiveInteger(&stack);

        if (stack) {
            m_Stack          = new ScriptStack;
            m_Stack->m_Array = new ScriptVariable[stack];
            m_Stack->m_Count = stack;
        }
    }

    for (int i = 1; i <= stack; i++) {
        m_Stack->m_Array[i].ArchiveInternal(arc);
    }

    m_ReturnValue.ArchiveInternal(arc);
    m_ScriptClass->ArchiveCodePos(arc, &m_PrevCodePos);
    m_ScriptClass->ArchiveCodePos(arc, &m_CodePos);
    arc.ArchiveByte(&state);
    arc.ArchiveByte(&m_ThreadState);
}

/*
====================
error

Triggers an error
====================
*/
void ScriptVM::error(const char *format, ...)
{
    char    buffer[4000];
    va_list va;

    va_start(va, format);
    vsprintf(buffer, format, va);
    va_end(va);

    glbs.Printf("----------------------------------------------------------\n%s\n", buffer);
    m_ReturnValue.setStringValue("$.INTERRUPTED");
}

void ScriptVM::jump(unsigned int offset)
{
    m_CodePos += offset;
}

void ScriptVM::jumpBack(unsigned int offset)
{
    m_CodePos -= offset;
}

void ScriptVM::jumpBool(unsigned int offset, bool booleanValue)
{
    if (booleanValue) {
        jump(offset);
    }
}

bool ScriptVM::jumpVar(unsigned int offset, bool booleanValue)
{
    if (booleanValue) {
        jump(offset);
        return true;
    } else {
        m_VMStack.Pop();
        return false;
    }
}

void ScriptVM::doJumpIf(bool booleanValue)
{
    const unsigned int offset = fetchOpcodeValue<unsigned int>();
    jumpBool(offset, booleanValue);
}

bool ScriptVM::doJumpVarIf(bool booleanValue)
{
    const unsigned int offset = fetchOpcodeValue<unsigned int>();
    return jumpVar(offset, booleanValue);
}

void ScriptVM::loadTopInternal(Listener *listener)
{
    const const_str variable = fetchOpcodeValue<op_name_t>();

    if (!executeSetter(listener, variable)) {
        // just set the variable
        ScriptVariable& pTop = m_VMStack.GetTop();
        listener->Vars()->SetVariable(variable, std::move(pTop));
    }
}

ScriptVariable *ScriptVM::storeTopInternal(Listener *listener)
{
    const const_str variable = fetchOpcodeValue<op_name_t>();
    ScriptVariable *listenerVar;

    if (!executeGetter(listener, variable)) {
        ScriptVariable& pTop = m_VMStack.GetTop();
        listenerVar          = listener->Vars()->GetOrCreateVariable(variable);

        pTop = *listenerVar;
    } else {
        listenerVar = nullptr;
    }

    return listenerVar;
}

void ScriptVM::loadStoreTop(Listener *listener)
{
    const const_str variable = fetchOpcodeValue<op_name_t>();

    if (!executeSetter(listener, variable)) {
        // just set the variable
        ScriptVariable& pTop = m_VMStack.GetTop();
        listener->Vars()->SetVariable(variable, pTop);
    }
}

void ScriptVM::skipField()
{
    m_CodePos += sizeof(unsigned int);
}

template<>
void ScriptVM::loadTop<false>(Listener *listener)
{
    loadTopInternal(listener);
    m_VMStack.Pop();
}

template<>
void ScriptVM::loadTop<true>(Listener *listener)
{
    loadTopInternal(listener);
}

template<>
ScriptVariable *ScriptVM::storeTop<false>(Listener *listener)
{
    m_VMStack.Push();

    return storeTopInternal(listener);
}

template<>
ScriptVariable *ScriptVM::storeTop<true>(Listener *listener)
{
    return storeTopInternal(listener);
}

template<>
void ScriptVM::executeCommandInternal<false>(
    Event& ev, Listener *listener, ScriptVariable *fromVar, op_parmNum_t iParamCount
)
{
    transferVarsToEvent(ev, fromVar, iParamCount);
    listener->ProcessScriptEvent(ev);
}

template<>
void ScriptVM::executeCommandInternal<true>(
    Event& ev, Listener *listener, ScriptVariable *fromVar, op_parmNum_t iParamCount
)
{
    transferVarsToEvent(ev, fromVar, iParamCount);

    try {
        listener->ProcessScriptEvent(ev);
    } catch (...) {
        m_VMStack.GetTop().Clear();
        throw;
    }

    ScriptVariable& pTop = m_VMStack.GetTop();
    pTop                 = std::move(ev.GetValue());
}

template<>
void ScriptVM::executeCommand<false, false>(Listener *listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
    ScriptCommandEvent ev = iParamCount ? ScriptCommandEvent(eventnum, iParamCount) : ScriptCommandEvent(eventnum);
    return executeCommandInternal<false>(ev, listener, m_VMStack.GetTopArray(1), iParamCount);
}

template<>
void ScriptVM::executeCommand<true, false>(Listener *listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
    ScriptCommandEvent ev = iParamCount ? ScriptCommandEvent(eventnum, iParamCount) : ScriptCommandEvent(eventnum);
    return executeCommandInternal<false>(ev, listener, m_VMStack.GetTopArray(1), iParamCount);
}

template<>
void ScriptVM::executeCommand<false, true>(Listener *listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
    ScriptCommandEvent ev = iParamCount ? ScriptCommandEvent(eventnum, iParamCount) : ScriptCommandEvent(eventnum);
    return executeCommandInternal<true>(ev, listener, m_VMStack.GetTopArray(), iParamCount);
}

template<>
void ScriptVM::executeCommand<true, true>(Listener *listener, op_parmNum_t iParamCount, op_evName_t eventnum)
{
    ScriptCommandEvent ev = iParamCount ? ScriptCommandEvent(eventnum, iParamCount) : ScriptCommandEvent(eventnum);
    return executeCommandInternal<true>(ev, listener, m_VMStack.GetTopArray(), iParamCount);
}

void ScriptVM::transferVarsToEvent(Event& ev, ScriptVariable *fromVar, op_parmNum_t count)
{
    ev.CopyValues(fromVar, count);
}

bool ScriptVM::executeGetter(Listener *listener, op_evName_t eventName)
{
    int eventNum = Event::FindGetterEventNum(eventName);

    if (eventNum && listener->classinfo()->GetDef(eventNum)) {
        ScriptCommandEvent ev(eventNum);

        listener->ProcessScriptEvent(ev);

        ScriptVariable& pTop = m_VMStack.GetTop();
        pTop                 = std::move(ev.GetValue());

        return true;
    } else {
        eventNum = Event::FindSetterEventNum(eventName);
        assert(!eventNum || !listener->classinfo()->GetDef(eventNum));
        if (eventNum && listener->classinfo()->GetDef(eventNum)) {
            ScriptError("Cannot set a read-only variable");
        }
    }

    return false;
}

bool ScriptVM::executeSetter(Listener *listener, op_evName_t eventName)
{
    int eventNum = Event::FindSetterEventNum(eventName);

    if (eventNum && listener->classinfo()->GetDef(eventNum)) {
        ScriptCommandEvent ev(eventNum, 1);

        ScriptVariable& pTop = m_VMStack.GetTop();
        ev.CopyValues(&pTop, 1);

        listener->ProcessScriptEvent(ev);

        return true;
    } else {
        eventNum = Event::FindSetterEventNum(eventName);
        assert(!eventNum || !listener->classinfo()->GetDef(eventNum));
        if (eventNum && listener->classinfo()->GetDef(eventNum)) {
            ScriptError("Cannot get a write-only variable");
        }
    }

    return false;
}

void ScriptVM::execCmdCommon(op_parmNum_t param)
{
    const op_ev_t eventNum = fetchOpcodeValue<op_ev_t>();

    m_VMStack.Pop(param);

    executeCommand(m_Thread, param, eventNum);
}

void ScriptVM::execCmdMethodCommon(op_parmNum_t param)
{
    const ScriptVariable& a        = m_VMStack.Pop();
    const op_ev_t         eventNum = fetchOpcodeValue<op_ev_t>();

    m_VMStack.Pop(param);

    const size_t arraysize = a.arraysize();
    if (arraysize == (size_t)-1) {
        throw ScriptException("command '%s' applied to NIL", Event::GetEventName(eventNum));
    }

    if (arraysize > 1) {
        if (a.IsConstArray()) {
            // copy the variable
            // because if it's a targetlist, the container object can be modified
            // while iterating
            ScriptVariable array = a;
            array.CastConstArrayValue();

            for (uintptr_t i = arraysize; i > 0; i--) {
                Listener *const listener = array.listenerAt(i);

                // if the listener is NULL, don't throw an exception
                // it would be unfair if the other listeners executed the command
                if (listener) {
                    executeCommand<true>(listener, param, eventNum);
                }
            }
        } else {
            ScriptVariable array = a;
            // must cast into a const array value
            array.CastConstArrayValue();

            for (uintptr_t i = array.arraysize(); i > 0; i--) {
                Listener *const listener = array[i]->listenerAt(i);
                if (listener) {
                    executeCommand<true>(listener, param, eventNum);
                }
            }
        }
    } else {
        // avoid useless allocations of const array
        Listener *const listener = a.listenerValue();
        if (!listener) {
            throw ScriptException("command '%s' applied to NULL listener", Event::GetEventName(eventNum));
        }

        executeCommand<true>(listener, param, eventNum);
    }
}

void ScriptVM::execMethodCommon(op_parmNum_t param)
{
    const ScriptVariable& a        = m_VMStack.Pop();
    const op_ev_t         eventNum = fetchOpcodeValue<op_ev_t>();

    m_VMStack.Pop(param);
    // push the return value
    m_VMStack.Push();

    Listener *const listener = a.listenerValue();
    if (!listener) {
        m_VMStack.GetTop().Clear();
        throw ScriptException("command '%s' applied to NULL listener", Event::GetEventName(eventNum));
    }

    executeCommand<true, true>(listener, param, eventNum);
}

void ScriptVM::execFunction(ScriptMaster& Director)
{
    if (!fetchOpcodeValue<bool>()) {
        const op_name_t    label  = fetchOpcodeValue<op_name_t>();
        const op_parmNum_t params = fetchOpcodeValue<op_parmNum_t>();

        Listener *listener;

        try {
            listener = m_VMStack.GetTop().listenerValue();

            if (!listener) {
                const str& labelName = Director.GetString(label);
                throw ScriptException("function '" + labelName + "' applied to NULL listener");
            }
        } catch (...) {
            m_VMStack.Pop(params);
            throw;
        }

        m_VMStack.Pop();

        Container<ScriptVariable> data;
        data.Resize(params + 1);

        ScriptVariable *labelVar = &data.ObjectAt(data.AddObject(ScriptVariable()));
        labelVar->setConstStringValue(label);

        const ScriptVariable *var = &m_VMStack.Pop(params);

        for (int i = 0; i < params; var++, i++) {
            data.AddObject(*var);
        }

        m_VMStack.Push();
        EnterFunction(std::move(data));

        GetScriptClass()->m_Self = listener;
    } else {
        const op_name_t    filename = fetchOpcodeValue<op_name_t>();
        const op_name_t    label    = fetchOpcodeValue<op_name_t>();
        const op_parmNum_t params   = fetchOpcodeValue<op_parmNum_t>();

        Listener *listener;
        try {
            listener = m_VMStack.GetTop().listenerValue();

            if (!listener) {
                const str& labelStr = Director.GetString(label);
                const str& fileStr  = Director.GetString(filename);
                throw ScriptException("function '" + labelStr + "' in '" + fileStr + "' applied to NULL listener");
            }
        } catch (...) {
            m_VMStack.Pop(params);
            throw;
        }

        m_VMStack.Pop();

        ScriptVariable  constarray;
        ScriptVariable *pVar = new ScriptVariable[2];

        pVar[0].setConstStringValue(filename);
        pVar[1].setConstStringValue(label);

        constarray.setConstArrayValue(pVar, 2);

        delete[] pVar;

        Event ev(EV_Listener_WaitCreateReturnThread);

        const ScriptVariable *var = &m_VMStack.Pop(params);

        for (int i = 0; i < params; var++, i++) {
            ev.AddValue(*var);
        }

        m_VMStack.Push();
        m_VMStack.GetTop() = listener->ProcessEventReturn(&ev);
    }
}

/*
====================
ProgBuffer

Returns the current program buffer
====================
*/
unsigned char *ScriptVM::ProgBuffer(void)
{
    return m_CodePos;
}

/*
====================
ScriptTrace1
====================
*/
void ScriptVM::ScriptTrace1() const
{
    GameScript *scr;
    str         sourceLine;
    int         column;
    int         line;

    scr = GetScript();

    if (!scr->GetSourceAt(m_CodePos, &sourceLine, column, line)) {
        return;
    }

    if (currentScriptFile == this && line == currentScriptLine) {
        return;
    }

    currentScriptFile = this;
    currentScriptLine = line;

    gi.DPrintf2("%s (%s, %d, %p, %d)\n", sourceLine.c_str(), Filename().c_str(), line, this, m_VMStack.GetIndex());
}

/*
====================
ScriptTrace2
====================
*/
void ScriptVM::ScriptTrace2() const
{
    gi.DPrintf2("%s, %p, %d\n", OpcodeName(*m_CodePos), this, m_VMStack.GetIndex());
    GetScript()->PrintSourcePos(m_CodePos, true);
}

/*
====================
GetSourcePos
====================
*/
const char *ScriptVM::GetSourcePos() const
{
    static str debugLine;
    str        sourceLine;
    int        column;
    int        line;

    debugLine =
        str(OpcodeName(*m_CodePos)) + " @" + str(m_CodePos - GetScript()->m_ProgBuffer) + " in " + Filename() + "\n";

    if (GetScript()->GetSourceAt(m_CodePos, &sourceLine, column, line)) {
        debugLine += str(" (") + str(line) + str(")\n");
    }

    return debugLine.c_str();
}

/*
====================
EnterFunction

Sets a new instruction pointer
====================
*/
void ScriptVM::EnterFunction(Container<ScriptVariable>&&)
{
#if 0
	ScriptCallStack* stack;
	str label = ev->GetString(1);

	SetFastData(ev->data + 1, ev->dataSize - 1);

	unsigned char* codePos = m_ScriptClass->FindLabel(label);

	if (!codePos)
	{
		ScriptError("ScriptVM::EnterFunction: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str());
	}

	stack = new ScriptCallStack;

	stack->codePos = m_CodePos;

	stack->pTop = &m_VMStack.GetTop();
	stack->returnValue = m_ReturnValue;
	stack->localStack = localStack;
	stack->m_Self = m_ScriptClass->GetSelf();

	callStack.AddObject(stack);

	m_CodePos = codePos;

	localStack = new ScriptVariable[localStackSize];

	pTop = localStack;
	m_ReturnValue.Clear();
#endif
}

/*
====================
LeaveFunction

Returns to the previous function
====================
*/
void ScriptVM::LeaveFunction()
{
#if 0
	int num = callStack.NumObjects();

	if (num)
	{
		ScriptCallStack* stack = callStack.ObjectAt(num);

		pTop = stack->pTop;
		*pTop = m_ReturnValue;

		m_CodePos = stack->codePos;
		m_ReturnValue = stack->returnValue;
		m_ScriptClass->m_Self = stack->m_Self;

		delete[] localStack;

		localStack = stack->localStack;

		delete stack;

		callStack.RemoveObjectAt(num);
	}
	else
	{
		delete m_Thread;
	}
#else
    delete m_Thread;
#endif
}

/*
====================
End

End with a return value
====================
*/
void ScriptVM::End(const ScriptVariable& returnValue)
{
    m_ReturnValue.setPointer(returnValue);

    LeaveFunction();
}

/*
====================
End
====================
*/
void ScriptVM::End()
{
    m_ReturnValue.ClearPointer();

    LeaveFunction();
}

/*
====================
Execute

Executes a program
====================
*/
void ScriptVM::Execute(ScriptVariable *data, int dataSize, str label)
{
    unsigned char *opcode;

    ScriptVariable *a;
    ScriptVariable *b;
    ScriptVariable *c;

    op_name_t fieldNameIndex;

    Listener *listener;

    Event           ev;
    ScriptVariable *var = NULL;

    ConSimple *targetList;

    if (Director.stackCount >= MAX_STACK_DEPTH) {
        state = STATE_EXECUTION;

        ScriptException::next_abort = -1;
        throw ScriptException("stack overflow");
    }

    if (label.length()) {
        // Throw if label is not found
        m_CodePos = m_ScriptClass->FindLabel(label);
        if (!m_CodePos) {
            ScriptError("ScriptVM::Execute: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str());
        }
    }

    if (g_scripttrace->integer && CanScriptTracePrint()) {
        gi.DPrintf2(
            "+++FRAME: %i (%p) +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n",
            Director.stackCount,
            this
        );
    }

    Director.stackCount++;

    if (dataSize) {
        SetFastData(data, dataSize);
    }

    state = STATE_RUNNING;

    Director.cmdTime  = glbs.Milliseconds();
    Director.cmdCount = 0;

    while (state == STATE_RUNNING) {
        if (g_scripttrace->integer && CanScriptTracePrint()) {
            switch (g_scripttrace->integer) {
            case 1:
            case 3:
                ScriptTrace1();
                break;
            case 2:
            case 4:
                ScriptTrace2();
                break;
            }
        }

        m_PrevCodePos = m_CodePos;

        Director.cmdCount++;

        try {
            if (Director.cmdCount > 9999 && glbs.Milliseconds() - Director.cmdTime > Director.maxTime) {
                if (level.m_LoopProtection) {
                    Director.cmdTime = glbs.Milliseconds();

                    GetScript()->PrintSourcePos(m_CodePos, true);
                    gi.DPrintf2("\n");

                    state = STATE_EXECUTION;

                    if (level.m_LoopDrop) {
                        ScriptException::next_abort = -1;
                    }

                    ScriptError("Command overflow. Possible infinite loop in thread.\n");
                }

                VM_DPrintf("Update of script position - This is not an error.\n");
                VM_DPrintf("=================================================\n");
                m_ScriptClass->GetScript()->PrintSourcePos(opcode, true);
                VM_DPrintf("=================================================\n");

                Director.cmdCount = 0;
            }

            if (!m_bMarkStack) {
                /*
				assert(pTop >= localStack && pTop < localStack + localStackSize);
				if (pTop < localStack)
				{
					deleteThread = true;
					state = STATE_EXECUTION;

					error("VM stack error. Negative stack value %d.\n", pTop - localStack);
					break;
				}
				else if (pTop >= localStack + localStackSize)
				{
					deleteThread = true;
					state = STATE_EXECUTION;

					error("VM stack error. Exceeded the maximum stack size %d.\n", localStackSize);
					break;
				}
				*/
            }

            opcode = m_CodePos++;
            switch (*opcode) {
            case OP_BIN_BITWISE_AND:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b &= *a;
                break;

            case OP_BIN_BITWISE_OR:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b |= *a;
                break;

            case OP_BIN_BITWISE_EXCL_OR:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b ^= *a;
                break;

            case OP_BIN_EQUALITY:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                b->setIntValue(*b == *a);
                break;

            case OP_BIN_INEQUALITY:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                b->setIntValue(*b != *a);
                break;

            case OP_BIN_GREATER_THAN:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                b->greaterthan(*a);
                break;

            case OP_BIN_GREATER_THAN_OR_EQUAL:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                b->greaterthanorequal(*a);
                break;

            case OP_BIN_LESS_THAN:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                b->lessthan(*a);
                break;

            case OP_BIN_LESS_THAN_OR_EQUAL:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                b->lessthanorequal(*a);
                break;

            case OP_BIN_PLUS:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b += *a;
                break;

            case OP_BIN_MINUS:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b -= *a;
                break;

            case OP_BIN_MULTIPLY:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b *= *a;
                break;

            case OP_BIN_DIVIDE:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b /= *a;
                break;

            case OP_BIN_PERCENTAGE:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b %= *a;
                break;

            case OP_BIN_SHIFT_LEFT:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b <<= *a;
                break;

            case OP_BIN_SHIFT_RIGHT:
                a = &m_VMStack.Pop();
                b = &m_VMStack.GetTop();

                *b >>= *a;
                break;

            case OP_BOOL_JUMP_FALSE4:
                doJumpIf(!m_VMStack.Pop().m_data.intValue);
                break;

            case OP_BOOL_JUMP_TRUE4:
                doJumpIf(m_VMStack.Pop().m_data.intValue);
                break;

            case OP_VAR_JUMP_FALSE4:
                doJumpIf(!m_VMStack.Pop().booleanValue());
                break;

            case OP_VAR_JUMP_TRUE4:
                doJumpIf(m_VMStack.Pop().booleanValue());
                break;

            case OP_BOOL_LOGICAL_AND:
                doJumpVarIf(!m_VMStack.GetTop().m_data.intValue);
                break;

            case OP_BOOL_LOGICAL_OR:
                doJumpVarIf(m_VMStack.GetTop().m_data.intValue);
                break;

            case OP_VAR_LOGICAL_AND:
                if (!doJumpVarIf(m_VMStack.GetTop().booleanValue())) {
                    m_VMStack.GetTop().SetFalse();
                }
                break;

            case OP_VAR_LOGICAL_OR:
                if (!doJumpVarIf(!m_VMStack.GetTop().booleanValue())) {
                    m_VMStack.GetTop().SetTrue();
                }
                break;

            case OP_BOOL_STORE_FALSE:
                m_VMStack.PushAndGet().SetFalse();
                break;

            case OP_BOOL_STORE_TRUE:
                m_VMStack.PushAndGet().SetTrue();
                break;

            case OP_BOOL_UN_NOT:
                m_VMStack.GetTop().m_data.intValue = (m_VMStack.GetTop().m_data.intValue == 0);
                break;

            case OP_CALC_VECTOR:
                c = &m_VMStack.Pop();
                b = &m_VMStack.Pop();
                a = &m_VMStack.GetTop();

                m_VMStack.GetTop().setVectorValue(Vector(a->floatValue(), b->floatValue(), c->floatValue()));
                break;

            case OP_EXEC_CMD0:
                {
                    execCmdCommon(0);
                    break;
                }

            case OP_EXEC_CMD1:
                {
                    execCmdCommon(1);
                    break;
                }

            case OP_EXEC_CMD2:
                {
                    execCmdCommon(2);
                    break;
                }

            case OP_EXEC_CMD3:
                {
                    execCmdCommon(3);
                    break;
                }

            case OP_EXEC_CMD4:
                {
                    execCmdCommon(4);
                    break;
                }

            case OP_EXEC_CMD5:
                {
                    execCmdCommon(5);
                    break;
                }

            case OP_EXEC_CMD_COUNT1:
                {
                    const op_parmNum_t numParms = fetchOpcodeValue<op_parmNum_t>();
                    execCmdCommon(numParms);
                    break;
                }

            case OP_EXEC_CMD_METHOD0:
                {
                    execCmdMethodCommon(0);
                    break;
                }

            case OP_EXEC_CMD_METHOD1:
                {
                    execCmdMethodCommon(1);
                    break;
                }

            case OP_EXEC_CMD_METHOD2:
                {
                    execCmdMethodCommon(2);
                    break;
                }

            case OP_EXEC_CMD_METHOD3:
                {
                    execCmdMethodCommon(3);
                    break;
                }

            case OP_EXEC_CMD_METHOD4:
                {
                    execCmdMethodCommon(4);
                    break;
                }

            case OP_EXEC_CMD_METHOD5:
                {
                    execCmdMethodCommon(5);
                    break;
                }

            case OP_EXEC_CMD_METHOD_COUNT1:
                {
                    const op_parmNum_t numParms = fetchOpcodeValue<op_parmNum_t>();
                    execCmdMethodCommon(numParms);
                    break;
                }

            case OP_EXEC_METHOD0:
                {
                    execMethodCommon(0);
                    break;
                }

            case OP_EXEC_METHOD1:
                {
                    execMethodCommon(1);
                    break;
                }

            case OP_EXEC_METHOD2:
                {
                    execMethodCommon(2);
                    break;
                }

            case OP_EXEC_METHOD3:
                {
                    execMethodCommon(3);
                    break;
                }

            case OP_EXEC_METHOD4:
                {
                    execMethodCommon(4);
                    break;
                }

            case OP_EXEC_METHOD5:
                {
                    execMethodCommon(5);
                    break;
                }

            case OP_EXEC_METHOD_COUNT1:
                {
                    const op_parmNum_t numParms = fetchOpcodeValue<op_parmNum_t>();
                    execMethodCommon(numParms);
                    break;
                }

            case OP_FUNC:
                {
                    execFunction(Director);
                    break;
                }

            case OP_JUMP4:
                jump(fetchOpcodeValue<unsigned int>());
                break;

            case OP_JUMP_BACK4:
                jumpBack(fetchActualOpcodeValue<unsigned int>());
                break;

            case OP_LOAD_ARRAY_VAR:
                a = &m_VMStack.Pop();
                b = &m_VMStack.Pop();
                c = &m_VMStack.Pop();

                b->setArrayAt(*a, *c);
                break;

            case OP_LOAD_FIELD_VAR:
                a = &m_VMStack.Pop();

                try {
                    try {
                        listener = a->listenerValue();

                        if (listener == NULL) {
                            fieldNameIndex = fetchActualOpcodeValue<op_name_t>();
                            ScriptError(
                                "Field '%s' applied to NULL listener", Director.GetString(fieldNameIndex).c_str()
                            );
                        }
                    } catch (...) {
                        skipField();
                        throw;
                    }

                    loadTop(listener);
                } catch (...) {
                    m_VMStack.Pop();
                    throw;
                }

                break;

            case OP_LOAD_CONST_ARRAY1:
                {
                    op_arrayParmNum_t numParms = fetchOpcodeValue<op_arrayParmNum_t>();

                    ScriptVariable& pTop = m_VMStack.PopAndGet(numParms - 1);
                    pTop.setConstArrayValue(&pTop, numParms);
                    break;
                }

            case OP_LOAD_GAME_VAR:
                loadTop(&game);
                break;

            case OP_LOAD_GROUP_VAR:
                loadTop(m_ScriptClass);
                break;

            case OP_LOAD_LEVEL_VAR:
                loadTop(&level);
                break;

            case OP_LOAD_LOCAL_VAR:
                loadTop(m_Thread);
                break;

            case OP_LOAD_OWNER_VAR:
                if (!m_ScriptClass->m_Self) {
                    m_VMStack.Pop();
                    m_CodePos += sizeof(unsigned int);
                    ScriptError("self is NULL");
                }

                if (!m_ScriptClass->m_Self->GetScriptOwner()) {
                    m_VMStack.Pop();
                    m_CodePos += sizeof(unsigned int);
                    ScriptError("self.owner is NULL");
                }

                loadTop(m_ScriptClass->m_Self->GetScriptOwner());
                break;

            case OP_LOAD_PARM_VAR:
                loadTop(&parm);
                break;

            case OP_LOAD_SELF_VAR:
                if (!m_ScriptClass->m_Self) {
                    m_VMStack.Pop();
                    m_CodePos += sizeof(unsigned int);
                    ScriptError("self is NULL");
                }

                loadTop(m_ScriptClass->m_Self);
                break;

            case OP_LOAD_STORE_GAME_VAR:
                loadStoreTop(&game);
                break;

            case OP_LOAD_STORE_GROUP_VAR:
                loadStoreTop(m_ScriptClass);
                break;

            case OP_LOAD_STORE_LEVEL_VAR:
                loadStoreTop(&level);
                break;

            case OP_LOAD_STORE_LOCAL_VAR:
                loadStoreTop(m_Thread);
                break;

            case OP_LOAD_STORE_OWNER_VAR:
                if (!m_ScriptClass->m_Self) {
                    m_CodePos += sizeof(unsigned int);
                    ScriptError("self is NULL");
                }

                if (!m_ScriptClass->m_Self->GetScriptOwner()) {
                    m_CodePos += sizeof(unsigned int);
                    ScriptError("self.owner is NULL");
                }

                loadStoreTop(m_ScriptClass->m_Self->GetScriptOwner());
                break;

            case OP_LOAD_STORE_PARM_VAR:
                loadStoreTop(&parm);
                break;

            case OP_LOAD_STORE_SELF_VAR:
                if (!m_ScriptClass->m_Self) {
                    ScriptError("self is NULL");
                }

                loadStoreTop(m_ScriptClass->m_Self);
                break;

            case OP_MARK_STACK_POS:
                m_StackPos   = &m_VMStack.GetTop();
                m_bMarkStack = true;
                break;

            case OP_STORE_PARAM:
                if (fastEvent.dataSize) {
                    m_VMStack.SetTop(*(fastEvent.data++));
                    fastEvent.dataSize--;
                } else {
                    m_VMStack.SetTop(*(m_StackPos + 1));
                    m_VMStack.GetTop().Clear();
                }
                break;

            case OP_RESTORE_STACK_POS:
                m_VMStack.SetTop(*m_StackPos);
                m_bMarkStack = false;
                break;

            case OP_STORE_ARRAY:
                m_VMStack.Pop();
                m_VMStack.GetTop().evalArrayAt(*(m_VMStack.GetTopPtr() + 1));
                break;

            case OP_STORE_ARRAY_REF:
                m_VMStack.Pop();
                m_VMStack.GetTop().setArrayRefValue(*(m_VMStack.GetTopPtr() + 1));
                break;

            case OP_STORE_FIELD_REF:
                try {
                    try {
                        listener = m_VMStack.GetTop().listenerValue();

                        if (listener == nullptr) {
                            fieldNameIndex = fetchActualOpcodeValue<op_name_t>();
                            ScriptError(
                                "Field '%s' applied to NULL listener", Director.GetString(fieldNameIndex).c_str()
                            );
                        }
                    } catch (...) {
                        skipField();
                        throw;
                    }

                    ScriptVariable *const listenerVar = storeTop<true>(listener);

                    if (listenerVar) {
                        // having a listener variable means the variable was just created
                        m_VMStack.GetTop().setRefValue(listenerVar);
                    }
                    break;
                } catch (...) {
                    ScriptVariable *const pTop = m_VMStack.GetTopPtr();
                    pTop->setRefValue(pTop);
                    throw;
                }

            case OP_STORE_FIELD:
                try {
                    listener = m_VMStack.GetTop().listenerValue();

                    if (listener == nullptr) {
                        fieldNameIndex = fetchActualOpcodeValue<op_name_t>();
                        ScriptError("Field '%s' applied to NULL listener", Director.GetString(fieldNameIndex).c_str());
                    }
                } catch (...) {
                    skipField();
                    throw;
                }

                storeTop<true>(listener);
                break;

            case OP_STORE_FLOAT:
                m_VMStack.Push();
                m_VMStack.GetTop().setFloatValue(fetchOpcodeValue<float>());
                break;

            case OP_STORE_INT0:
                m_VMStack.Push();
                m_VMStack.GetTop().setIntValue(0);
                break;

            case OP_STORE_INT1:
                m_VMStack.Push();
                m_VMStack.GetTop().setIntValue(fetchOpcodeValue<byte>());
                break;

            case OP_STORE_INT2:
                m_VMStack.Push();
                m_VMStack.GetTop().setIntValue(fetchOpcodeValue<short>());
                break;

            case OP_STORE_INT3:
                m_VMStack.Push();
                m_VMStack.GetTop().setIntValue(fetchOpcodeValue<short3>());
                break;

            case OP_STORE_INT4:
                m_VMStack.Push();
                m_VMStack.GetTop().setIntValue(fetchOpcodeValue<int>());
                break;

            case OP_STORE_GAME_VAR:
                storeTop(&game);
                break;

            case OP_STORE_GROUP_VAR:
                storeTop(m_ScriptClass);
                break;

            case OP_STORE_LEVEL_VAR:
                storeTop(&level);
                break;

            case OP_STORE_LOCAL_VAR:
                storeTop(m_Thread);
                break;

            case OP_STORE_OWNER_VAR:
                if (!m_ScriptClass->m_Self) {
                    m_VMStack.Push();
                    m_CodePos += sizeof(unsigned int);
                    ScriptError("self is NULL");
                }

                if (!m_ScriptClass->m_Self->GetScriptOwner()) {
                    m_VMStack.Push();
                    m_CodePos += sizeof(unsigned int);
                    ScriptError("self.owner is NULL");
                }

                storeTop(m_ScriptClass->m_Self->GetScriptOwner());
                break;

            case OP_STORE_PARM_VAR:
                storeTop(&parm);
                break;

            case OP_STORE_SELF_VAR:
                if (!m_ScriptClass->m_Self) {
                    m_VMStack.Push();
                    m_CodePos += sizeof(unsigned int);
                    ScriptError("self is NULL");
                }

                storeTop(m_ScriptClass->m_Self);
                break;

            case OP_STORE_GAME:
                m_VMStack.Push();
                m_VMStack.GetTop().setListenerValue(&game);
                break;

            case OP_STORE_GROUP:
                m_VMStack.Push();
                m_VMStack.GetTop().setListenerValue(m_ScriptClass);
                break;

            case OP_STORE_LEVEL:
                m_VMStack.Push();
                m_VMStack.GetTop().setListenerValue(&level);
                break;

            case OP_STORE_LOCAL:
                m_VMStack.Push();
                m_VMStack.GetTop().setListenerValue(m_Thread);
                break;

            case OP_STORE_OWNER:
                m_VMStack.Push();

                if (!m_ScriptClass->m_Self) {
                    m_VMStack.Push();
                    ScriptError("self is NULL");
                }

                m_VMStack.GetTop().setListenerValue(m_ScriptClass->m_Self->GetScriptOwner());
                break;

            case OP_STORE_PARM:
                m_VMStack.Push();
                m_VMStack.GetTop().setListenerValue(&parm);
                break;

            case OP_STORE_SELF:
                m_VMStack.Push();
                m_VMStack.GetTop().setListenerValue(m_ScriptClass->m_Self);
                break;

            case OP_STORE_NIL:
                m_VMStack.Push();
                m_VMStack.GetTop().Clear();
                break;

            case OP_STORE_NULL:
                m_VMStack.Push();
                m_VMStack.GetTop().setListenerValue(NULL);
                break;

            case OP_STORE_STRING:
                m_VMStack.Push();
                m_VMStack.GetTop().setConstStringValue(fetchOpcodeValue<unsigned int>());
                break;

            case OP_STORE_VECTOR:
                m_VMStack.Push();
                m_VMStack.GetTop().setVectorValue(fetchOpcodeValue<Vector>());
                break;

            case OP_SWITCH:
                if (!Switch(fetchActualOpcodeValue<StateScript *>(), m_VMStack.Pop())) {
                    m_CodePos += sizeof(StateScript *);
                }
                break;

            case OP_UN_CAST_BOOLEAN:
                m_VMStack.GetTop().CastBoolean();
                break;

            case OP_UN_COMPLEMENT:
                m_VMStack.GetTop().complement();
                break;

            case OP_UN_MINUS:
                m_VMStack.GetTop().minus();
                break;

            case OP_UN_DEC:
                m_VMStack.GetTop()--;
                break;

            case OP_UN_INC:
                m_VMStack.GetTop()++;
                break;

            case OP_UN_SIZE:
                m_VMStack.GetTop().setIntValue((int)m_VMStack.GetTop().size());
                break;

            case OP_UN_TARGETNAME:
                // retrieve the target name
                targetList = world->GetExistingTargetList(m_VMStack.GetTop().constStringValue());

                if (!targetList || !targetList->NumObjects()) {
                    // the target name was not found
                    m_VMStack.GetTop().setListenerValue(NULL);

                    if ((*m_CodePos >= OP_BIN_EQUALITY && *m_CodePos <= OP_BIN_GREATER_THAN_OR_EQUAL)
                        || (*m_CodePos >= OP_BOOL_UN_NOT && *m_CodePos <= OP_UN_CAST_BOOLEAN)) {
                        ScriptError("Targetname '%s' does not exist.", m_VMStack.GetTop().stringValue().c_str());
                    }
                } else if (targetList->NumObjects() == 1) {
                    // single listener
                    m_VMStack.GetTop().setListenerValue(targetList->ObjectAt(1));
                } else if (targetList->NumObjects() > 1) {
                    // multiple listeners
                    m_VMStack.GetTop().setContainerValue((Container<SafePtr<Listener>> *)targetList);
                }
                break;

            case OP_VAR_UN_NOT:
                m_VMStack.GetTop().setIntValue(m_VMStack.GetTop().booleanValue());
                break;

            case OP_DONE:
                End();
                break;

            case OP_NOP:
                break;

            default:
                assert(!"Invalid opcode");
                if (*opcode < OP_MAX) {
                    glbs.DPrintf("unknown opcode %d ('%s')\n", *opcode, OpcodeName(*opcode));
                } else {
                    glbs.DPrintf("unknown opcode %d\n", *opcode);
                }
                break;
            }
        } catch (ScriptException& exc) {
            HandleScriptException(exc);
        }
    }

    Director.stackCount--;

    if (g_scripttrace->integer && CanScriptTracePrint()) {
        gi.DPrintf2(
            "---FRAME: %i (%p) -------------------------------------------------------------------\n",
            Director.stackCount,
            this
        );
    }

    switch (state) {
    case STATE_WAITING:
        delete m_Thread;
        delete this;
        break;
    case STATE_SUSPENDED:
        state = STATE_EXECUTION;
        break;
    case STATE_DESTROYED:
        delete this;
        break;
    }
}

/*
====================
HandleScriptException
====================
*/
void ScriptVM::HandleScriptException(ScriptException& exc)
{
    if (m_ScriptClass) {
        m_ScriptClass->GetScript()->PrintSourcePos(m_PrevCodePos, true);
    } else {
        glbs.DPrintf("unknown source pos");
    }

    if (exc.bAbort) {
        ScriptException e(exc.string);

        e.bAbort     = exc.bAbort;
        e.bIsForAnim = exc.bIsForAnim;

        state = STATE_EXECUTION;
        throw e;
    }

    gi.DPrintf2("^~^~^ Script Error : %s\n\n", exc.string.c_str());

    if (m_ScriptClass->GetScript()->ScriptCheck()) {
        if (g_scriptcheck->integer != 2 || !exc.bIsForAnim) {
            ScriptException e("Script check failed");

            e.bAbort     = exc.bAbort;
            e.bIsForAnim = exc.bIsForAnim;

            state = STATE_EXECUTION;
            throw e;
        }
    }
    Director.cmdCount += 100;
}

/*
====================
SetFastData

Sets the starting virtual machine parameters
====================
*/
void ScriptVM::SetFastData(ScriptVariable *data, int dataSize)
{
    if (fastEvent.data) {
        fastEvent.data     = m_pOldData;
        fastEvent.dataSize = m_OldDataSize;

        fastEvent.Clear();

        m_pOldData    = NULL;
        m_OldDataSize = 0;
    }

    if (dataSize) {
        fastEvent.data     = new ScriptVariable[dataSize];
        fastEvent.dataSize = dataSize;

        for (int i = 0; i < dataSize; i++) {
            fastEvent.data[i] = std::move(data[i]);
        }

        m_pOldData    = fastEvent.data;
        m_OldDataSize = fastEvent.dataSize;
    }
}

/*
====================
NotifyDelete
====================
*/
void ScriptVM::NotifyDelete(void)
{
    if (g_scripttrace->integer && CanScriptTracePrint()) {
        gi.DPrintf2("---THREAD: %p\n", this);
    }

    switch (state) {
    case STATE_RUNNING:
    case STATE_SUSPENDED:
    case STATE_WAITING:
        state = STATE_DESTROYED;

        if (m_ScriptClass) {
            m_ScriptClass->RemoveThread(this);
        }

        break;

    case STATE_EXECUTION:
        state = STATE_DESTROYED;

        if (m_ScriptClass) {
            m_ScriptClass->RemoveThread(this);
        }

        delete this;

        break;

    case STATE_DESTROYED:
        ScriptError("Attempting to delete a dead thread.");

        state = STATE_DESTROYED;

        if (m_ScriptClass) {
            m_ScriptClass->RemoveThread(this);
        }
        break;
    }
}

/*
====================
Resume
====================
*/
void ScriptVM::Resume(qboolean bForce)
{
    if (state == STATE_SUSPENDED || (bForce && state != STATE_DESTROYED)) {
        state = STATE_RUNNING;
    }
}

/*
====================
Suspend
====================
*/
void ScriptVM::Suspend()
{
    if (state == STATE_DESTROYED) {
        ScriptError("Cannot suspend a dead thread.");
    } else if (state == STATE_RUNNING) {
        state = STATE_SUSPENDED;
    }
}

/*
====================
Switch

Switch statement
====================
*/
bool ScriptVM::Switch(StateScript *stateScript, ScriptVariable& var)
{
    unsigned char *pos;

    fastEvent.dataSize = 0;

    pos = stateScript->FindLabel(var.stringValue());

    if (!pos) {
        pos = stateScript->FindLabel(STRING_DEFAULT);

        if (!pos) {
            return false;
        }
    }

    m_CodePos = pos;

    return true;
}

/*
====================
Filename
====================
*/
str ScriptVM::Filename(void) const
{
    return m_ScriptClass->Filename();
}

/*
====================
Label
====================
*/
str ScriptVM::Label(void) const
{
    const_str label = m_ScriptClass->NearestLabel(m_CodePos);

    if (!label) {
        return "";
    }

    return Director.GetString(label);
}

/*
====================
GetScriptClass
====================
*/
ScriptClass *ScriptVM::GetScriptClass(void) const
{
    return m_ScriptClass;
}

/*
====================
GetScript
====================
*/
GameScript *ScriptVM::GetScript() const
{
    return m_ScriptClass->GetScript();
}

/*
====================
IsSuspended
====================
*/
bool ScriptVM::IsSuspended(void)
{
    return state == STATE_EXECUTION;
}

/*
====================
State
====================
*/
int ScriptVM::State(void)
{
    return state;
}

/*
====================
ThreadState
====================
*/
int ScriptVM::ThreadState(void)
{
    return m_ThreadState;
}

/*
====================
EventGoto
====================
*/
void ScriptVM::EventGoto(Event *ev)
{
    unsigned char        *codePos;
    const ScriptVariable& value = ev->GetValue(1);
    if (value.type == VARIABLE_CONSTSTRING) {
        const_str label = value.constStringValue();
        codePos         = m_ScriptClass->FindLabel(label);

        if (!codePos) {
            ScriptError(
                "ScriptVM::EventGoto: label '%s' does not exist in '%s'.",
                value.stringValue().c_str(),
                Filename().c_str()
            );
        }

    } else {
        str label = value.stringValue();
        codePos   = m_ScriptClass->FindLabel(label);

        if (!codePos) {
            ScriptError("ScriptVM::EventGoto: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str());
        }
    }

    SetFastData(ev->data + 1, ev->dataSize - 1);

    m_CodePos = codePos;
}

/*
====================
EventThrow

Called when throwing an exception
====================
*/
bool ScriptVM::EventThrow(Event *ev)
{
    str label = ev->GetString(1);

    SetFastData(ev->data, ev->dataSize);

    fastEvent.eventnum = ev->eventnum;

    while (1) {
        StateScript *stateScript = m_ScriptClass->GetCatchStateScript(m_PrevCodePos, m_PrevCodePos);

        if (!stateScript) {
            break;
        }

        m_CodePos = stateScript->FindLabel(label);

        if (m_CodePos) {
            fastEvent.data++;
            fastEvent.dataSize--;

            return true;
        }
    }

    return false;
}

bool ScriptVM::CanScriptTracePrint(void)
{
    if (g_scripttrace->integer < 1 || g_scripttrace->integer > 4) {
        return false;
    }
    if (g_scripttrace->integer <= 2) {
        return true;
    }
    if (!m_ScriptClass) {
        return false;
    }
    if (!*g_monitor->string || !m_ScriptClass->m_Self || !m_ScriptClass->m_Self->isInheritedBy(&SimpleEntity::ClassInfo)
        || ((SimpleEntity *)m_ScriptClass->m_Self.Pointer())->targetname != g_monitor->string) {
        if (g_monitorNum->integer >= 0) {
            if (m_ScriptClass->m_Self && m_ScriptClass->m_Self->isInheritedBy(&Entity::ClassInfo)
                && ((Entity *)m_ScriptClass->m_Self.Pointer())->entnum == g_monitorNum->integer) {
                return true;
            }
        }
        return false;
    }
    return true;
}

void ScriptVM::fetchOpcodeValue(void *outValue, size_t size)
{
    Com_Memcpy(outValue, m_CodePos, size);
    m_CodePos += size;
}

void ScriptVM::fetchActualOpcodeValue(void *outValue, size_t size)
{
    Com_Memcpy(outValue, m_CodePos, size);
}
