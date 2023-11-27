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

// scriptvm.h: Script virtual machine interpreter.

#pragma once

#include "listener.h"

#include "../fgame/gamescript.h"

#include "scriptvariable.h"
#include "scriptopcodes.h"
#include "../qcommon/con_set.h"

#define MAX_STACK_DEPTH 20 // 9 in mohaa
//#define	LOCALSTACK_SIZE		255		// pre-allocated localstack size for each VM
#define MAX_SCRIPTCYCLES 9999 // max cmds

enum eVMState {
    STATE_RUNNING,   // Running
    STATE_SUSPENDED, // Suspended
    STATE_WAITING,   // Waiting for something
    STATE_EXECUTION, // Resume to execution
    STATE_DESTROYED  // Pending deletion
};

enum eThreadState {
    THREAD_RUNNING,
    THREAD_WAITING,
    THREAD_SUSPENDED
};

class ScriptException;
class ScriptThread;
class ScriptVM;

class ScriptCallStack
{
public:
    // opcode variable
    unsigned char *codePos; // opcode will be restored once a DONE was hit

    // stack variables
    ScriptVariable *localStack;
    ScriptVariable *pTop;

    // return variable
    ScriptVariable returnValue;

    // OLD self value
    SafePtr<Listener> m_Self;
};

class ScriptStack
{
public:
    ScriptVariable *m_Array;
    int             m_Count;
};

class ScriptVMStack
{
public:
    ScriptVMStack();
    ScriptVMStack(size_t stackSize);
    ~ScriptVMStack();

    ScriptVMStack(const ScriptVMStack& other)            = delete;
    ScriptVMStack& operator=(const ScriptVMStack& other) = delete;
    ScriptVMStack(ScriptVMStack&& other);
    ScriptVMStack& operator=(ScriptVMStack&& other);

    size_t          GetStackSize() const;
    ScriptVariable& SetTop(ScriptVariable& newTop);
    ScriptVariable& GetTop() const;
    ScriptVariable& GetTop(size_t offset) const;
    ScriptVariable *GetTopPtr() const;
    ScriptVariable *GetTopPtr(size_t offset) const;
    ScriptVariable *GetTopArray(size_t offset = 0) const;
    uintptr_t       GetIndex() const;
    void            MoveTop(ScriptVariable&& other);

    /** Pop and return the previous value. */
    ScriptVariable& Pop();
    ScriptVariable& Pop(size_t offset);
    ScriptVariable& PopAndGet();
    ScriptVariable& PopAndGet(size_t offset);
    /** Push and return the previous value. */
    ScriptVariable& Push();
    ScriptVariable& Push(size_t offset);
    ScriptVariable& PushAndGet();
    ScriptVariable& PushAndGet(size_t offset);

    void Archive(Archiver& arc);

private:
    void Allocate(size_t stackSize);
    void Free();

private:
    /** The VM's local stack. */
    ScriptVariable *localStack;
    /** The local stack size. */
    ScriptVariable *stackBottom;
    /** Variable from the top stack of the local stack. */
    ScriptVariable *pTop;
};

class ScriptVM
{
    friend class ScriptThread;

public:
    // important thread variables
    ScriptVM *next; // next VM in the current ScriptClass

    ScriptThread *m_Thread;      // script thread
    ScriptClass  *m_ScriptClass; // current group of threads

public:
    // return variables
    ScriptStack   *m_Stack; // Currently unused
    ScriptVMStack  m_VMStack;
    ScriptVariable m_ReturnValue; // VM return value

    // opcode variables
    unsigned char *m_PrevCodePos; // previous opcode, for use with script exceptions
    unsigned char *m_CodePos;     // check compiler.h for the list of all opcodes

public:
    // states
    unsigned char state;         // current VM state
    unsigned char m_ThreadState; // current thread state

