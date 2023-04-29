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

// scriptopcodes.cpp

#include "scriptopcodes.h"
#include "short3.h"
#include "vector.h"

static opcode_t OpcodeInfo[] =
{
		{ "OPCODE_EOF",								0,				0,			0 },
		{ "OPCODE_BOOL_JUMP_FALSE4",				5,				-1,			0 },
		{ "OPCODE_BOOL_JUMP_TRUE4",					5,				-1,			0 },
		{ "OPCODE_VAR_JUMP_FALSE4",					5,				-1,			0 },
		{ "OPCODE_VAR_JUMP_TRUE4",					5,				-1,			0 },

		{ "OPCODE_BOOL_LOGICAL_AND",				5,				-1,			0 },
		{ "OPCODE_BOOL_LOGICAL_OR",					5,				-1,			0 },
		{ "OPCODE_VAR_LOGICAL_AND",					5,				-1,			0 },
		{ "OPCODE_VAR_LOGICAL_OR",					5,				-1,			0 },

		{ "OPCODE_BOOL_TO_VAR",						0,				0,			0 },

		{ "OPCODE_JUMP4",							1 + sizeof( unsigned int ),		0,			0 },
		{ "OPCODE_JUMP_BACK4",						1 + sizeof( unsigned int ),		0,			0 },

		{ "OPCODE_STORE_INT0",						1,				1,			0 },
		{ "OPCODE_STORE_INT1",						1 + sizeof( char ),				1,			0 },
		{ "OPCODE_STORE_INT2",						1 + sizeof( short ),			1,			0 },
		{ "OPCODE_STORE_INT3",						1 + sizeof( short3 ),			1,			0 },
		{ "OPCODE_STORE_INT4",						1 + sizeof( int ),				1,			0 },

		{ "OPCODE_BOOL_STORE_FALSE",				1,				1,			0 },
		{ "OPCODE_BOOL_STORE_TRUE",					1,				1,			0 },

		{ "OPCODE_STORE_STRING",					1 + sizeof( unsigned int ),			1,			0 },
		{ "OPCODE_STORE_FLOAT",						1 + sizeof( float ),				1,			0 },
		{ "OPCODE_STORE_VECTOR",					1 + sizeof( Vector ),				1,			0 },
		{ "OPCODE_CALC_VECTOR",						1,				-2,			0 },
		{ "OPCODE_STORE_NULL",						1,				1,			0 },
		{ "OPCODE_STORE_NIL",						1,				1,			0 },

		{ "OPCODE_EXEC_CMD0",						5,				0,			1 },
		{ "OPCODE_EXEC_CMD1",						5,				-1,			1 },
		{ "OPCODE_EXEC_CMD2",						5,				-2,			1 },
		{ "OPCODE_EXEC_CMD3",						5,				-3,			1 },
		{ "OPCODE_EXEC_CMD4",						5,				-4,			1 },
		{ "OPCODE_EXEC_CMD5",						5,				-5,			1 },
		{ "OPCODE_EXEC_CMD_COUNT1",					6,				-128,		1 },

		{ "OPCODE_EXEC_CMD_METHOD0",				5,				-1,			1 },
		{ "OPCODE_EXEC_CMD_METHOD1",				5,				-2,			1 },
		{ "OPCODE_EXEC_CMD_METHOD2",				5,				-3,			1 },
		{ "OPCODE_EXEC_CMD_METHOD3",				5,				-4,			1 },
		{ "OPCODE_EXEC_CMD_METHOD4",				5,				-5,			1 },
		{ "OPCODE_EXEC_CMD_METHOD5",				5,				-6,			1 },
		{ "OPCODE_EXEC_CMD_METHOD_COUNT1",			6,				-128,		1 },

		{ "OPCODE_EXEC_METHOD0",					5,				0,			1 },
		{ "OPCODE_EXEC_METHOD1",					5,				-1,			1 },
		{ "OPCODE_EXEC_METHOD2",					5,				-2,			1 },
		{ "OPCODE_EXEC_METHOD3",					5,				-3,			1 },
		{ "OPCODE_EXEC_METHOD4",					5,				-4,			1 },
		{ "OPCODE_EXEC_METHOD5",					5,				-5,			1 },
		{ "OPCODE_EXEC_METHOD_COUNT1",				6,				-128,		1 },

		{ "OPCODE_LOAD_GAME_VAR",					5,				-1,			0 },
		{ "OPCODE_LOAD_LEVEL_VAR",					5,				-1,			0 },
		{ "OPCODE_LOAD_LOCAL_VAR",					5,				-1,			0 },
		{ "OPCODE_LOAD_PARM_VAR",					5,				-1,			0 },
		{ "OPCODE_LOAD_SELF_VAR",					5,				-1,			0 },
		{ "OPCODE_LOAD_GROUP_VAR",					5,				-1,			0 },
		{ "OPCODE_LOAD_OWNER_VAR",					5,				-1,			0 },
		{ "OPCODE_LOAD_FIELD_VAR",					5,				-2,			0 },
		{ "OPCODE_LOAD_ARRAY_VAR",					1,				-3,			0 },
		{ "OPCODE_LOAD_CONST_ARRAY1",				2,				-128,		0 },

		{ "OPCODE_STORE_FIELD_REF",					5,				0,			0 },
		{ "OPCODE_STORE_ARRAY_REF",					1,				-1,			0 },

		{ "OPCODE_MARK_STACK_POS",					1,				0,			0 },

		{ "OPCODE_STORE_PARAM",						1,				1,			0 },

		{ "OPCODE_RESTORE_STACK_POS",				1,				0,			0 },

		{ "OPCODE_LOAD_STORE_GAME_VAR",				5,				0,			0 },
		{ "OPCODE_LOAD_STORE_LEVEL_VAR",			5,				0,			0 },
		{ "OPCODE_LOAD_STORE_LOCAL_VAR",			5,				0,			0 },
		{ "OPCODE_LOAD_STORE_PARM_VAR",				5,				0,			0 },
		{ "OPCODE_LOAD_STORE_SELF_VAR",				5,				0,			0 },
		{ "OPCODE_LOAD_STORE_GROUP_VAR",			5,				0,			0 },
		{ "OPCODE_LOAD_STORE_OWNER_VAR",			5,				0,			0 },

		{ "OPCODE_STORE_GAME_VAR",					5,				1,			0 },
		{ "OPCODE_STORE_LEVEL_VAR",					5,				1,			0 },
		{ "OPCODE_STORE_LOCAL_VAR",					5,				1,			0 },
		{ "OPCODE_STORE_PARM_VAR",					5,				1,			0 },
		{ "OPCODE_STORE_SELF_VAR",					5,				1,			0 },
		{ "OPCODE_STORE_GROUP_VAR",					5,				1,			0 },
		{ "OPCODE_STORE_OWNER_VAR",					5,				1,			0 },
		{ "OPCODE_STORE_FIELD",						5,				0,			1 },
		{ "OPCODE_STORE_ARRAY",						1,				-1,			0 },
		{ "OPCODE_STORE_GAME",						1,				1,			0 },
		{ "OPCODE_STORE_LEVEL",						1,				1,			0 },
		{ "OPCODE_STORE_LOCAL",						1,				1,			0 },
		{ "OPCODE_STORE_PARM",						1,				1,			0 },
		{ "OPCODE_STORE_SELF",						1,				1,			0 },
		{ "OPCODE_STORE_GROUP",						1,				1,			0 },
		{ "OPCODE_STORE_OWNER",						1,				1,			0 },

		{ "OPCODE_BIN_BITWISE_AND",					1,				-1,			0 },
		{ "OPCODE_BIN_BITWISE_OR",					1,				-1,			0 },
		{ "OPCODE_BIN_BITWISE_EXCL_OR",				1,				-1,			0 },
		{ "OPCODE_BIN_EQUALITY",					1,				-1,			0 },
		{ "OPCODE_BIN_INEQUALITY",					1,				-1,			0 },
		{ "OPCODE_BIN_LESS_THAN",					1,				-1,			0 },
		{ "OPCODE_BIN_GREATER_THAN",				1,				-1,			0 },
		{ "OPCODE_BIN_LESS_THAN_OR_EQUAL",			1,				-1,			0 },
		{ "OPCODE_BIN_GREATER_THAN_OR_EQUAL",		1,				-1,			0 },
		{ "OPCODE_BIN_PLUS",						1,				-1,			0 },
		{ "OPCODE_BIN_MINUS",						1,				-1,			0 },
		{ "OPCODE_BIN_MULTIPLY",					1,				-1,			0 },
		{ "OPCODE_BIN_DIVIDE",						1,				-1,			0 },
		{ "OPCODE_BIN_PERCENTAGE",					1,				-1,			0 },

		{ "OPCODE_UN_MINUS",						1,				0,			0 },
		{ "OPCODE_UN_COMPLEMENT",					1,				0,			0 },
		{ "OPCODE_UN_TARGETNAME",					1,				0,			0 },
		{ "OPCODE_BOOL_UN_NOT",						1,				0,			0 },
		{ "OPCODE_VAR_UN_NOT",						1,				0,			0 },
		{ "OPCODE_UN_CAST_BOOLEAN",					1,				0,			0 },
		{ "OPCODE_UN_INC",							1,				0,			0 },
		{ "OPCODE_UN_DEC",							1,				0,			0 },
		{ "OPCODE_UN_SIZE",							1,				0,			0 },

		{ "OPCODE_SWITCH",							5,				-1,			0 },

		{ "OPCODE_FUNC",							11,				-128,		1 },

		{ "OPCODE_NOP",								1,				0,			0 },

		{ "OPCODE_BIN_SHIFT_LEFT",					1,				-1,			0 },
		{ "OPCODE_BIN_SHIFT_RIGHT",					1,				-1,			0 },

		{ "OPCODE_END",								1,				-1,			0 },
		{ "OPCODE_RETURN",							1,				-1,			0 },
};

static const char *aszVarGroupNames[] =
{
	"game",
	"level",
	"local",
	"parm",
	"self"
};

/*
====================
VarGroupName
====================
*/
const char *VarGroupName( int iVarGroup )
{
	return aszVarGroupNames[ iVarGroup ];
}

/*
====================
OpcodeName
====================
*/
const char *OpcodeName( int opcode )
{
	return OpcodeInfo[ opcode ].opcodename;
}

/*
====================
OpcodeLength
====================
*/
int OpcodeLength( int opcode )
{
	return OpcodeInfo[ opcode ].opcodelength;
}

/*
====================
OpcodeVarStackOffset
====================
*/
int OpcodeVarStackOffset( int opcode )
{
	return OpcodeInfo[ opcode ].opcodestackoffset;
}

/*
====================
SetOpcodeVarStackOffset
====================
*/
void SetOpcodeVarStackOffset( int opcode, int iVarStackOffset )
{
	OpcodeInfo[ opcode ].opcodestackoffset = iVarStackOffset;
}

/*
====================
IsExternalOpcode
====================
*/
bool IsExternalOpcode( int opcode )
{
	return OpcodeInfo[ opcode ].isexternal ? true : false;
}
