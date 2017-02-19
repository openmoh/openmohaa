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

// compiler.cpp : Parse, then compile to op-codes.

#include "glb_local.h"
#include "compiler.h"
#include "scriptvm.h"
#include "level.h"
#include "parm.h"
#include "scriptmaster.h"
#include "parser/parsetree.h"
#include "parser/yyParser.h"
#include "parser/yyLexer.h"

ScriptCompiler Compiler;
int ScriptCompiler::current_label;

ScriptCompiler::ScriptCompiler()
{
	Reset();
}

void ScriptCompiler::Reset()
{
	code_pos = NULL;
	code_ptr = NULL;
	prog_ptr = NULL;
	prog_end_ptr = NULL;

	for( int i = 0; i < BREAK_JUMP_LOCATION_COUNT; i++ )
	{
		apucBreakJumpLocations[ i ] = 0;
		apucContinueJumpLocations[ i ] = 0;
		prev_opcodes[ i ].opcode = 0;
		prev_opcodes[ i ].VarStackOffset = 0;
	}

	iBreakJumpLocCount = 0;
	iContinueJumpLocCount = 0;

	m_iHasExternal = 0;
	m_iInternalMaxVarStackOffset = 0;
	m_iMaxCallStackOffset = 0;
	m_iMaxExternalVarStackOffset = 0;
	m_iVarStackOffset = 0;

	bCanBreak = false;
	bCanContinue = false;

	prev_opcode_pos = 0;
}

unsigned char ScriptCompiler::PrevOpcode()
{
	return prev_opcodes[ prev_opcode_pos ].opcode;
}

char ScriptCompiler::PrevVarStackOffset()
{
	return prev_opcodes[ prev_opcode_pos ].VarStackOffset;
}

void ScriptCompiler::AbsorbPrevOpcode()
{
	m_iVarStackOffset -= PrevVarStackOffset();

	code_pos -= OpcodeLength( PrevOpcode() );

	if( !prev_opcode_pos ) {
		prev_opcode_pos = 100;
	}

	prev_opcode_pos--;
}

void ScriptCompiler::ClearPrevOpcode()
{
	prev_opcodes[ prev_opcode_pos ].opcode = OP_PREVIOUS;
}

void ScriptCompiler::AddBreakJumpLocation( unsigned char *pos )
{
	if( iBreakJumpLocCount < BREAK_JUMP_LOCATION_COUNT )
	{
		apucBreakJumpLocations[ iBreakJumpLocCount++ ] = pos;
	}
	else
	{
		iBreakJumpLocCount = 0;
		CompileError( -1, "Increase BREAK_JUMP_LOCATION_COUNT and recompile.\n" );
	}
}

void ScriptCompiler::AddContinueJumpLocation( unsigned char *pos )
{
	if( iContinueJumpLocCount < CONTINUE_JUMP_LOCATION_COUNT )
	{
		apucContinueJumpLocations[ iContinueJumpLocCount++ ] = pos;
	}
	else
	{
		iContinueJumpLocCount = 0;
		CompileError( -1, "Increase CONTINUE_JUMP_LOCATION_COUNT and recompile.\n" );
	}
}

void ScriptCompiler::AddJumpLocation( unsigned char *pos )
{
	unsigned int offset = code_pos - sizeof( unsigned int ) - pos;

	*reinterpret_cast< unsigned int * >( pos ) = offset;
	ClearPrevOpcode();
}

void ScriptCompiler::AddJumpBackLocation( unsigned char *pos )
{
	int offset = ( code_pos - pos );

	*reinterpret_cast< int * >( code_pos ) = offset;
	code_pos += sizeof( unsigned int );

	ClearPrevOpcode();
}

void ScriptCompiler::AddJumpToLocation( unsigned char *pos )
{
	int offset = ( pos - code_pos - 1 );

	*reinterpret_cast< unsigned int * >( code_pos ) = offset;
	code_pos += sizeof( unsigned int );

	ClearPrevOpcode();
}

