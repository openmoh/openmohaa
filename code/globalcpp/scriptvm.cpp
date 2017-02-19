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

// scriptvm.cpp : Script virtual machine, interprets and execute scripts

#include "glb_local.h"
#include "scriptmaster.h"
#include "scriptvm.h"
#include "compiler.h"
#include "game.h"
#include "level.h"
#include "parm.h"
#include "world.h"

#ifdef CGAME_DLL

#define VM_Printf cgi.Printf
#define VM_DPrintf cgi.DPrintf

#elif defined GAME_DLL

#define VM_Printf gi.Printf
#define VM_DPrintf gi.DPrintf2

#else

#define VM_Printf printf
#define VM_DPrintf printf

#endif

//====================
// ScriptClass
//====================

MEM_BlockAlloc< ScriptClass, MEM_BLOCKSIZE > ScriptClass_allocator;

CLASS_DECLARATION( Listener, ScriptClass, NULL )
{
	{ NULL, NULL }
};

/*
====================
new ScriptClass
====================
*/
void *ScriptClass::operator new( size_t size )
{
	return ScriptClass_allocator.Alloc();
}

/*
====================
delete ptr
====================
*/
void ScriptClass::operator delete( void *ptr )
{
	ScriptClass_allocator.Free( ptr );
}

/*
====================
ScriptClass
====================
*/
ScriptClass::ScriptClass( GameScript *gameScript, Listener *self )
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
	if( m_Script == NULL ) {
		ScriptError( "Attempting to delete dead class." );
	}

	KillThreads();

	if( !m_Script->m_Filename )
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
void ScriptClass::Archive( Archiver& arc )
{
}

/*
====================
ArchiveInternal
====================
*/
void ScriptClass::ArchiveInternal( Archiver& arc )
{
	Listener::Archive( arc );

	arc.ArchiveObjectPosition( this );
	arc.ArchiveSafePointer( &m_Self );
	GameScript::Archive( arc, m_Script );
}

/*
====================
ArchiveScript
====================
*/
void ScriptClass::ArchiveScript( Archiver& arc, ScriptClass **obj )
{
	ScriptClass *scr;
	ScriptVM *m_current;
	ScriptThread *m_thread;
	int num;
	int i;

	if( arc.Saving() )
	{
		scr = *obj;
		scr->ArchiveInternal( arc );

		num = 0;
		for( m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next )
			num++;

		arc.ArchiveInteger( &num );

		for( m_current = scr->m_Threads; m_current != NULL; m_current = m_current->next )
			m_current->m_Thread->ArchiveInternal( arc );
	}
	else
	{
		scr = new ScriptClass();
		scr->ArchiveInternal( arc );

		arc.ArchiveInteger( &num );

		for( i = 0; i < num; i++ )
		{
			m_thread = new ScriptThread( scr, NULL );
			m_thread->ArchiveInternal( arc );
		}

		*obj = scr;
	}
}

/*
====================
ArchiveCodePos
====================
*/
void ScriptClass::ArchiveCodePos( Archiver& arc, unsigned char **codePos )
{
	m_Script->ArchiveCodePos( arc, codePos );
}

/*
====================
CreateThreadInternal
====================
*/
ScriptThread *ScriptClass::CreateThreadInternal( const ScriptVariable& label )
{
	GameScript *scr;
	ScriptThread *thread = NULL;

	if( label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING )
	{
		ScriptClass *scriptClass = Director.CurrentScriptClass();
		scr = scriptClass->GetScript();

		if( label.GetType() == VARIABLE_CONSTSTRING )
			thread = Director.CreateScriptThread( scr, m_Self, label.constStringValue() );
		else
			thread = Director.CreateScriptThread( scr, m_Self, label.stringValue() );
	}
	else if( label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1 )
	{
		ScriptVariable *script = label[ 1 ];
		ScriptVariable *labelname = label[ 2 ];

		if( script->GetType() == VARIABLE_CONSTSTRING )
			scr = Director.GetGameScript( script->constStringValue() );
		else
			scr = Director.GetGameScript( script->stringValue() );

		if( labelname->GetType() == VARIABLE_CONSTSTRING )
			thread = Director.CreateScriptThread( scr, m_Self, labelname->constStringValue() );
		else
			thread = Director.CreateScriptThread( scr, m_Self, labelname->stringValue() );
	}
	else
	{
		ScriptError( "ScriptClass::CreateThreadInternal: bad argument format" );
	}

	return thread;
}

/*
====================
CreateScriptInternal
====================
*/
ScriptThread *ScriptClass::CreateScriptInternal( const ScriptVariable& label )
{
	GameScript *scr;
	ScriptThread *thread = NULL;

	if( label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING )
	{
		if( label.GetType() == VARIABLE_CONSTSTRING )
			thread = Director.CreateScriptThread( Director.GetGameScript( label.stringValue() ), m_Self, "" );
		else
			thread = Director.CreateScriptThread( Director.GetGameScript( label.constStringValue() ), m_Self, "" );
	}
	else if( label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1 )
	{
		ScriptVariable *script = label[ 1 ];
		ScriptVariable *labelname = label[ 2 ];

		if( script->GetType() == VARIABLE_CONSTSTRING )
			scr = Director.GetGameScript( script->constStringValue() );
		else
			scr = Director.GetGameScript( script->stringValue() );

		if( labelname->GetType() == VARIABLE_CONSTSTRING )
			thread = Director.CreateScriptThread( scr, m_Self, labelname->constStringValue() );
		else
			thread = Director.CreateScriptThread( scr, m_Self, labelname->stringValue() );
	}
	else
	{
		ScriptError( "ScriptClass::CreateScriptInternal: bad label type '%s'", label.GetTypeName() );
	}

	return thread;
}

/*
====================
AddThread
====================
*/
void ScriptClass::AddThread( ScriptVM *m_ScriptVM )
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
	if( !m_Threads ) {
		return;
	}

	ScriptVM *m_current;
	ScriptVM *m_next;

	m_current = m_Threads;

	do
	{
		m_current->m_ScriptClass = NULL;

		m_next = m_current->next;
		delete m_current->m_Thread;

	} while( m_current = m_next );

	m_Threads = NULL;
}

