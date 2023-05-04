#pragma once

#include "ida_game.h"
#include "ida_common_ta.h"

typedef struct gclient_s
{
  playerState_t ps;
  int ping;
  client_persistant_t pers;
  float cmd_angles[3];
  int lastActiveTime;
  int activeWarning;

} gclient_t;

typedef struct ScriptMaster_s {
	Listener_t				baseListener;

	int						stackCount;			// 0x0014 (0020)

	ScriptVariable_t		avar_Stack[1025];	// 0x0018 (0024)
	Event_t					fastEvent[11];		// 0x2020 (8224)
	ScriptVariable_t		*pTop;				// 0x20FC (8444)

	unsigned int			cmdCount;			// 0x2100 (8448) - cmd count
	int						cmdTime;			// 0x2104 (8452) - Elapsed time since the maximum reached LOCALSTACK_SIZE
	int						maxTime;			// 0x2108 (8456) - Maximum time for LOCALSTACK_SIZE

	SafePtr2_t				m_PreviousThread;	// 0x210C (8460) - parm.previousthread
	SafePtr2_t				m_CurrentThread;	// 0x211C (8476) - Current thread

	con_map					m_GameScripts;
	Container_t				m_menus;
	con_timer				timingList;
	con_arrayset			StringDict;
	int						iPaused;
} ScriptMaster_t;

typedef struct ScriptVM_s {
	void			*next;

	struct ScriptThread_s	*m_Thread;
	ScriptClass_t	*m_ScriptClass;

	void			*m_Stack;

	ScriptVariable_t m_ReturnValue;

	unsigned char	*m_PrevCodePos;
	unsigned char	*m_CodePos;

	unsigned char	state;
	unsigned char	m_ThreadState;
} ScriptVM_t;

typedef struct Game_s {
	gclient_t		*clients;
	qboolean		autosaved;
	int				maxclients;
	int				maxentities;
} Game_t;

typedef struct block_scriptclass_s {
  unsigned char data[256][49];
  unsigned char prev_data[256];
  unsigned char next_data[256];
  short int free_data;
  short int used_data;
  struct block_scriptclass_s *prev_block;
  struct block_scriptclass_s *next_block;
} block_scriptclass_t;

typedef struct MEM_BlockAlloc_scriptclass_s {
  block_scriptclass_t *m_FreeBlock;
  block_scriptclass_t *m_StartUsedBlock;
  block_scriptclass_t *m_StartFullBlock;
  unsigned int m_BlockCount;
} MEM_BlockAlloc_scriptclass_t;

typedef struct MEM_BlockAlloc_enum_scriptclass_s {
	MEM_BlockAlloc_scriptclass_t     *m_Owner;
	block_scriptclass_t				*m_CurrentBlock;
	unsigned char						m_CurrentData;
	int									m_CurrentBlockType;
} MEM_BlockAlloc_enum_scriptclass_t;
