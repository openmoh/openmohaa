/*
===========================================================================
Copyright (C) 2015 the OpenMoHAA team

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

#ifndef __SCRIPTVM_H__
#define __SCRIPTVM_H__

#include "listener.h"

#include <gamescript.h>

#include "scriptvariable.h"
#include "con_set.h"

#define	MAX_STACK_DEPTH		20		// 9 in mohaa
//#define	LOCALSTACK_SIZE		255		// pre-allocated localstack size for each VM
#define MAX_SCRIPTCYCLES	9999	// max cmds

#define STATE_RUNNING					0		// Running
#define STATE_SUSPENDED					1		// Suspended
#define STATE_WAITING					2		// Waiting for something
#define STATE_EXECUTION					3		// Resume to execution
#define STATE_DESTROYED					4		// Pending deletion

#define THREAD_RUNNING					0		// Running
#define THREAD_WAITING					1		// Waiting
#define THREAD_SUSPENDED				2		// Suspended
#define THREAD_CONTEXT_SWITCH			3		// Resume from context switch

class ScriptThread;
class ScriptVM;

class ScriptCallStack {
public:
	// opcode variable
	unsigned char		*codePos;	// opcode will be restored once a DONE was hit

	// stack variables
	ScriptVariable		*localStack;
	ScriptVariable		*pTop;

	// return variable
	ScriptVariable		returnValue;

	// OLD self value
	SafePtr< Listener > m_Self;
};

// Ley0k: I'm unsure about this class, MOHAA use it
class ScriptStack
{
public:
	ScriptVariable		*m_Array;
	int					m_Count;
};

class ScriptVM
{
	friend class ScriptThread;

public:
	// important thread variables
	ScriptVM		*next;				// next VM in the current ScriptClass

	ScriptThread	*m_Thread;			// script thread
	ScriptClass		*m_ScriptClass;		// current group of threads

public:
	// return variables
	ScriptStack		*m_Stack;		// Currently unused
	ScriptVariable	m_ReturnValue;	// VM return value

	// opcode variables
	unsigned char *m_PrevCodePos;		// previous opcode, for use with script exceptions
	unsigned char *m_CodePos;			// check compiler.h for the list of all opcodes

public:
	// states
	unsigned char state;			// current VM state
	unsigned char m_ThreadState;	// current thread state

	// stack variables
	Container< ScriptCallStack * >	callStack;			// thread's call stack
	ScriptVariable					*localStack;		// thread's local stack
	int								localStackSize;		// dynamically allocated at initialization
	ScriptVariable					*pTop;				// top stack from the local stack
	ScriptVariable					*m_StackPos;		// marked stack position

	// parameters variables
	ScriptVariable	*m_pOldData;		// old fastEvent data, to cleanup
	int				m_OldDataSize;
	bool			m_bMarkStack;		// changed by OP_MARK_STACK_POS and OP_RESTORE_STACK_POS
	Event			fastEvent;			// parameter list, set when the VM is executed

	// miscellaneous
	bool m_bAllowContextSwitch;			// allow parallel VM executions [experimental feature]

private:
	void			error( const char *format, ... );

	void			executeCommand( Listener *listener, int iParamCount, int eventnum, bool bReturn = false );
	bool			executeGetter( Listener *listener, str& name );
	bool			executeSetter( Listener *listener, str& name );

	void			jump( int offset );
	void			jumpBool( int offset, bool value );

	void			loadTop( Listener *listener, bool noTop = false );
	void			storeTop( Listener *listener, bool noTop = false );

	void			SetFastData( ScriptVariable *data, int dataSize );

	bool			Switch( StateScript *stateScript, ScriptVariable &var );

	unsigned char	*ProgBuffer();
	void			HandleScriptException( ScriptException& exc );

public:
#ifndef _DEBUG_MEM
	void *operator new( size_t size );
	void operator delete( void *ptr );
#endif

	ScriptVM( ScriptClass *scriptClass, unsigned char *pCodePos, ScriptThread *thread );
	~ScriptVM();

	void			Archive( Archiver& arc );

	void			EnterFunction( Event *ev );
	void			LeaveFunction();

	void			End( const ScriptVariable& returnValue );
	void			End( void );

	void			Execute( ScriptVariable *data = NULL, int dataSize = 0, str label = "" );
	void			NotifyDelete( void );
	void			Resume( qboolean bForce = false );
	void			Suspend( void );

	str				Filename( void );
	str				Label( void );
	ScriptClass		*GetScriptClass( void );

	bool			IsSuspended( void );
	int				State( void );
	int				ThreadState( void );

	void			EventGoto( Event *ev );
	bool			EventThrow( Event *ev );

	bool			CanScriptTracePrint( void );
};

extern MEM_BlockAlloc<ScriptClass> ScriptClass_allocator;

#endif