/*
====================
RemoveThread
====================
*/
void ScriptClass::RemoveThread( ScriptVM *m_ScriptVM )
{
	if( m_Threads == m_ScriptVM )
	{
		m_Threads = m_ScriptVM->next;

		if( m_Threads == NULL ) {
			delete this;
		}
	}
	else
	{
		ScriptVM *m_current = m_Threads;
		ScriptVM *i;

		for( i = m_Threads->next; i != m_ScriptVM; i = i->next ) {
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
unsigned char *ScriptClass::FindLabel( str label )
{
	return m_Script->m_State.FindLabel( label );
}

/*
====================
FindLabel
====================
*/
unsigned char *ScriptClass::FindLabel( const_str label )
{
	return m_Script->m_State.FindLabel( label );
}

/*
====================
NearestLabel
====================
*/
const_str ScriptClass::NearestLabel( unsigned char *pos )
{
	return m_Script->m_State.NearestLabel( pos );
}

/*
====================
GetCatchStateScript
====================
*/
StateScript *ScriptClass::GetCatchStateScript( unsigned char *in, unsigned char *&out )
{
	return m_Script->GetCatchStateScript( in, out );
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
	return static_cast< Listener * >( m_Self.Pointer() );
}

//====================
// ScriptVM
//====================


MEM_BlockAlloc< ScriptVM, char[ 256 ] > ScriptVM_allocator;

/*
====================
new ScriptVM
====================
*/
void *ScriptVM::operator new( size_t size )
{
	return ScriptVM_allocator.Alloc();
}

/*
====================
delete ptr
====================
*/
void ScriptVM::operator delete( void *ptr )
{
	ScriptVM_allocator.Free( ptr );
}

/*
====================
ScriptVM
====================
*/
ScriptVM::ScriptVM( ScriptClass *scriptClass, unsigned char *pCodePos, ScriptThread *thread )
{
	next = NULL;

	m_Thread = thread;
	m_ScriptClass = scriptClass;

	m_Stack = NULL;

	m_PrevCodePos = NULL;
	m_CodePos = pCodePos;

	state = STATE_RUNNING;
	m_ThreadState = THREAD_RUNNING;

	m_pOldData = NULL;
	m_OldDataSize = 0;

	m_bMarkStack = false;
	m_StackPos = NULL;

	m_bAllowContextSwitch = true;

	localStackSize = m_ScriptClass->GetScript()->GetRequiredStackSize();

	if( localStackSize <= 0 ) {
		localStackSize = 1;
	}

	localStack = new ScriptVariable[ localStackSize ];

	pTop = localStack;

	m_ScriptClass->AddThread( this );
}

/*
====================
~ScriptVM
====================
*/
ScriptVM::~ScriptVM()
{
	fastEvent.data = m_pOldData;
	fastEvent.dataSize = m_OldDataSize;

	// clean-up the call stack
	while( callStack.NumObjects() )
	{
		LeaveFunction();
	}

	delete[] localStack;
}

/*
====================
Archive
====================
*/
void ScriptVM::Archive( Archiver& arc )
{
	int stack = 0;

	if( arc.Saving() )
	{
		if( m_Stack )
			stack = m_Stack->m_Count;

		arc.ArchiveInteger( &stack );
	}
	else
	{
		arc.ArchiveInteger( &stack );

		if( stack )
		{
			m_Stack = new ScriptStack;
			m_Stack->m_Array = new ScriptVariable[ stack ];
			m_Stack->m_Count = stack;
		}
	}

	for( int i = 1; i <= stack; i++ )
	{
		m_Stack->m_Array[ i ].ArchiveInternal( arc );
	}

	m_ReturnValue.ArchiveInternal( arc );
	m_ScriptClass->ArchiveCodePos( arc, &m_PrevCodePos );
	m_ScriptClass->ArchiveCodePos( arc, &m_CodePos );
	arc.ArchiveByte( &state );
	arc.ArchiveByte( &m_ThreadState );
}

/*
====================
error

Triggers an error
====================
*/
void ScriptVM::error( const char *format, ... )
{
	char buffer[ 4000 ];
	va_list va;

	va_start( va, format );
	vsprintf( buffer, format, va );
	va_end( va );

	glbs.Printf( "----------------------------------------------------------\n%s\n", buffer );
	m_ReturnValue.setStringValue( "$.INTERRUPTED" );
}

/*
====================
executeCommand
====================
*/
void ScriptVM::executeCommand( Listener *listener, int iParamCount, int eventnum, bool bReturn )
{
	Event ev;
	ScriptVariable *var;

	ev = Event( eventnum );

	if( bReturn )
	{
		var = pTop;
	}
	else
	{
		var = pTop + 1;
	}

	ev.dataSize = iParamCount;
	ev.data = new ScriptVariable[ ev.dataSize ];
	ev.fromScript = true;

	for( int i = 0; i < iParamCount; i++ ) {
		ev.data[ i ] = var[ i ];
	}

	listener->ProcessScriptEvent( ev );

	if( ev.NumArgs() > iParamCount ) {
		*pTop = ev.GetValue( ev.NumArgs() );
	} else {
		pTop->Clear();
	}
}

/*
====================
executeGetter
====================
*/
bool ScriptVM::executeGetter( Listener *listener, str& name )
{
	Event ev;
	int eventnum = Event::FindGetterEventNum( name );

	if( eventnum && listener->classinfo()->GetDef( eventnum ) )
	{
		ev = Event( eventnum );
		ev.fromScript = true;

		if( listener->ProcessScriptEvent( ev ) )
		{

			if( ev.NumArgs() > 0 ) {
				*pTop = ev.GetValue( ev.NumArgs() );
			} else {
				pTop->Clear();
			}

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		eventnum = Event::FindSetterEventNum( name );
		assert( !eventnum || !listener->classinfo()->GetDef( eventnum ) );
		if( eventnum && listener->classinfo()->GetDef( eventnum ) )
		{
			ScriptError( "Cannot get a write-only variable" );
		}
	}

	return false;
}

/*
====================
executeSetter
====================
*/
bool ScriptVM::executeSetter( Listener *listener, str& name )
{
	Event ev;
	int eventnum = Event::FindSetterEventNum( name );

	if( eventnum && listener->classinfo()->GetDef( eventnum ) )
	{
		ev = Event( eventnum );
		ev.fromScript = true;

		ev.AddValue( *pTop );

		if( listener->ProcessScriptEvent( ev ) )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		eventnum = Event::FindGetterEventNum( name );
		if( eventnum && listener->classinfo()->GetDef( eventnum ) )
		{
			ScriptError( "Cannot set a read-only variable" );
		}
	}

	return false;
}

/*
====================
jump
====================
*/
void ScriptVM::jump( int offset )
{
	m_CodePos += offset;
}

/*
====================
jumpBool
====================
*/
void ScriptVM::jumpBool( int offset, bool booleanValue )
{
	if( booleanValue )
	{
		jump( offset );
	}
	else
	{
		m_CodePos += sizeof( unsigned int );
	}
}

/*
====================
loadTop
====================
*/
void ScriptVM::loadTop( Listener *listener, bool noTop )
{
	int index;

	index = *reinterpret_cast< int * >( m_CodePos );
	m_CodePos += sizeof( unsigned int );

	if( index != -1 )
	{
		str& variable = Director.GetString( index );

		if( !executeSetter( listener, variable ) )
		{
			listener->Vars()->SetVariable( variable, *pTop );
		}
	}

	if( !noTop ) {
		pTop--;
	}
}

/*
====================
storeTop
====================
*/
void ScriptVM::storeTop( Listener *listener, bool noTop )
{
	str variable;
	int index;

	index = *reinterpret_cast< int * >( m_CodePos );
	m_CodePos += sizeof( unsigned int );

	if( index != -1 )
	{
		variable = Director.GetString( index );
	}

	if( !noTop ) {
		pTop++;
	}

	if( index != -1 && !executeGetter( listener, variable ) )
	{
		*pTop = *listener->Vars()->GetOrCreateVariable( index );
	}
}

/*
====================
ProgBuffer

Returns the current program buffer
====================
*/
unsigned char *ScriptVM::ProgBuffer( void )
{
	return m_CodePos;
}

/*
====================
EnterFunction

Sets a new instruction pointer
====================
*/
void ScriptVM::EnterFunction( Event *ev )
{
	ScriptCallStack *stack;
	str label = ev->GetString( 1 );

	SetFastData( ev->data + 1, ev->dataSize - 1 );

	unsigned char *codePos = m_ScriptClass->FindLabel( label );

	if( !codePos )
	{
		ScriptError( "ScriptVM::EnterFunction: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str() );
	}

	stack = new ScriptCallStack;

	stack->codePos = m_CodePos;

	stack->pTop = pTop;
	stack->returnValue = m_ReturnValue;
	stack->localStack = localStack;
	stack->m_Self = m_ScriptClass->GetSelf();

	callStack.AddObject( stack );

	m_CodePos = codePos;

	localStack = new ScriptVariable[ localStackSize ];

	pTop = localStack;
	m_ReturnValue.Clear();
}

/*
====================
LeaveFunction

Returns to the previous function
====================
*/
void ScriptVM::LeaveFunction()
{
	int num = callStack.NumObjects();

	if( num )
	{
		ScriptCallStack *stack = callStack.ObjectAt( num );

		pTop = stack->pTop;
		*pTop = m_ReturnValue;

		m_CodePos = stack->codePos;
		m_ReturnValue = stack->returnValue;
		m_ScriptClass->m_Self = stack->m_Self;

		delete[] localStack;

		localStack = stack->localStack;

		delete stack;

		callStack.RemoveObjectAt( num );
	}
	else
	{
		delete m_Thread;
	}
}

/*
====================
End

End with a return value
====================
*/
void ScriptVM::End( const ScriptVariable& returnValue )
{
	m_ReturnValue.setPointer( returnValue );

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
void ScriptVM::Execute( ScriptVariable *data, int dataSize, str label )
{
	unsigned char		*opcode;
	bool				doneProcessing = false;
	bool				deleteThread = false;
	bool				eventCalled = false;

	ScriptVariable		*a;
	ScriptVariable		*b;
	ScriptVariable		*c;

	int					index, iParamCount;

	Listener			*listener;

	Event				ev;
	Event				*ev2;
	ScriptVariable		*var = NULL;

	static str			str_null = "";
	str&				value = str_null;

	ConSimple			*targetList;

	if( label != "" )
	{
		// Throw if label is not found
		if( !( m_CodePos = m_ScriptClass->FindLabel( label ) ) )
		{
			ScriptError( "ScriptVM::Execute: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str() );
		}
	}

	if( Director.stackCount >= MAX_STACK_DEPTH )
	{
		state = STATE_EXECUTION;

		ScriptException::next_abort = -1;
		ScriptException exc( "stack overflow" );

		throw exc;
	}

	Director.stackCount++;

	if( dataSize )
	{
		SetFastData( data, dataSize );
	}

	state = STATE_RUNNING;

	Director.cmdTime = glbs.Milliseconds();
	Director.cmdCount = 0;

	while( !doneProcessing && state == STATE_RUNNING )
	{
		m_PrevCodePos = m_CodePos;

		Director.cmdCount++;

		try
		{
		if( !Director.m_bAllowContextSwitch )
		{
			if( Director.cmdCount > 9999 && glbs.Milliseconds() - Director.cmdTime > Director.maxTime )
			{
				if( level.m_LoopProtection )
				{
					Director.cmdTime = glbs.Milliseconds();

					deleteThread = true;
					state = STATE_EXECUTION;

					if( level.m_LoopDrop ) {
						ScriptException::next_abort = -1;
					}

					ScriptError( "Command overflow. Possible infinite loop in thread.\n" );
				}

				VM_DPrintf( "Update of script position - This is not an error.\n" );
				VM_DPrintf( "=================================================\n" );
				m_ScriptClass->GetScript()->PrintSourcePos( opcode, true );
				VM_DPrintf( "=================================================\n" );

				Director.cmdCount = 0;
			}
		}
		else
		{
			if( glbs.Milliseconds() - Director.cmdTime > Director.maxTime )
			{
				// Request a context switch
				RequestContextSwitch();
			}
		}

		if( !m_bMarkStack )
		{
			assert( pTop >= localStack && pTop < localStack + localStackSize );
			if( pTop < localStack )
			{
				deleteThread = true;
				state = STATE_EXECUTION;

				error( "VM stack error. Negative stack value %d.\n", pTop - localStack );
				break;
			}
			else if( pTop >= localStack + localStackSize )
			{
				deleteThread = true;
				state = STATE_EXECUTION;

				error( "VM stack error. Exceeded the maximum stack size %d.\n", localStackSize );
				break;
			}
		}

		index = 0;
		eventCalled = false;

		opcode = m_CodePos++;
		switch( *opcode )
		{
		case OP_BIN_BITWISE_AND:
			a = pTop--;
			b = pTop;

			*b &= *a;
			break;

		case OP_BIN_BITWISE_OR:
			a = pTop--;
			b = pTop;

			*b |= *a;
			break;

		case OP_BIN_BITWISE_EXCL_OR:
			a = pTop--;
			b = pTop;

			*b ^= *a;
			break;

		case OP_BIN_EQUALITY:
			a = pTop--;
			b = pTop;

			b->setIntValue( *b == *a );
			break;

		case OP_BIN_INEQUALITY:
			a = pTop--;
			b = pTop;

			b->setIntValue( *b != *a );
			break;

		case OP_BIN_GREATER_THAN:
			a = pTop--;
			b = pTop;

			b->greaterthan( *a );
			break;

		case OP_BIN_GREATER_THAN_OR_EQUAL:
			a = pTop--;
			b = pTop;

			b->greaterthanorequal( *a );
			break;

		case OP_BIN_LESS_THAN:
			a = pTop--;
			b = pTop;

			b->lessthan( *a );
			break;

		case OP_BIN_LESS_THAN_OR_EQUAL:
			a = pTop--;
			b = pTop;

			b->lessthanorequal( *a );
			break;

		case OP_BIN_PLUS:
			a = pTop--;
			b = pTop;

			*b += *a;
			break;

		case OP_BIN_MINUS:
			a = pTop--;
			b = pTop;

			*b -= *a;
			break;

		case OP_BIN_MULTIPLY:
			a = pTop--;
			b = pTop;

			*b *= *a;
			break;

		case OP_BIN_DIVIDE:
			a = pTop--;
			b = pTop;

			*b /= *a;
			break;

		case OP_BIN_PERCENTAGE:
			a = pTop--;
			b = pTop;

			*b %= *a;
			break;

		case OP_BIN_SHIFT_LEFT:
			a = pTop--;
			b = pTop;

			*b <<= *a;
			break;

		case OP_BIN_SHIFT_RIGHT:
			a = pTop--;
			b = pTop;

			*b >>= *a;
			break;

		case OP_BOOL_JUMP_FALSE4:
			jumpBool( *reinterpret_cast< unsigned int * >( m_CodePos ) + sizeof( unsigned int ), !pTop->m_data.intValue );

			pTop--;

			break;

		case OP_BOOL_JUMP_TRUE4:
			jumpBool( *reinterpret_cast< unsigned int * >( m_CodePos ) + sizeof( unsigned int ), pTop->m_data.intValue ? true : false );

			pTop--;

			break;

		case OP_VAR_JUMP_FALSE4:
			jumpBool( *reinterpret_cast< unsigned int * >( m_CodePos ) + sizeof( unsigned int ), !pTop->booleanValue() );

			pTop--;

			break;

		case OP_VAR_JUMP_TRUE4:
			jumpBool( *reinterpret_cast< unsigned int * >( m_CodePos ) + sizeof( unsigned int ), pTop->booleanValue() );

			pTop--;

			break;

		case OP_BOOL_LOGICAL_AND:
			if( pTop->m_data.intValue )
			{
				pTop--;
				m_CodePos += sizeof( unsigned int );
			}
			else
			{
				m_CodePos += *reinterpret_cast< unsigned int * >( m_CodePos ) + sizeof( unsigned int );
			}

			break;

		case OP_BOOL_LOGICAL_OR:
			if( !pTop->m_data.intValue )
			{
				pTop--;
				m_CodePos += sizeof( unsigned int );
			}
			else
			{
				m_CodePos += *reinterpret_cast< unsigned int * >( m_CodePos ) + sizeof( unsigned int );
			}

			break;

		case OP_VAR_LOGICAL_AND:
			if( pTop->booleanValue() )
			{
				pTop--;
				m_CodePos += sizeof( unsigned int );
			}
			else
			{
				pTop->SetFalse();
				m_CodePos += *reinterpret_cast< unsigned int * >( m_CodePos ) + sizeof( unsigned int );
			}
			break;

		case OP_VAR_LOGICAL_OR:
			if( !pTop->booleanValue() )
			{
				pTop--;
				m_CodePos += sizeof( unsigned int );
			}
			else
			{
				pTop->SetTrue();
				m_CodePos += *reinterpret_cast< unsigned int * >( m_CodePos ) + sizeof( unsigned int );
			}
			break;

		case OP_BOOL_STORE_FALSE:
			pTop++;
			pTop->SetFalse();
			break;

		case OP_BOOL_STORE_TRUE:
			pTop++;
			pTop->SetTrue();
			break;

		case OP_BOOL_UN_NOT:
			pTop->m_data.intValue = ( pTop->m_data.intValue == 0 );
			break;

		case OP_CALC_VECTOR:
			c = pTop--;
			b = pTop--;
			a = pTop;

			pTop->setVectorValue( Vector( a->floatValue(), b->floatValue(), c->floatValue() ) );
			break;

		case OP_EXEC_CMD0:
			iParamCount = 0;
			goto __execCmd;

		case OP_EXEC_CMD1:
			iParamCount = 1;
			goto __execCmd;

		case OP_EXEC_CMD2:
			iParamCount = 2;
			goto __execCmd;

		case OP_EXEC_CMD3:
			iParamCount = 3;
			goto __execCmd;

		case OP_EXEC_CMD4:
			iParamCount = 4;
			goto __execCmd;

		case OP_EXEC_CMD5:
			iParamCount = 5;
			goto __execCmd;

		case OP_EXEC_CMD_COUNT1:
			iParamCount = *m_CodePos++;

__execCmd:
			index = *reinterpret_cast< unsigned int * >( m_CodePos );

			m_CodePos += sizeof( unsigned int );

			pTop -= iParamCount;

			try
			{
				executeCommand( m_Thread, iParamCount, index );
			}
			catch( ScriptException& exc )
			{
				throw exc;
			}

			break;

		case OP_EXEC_CMD_METHOD0:
			iParamCount = 0;
			goto __execCmdMethod;

		case OP_EXEC_CMD_METHOD1:
			iParamCount = 1;
			goto __execCmdMethod;

		case OP_EXEC_CMD_METHOD2:
			iParamCount = 2;
			goto __execCmdMethod;

		case OP_EXEC_CMD_METHOD3:
			iParamCount = 3;
			goto __execCmdMethod;

		case OP_EXEC_CMD_METHOD4:
			iParamCount = 4;
			goto __execCmdMethod;

		case OP_EXEC_CMD_METHOD5:
			iParamCount = 5;
			goto __execCmdMethod;

__execCmdMethod:
			m_CodePos--;
			goto __execCmdMethodInternal;

		case OP_EXEC_CMD_METHOD_COUNT1:
			iParamCount = *m_CodePos;

__execCmdMethodInternal:
			a = pTop--;

			try
			{
				index = *reinterpret_cast< unsigned int * >( m_CodePos + sizeof( byte ) );

				pTop -= iParamCount;

				if( a->arraysize() < 0 )
				{
					ScriptError( "command '%s' applied to NIL", Event::GetEventName( index ).c_str() );
				}

				ScriptVariable array = *a;
				Listener *listener;

				array.CastConstArrayValue();

				for( int i = array.arraysize(); i > 0; i-- )
				{
					if( !( listener = array[ i ]->listenerValue() ) )
					{
						ScriptError( "command '%s' applied to NULL listener", Event::GetEventName( index ).c_str() );
					}

					executeCommand( listener, iParamCount, index );
				}
			}
			catch( ScriptException& exc )
			{
				m_CodePos += sizeof( byte ) + sizeof( unsigned int );

				throw exc;
			}

			m_CodePos += sizeof( byte ) + sizeof( unsigned int );

			break;

		case OP_EXEC_METHOD0:
			iParamCount = 0;
			goto __execMethod;

		case OP_EXEC_METHOD1:
			iParamCount = 1;
			goto __execMethod;

		case OP_EXEC_METHOD2:
			iParamCount = 2;
			goto __execMethod;

		case OP_EXEC_METHOD3:
			iParamCount = 3;
			goto __execMethod;

		case OP_EXEC_METHOD4:
			iParamCount = 4;
			goto __execMethod;

		case OP_EXEC_METHOD5:
			iParamCount = 5;

__execMethod:
			m_CodePos--;
			goto __execMethodInternal;

		case OP_EXEC_METHOD_COUNT1:
			iParamCount = *m_CodePos;

__execMethodInternal:
			a = pTop--;

			try
			{
				index = *reinterpret_cast< unsigned int * >( m_CodePos + sizeof( byte ) );

				pTop -= iParamCount;
				pTop++; // push the return value

				Listener *listener = a->listenerValue();

				if( !listener )
				{
					ScriptError( "command '%s' applied to NULL listener", Event::GetEventName( index ).c_str() );
				}

				executeCommand( listener, iParamCount, index, true );
			}
			catch( ScriptException& exc )
			{
				m_CodePos += sizeof( byte ) + sizeof( unsigned int );

				throw exc;
			}

			m_CodePos += sizeof( byte ) + sizeof( unsigned int );

			break;

		case OP_FUNC:
			ev.Clear();

			if( !*m_CodePos++ )
			{
				str& label = Director.GetString( *reinterpret_cast< unsigned int * >( m_CodePos ) );

				m_CodePos += sizeof( unsigned int );

				try
				{
					listener = pTop->listenerValue();

					if( !listener )
					{
						ScriptError( "function '%s' applied to NULL listener", label.c_str() );
					}
				}
				catch( ScriptException& exc )
				{
					pTop -= *m_CodePos++;

					throw exc;
				}

				pTop--;

				ev.AddString( label );

				int params = *m_CodePos++;

				var = pTop;
				pTop -= params;

				for( int i = 0; i < params; var++, i++ ) {
					ev.AddValue( *var );
				}

				pTop++;
				EnterFunction( &ev );

				m_ScriptClass->m_Self = listener;
			}
			else
			{
				str filename, label;

				filename = Director.GetString( *reinterpret_cast< unsigned int * >( m_CodePos ) );
				m_CodePos += sizeof( unsigned int );
				label = Director.GetString( *reinterpret_cast< unsigned int * >( m_CodePos ) );
				m_CodePos += sizeof( unsigned int );

				try
				{
					listener = pTop->listenerValue();

					if( !listener )
					{
						ScriptError( "function '%s' in '%s' applied to NULL listener", label.c_str(), filename.c_str() );
					}
				}
				catch( ScriptException& exc )
				{
					pTop -= *m_CodePos++;

					throw exc;
				}

				pTop--;

				ScriptVariable constarray;
				ScriptVariable *pVar = new ScriptVariable[ 2 ];

				pVar[ 0 ].setStringValue( filename );
				pVar[ 1 ].setStringValue( label );

				constarray.setConstArrayValue( pVar, 2 );

				delete[] pVar;

				ev2 = new Event( EV_Listener_WaitCreateReturnThread );
				ev2->AddValue( constarray );

				int params = *m_CodePos++;

				var = pTop;
				pTop -= params;

				for( int i = 0; i < params; var++, i++ ) {
					ev2->AddValue( *var );
				}

				pTop++;
				*pTop = listener->ProcessEventReturn( ev2 );
			}
			break;

		case OP_JUMP4:
			m_CodePos += *reinterpret_cast< int * >( m_CodePos ) + sizeof( unsigned int );
			break;

		case OP_JUMP_BACK4:
			m_CodePos -= *reinterpret_cast< int * >( m_CodePos );
			break;

		case OP_LOAD_ARRAY_VAR:
			a = pTop--;
			b = pTop--;
			c = pTop--;

			b->setArrayAt( *a, *c );
			break;

		case OP_LOAD_FIELD_VAR:
			a = pTop--;

			try
			{
				listener = a->listenerValue();

				if( listener == NULL )
				{
					value = Director.GetString( *reinterpret_cast< int * >( m_CodePos ) );
					ScriptError( "Field '%s' applied to NULL listener", value.c_str() );
				}
				else
				{
					eventCalled = true;
					loadTop( listener );
				}
			}
			catch( ScriptException& exc )
			{
				pTop--;

				if( !eventCalled ) {
					m_CodePos += sizeof( unsigned int );
				}

				throw exc;
			}

			break;

		case OP_LOAD_CONST_ARRAY1:
			index = *reinterpret_cast< short * >( m_CodePos );
			m_CodePos += sizeof( short );

			pTop -= index - 1;
			pTop->setConstArrayValue( pTop, index );
			break;

		case OP_LOAD_GAME_VAR:
			loadTop( &game );
			break;

		case OP_LOAD_GROUP_VAR:
			loadTop( m_ScriptClass );
			break;

		case OP_LOAD_LEVEL_VAR:
			loadTop( &level );
			break;

		case OP_LOAD_LOCAL_VAR:
			loadTop( m_Thread );
			break;

		case OP_LOAD_OWNER_VAR:
			if( !m_ScriptClass->m_Self )
			{
				pTop--;
				m_CodePos += sizeof( unsigned int );
				ScriptError( "self is NULL" );
			}

			if( !m_ScriptClass->m_Self->GetScriptOwner() )
			{
				pTop--;
				m_CodePos += sizeof( unsigned int );
				ScriptError( "self.owner is NULL" );
			}

			loadTop( m_ScriptClass->m_Self->GetScriptOwner() );
			break;

		case OP_LOAD_PARM_VAR:
			loadTop( &parm );
			break;

		case OP_LOAD_SELF_VAR:
			if( !m_ScriptClass->m_Self )
			{
				pTop--;
				m_CodePos += sizeof( unsigned int );
				ScriptError( "self is NULL" );
			}

			loadTop( m_ScriptClass->m_Self );
			break;

		case OP_LOAD_STORE_GAME_VAR:
			loadTop( &game, true );
			break;

		case OP_LOAD_STORE_GROUP_VAR:
			loadTop( m_ScriptClass, true );
			break;

		case OP_LOAD_STORE_LEVEL_VAR:
			loadTop( &level, true );
			break;

		case OP_LOAD_STORE_LOCAL_VAR:
			loadTop( m_Thread, true );
			break;

		case OP_LOAD_STORE_OWNER_VAR:
			if( !m_ScriptClass->m_Self )
			{
				m_CodePos += sizeof( unsigned int );
				ScriptError( "self is NULL" );
			}

			if( !m_ScriptClass->m_Self->GetScriptOwner() )
			{
				m_CodePos += sizeof( unsigned int );
				ScriptError( "self.owner is NULL" );
			}

			loadTop( m_ScriptClass->m_Self->GetScriptOwner(), true );
			break;

		case OP_LOAD_STORE_PARM_VAR:
			loadTop( &parm, true );
			break;

		case OP_LOAD_STORE_SELF_VAR:
			if( !m_ScriptClass->m_Self )
			{
				ScriptError( "self is NULL" );
			}

			loadTop( m_ScriptClass->m_Self, true );
			break;

		case OP_MARK_STACK_POS:
			m_StackPos = pTop;
			m_bMarkStack = true;
			break;

		case OP_STORE_PARAM:
			if( fastEvent.dataSize )
			{
				pTop = fastEvent.data++;
				fastEvent.dataSize--;
			}
			else
			{
				pTop = m_StackPos + 1;
				pTop->Clear();
			}
			break;

		case OP_RESTORE_STACK_POS:
			pTop = m_StackPos;
			m_bMarkStack = false;
			break;

		case OP_STORE_ARRAY:
			pTop--;
			pTop->evalArrayAt( *( pTop + 1 ) );
			break;

		case OP_STORE_ARRAY_REF:
			pTop--;
			pTop->setArrayRefValue( *( pTop + 1 ) );
			break;

		case OP_STORE_FIELD_REF:
		case OP_STORE_FIELD:
			try
			{
				value = Director.GetString( *reinterpret_cast< int * >( m_CodePos ) );

				listener = pTop->listenerValue();

				if( listener == NULL )
				{
					ScriptError( "Field '%s' applied to NULL listener", value.c_str() );
				}
				else
				{
					eventCalled = true;
					storeTop( listener, true );
				}

				if( *opcode == OP_STORE_FIELD_REF )
				{
					pTop->setRefValue( listener->vars->GetOrCreateVariable( value ) );
				}
			}
			catch( ScriptException& exc )
			{
				if( *opcode == OP_STORE_FIELD_REF )
				{
					pTop->setRefValue( pTop );
				}

				if( !eventCalled ) {
					m_CodePos += sizeof( unsigned int );
				}

				throw exc;
			}
			break;

		case OP_STORE_FLOAT:
			pTop++;
			pTop->setFloatValue( *reinterpret_cast< float * >( m_CodePos ) );

			m_CodePos += sizeof( float );

			break;

		case OP_STORE_INT0:
			pTop++;
			pTop->setIntValue( 0 );

			break;

		case OP_STORE_INT1:
			pTop++;
			pTop->setIntValue( *m_CodePos++ );

			break;

		case OP_STORE_INT2:
			pTop++;
			pTop->setIntValue( *reinterpret_cast< short * >( m_CodePos ) );

			m_CodePos += sizeof( short );

			break;

		case OP_STORE_INT3:
			pTop++;
			pTop->setIntValue( *reinterpret_cast< short3 * >( m_CodePos ) );

			m_CodePos += sizeof( short3 );
			break;

		case OP_STORE_INT4:
			pTop++;
			pTop->setIntValue( *reinterpret_cast< int * >( m_CodePos ) );

			m_CodePos += sizeof( int );

			break;

		case OP_STORE_GAME_VAR:
			storeTop( &game );
			break;

		case OP_STORE_GROUP_VAR:
			storeTop( m_ScriptClass );
			break;

		case OP_STORE_LEVEL_VAR:
			storeTop( &level );
			break;

		case OP_STORE_LOCAL_VAR:
			storeTop( m_Thread );
			break;

		case OP_STORE_OWNER_VAR:
			if( !m_ScriptClass->m_Self )
			{
				pTop++;
				m_CodePos += sizeof( unsigned int );
				ScriptError( "self is NULL" );
			}

			if( !m_ScriptClass->m_Self->GetScriptOwner() )
			{
				pTop++;
				m_CodePos += sizeof( unsigned int );
				ScriptError( "self.owner is NULL" );
			}

			storeTop( m_ScriptClass->m_Self->GetScriptOwner() );
			break;

		case OP_STORE_PARM_VAR:
			storeTop( &parm );
			break;

		case OP_STORE_SELF_VAR:
			if( !m_ScriptClass->m_Self )
			{
				pTop++;
				m_CodePos += sizeof( unsigned int );
				ScriptError( "self is NULL" );
			}

			storeTop( m_ScriptClass->m_Self );
			break;

		case OP_STORE_GAME:
			pTop++;
			pTop->setListenerValue( &game );
			break;

		case OP_STORE_GROUP:
			pTop++;
			pTop->setListenerValue( m_ScriptClass );
			break;

		case OP_STORE_LEVEL:
			pTop++;
			pTop->setListenerValue( &level );
			break;

		case OP_STORE_LOCAL:
			pTop++;
			pTop->setListenerValue( m_Thread );
			break;

		case OP_STORE_OWNER:
			pTop++;

			if( !m_ScriptClass->m_Self )
			{
				pTop++;
				ScriptError( "self is NULL" );
			}

			pTop->setListenerValue( m_ScriptClass->m_Self->GetScriptOwner() );
			break;

		case OP_STORE_PARM:
			pTop++;
			pTop->setListenerValue( &parm );
			break;

		case OP_STORE_SELF:
			pTop++;
			pTop->setListenerValue( m_ScriptClass->m_Self );
			break;

		case OP_STORE_NIL:
			pTop++;
			pTop->Clear();
			break;

		case OP_STORE_NULL:
			pTop++;
			pTop->setListenerValue( NULL );
			break;

		case OP_STORE_STRING:
			pTop++;
			pTop->setConstStringValue( *reinterpret_cast< unsigned int * >( m_CodePos ) );

			m_CodePos += sizeof( unsigned int );

			break;

		case OP_STORE_VECTOR:
			pTop++;
			pTop->setVectorValue( *reinterpret_cast< Vector * >( m_CodePos ) );

			m_CodePos += sizeof( Vector );

			break;

		case OP_SWITCH:
			if( !Switch( *reinterpret_cast< StateScript ** >( m_CodePos ), *pTop ) )
			{
				m_CodePos += sizeof( unsigned int );
			}

			pTop--;
			break;

		case OP_UN_CAST_BOOLEAN:
			pTop->CastBoolean();
			break;

		case OP_UN_COMPLEMENT:
			pTop->complement();
			break;

		case OP_UN_MINUS:
			pTop->minus();
			break;

		case OP_UN_DEC:
			( *pTop )--;
			break;

		case OP_UN_INC:
			( *pTop )++;
			break;

		case OP_UN_SIZE:
			pTop->setIntValue( ( int )pTop->size() );
			break;

		case OP_UN_TARGETNAME:
			targetList = world->GetExistingTargetList( pTop->stringValue() );

			if( !targetList )
			{
				value = pTop->stringValue();

				pTop->setListenerValue( NULL );

				if( *m_CodePos >= OP_BIN_EQUALITY && *m_CodePos <= OP_BIN_GREATER_THAN_OR_EQUAL || *m_CodePos >= OP_BOOL_UN_NOT && *m_CodePos <= OP_UN_CAST_BOOLEAN ) {
					ScriptError( "Targetname '%s' does not exist.", value.c_str() );
				}

				break;
			}

			if( targetList->NumObjects() == 1 )
			{
				pTop->setListenerValue( targetList->ObjectAt( 1 ) );
			}
			else if( targetList->NumObjects() > 1 )
			{
				pTop->setContainerValue( ( Container< SafePtr< Listener > > * )targetList );
			}
			else
			{
				value = pTop->stringValue();

				pTop->setListenerValue( NULL );

				if( *m_CodePos >= OP_BIN_EQUALITY && *m_CodePos <= OP_BIN_GREATER_THAN_OR_EQUAL || *m_CodePos >= OP_BOOL_UN_NOT && *m_CodePos <= OP_UN_CAST_BOOLEAN ) {
					ScriptError( "Targetname '%s' does not exist.", value.c_str() );
				}

				break;
			}

			break;

		case OP_VAR_UN_NOT:
			pTop->setIntValue( pTop->booleanValue() );
			break;

		case OP_DONE:
			End();
			break;

		case OP_NOP:
			break;

		default:
			if( *opcode < OP_MAX )
			{
				glbs.DPrintf( "unknown opcode %d ('%s')\n", *opcode, OpcodeName( *opcode ) );
			}
			else
			{
				glbs.DPrintf( "unknown opcode %d\n", *opcode );
			}
			break;
		}
		}
		catch( ScriptException& exc )
		{
			HandleScriptException( exc );
		}
	}

	Director.stackCount--;

	if( deleteThread || state == STATE_WAITING )
	{
		delete m_Thread;
	}
	else if( state == STATE_SUSPENDED )
	{
		state = STATE_EXECUTION;
	}

	if( state == STATE_DESTROYED )
	{
		delete this;
	}
}

/*
====================
HandleScriptException
====================
*/
void ScriptVM::HandleScriptException( ScriptException& exc )
{
	if( m_ScriptClass )
	{
		m_ScriptClass->GetScript()->PrintSourcePos( m_PrevCodePos, true );
	}
	else
	{
		glbs.DPrintf( "unknown source pos" );
	}

	if( exc.bAbort )
	{
		ScriptException e( exc.string );

		e.bAbort = exc.bAbort;
		e.bIsForAnim = exc.bIsForAnim;

		state = STATE_EXECUTION;
		throw e;
	}

	glbs.Printf( "^~^~^ Script Error : %s\n\n", exc.string.c_str() );
}

/*
====================
SetFastData

Sets the starting virtual machine parameters
====================
*/
void ScriptVM::SetFastData( ScriptVariable *data, int dataSize )
{
	if( fastEvent.data )
	{
		fastEvent.data = m_pOldData;
		fastEvent.dataSize = m_OldDataSize;

		fastEvent.Clear();

		m_pOldData = NULL;
		m_OldDataSize = 0;
	}

	if( dataSize )
	{
		fastEvent.data = new ScriptVariable[ dataSize ];
		fastEvent.dataSize = dataSize;

		for( int i = 0; i < dataSize; i++ )
		{
			fastEvent.data[ i ] = data[ i ];
		}

		m_pOldData = fastEvent.data;
		m_OldDataSize = fastEvent.dataSize;
	}
}

/*
====================
NotifyDelete
====================
*/
void ScriptVM::NotifyDelete( void )
{
	switch( state )
	{
	case STATE_DESTROYED:
		ScriptError( "Attempting to delete a dead thread." );
		break;

	case STATE_RUNNING:
	case STATE_SUSPENDED:
	case STATE_WAITING:
		state = STATE_DESTROYED;

		if( m_ScriptClass ) {
			m_ScriptClass->RemoveThread( this );
		}

		break;

	case STATE_EXECUTION:
		state = STATE_DESTROYED;

		if( m_ScriptClass ) {
			m_ScriptClass->RemoveThread( this );
		}

		delete this;

		break;
	}
}

/*
====================
Resume
====================
*/
void ScriptVM::Resume( qboolean bForce )
{
	if( state == STATE_SUSPENDED || ( bForce && state != STATE_DESTROYED ) ) {
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
	if( state == STATE_DESTROYED ) {
		ScriptError( "Cannot suspend a dead thread." );
	} else if( !state ) {
		state = STATE_SUSPENDED;
	}
}

/*
====================
Switch

Switch statement
====================
*/
bool ScriptVM::Switch( StateScript *stateScript, ScriptVariable& var )
{
	unsigned char *pos;

	fastEvent.dataSize = 0;

	pos = stateScript->FindLabel( var.stringValue() );

	if( !pos )
	{
		pos = stateScript->FindLabel( "" );

		if( !pos ) {
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
str ScriptVM::Filename( void )
{
	return m_ScriptClass->Filename();
}

/*
====================
Label
====================
*/
str ScriptVM::Label( void )
{
	const_str label = m_ScriptClass->NearestLabel( m_CodePos );

	if( !label )
	{
		return "";
	}

	return Director.GetString( label );
}

/*
====================
GetScriptClass
====================
*/
ScriptClass *ScriptVM::GetScriptClass( void )
{
	return m_ScriptClass;
}

/*
====================
IsSuspended
====================
*/
bool ScriptVM::IsSuspended( void )
{
	return state == STATE_SUSPENDED;
}

/*
====================
State
====================
*/
int ScriptVM::State( void )
{
	return state;
}

/*
====================
ThreadState
====================
*/
int ScriptVM::ThreadState( void )
{
	return m_ThreadState;
}

/*
====================
EventGoto
====================
*/
void ScriptVM::EventGoto( Event *ev )
{
	str label = ev->GetString( 1 );

	SetFastData( ev->data + 1, ev->dataSize - 1 );

	unsigned char *codePos = m_ScriptClass->FindLabel( label );

	if( !codePos )
	{
		ScriptError( "ScriptVM::EventGoto: label '%s' does not exist in '%s'.", label.c_str(), Filename().c_str() );
	}

	m_CodePos = codePos;
}

/*
====================
EventThrow

Called when throwing an exception
====================
*/
bool ScriptVM::EventThrow( Event *ev )
{
	str label = ev->GetString( 1 );

	SetFastData( ev->data, ev->dataSize );

	fastEvent.eventnum = ev->eventnum;

	while( 1 )
	{
		StateScript *stateScript = m_ScriptClass->GetCatchStateScript( m_PrevCodePos, m_PrevCodePos );

		if( !stateScript ) {
			break;
		}

		m_CodePos = stateScript->FindLabel( label );

		if( m_CodePos )
		{
			fastEvent.data++;
			fastEvent.dataSize--;

			return true;
		}
	}

	return false;
}

/*
====================
AllowContextSwitch
====================
*/
void ScriptVM::AllowContextSwitch( bool allow )
{
	m_bAllowContextSwitch = allow;
}

/*
====================
RequestContextSwitch

Requests a context switch
====================
*/
void ScriptVM::RequestContextSwitch( void )
{
	if( !m_bAllowContextSwitch || !Director.m_bAllowContextSwitch ) {
		return;
	}

	//glbs.DPrintf( "Performing context switch\n" );

	Director.AddContextSwitch( m_Thread );

	m_ThreadState = THREAD_CONTEXT_SWITCH;
	Suspend();
}