bool ScriptCompiler::BuiltinReadVariable( unsigned int sourcePos, int type, int eventnum )
{
	ClassDef *c;
	EventDef *def;

	switch( type )
	{
	case method_game:
		c = NULL;
		break;

	case method_level:
		c = &Level::ClassInfo;
		break;

	case method_local:
		c = &ScriptThread::ClassInfo;
		break;

	case method_parm:
		c = &Parm::ClassInfo;
		break;

	case method_group:
		c = &ScriptClass::ClassInfo;
		break;

	default:
		return true;
	}

	def = c->GetDef( eventnum );

	if( def )
	{
		if( def->type == EV_GETTER )
		{
			return true;
		}
		else
		{
			CompileError( sourcePos, "Cannot get a write-only variable" );
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool ScriptCompiler::BuiltinWriteVariable( unsigned int sourcePos, int type, int eventnum )
{
	ClassDef *c;
	EventDef *def;

	switch( type )
	{
	case method_game:
		c = NULL;
		break;

	case method_level:
		c = &Level::ClassInfo;
		break;

	case method_local:
		c = &ScriptThread::ClassInfo;
		break;

	case method_parm:
		c = &Parm::ClassInfo;
		break;

	case method_group:
		c = &ScriptClass::ClassInfo;
		break;

	default:
		return true;
	}

	def = c->GetDef( eventnum );

	if( def )
	{
		if( def->type == EV_SETTER )
		{
			return true;
		}
		else
		{
			CompileError( sourcePos, "Cannot get a read-only variable" );
			return false;
		}
	}
	else
	{
		return false;
	}
}

void ScriptCompiler::EmitAssignmentStatement( sval_t lhs, unsigned int sourcePos )
{
	int eventnum;
	sval_t listener_val;
	char *name = lhs.node[ 2 ].stringValue;
	str name2 = name;
	name2.tolower();

	if( lhs.node[ 0 ].type != sval_field )
	{
		if( lhs.node[ 0 ].type == sval_array )
		{
			EmitRef( lhs.node[ 1 ], sourcePos );
			EmitValue( lhs.node[ 2 ] );
			EmitOpcode( OP_LOAD_ARRAY_VAR, lhs.node[ 3 ].sourcePosValue );

			return;
		}
		else
		{
			CompileError( sourcePos, "bad lvalue: %d (expecting field or array)", lhs.node[ 0 ].type );
		}
	}

	unsigned int index = Director.AddString( name );

	eventnum = Event::FindSetterEventNum( name2 );

	if( eventnum )
	{
		index = Director.GetString( name2 );
	}

	listener_val = lhs.node[ 1 ];

	if( listener_val.node[ 0 ].type != sval_store_method || ( eventnum && BuiltinWriteVariable( sourcePos, listener_val.node[ 1 ].byteValue, eventnum ) ) )
	{
		EmitValue( listener_val );
		EmitOpcode( OP_LOAD_FIELD_VAR, sourcePos );
	}
	else
	{
		EmitOpcode( OP_LOAD_GAME_VAR + listener_val.node[ 1 ].byteValue, sourcePos );
	}

	*reinterpret_cast< unsigned int * >( code_pos ) = index;
	code_pos += sizeof( unsigned int );
}

void ScriptCompiler::EmitBoolJumpFalse( unsigned int sourcePos )
{
	if( PrevOpcode() == OP_UN_CAST_BOOLEAN )
	{
		AbsorbPrevOpcode();
		EmitOpcode( OP_VAR_JUMP_FALSE4, sourcePos );
	}
	else
	{
		EmitOpcode( OP_BOOL_JUMP_FALSE4, sourcePos );
	}
}

void ScriptCompiler::EmitBoolJumpTrue( unsigned int sourcePos )
{
	if( PrevOpcode() == OP_UN_CAST_BOOLEAN )
	{
		AbsorbPrevOpcode();
		EmitOpcode( OP_VAR_JUMP_TRUE4, sourcePos );
	}
	else
	{
		EmitOpcode( OP_BOOL_JUMP_TRUE4, sourcePos );
	}
}

void ScriptCompiler::EmitBoolNot( unsigned int sourcePos )
{
	int prev = PrevOpcode();

	if( prev == OP_BOOL_STORE_TRUE )
	{
		AbsorbPrevOpcode();
		return EmitOpcode( OP_BOOL_STORE_FALSE, sourcePos );
	}
	else if( prev > OP_BOOL_STORE_TRUE && prev == OP_BOOL_UN_NOT )
	{
		AbsorbPrevOpcode();
		return EmitNil( sourcePos );
	}
	else if( prev == OP_BOOL_STORE_FALSE )
	{
		AbsorbPrevOpcode();
		return EmitOpcode( OP_BOOL_STORE_TRUE, sourcePos );
	}

	return EmitOpcode( OP_BOOL_UN_NOT, sourcePos );
}

void ScriptCompiler::EmitBoolToVar( unsigned int sourcePos )
{
	if( PrevOpcode() == OP_UN_CAST_BOOLEAN )
	{
		AbsorbPrevOpcode();
		EmitOpcode( OP_UN_CAST_BOOLEAN, sourcePos );
	}
	else
	{
		if( showopcodes->integer )
		{
			glbs.DPrintf( "\t\t%08d: %s (%d)\n", code_pos - code_ptr, OpcodeName( OP_BOOL_TO_VAR ), m_iVarStackOffset );
		}

		int pos = ( prev_opcode_pos + 1 ) % 100;

		prev_opcode_pos = pos;
		prev_opcodes[ pos ].opcode = OP_BOOL_TO_VAR;
		prev_opcodes[ pos ].VarStackOffset = 0;
		prev_opcodes[ ( pos + 1 ) % 100 ].opcode = OP_PREVIOUS;
	}
}

void ScriptCompiler::EmitBreak( unsigned int sourcePos )
{
	if( bCanBreak )
	{
		EmitOpcode( OP_JUMP4, sourcePos );
		unsigned char *pos = code_pos;
		code_pos += 4;
		ClearPrevOpcode();

		AddBreakJumpLocation( pos );
	}
	else
	{
		CompileError( sourcePos, "illegal break\n" );
	}
}

void ScriptCompiler::EmitCatch( sval_t val, unsigned char *try_begin_code_pos, unsigned int sourcePos )
{
	unsigned char *old_code_pos;
	StateScript *m_oldStateScript;

	EmitOpcode( OP_JUMP4, sourcePos );

	old_code_pos = code_pos;
	code_pos += sizeof( unsigned int );

	ClearPrevOpcode();

	m_oldStateScript = stateScript;
	stateScript = script->CreateCatchStateScript( try_begin_code_pos, code_pos );

	EmitValue( val );

	stateScript = m_oldStateScript;

	AddJumpLocation( old_code_pos );
}

void ScriptCompiler::EmitConstArray( sval_t lhs, sval_t rhs, unsigned int sourcePos )
{
	int iCount = 1;

	while( 1 )
	{
		iCount++;

		EmitValue( lhs );

		if( rhs.node[ 0 ].type != sval_constarray ) {
			break;
		}

		lhs = rhs.node[ 1 ];
		rhs = rhs.node[ 2 ];
	}

	EmitValue( rhs );
	EmitConstArrayOpcode( iCount );
}

void ScriptCompiler::EmitConstArrayOpcode( int iCount )
{
	/*if( iCount > 255 )
	{
		CompileError( -1, "Max const array parameters exceeded" );
	}
	else
	{
		SetOpcodeVarStackOffset( OP_LOAD_CONST_ARRAY1, 1 - iCount );
		EmitOpcode( OP_LOAD_CONST_ARRAY1, -1 );

		*code_pos++ = iCount;
	}*/

	SetOpcodeVarStackOffset( OP_LOAD_CONST_ARRAY1, 1 - iCount );
	EmitOpcode( OP_LOAD_CONST_ARRAY1, -1 );

	*reinterpret_cast< short * >( code_pos ) = iCount;
	code_pos += sizeof( short );
}

void ScriptCompiler::EmitContinue( unsigned int sourcePos )
{
	if( bCanContinue )
	{
		EmitOpcode( OP_JUMP4, sourcePos );
		unsigned char *pos = code_pos;
		code_pos += sizeof( unsigned int );
		ClearPrevOpcode();

		AddContinueJumpLocation( pos );
	}
	else
	{
		CompileError( sourcePos, "illegal continue\n" );
	}
}

void ScriptCompiler::EmitDoWhileJump( sval_t while_stmt, sval_t while_expr, unsigned int sourcePos )
{
	unsigned char *pos = code_pos;
	int label1, label2;

	if( showopcodes->integer )
	{
		label1 = current_label++;
		glbs.DPrintf( "<LABEL%d>:\n", label1 );
	}

	ClearPrevOpcode();

	bool old_bCanBreak = bCanBreak;
	bool old_bCanContinue = bCanContinue;
	int breakCount = iBreakJumpLocCount;
	int continueCount = iContinueJumpLocCount;

	bCanBreak = true;
	bCanContinue = true;

	EmitValue( while_stmt );

	ProcessContinueJumpLocations( continueCount );

	bCanContinue = old_bCanContinue;

	EmitValue( while_expr );
	EmitVarToBool( sourcePos );

	label2 = EmitNot( sourcePos );

	unsigned char *jmp = code_pos;

	code_pos += sizeof( unsigned int );

	if( showopcodes->integer )
	{
		glbs.DPrintf( "JUMP_BACK4 <LABEL%d>\n", label1 );
	}

	EmitJumpBack( pos, sourcePos );

	ClearPrevOpcode();

	if( showopcodes->integer )
	{
		glbs.DPrintf( "<LABEL%d>:\n", label2 );
	}

	AddJumpLocation( jmp );

	ProcessBreakJumpLocations( breakCount );

	bCanBreak = old_bCanBreak;
}

void ScriptCompiler::EmitEof( unsigned int sourcePos )
{
	if( PrevOpcode() ) {
		EmitOpcode( OP_DONE, -1 );
	}
}

void ScriptCompiler::EmitField( sval_t listener_val, sval_t field_val, unsigned int sourcePos )
{
	unsigned int eventnum = 0;
	unsigned int index = -1;
	unsigned int prev_index;

	if( field_val.node[ 0 ].stringValue )
	{
		str name = field_val.stringValue;
		str name2 = field_val.stringValue;
		name2.tolower();

		index = Director.AddString( name );
		eventnum = Event::FindGetterEventNum( name2 );

		if( eventnum )
		{
			index = Director.GetString( name2 );
		}
	}

	prev_index = *reinterpret_cast< unsigned int * >( code_pos - sizeof( unsigned int ) );

	if( listener_val.node[ 0 ].type != sval_store_method || ( eventnum && BuiltinReadVariable( sourcePos, listener_val.node[ 1 ].byteValue , eventnum ) ) )
	{
		EmitValue( listener_val );
		EmitOpcode( OP_STORE_FIELD, sourcePos );

		*reinterpret_cast< unsigned int * >( code_pos ) = index;
	}
	else if( PrevOpcode() != ( OP_LOAD_GAME_VAR + listener_val.node[ 1 ].byteValue ) || prev_index != index )
	{
		EmitOpcode( OP_STORE_GAME_VAR + listener_val.node[ 1 ].byteValue, sourcePos );

		*reinterpret_cast< unsigned int * >( code_pos ) = index;
	}
	else
	{
		AbsorbPrevOpcode();
		EmitOpcode( OP_LOAD_STORE_GAME_VAR + listener_val.node[ 1 ].byteValue, sourcePos );
	}

	code_pos += sizeof( unsigned int );
}

void ScriptCompiler::EmitFloat( float value, unsigned int sourcePos )
{
	// ley0k: optimization
	if( value == ( float )( int )value )
	{
		return EmitInteger( ( unsigned int )value, sourcePos );
	}

	EmitOpcode( OP_STORE_FLOAT, sourcePos );

	*reinterpret_cast< float * >( code_pos ) = value;
	code_pos += sizeof( float );
}

void ScriptCompiler::EmitFunc1( int opcode, unsigned int sourcePos )
{
	if( opcode == OP_UN_MINUS )
	{
		ScriptVariable var;

		if( EvalPrevValue( var ) )
		{
			AbsorbPrevOpcode();
			var.minus();

			return EmitValue( var, sourcePos );
		}
	}

	EmitOpcode( opcode, sourcePos );
}

void ScriptCompiler::EmitFunction( int iParamCount, sval_t val, unsigned int sourcePos )
{
	char *p = val.stringValue;

	str filename;
	str label;
	bool found = false;

	while( *p )
	{
		if( p[ 0 ] == ':' && p[ 1 ] == ':' )
		{
			p[ 0 ] = 0;

			filename = val.stringValue;
			label = p + 2;

			found = true;

			break;
		}

		p++;
	}

	SetOpcodeVarStackOffset( OP_FUNC, -iParamCount );
	EmitOpcode( OP_FUNC, sourcePos );

	if( !found )
	{
		label = val.stringValue;

		*code_pos++ = false;
		*reinterpret_cast< unsigned int * >( code_pos ) = Director.AddString( label );
	}
	else
	{
		*code_pos++ = true;
		*reinterpret_cast< unsigned int * >( code_pos ) = Director.AddString( filename );
		code_pos += sizeof( unsigned int );
		*reinterpret_cast< unsigned int * >( code_pos ) = Director.AddString( label );
	}

	code_pos += sizeof( unsigned int );

	*code_pos++ = iParamCount;
}

void ScriptCompiler::EmitIfElseJump( sval_t if_stmt, sval_t else_stmt, unsigned int sourcePos )
{
	unsigned char *jmp1, *jmp2;
	int label1, label2;

	label1 = EmitNot( sourcePos );
	jmp1 = code_pos;
	code_pos += sizeof( unsigned int );
	ClearPrevOpcode();

	EmitValue( if_stmt );

	if( showopcodes->integer )
	{
		label2 = current_label++;
		glbs.DPrintf( "JUMP <LABEL%d>\n", label2 );
	}

	EmitOpcode( OP_JUMP4, sourcePos );
	jmp2 = code_pos;
	code_pos += sizeof( unsigned int );

	ClearPrevOpcode();

	if( showopcodes->integer )
	{
		glbs.DPrintf( "<LABEL%d>:\n", label1 );
	}

	AddJumpLocation( jmp1 );
	EmitValue( else_stmt );

	if( showopcodes->integer )
	{
		glbs.DPrintf( "<LABEL%d>:\n", label1 );
	}

	AddJumpLocation( jmp2 );
}

void ScriptCompiler::EmitIfJump( sval_t if_stmt, unsigned int sourcePos )
{
	unsigned char *jmp;

	int label = EmitNot( sourcePos );
	jmp = code_pos;
	code_pos += sizeof( unsigned int );

	ClearPrevOpcode();

	EmitValue( if_stmt );

	if( showopcodes->integer )
	{
		glbs.DPrintf( "<LABEL%d>:\n", label );
	}

	AddJumpLocation( jmp );
}

void ScriptCompiler::EmitInteger( unsigned int value, unsigned int sourcePos )
{
	if( value == 0 )
	{
		EmitOpcode( OP_STORE_INT0, sourcePos );
	}
	else if( value < 127 )
	{
		EmitOpcode( OP_STORE_INT1, sourcePos );

		*code_pos++ = value;
	}
	else if( value < 32767 )
	{
		EmitOpcode( OP_STORE_INT2, sourcePos );

		*reinterpret_cast< short * >( code_pos ) = value;
		code_pos += sizeof( short );
	}
	else if( value < 8388607 )
	{
		EmitOpcode( OP_STORE_INT3, sourcePos );

		*reinterpret_cast< short3 * >( code_pos ) = value;
		code_pos += sizeof( short3 );
	}
	else
	{
		EmitOpcode( OP_STORE_INT4, sourcePos );

		*reinterpret_cast< int * >( code_pos ) = value;
		code_pos += sizeof( int );
	}
}

void ScriptCompiler::EmitJump( unsigned char *pos, unsigned int sourcePos )
{
	EmitOpcode( OP_JUMP4, sourcePos );
	AddJumpToLocation( pos );
}

void ScriptCompiler::EmitJumpBack( unsigned char *pos, unsigned int sourcePos )
{
	EmitOpcode( OP_JUMP_BACK4, sourcePos );
	AddJumpBackLocation( pos );
}

void ScriptCompiler::EmitLabel( str name, unsigned int sourcePos )
{
	if( showopcodes->integer )
	{
		glbs.DPrintf( "<%s>:\n", name.c_str() );
	}

	if( stateScript->AddLabel( name, code_pos ) )
	{
		CompileError( sourcePos, "Duplicate label '%s'", name.c_str() );
	}
}

void ScriptCompiler::EmitLabelParameterList( sval_t parameter_list, unsigned int sourcePos )
{
	sval_u param;

	ClearPrevOpcode();

	param = parameter_list.node[ 0 ];

	if( param.node && param.node[ 0 ].node[ 0 ].type != sval_none )
	{
		EmitOpcode( OP_MARK_STACK_POS, sourcePos );

		while( param.node )
		{
			EmitParameter( param.node[ 0 ], param.node[ 2 ].sourcePosValue );
			param = param.node[ 1 ];
		}

		EmitOpcode( OP_RESTORE_STACK_POS, sourcePos );
	}
}

void ScriptCompiler::EmitLabelPrivate( str name, unsigned int sourcePos )
{
	if( showopcodes->integer )
	{
		glbs.DPrintf( "<%s>:\n", name.c_str() );
	}

	if( stateScript->AddLabel( name, code_pos, true ) )
	{
		CompileError( sourcePos, "Duplicate label '%s'", name.c_str() );
	}
}

void ScriptCompiler::EmitLogicJump( sval_t logic_stmt, bool isOr, unsigned int sourcePos )
{
	unsigned char *jmp;
	int label;

	if( showopcodes->integer )
	{
		label = current_label++;

		if( isOr )
		{
			glbs.DPrintf( "BOOL_LOGICAL_OR <LABEL%d>\n", label );
		}
		else
		{
			glbs.DPrintf( "BOOL_LOGICAL_AND <LABEL%d>\n", label );
		}
	}

	EmitOpcode( OP_BOOL_LOGICAL_AND + isOr, sourcePos );
	jmp = code_pos;
	code_pos += sizeof( unsigned int );

	ClearPrevOpcode();

	EmitValue( logic_stmt );
	EmitVarToBool( sourcePos );

	if( showopcodes->integer )
	{
		glbs.DPrintf( "<LABEL%d>:\n", label );
	}

	AddJumpLocation( jmp );
	EmitBoolToVar( sourcePos );
}

void ScriptCompiler::EmitMakeArray( sval_t val )
{
	int iCount = 0;
	sval_t *node;

	for( node = val.node[ 0 ].node; node != NULL; iCount++, node = node[ 1 ].node )
	{
		EmitValue( node[ 0 ] );
	}

	EmitConstArrayOpcode( iCount );
}

void ScriptCompiler::EmitMethodExpression( int iParamCount, int eventnum, unsigned int sourcePos )
{
	if( iParamCount > 5 )
	{
		SetOpcodeVarStackOffset( OP_EXEC_METHOD_COUNT1, -iParamCount );
		EmitOpcode( OP_EXEC_METHOD_COUNT1, sourcePos );

		*code_pos++ = iParamCount;
	}
	else
	{
		EmitOpcode( OP_EXEC_METHOD0 + iParamCount, sourcePos );
	}

	*reinterpret_cast< unsigned int * >( code_pos ) = eventnum;
	code_pos += sizeof( unsigned int );
}

void ScriptCompiler::EmitNil( unsigned int sourcePos )
{
	if( showopcodes->integer )
	{
		glbs.DPrintf( "\t\t%08d:\n", code_pos - code_ptr );
	}
}

void ScriptCompiler::EmitNop()
{
	EmitOpcode( OP_NOP, -1 );
}

int ScriptCompiler::EmitNot( unsigned int sourcePos )
{
	int label = 0;

	if( PrevOpcode() == OP_BOOL_UN_NOT )
	{
		AbsorbPrevOpcode();

		if( showopcodes->integer )
		{
			label = current_label++;
			glbs.DPrintf( "BOOL_JUMP_TRUE <LABEL%d>\n", label );
		}

		EmitBoolJumpTrue( sourcePos );
	}
	else
	{
		if( showopcodes->integer )
		{
			label = current_label++;
			glbs.DPrintf( "BOOL_JUMP_FALSE <LABEL%d>\n", label );
		}

		EmitBoolJumpFalse( sourcePos );
	}

	return label;
}

void ScriptCompiler::EmitOpcode( int opcode, unsigned int sourcePos )
{
	int IsExternal;
	int iVarStackOffset;

	if( code_pos == NULL )
	{
		Com_Printf( "Compiler not initialized !\n" );
		return;
	}

	if( script->m_ProgToSource )
	{
		str sourceLine;
		sourceinfo_t info;

		info.sourcePos = sourcePos;

		script->GetSourceAt( sourcePos, sourceLine, info.column, info.line );
		script->m_ProgToSource->addKeyValue( code_pos ) = info;
	}

	IsExternal = IsExternalOpcode( opcode );
	iVarStackOffset = OpcodeVarStackOffset( opcode );

	if( IsExternal )
	{
		if( m_iVarStackOffset > m_iMaxExternalVarStackOffset ) {
			m_iMaxExternalVarStackOffset = m_iVarStackOffset;
		}

		m_iHasExternal = 1;
	}

	m_iVarStackOffset += iVarStackOffset;

	if( !IsExternal )
	{
		if( m_iVarStackOffset > m_iInternalMaxVarStackOffset ) {
			m_iInternalMaxVarStackOffset = m_iVarStackOffset;
		}
	}

	/*if( m_iInternalMaxVarStackOffset + 9 * m_iMaxExternalVarStackOffset + 1 > 255 )
	{
		CompileError( sourcePos,
			"The required variable stack size of %d exceeds the statically allocated variable stack of size %d.\nIncrease SCRIPTTHREAD_VARSTACK_SIZE to at least %d and recompile.\n",
			m_iInternalMaxVarStackOffset + 9 * m_iMaxExternalVarStackOffset + 1,
			255,
			m_iInternalMaxVarStackOffset + 9 * m_iMaxExternalVarStackOffset + 1
		);
	}*/

	if( showopcodes->integer )
	{
		glbs.DPrintf( "\t\t%08d: %s (%d) %s\n", code_pos - code_ptr, OpcodeName( opcode ), m_iVarStackOffset, IsExternal ? "[external]" : "" );
	}

	prev_opcode_pos = ( prev_opcode_pos + 1 ) % 100;
	prev_opcodes[ prev_opcode_pos ].opcode = opcode;
	prev_opcodes[ prev_opcode_pos ].VarStackOffset = iVarStackOffset;
	prev_opcodes[ ( prev_opcode_pos + 1 ) % 100 ].opcode = OP_PREVIOUS;

	*code_pos++ = opcode;
}

void ScriptCompiler::EmitParameter( sval_t lhs, unsigned int sourcePos )
{
	if( lhs.node[ 0 ].type != sval_field )
	{
		CompileError( sourcePos, "bad parameter lvalue: %d (expecting field)", lhs.node[ 0 ].type );
	}

	sval_u listener_val = lhs.node[ 1 ];
	const char *name = lhs.node[ 2 ].stringValue;
	unsigned int index = Director.AddString( name );

	int eventnum = Event::FindSetterEventNum( name );

	if( listener_val.node[ 0 ].type != sval_store_method || ( eventnum && BuiltinWriteVariable( sourcePos, listener_val.node[ 1 ].byteValue, eventnum ) ) )
	{
		CompileError( sourcePos, "built-in field '%s' not allowed", name );
	}
	else
	{
		EmitOpcode( OP_STORE_PARAM, sourcePos );
		EmitOpcode( OP_LOAD_GAME_VAR + listener_val.node[ 1 ].byteValue, sourcePos );

		*reinterpret_cast< unsigned int * >( code_pos ) = index;
		code_pos += sizeof( unsigned int );
	}
}

int ScriptCompiler::EmitParameterList( sval_t event_parameter_list )
{
	sval_t *node;
	int iParamCount = 0;

	for( node = event_parameter_list.node[ 1 ].node[ 0 ].node; node != NULL; node = node[ 1 ].node )
	{
		if( node[ 0 ].node[ 0 ].type == sval_none )
		{
			continue;
		}

		EmitValue( *node );

		iParamCount++;
	}

	return iParamCount;
}

void ScriptCompiler::EmitRef( sval_t val, unsigned int sourcePos )
{
	unsigned int index;

	if( val.node[ 0 ].type != sval_field )
	{
		if( val.node[ 0 ].type == sval_array )
		{
			EmitRef( val.node[ 1 ], sourcePos );
			EmitValue( val.node[ 2 ] );
			EmitOpcode( OP_STORE_ARRAY_REF, val.node[ 3 ].sourcePosValue );

			return;
		}
		else
		{
			CompileError( sourcePos, "bad lvalue: %d (expecting field or array)", val.node[ 0 ].type );
			return;
		}
	}

	index = Director.AddString( val.node[ 2 ].stringValue );

	EmitValue( val.node[ 1 ] );
	EmitOpcode( OP_STORE_FIELD_REF, sourcePos );

	*reinterpret_cast< unsigned int * >( code_pos ) = index;
	code_pos += sizeof( unsigned int );
}

void ScriptCompiler::EmitStatementList( sval_t val )
{
	sval_t *node;

	for( node = val.node[ 0 ].node; node != NULL; node = node[ 1 ].node )
	{
		EmitValue( *node );
	}
}

void ScriptCompiler::EmitString( str value, unsigned int sourcePos )
{
	unsigned int index = Director.AddString( value );

	if( showopcodes->integer ) {
		glbs.DPrintf( "\t\tSTRING \"%s\"\n", value.c_str() );
	}

	EmitOpcode( OP_STORE_STRING, sourcePos );

	*reinterpret_cast< unsigned int * >( code_pos ) = index;
	code_pos += sizeof( unsigned int );
}

void ScriptCompiler::EmitSwitch( sval_t val, unsigned int sourcePos )
{
	bool bStartCanBreak;
	int iStartBreakJumpLocCount;
	StateScript *m_oldStateScript;

	m_oldStateScript = stateScript;
	stateScript = script->CreateSwitchStateScript();

	EmitOpcode( OP_SWITCH, sourcePos );

	*reinterpret_cast< StateScript ** >( code_pos ) = stateScript;
	code_pos += sizeof( unsigned int );

	bStartCanBreak = bCanBreak;
	iStartBreakJumpLocCount = iBreakJumpLocCount;

	bCanBreak = true;

	EmitBreak( sourcePos );

	EmitValue( val );

	ProcessBreakJumpLocations( iStartBreakJumpLocCount );

	bCanBreak = bStartCanBreak;

	stateScript = m_oldStateScript;
}

void ScriptCompiler::EmitValue( ScriptVariable &var, unsigned int sourcePos )
{
	if( var.GetType() == VARIABLE_INTEGER )
	{
		EmitInteger( var.intValue(), sourcePos );
	}
	else if( var.GetType() == VARIABLE_FLOAT )
	{
		EmitFloat( var.floatValue(), sourcePos );
	}
}

void ScriptCompiler::EmitValue( sval_t val )
{
__emit:

	switch( val.node[ 0 ].type )
	{
	case sval_none:
		break;

	case sval_and:
	case sval_or:
		EmitValue( val.node[ 1 ] );
		EmitVarToBool( val.node[ 3 ].sourcePosValue );
		EmitLogicJump( val.node[ 2 ], val.node[ 0 ].type == sval_or, val.node[ 3 ].sourcePosValue );
		break;

	case sval_array:
		EmitValue( val.node[ 1 ] );
		EmitValue( val.node[ 2 ] );
		EmitOpcode( OP_STORE_ARRAY, val.node[ 3 ].sourcePosValue );
		break;

	case sval_assignment:
		EmitValue( val.node[ 2 ] );
		EmitAssignmentStatement( val.node[ 1 ], val.node[ 3 ].sourcePosValue );
		break;

	case sval_break:
		return EmitBreak( val.node[ 1 ].sourcePosValue );

	case sval_calc_vector:
		EmitValue( val.node[ 1 ] );
		EmitValue( val.node[ 2 ] );
		EmitValue( val.node[ 3 ] );
		EmitOpcode( OP_CALC_VECTOR, val.node[ 4 ].sourcePosValue );
		break;

	case sval_negative:
		val.node[ 1 ].intValue = -val.node[ 1 ].intValue;
	case sval_case:
	{
		sval_u case_parm = val.node[ 1 ];

		if( case_parm.node[ 0 ].type == sval_store_integer )
		{
			EmitLabel( case_parm.node[ 1 ].intValue, val.node[ 3 ].sourcePosValue );
		}
		else if( case_parm.node[ 0 ].type == sval_store_string )
		{
			EmitLabel( case_parm.node[ 1 ].stringValue, val.node[ 3 ].sourcePosValue );
		}
		else if( case_parm.node[ 0 ].type == sval_func1 && case_parm.node[ 1 ].byteValue == OP_UN_MINUS )
		{
			EmitLabel( -case_parm.node[ 2 ].node[ 1 ].intValue, val.node[ 3 ].sourcePosValue );
		}
		else
		{
			CompileError( val.node[ 3 ].sourcePosValue, "bad case value: %d (expected integer or string)", case_parm.node[ 0 ].type );
		}

		EmitLabelParameterList( val.node[ 2 ], val.node[ 3 ].sourcePosValue );

		break;
	}

	case sval_catch:
	{
		unsigned char *old_code_pos = code_pos;

		ClearPrevOpcode();
		EmitValue( val.node[ 1 ] );
		EmitCatch( val.node[ 2 ], old_code_pos, val.node[ 3 ].sourcePosValue );
		break;
	}

	case sval_cmd:
	{
		str cmd = val.node[ 1 ].stringValue;
		cmd.tolower();
		int eventnum = Event::FindNormalEventNum( cmd );
		int iParamCount = EmitParameterList( val.node[ 2 ] );

		if( !eventnum )
		{
			CompileError( val.node[ 3 ].sourcePosValue, "unknown command: %s", val.node[ 1 ].stringValue );
			
			/*EmitOpcode( OP_STORE_LOCAL, val.node[ 3 ].sourcePosValue );
			EmitFunction( iParamCount, val.node[ 1 ], val.node[ 3 ].sourcePosValue );
			EmitOpcode( OP_LOAD_LOCAL_VAR, val.node[ 3 ].sourcePosValue );

			*reinterpret_cast< unsigned int * >( code_pos ) = Director.AddString( "" );
			code_pos += sizeof( unsigned int );*/
		}
		else
		{
			if( iParamCount > 5 )
			{
				SetOpcodeVarStackOffset( OP_EXEC_CMD_COUNT1, -iParamCount );
				EmitOpcode( OP_EXEC_CMD_COUNT1, val.node[ 3 ].sourcePosValue );

				*code_pos++ = iParamCount;
			}
			else
			{
				EmitOpcode( OP_EXEC_CMD0 + iParamCount, val.node[ 3 ].sourcePosValue );
			}

			*reinterpret_cast< unsigned int * >( code_pos ) = eventnum;
			code_pos += sizeof( unsigned int );
		}
		break;
	}

	case sval_cmd_default_ret:
	{
		str cmd = val.node[ 1 ].stringValue;
		cmd.tolower();
		int eventnum = Event::FindReturnEventNum( cmd );
		int iParamCount = EmitParameterList( val.node[ 2 ] );

		EmitOpcode( OP_STORE_LOCAL, val.node[ 3 ].sourcePosValue );

		if( !eventnum )
		{
			CompileError( val.node[ 3 ].sourcePosValue, "unknown return command: %s", val.node[ 1 ].stringValue );
			//EmitFunction( iParamCount, val.node[ 1 ], val.node[ 3 ].sourcePosValue );
		}
		else
		{
			EmitMethodExpression( iParamCount, eventnum, val.node[ 3 ].sourcePosValue );
		}
		break;
	}

	case sval_cmd_method:
	{
		str cmd = val.node[ 2 ].stringValue;
		cmd.tolower();
		int eventnum = Event::FindNormalEventNum( cmd );
		int iParamCount = EmitParameterList( val.node[ 3 ] );

		EmitValue( val.node[ 1 ] );

		if( !eventnum )
		{
			CompileError( val.node[ 4 ].sourcePosValue, "unknown command: %s", val.node[ 2 ].stringValue );

			/*EmitFunction( iParamCount, val.node[ 2 ], val.node[ 4 ].sourcePosValue );
			EmitOpcode( OP_LOAD_LOCAL_VAR, val.node[ 3 ].sourcePosValue );

			*reinterpret_cast< unsigned int * >( code_pos ) = Director.AddString( "" );
			code_pos += sizeof( unsigned int );*/
		}
		else
		{
			if( iParamCount > 5 )
			{
				SetOpcodeVarStackOffset( OP_EXEC_CMD_METHOD_COUNT1, -iParamCount - 1 );
				EmitOpcode( OP_EXEC_CMD_METHOD_COUNT1, val.node[ 4 ].sourcePosValue );

				*code_pos++ = iParamCount;
			}
			else
			{
				EmitOpcode( OP_EXEC_CMD_METHOD0 + iParamCount, val.node[ 4 ].sourcePosValue );
			}

			*reinterpret_cast< unsigned int * >( code_pos ) = eventnum;
			code_pos += sizeof( unsigned int );
		}
		break;
	}

	case sval_cmd_method_ret:
	{
		str cmd = val.node[ 2 ].stringValue;
		cmd.tolower();
		int eventnum = Event::FindReturnEventNum( cmd );
		int iParamCount = EmitParameterList( val.node[ 3 ] );

		EmitValue( val.node[ 1 ] );

		if( !eventnum )
		{
			CompileError( val.node[ 4 ].sourcePosValue, "unknown return command: %s", val.node[ 2 ].stringValue );
			//EmitFunction( iParamCount ,val.node[ 2 ], val.node[ 4 ].sourcePosValue );
		}
		else
		{
			EmitMethodExpression( iParamCount, eventnum, val.node[ 4 ].sourcePosValue );
		}
		break;
	}

	case sval_constarray:
		return EmitConstArray( val.node[ 1 ], val.node[ 2 ], val.node[ 3 ].sourcePosValue );

	case sval_continue:
		return EmitContinue( val.node[ 1 ].sourcePosValue );

	case sval_do:
		return EmitDoWhileJump( val.node[ 1 ], val.node[ 2 ], val.node[ 3 ].sourcePosValue );

	case sval_field:
		return EmitField( val.node[ 1 ], val.node[ 2 ], val.node[ 3 ].sourcePosValue );

	case sval_func1:
		EmitValue( val.node[ 2 ] );
		EmitFunc1( val.node[ 1 ].byteValue, val.node[ 3 ].sourcePosValue );
		break;

	case sval_if:
		EmitValue( val.node[ 1 ] );
		EmitVarToBool( val.node[ 4 ].sourcePosValue );
		EmitIfJump( val.node[ 2 ], val.node[ 3 ].sourcePosValue );
		break;

	case sval_ifelse:
		EmitValue( val.node[ 1 ] );
		EmitVarToBool( val.node[ 4 ].sourcePosValue );
		EmitIfElseJump( val.node[ 2 ], val.node[ 3 ], val.node[ 4 ].sourcePosValue );
		break;

	case sval_label:
		EmitLabel( val.node[ 1 ].stringValue, val.node[ 3 ].sourcePosValue );
		EmitLabelParameterList( val.node[ 2 ], val.node[ 3 ].sourcePosValue );
		break;

	case sval_privatelabel:
		EmitLabelPrivate( val.node[ 1 ].stringValue, val.node[ 3 ].sourcePosValue );
		EmitLabelParameterList( val.node[ 2 ], val.node[ 3 ].sourcePosValue );
		break;

	case sval_makearray:
		return EmitMakeArray( val.node[ 1 ] );

	case sval_not:
		EmitValue( val.node[ 1 ] );
		EmitVarToBool( val.node[ 2 ].sourcePosValue );
		EmitBoolNot( val.node[ 2 ].sourcePosValue );
		EmitBoolToVar( val.node[ 2 ].sourcePosValue );
		break;

	case sval_operation:
		EmitValue( val.node[ 2 ] );
		EmitValue( val.node[ 3 ] );
		EmitOpcode( val.node[ 1 ].byteValue, val.node[ 4 ].sourcePosValue );
		break;

	case sval_statement_list:
		return EmitStatementList( val.node[ 1 ] );

	case sval_store_float:
		EmitFloat( val.node[ 1 ].floatValue, val.node[ 2 ].sourcePosValue );
		break;

	case sval_store_integer:
		EmitInteger( val.node[ 1 ].intValue, val.node[ 2 ].sourcePosValue );
		break;

	case sval_store_method:
		EmitOpcode( OP_STORE_GAME + val.node[ 1 ].byteValue, val.node[ 2 ].sourcePosValue );
		break;

	case sval_store_nil:
		EmitOpcode( OP_STORE_NIL, val.node[ 1 ].sourcePosValue );
		break;

	case sval_store_null:
		EmitOpcode( OP_STORE_NULL, val.node[ 1 ].sourcePosValue );
		break;

	case sval_store_string:
		EmitString( val.node[ 1 ].stringValue, val.node[ 2 ].sourcePosValue );
		break;

	case sval_switch:
		EmitValue( val.node[ 1 ] );
		EmitSwitch( val.node[ 2 ], val.node[ 3 ].sourcePosValue );
		break;

	case sval_next:
		val = val.node[ 1 ];
		goto __emit; // prevent stack overflow

	case sval_while:
		return EmitWhileJump( val.node[ 1 ], val.node[ 2 ], val.node[ 3 ], val.node[ 4 ].sourcePosValue );

	default:
		CompileError( -1, "unknown type %d\n", val.node[ 0 ].type );
		break;
	}
}

void ScriptCompiler::EmitVarToBool( unsigned int sourcePos )
{
	int prev = PrevOpcode();

	if( prev == OP_STORE_INT0 )
	{
		AbsorbPrevOpcode();
		return EmitOpcode( OP_BOOL_STORE_FALSE, sourcePos );
	}
	else if( prev > OP_STORE_INT0 )
	{
		if( prev <= OP_STORE_INT4 )
		{
			AbsorbPrevOpcode();
			return EmitOpcode( OP_BOOL_STORE_TRUE, sourcePos );
		}
	}
	else if( prev == OP_BOOL_TO_VAR )
	{
		AbsorbPrevOpcode();
		return EmitNil( sourcePos );
	}

	return EmitOpcode( OP_UN_CAST_BOOLEAN, sourcePos );
}

void ScriptCompiler::EmitWhileJump( sval_t while_expr, sval_t while_stmt, sval_t inc_stmt, unsigned int sourcePos )
{
	unsigned char *pos = code_pos;
	int label1, label2;

	if( showopcodes->integer )
	{
		label1 = current_label++;
		glbs.DPrintf( "<LABEL%d>:\n", label1 );
	}

	ClearPrevOpcode();

	EmitValue( while_expr );
	EmitVarToBool( sourcePos );

	label2 = EmitNot( sourcePos );
	unsigned char *jmp = code_pos;
	code_pos += sizeof( unsigned int );
	ClearPrevOpcode();

	bool old_bCanBreak = bCanBreak;
	bool old_bCanContinue = bCanContinue;
	int breakCount = iBreakJumpLocCount;
	int continueCount = iContinueJumpLocCount; 

	bCanBreak = true;
	bCanContinue = true;

	EmitValue( while_stmt );
	ProcessContinueJumpLocations( continueCount );

	bCanContinue = old_bCanContinue;

	EmitValue( inc_stmt );

	if( showopcodes->integer )
	{
		glbs.DPrintf( "JUMP_BACK4 <LABEL%d>\n", label1 );
	}

	EmitJumpBack( pos, sourcePos );

	ClearPrevOpcode();

	if( showopcodes->integer )
	{
		glbs.DPrintf( "<LABEL%d>:\n", label2 );
	}

	AddJumpLocation( jmp );

	ProcessBreakJumpLocations( breakCount );

	bCanBreak = old_bCanBreak;
}

bool ScriptCompiler::EvalPrevValue( ScriptVariable& var )
{
	int intValue = 0;
	float floatValue = 0.0f;

	switch( PrevOpcode() )
	{
	case OP_STORE_INT0:
		intValue = 0;
		break;

	case OP_STORE_INT1:
		intValue = *( code_pos - sizeof( char ) );
		break;

	case OP_STORE_INT2:
		intValue = *reinterpret_cast< short * >( code_pos - sizeof( short ) );
		break;

	case OP_STORE_INT3:
		intValue = *reinterpret_cast< short3 * >( code_pos - sizeof( short3 ) );
		break;

	case OP_STORE_INT4:
		intValue = *reinterpret_cast< int * >( code_pos - sizeof( int ) );
		break;

	case OP_STORE_FLOAT:
		floatValue = *reinterpret_cast< float * >( code_pos - sizeof( float ) );
		var.setFloatValue( floatValue );
		return true;

	default:
		return false;
	}

	var.setIntValue( intValue );

	return true;
}

void ScriptCompiler::OptimizeInstructions( unsigned char *code, unsigned char *op1, unsigned char *op2 )
{
	int intValue1 = 0, intValue2 = 0, intValue3 = 0;
	float floatValue1 = 0.0f, floatValue2 = 0.0f, floatValue3 = 0.0f;
	int type1, type2;

	if( !( *op1 >= OP_STORE_INT0 && *op1 <= OP_STORE_FLOAT &&
		*op2 >= OP_STORE_INT0 && *op2 <= OP_STORE_FLOAT
		) )
	{
		return;
	}

	if( *op1 >= OP_STORE_INT0 && *op1 <= OP_STORE_INT4 )
	{
		memcpy( &intValue1, op1 + 1, *op1 - OP_STORE_INT0 );
		type1 = OP_STORE_INT4;
	}
	else
	{
		type1 = *op1;
	}

	if( *op2 >= OP_STORE_INT0 && *op2 <= OP_STORE_INT4 )
	{
		memcpy( &intValue2, op2 + 1, *op2 - OP_STORE_INT0 );
		type2 = OP_STORE_INT4;
	}
	else
	{
		type2 = *op2;
	}

	if( *op1 == OP_STORE_FLOAT )
	{
		memcpy( &floatValue1, op1 + 1, sizeof( float ) );
	}

	if( *op2 == OP_STORE_FLOAT )
	{
		memcpy( &floatValue2, op2 + 1, sizeof( float ) );
	}

	*op1 = OP_NOP;
	*op2 = OP_NOP;

	switch( type1 + type2 * OP_MAX )
	{
	case OP_STORE_INT4 + OP_STORE_INT4 * OP_MAX:
		*op1 = OP_STORE_INT4;
		*( unsigned int * )( op1 + 1 ) = OptimizeValue( intValue1, intValue2, *code );
		break;
	}

	*code = OP_NOP;
}

int ScriptCompiler::OptimizeValue( int val1, int val2, unsigned char opcode )
{
	switch( opcode )
	{
	case OP_BIN_BITWISE_AND:
		return val1 & val2;
	case OP_BIN_BITWISE_OR:
		return val1 | val2;
	case OP_BIN_BITWISE_EXCL_OR:
		return val1 ^ val2;
	case OP_BIN_EQUALITY:
		return val1 == val2;
	case OP_BIN_INEQUALITY:
		return val1 != val2;
	case OP_BIN_LESS_THAN:
		return val1 > val2;
	case OP_BIN_GREATER_THAN:
		return val1 < val2;
	case OP_BIN_LESS_THAN_OR_EQUAL:
		return val1 >= val2;
	case OP_BIN_GREATER_THAN_OR_EQUAL:
		return val1 <= val2;
	case OP_BIN_PLUS:
		return val1 + val2;
	case OP_BIN_MINUS:
		return val1 - val2;
	case OP_BIN_MULTIPLY:
		return val1 * val2;
	case OP_BIN_DIVIDE:
		return val1 / val2;
	case OP_BIN_PERCENTAGE:
		return val1 % val2;
	default:
		return 0;
	}
}

void ScriptCompiler::ProcessBreakJumpLocations( int iStartBreakJumpLocCount )
{
	if( iBreakJumpLocCount > iStartBreakJumpLocCount )
	{
		do
		{
			iBreakJumpLocCount--;

			unsigned int offset = code_pos - sizeof( unsigned int ) - apucBreakJumpLocations[ iBreakJumpLocCount ];

			*reinterpret_cast< unsigned int * >( apucBreakJumpLocations[ iBreakJumpLocCount ] ) = offset;
		} while( iBreakJumpLocCount > iStartBreakJumpLocCount );

		ClearPrevOpcode();
	}
}

void ScriptCompiler::ProcessContinueJumpLocations( int iStartContinueJumpLocCount )
{
	if( iContinueJumpLocCount > iStartContinueJumpLocCount )
	{
		do
		{
			iContinueJumpLocCount--;

			unsigned int offset = code_pos - sizeof( unsigned int ) - apucContinueJumpLocations[ iContinueJumpLocCount ];

			*reinterpret_cast< unsigned int * >( apucContinueJumpLocations[ iContinueJumpLocCount ] ) = offset;
		} while( iContinueJumpLocCount > iStartContinueJumpLocCount );

		ClearPrevOpcode();
	}
}

unsigned char *ScriptCompiler::GetPosition()
{
	return code_pos;
}

void ScriptCompiler::CompileError( unsigned int sourcePos, const char *format, ... )
{
	char buffer[ 4000 ];
	va_list va;

	va_start( va, format );
	vsprintf( buffer, format, va );
	va_end( va );

	compileSuccess = false;

	glbs.DPrintf( "%s\n", buffer );
	script->PrintSourcePos( sourcePos, false );

	throw ScriptException( buffer );
}

int yyparse();

char *ScriptCompiler::Preprocess( char *sourceBuffer )
{
	// FIXME (ley0k): Preprocessor (#defines and #includes)

	return sourceBuffer;
}

void ScriptCompiler::Preclean( char *processedBuffer )
{

}

extern int prev_yylex;

int yyerror( const char *msg )
{
	//parsedata.pos -= yyleng;
	parsedata.exc.yylineno = prev_yylex != TOKEN_EOL ? yylineno : yylineno - 1;
	parsedata.exc.yytext = yytext;
	parsedata.exc.yytoken = msg;

	//str line = ScriptCompiler::GetLine( parsedata.sourceBuffer, parsedata.exc.yylineno );

	glbs.Printf( "parse error:\n%s:\n", parsedata.exc.yytoken.c_str() );

	parsedata.gameScript->PrintSourcePos( parsedata.pos - yyleng, false );
	parsedata.pos++;

	return 1;
}

size_t ScriptCompiler::Parse( GameScript *gameScript, char *sourceBuffer )
{
	parsedata = yyparsedata();

	parsedata.sourceBuffer = sourceBuffer;
	parsedata.gameScript = gameScript;
	parsedata.braces_count = 0;

	prev_yylex = 0;
	script = gameScript;
	stateScript = &gameScript->m_State;
	yy_scan_string( sourceBuffer );

	parsetree_init();

	try
	{
		if( yyparse() != 0 || parsedata.exc.yytoken != "" )
		{
			// an error occured

			yylex_destroy();

			if( !parsedata.exc.yytext )
			{
				if( parsedata.braces_count ) {
					glbs.DPrintf( "unmatching {} pair\n" );
				}
				else {
					glbs.DPrintf( "unexpected end of file found\n" );
				}
			}

			return 0;
		}
	}
	catch( ScriptException& exc )
	{
		exc;
		return 0;
	}

	yylex_destroy();

	return parsedata.total_length;
}

size_t ScriptCompiler::Compile( GameScript *gameScript, unsigned char *progBuffer )
{
	size_t length;

	if( progBuffer == NULL )
	{
		glbs.DPrintf( "Invalid program buffer\n" );
		return 0;
	}

	code_pos = progBuffer;
	code_ptr = progBuffer;
	prog_ptr = progBuffer;

	gameScript->m_ProgToSource = new con_set < unsigned char *, sourceinfo_t >;

	compileSuccess = true;

	prev_opcodes[ prev_opcode_pos ].opcode = OP_PREVIOUS;

	try
	{

		EmitValue( parsedata.val );
		EmitEof( -1 );

		if( compileSuccess )
		{
			stateScript->AddLabel( "", code_ptr );

			length = code_pos - code_ptr;
		}
		else
		{
			length = 0;
		}

		prog_end_ptr = code_pos;
	}
	catch( ScriptException& exc )
	{
		exc;
		length = 0;
		prog_end_ptr = code_pos;
	}

	// FIXME (ley0k): optimization can be worked

	//if( compileSuccess ) {
	//	Optimize( code_ptr );
	//}

	parsetree_freeall();

	return length;
}

void ScriptCompiler::Optimize( unsigned char *sourceBuffer )
{
	int length = code_pos - code_ptr;
	unsigned char *prevcodePos1 = NULL;
	unsigned char *prevcodePos2 = NULL;

	code_pos = sourceBuffer;

	if( !length ) {
		return;
	}

	while( code_pos < prog_end_ptr )
	{
		if( *code_pos >= OP_BIN_BITWISE_AND && *code_pos <= OP_BIN_PERCENTAGE )
		{
			assert( prevcodePos1 );
			assert( prevcodePos2 );

			OptimizeInstructions( code_pos, prevcodePos1, prevcodePos2 );
		}

		prevcodePos2 = prevcodePos1;
		prevcodePos1 = code_pos;

		code_pos += OpcodeLength( *code_pos );

		if( *code_pos == OP_DONE ) {
			break;
		}
	}
}

str ScriptCompiler::GetLine( str content, int line )
{
	char *p;
	str found;
	int i = 1;

	p = ( char * )content.c_str();

	while( *p )
	{
		if( *p == '\n' )
		{
			i++;
		}

		if( i >= line )
		{
			found = strtok( p, "\n" );
			break;
		}

		p++;
	}

	return found;
}

void CompileAssemble( const char *filename, const char *outputfile )
{
	GameScript *gameScript = Director.GetGameScript( filename );
	Archiver arc;

	if( !gameScript->m_ProgBuffer || !gameScript->m_ProgLength ) {
		return;
	}

	arc.Create( outputfile );

	if( !arc.NoErrors() )
	{
		glbs.DPrintf( "Error saving to file '%s'\n", filename );
		return;
	}

	gameScript->m_bPrecompiled = true;
	gameScript->Archive( arc );

	// dump the compiled script to file
	arc.Close();
}

bool GetCompiledScript( GameScript *scr )
{
	Archiver arc;

	arc.SetSilent( true );
	arc.Read( scr->Filename(), false );

	if( !arc.NoErrors() )
	{
		return false;
	}

	arc.SetSilent( false );

	scr->Archive( arc );

	arc.Close();

	return true;
}
