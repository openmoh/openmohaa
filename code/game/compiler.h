/*
===========================================================================
Copyright (C) 2008 the OpenMoHAA team

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

// compiler.h: Script Compiler

#ifndef __COMPILER_H__
#define __COMPILER_H__

#include "glb_local.h"
#include <scriptopcodes.h>
#include <gamescript.h>
#include <parser/parsetree.h>

enum
{
	method_game,
	method_level,
	method_local,
	method_parm,
	method_self,
	method_group,
	method_owner,
	method_field,
	method_array,
};

typedef struct scriptmacro {
	str		name;
	str		parameters;
} scriptmacro_t;

#define BREAK_JUMP_LOCATION_COUNT		100
#define CONTINUE_JUMP_LOCATION_COUNT	100

class ScriptCompiler
{
public:
	unsigned char			*code_pos;
	unsigned char			*code_ptr;
	unsigned char			*prog_ptr;
	unsigned char			*prog_end_ptr;

	GameScript				*script;
	StateScript				*stateScript;

	bool					bCanBreak;
	bool					bCanContinue;

	opcode_info_t			prev_opcodes[ 100 ];
	unsigned int			prev_opcode_pos;

	int						m_iVarStackOffset;
	int						m_iInternalMaxVarStackOffset;
	int						m_iMaxExternalVarStackOffset;
	int						m_iMaxCallStackOffset;
	int						m_iHasExternal;

	unsigned char			*apucBreakJumpLocations[ BREAK_JUMP_LOCATION_COUNT ];
	int						iBreakJumpLocCount;
	unsigned char			*apucContinueJumpLocations[ CONTINUE_JUMP_LOCATION_COUNT ];
	int						iContinueJumpLocCount;

	bool					compileSuccess;

	static int				current_label;

public:
	ScriptCompiler();
	void				Reset();

	unsigned char		PrevOpcode();
	char				PrevVarStackOffset();
	void				AbsorbPrevOpcode();
	void				ClearPrevOpcode();

	void				AddBreakJumpLocation( unsigned char *pos );
	void				AddContinueJumpLocation( unsigned char *pos );
	void				AddJumpLocation( unsigned char *pos );
	void				AddJumpBackLocation( unsigned char *pos );
	void				AddJumpToLocation( unsigned char *pos );

	bool				BuiltinReadVariable( unsigned int sourcePos, int type, int eventnum );
	bool				BuiltinWriteVariable( unsigned int sourcePos, int type, int eventnum );

	void				EmitAssignmentStatement( sval_t lhs, unsigned int sourcePos );

	void				EmitBoolJumpFalse( unsigned int sourcePos );
	void				EmitBoolJumpTrue( unsigned int sourcePos );
	void				EmitBoolNot( unsigned int sourcePos );
	void				EmitBoolToVar( unsigned int sourcePos );

	void				EmitBreak( unsigned int sourcePos );
	void				EmitCatch( sval_t val, unsigned char *try_begin_code_pos, unsigned int sourcePos );
	void				EmitConstArray( sval_t lhs, sval_t rhs, unsigned int sourcePos );
	void				EmitConstArrayOpcode( int iCount );
	void				EmitContinue( unsigned int sourcePos );
	void				EmitDoWhileJump( sval_t while_stmt, sval_t while_expr, unsigned int sourcePos );
	void				EmitEof( unsigned int sourcePos );
	void				EmitField( sval_t listener_val, sval_t field_val, unsigned int sourcePos );
	void				EmitFloat( float value, unsigned int sourcePos );
	void				EmitFunc1( int opcode, unsigned int sourcePos );
	void				EmitFunction( int iParamCount, sval_t val, unsigned int sourcePos );
	void				EmitIfElseJump( sval_t if_stmt, sval_t else_stmt, unsigned int sourcePos );
	void				EmitIfJump( sval_t if_stmt, unsigned int sourcePos );
	void				EmitInteger( unsigned int value, unsigned int sourcePos );
	void				EmitJump( unsigned char *pos, unsigned int sourcePos );
	void				EmitJumpBack( unsigned char *pos, unsigned int sourcePos );
	void				EmitLabel( str name, unsigned int sourcePos );
	void				EmitLabelParameterList( sval_t parameter_list, unsigned int sourcePos );
	void				EmitLabelPrivate( str name, unsigned int sourcePos );
	void				EmitLogicJump( sval_t logic_stmt, bool isOr, unsigned int sourcePos );
	void				EmitMakeArray( sval_t val );
	void				EmitMethodExpression( int iParamCount, int eventnum, unsigned int sourcePos );
	void				EmitNil( unsigned int sourcePos );
	void				EmitNop();
	int					EmitNot( unsigned int sourcePos );
	void				EmitOpcode( int opcode, unsigned int sourcePos );
	void				EmitParameter( sval_u lhs, unsigned int sourcePos );
	int					EmitParameterList( sval_t event_parameter_list );
	void				EmitRef( sval_t val, unsigned int sourcePos );
	void				EmitStatementList( sval_t val );
	void				EmitString( str value, unsigned int sourcePos );
	void				EmitSwitch( sval_t val, unsigned int sourcePos );
	void				EmitValue( sval_t val );
	void				EmitValue( ScriptVariable& var, unsigned int sourcePos );
	void				EmitVarToBool( unsigned int sourcePos );
	void				EmitWhileJump( sval_t while_expr, sval_t while_stmt, sval_t inc_stmt, unsigned int sourcePos );

	bool				EvalPrevValue( ScriptVariable& var );

	void				OptimizeInstructions( unsigned char *code, unsigned char *op1, unsigned char *op2 );
	int					OptimizeValue( int val1, int val2, unsigned char opcode );

	void				ProcessBreakJumpLocations( int iStartBreakJumpLocCount );
	void				ProcessContinueJumpLocations( int iStartContinueJumpLocCount );

	unsigned char		*GetPosition();

	// compile
	void				CompileError( unsigned int sourcePos, const char *format, ... );

	scriptmacro_t		*GetMacro( char *sourceLine );

	char				*Preprocess( char *sourceBuffer );
	void				Preclean( char *processedBuffer );
	size_t				Parse( GameScript *m_GameScript, char *sourceBuffer );
	size_t				Compile( GameScript *m_GameScript, unsigned char *progBuffer );
	void				Optimize( unsigned char *progBuffer );

	static str			GetLine( str content, int line );
};

extern ScriptCompiler Compiler;

void				CompileAssemble( const char *filename, const char *outputfile );
bool				GetCompiledScript( GameScript *scr );

#endif