    // stack variables
    Container<ScriptCallStack *> callStack;  // thread's call stack
    ScriptVariable              *m_StackPos; // marked stack position

    // parameters variables
    ScriptVariable *m_pOldData; // old fastEvent data, to cleanup
    int             m_OldDataSize;
    bool            m_bMarkStack; // changed by OP_MARK_STACK_POS and OP_RESTORE_STACK_POS
    Event           fastEvent;    // parameter list, set when the VM is executed

private:
    void error(const char *format, ...);

    template<bool bMethod = false, bool bReturn = false>
    void executeCommand(Listener *listener, op_parmNum_t iParamCount, op_evName_t eventnum);
    template<bool bReturn>
    void executeCommandInternal(Event& ev, Listener *listener, ScriptVariable *fromVar, op_parmNum_t iParamCount);
    bool executeGetter(Listener *listener, op_evName_t eventName);
    bool executeSetter(Listener *listener, op_evName_t eventName);
    void transferVarsToEvent(Event& ev, ScriptVariable *fromVar, op_parmNum_t count);

    void            loadTopInternal(Listener *listener);
    ScriptVariable *storeTopInternal(Listener *listener);
    template<bool noTop = false>
    void loadTop(Listener *listener);
    template<bool noTop = false>
    ScriptVariable *storeTop(Listener *listener);
    void            loadStoreTop(Listener *listener);
    void            skipField();

    void SetFastData(ScriptVariable *data, int dataSize);

    bool Switch(StateScript *stateScript, ScriptVariable& var);

    unsigned char *ProgBuffer();
    void           HandleScriptException(ScriptException& exc);

public:
    void *operator new(size_t size);
    void  operator delete(void *ptr);

    ScriptVM();
    ScriptVM(ScriptClass *scriptClass, unsigned char *pCodePos, ScriptThread *thread);
    ~ScriptVM();

    void Archive(Archiver& arc);

    void EnterFunction(Container<ScriptVariable>&&);
    void LeaveFunction();

    void End(const ScriptVariable& returnValue);
    void End(void);

    void Execute(ScriptVariable *data = NULL, int dataSize = 0, str label = "");
    void NotifyDelete(void);
    void Resume(qboolean bForce = false);
    void Suspend(void);

    str          Filename(void) const;
    str          Label(void) const;
    ScriptClass *GetScriptClass(void) const;
    GameScript  *GetScript() const;

    bool IsSuspended(void);
    int  State(void);
    int  ThreadState(void);

    void EventGoto(Event *ev);
    bool EventThrow(Event *ev);

    bool        CanScriptTracePrint(void);
    void        ScriptTrace1() const;
    void        ScriptTrace2() const;
    const char *GetSourcePos() const;

private:
    void jump(unsigned int offset);
    void jumpBack(unsigned int offset);
    void jumpBool(unsigned int offset, bool booleanValue);
    bool jumpVar(unsigned int offset, bool booleanValue);
    void doJumpIf(bool booleanValue);
    bool doJumpVarIf(bool booleanValue);

    void fetchOpcodeValue(void *outValue, size_t size);
    void fetchActualOpcodeValue(void *outValue, size_t size);

    template<typename T>
    T fetchOpcodeValue()
    {
        T value;
        fetchOpcodeValue(&value, sizeof(T));
        return value;
    }

    template<typename T>
    T fetchOpcodeValue(size_t offset)
    {
        T value;
        fetchOpcodeValue(&value, sizeof(T));
        return value;
    }

    template<typename T>
    T fetchActualOpcodeValue()
    {
        T value;
        fetchActualOpcodeValue(&value, sizeof(T));
        return value;
    }

    void execCmdCommon(op_parmNum_t param);
    void execCmdMethodCommon(op_parmNum_t param);
    void execMethodCommon(op_parmNum_t param);
    void execFunction(ScriptMaster& Director);
};

extern MEM_BlockAlloc<ScriptClass> ScriptClass_allocator;
