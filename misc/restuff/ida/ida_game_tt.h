#pragma once

#include "ida_game.h"
#include "ida_common_tt.h"

typedef struct gameExport_s
{
	int apiVersion;

	void ( *Init )( int svsStartTime, int randomSeed );
	void ( *Shutdown )( );
	void ( *Cleanup )( qboolean sameMap );
	void ( *Precache )( );
	void ( *SetMap )( const char *mapName );
	void ( *Restart )( );
	void ( *SetTime )( int svsStartTime, int svsTime );
	void ( *SpawnEntities )( const char *entities, int svsTime);
	const char *( *ClientConnect )( int clientNum, qboolean firstTime );
	void ( *ClientBegin )( gentity_t *ent, userCmd_t *cmd );
	void ( *ClientUserinfoChanged )( gentity_t *ent, const char *userInfo );
	void ( *ClientDisconnect )( gentity_t *ent );
	void ( *ClientCommand )( gentity_t *ent );
	void ( *ClientThink )( gentity_t *ent, userCmd_t *ucmd, userEyes_t *eyeInfo );
	void ( *BotBegin )( gentity_t *ent, userCmd_t *cmd );
	void ( *BotThink )( gentity_t *ent, userCmd_t *ucmd, userEyes_t *eyeInfo );
	void ( *PrepFrame )( );
	void ( *RunFrame )( int svsTime, int frameTime );
	void ( *ServerSpawned )( );
	void ( *RegisterSounds )( );
	qboolean ( *AllowPaused )( );
	qboolean ( *ConsoleCommand )( );
	void ( *ArchivePersistant )( const char *name, qboolean loading );
	void ( *WriteLevel )( const char *fileName, qboolean autoSave );
	qboolean ( *ReadLevel )( const char *fileName );
	qboolean ( *LevelArchiveValid )( const char *fileName );
	void ( *ArchiveInteger )( int *i );
	void ( *ArchiveFloat )( float *fl );
	void ( *ArchiveString )( char *s );
	void ( *ArchiveSvsTime )( int *pi );
	orientation_t ( *TIKI_Orientation )( gentity_t *edict, int num );
	void ( *DebugCircle )( float *org, float radius, float r, float g, float b, float alpha, qboolean horizontal );
	void ( *SetFrameNumber )( int frameNumber );
	void ( *SoundCallback )( int entNum, soundChannel_t channelNumber, const char *name );

	profGame_t *profStruct;
	gentity_t *gentities;
	int gentitySize;
	int numEntities;
	int maxEntities;
	char *errorMessage;

} gameExport_t;

typedef struct block_scriptclass_s {
  unsigned char data[256][48];
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
