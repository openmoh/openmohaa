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

// gamescript.cpp : Subclass of script that preprocesses labels

#include "glb_local.h"
#include "gamescript.h"
#include "compiler.h"
#include "scriptmaster.h"

static unsigned char *current_progBuffer = NULL;

str& AbstractScript::Filename( void )
{
	return Director.GetString( m_Filename );
}

const_str AbstractScript::ConstFilename( void )
{
	return m_Filename;
}

bool AbstractScript::GetSourceAt( size_t sourcePos, str &sourceLine, int &column, int &line )
{
	if( !m_SourceBuffer || sourcePos >= m_SourceLength ) {
		return false;
	}

	line = 1;
	column = 0;
	int posLine = 0;

	char *p = m_SourceBuffer;
	char old_token;

	for( int i = 0; i < sourcePos; i++, p++ )
	{
		column++;

		if( *p == '\n' )
		{
			line++;
			column = 0;
			posLine = i + 1;
		}
		else if( *p == '\0' )
		{
			break;
		}
	}

	while( *p != '\0' && *p != '\n' )
	{
		p++;
	}

	old_token = *p;
	*p = '\0';

	sourceLine = ( m_SourceBuffer + posLine );

	*p = old_token;

	return true;
}

void AbstractScript::PrintSourcePos( sourceinfo_t *sourcePos, bool dev )
{
	int line;
	int column;
	str sourceLine;

	if( GetSourceAt( sourcePos->sourcePos, sourceLine, column, line ) )
	{
		PrintSourcePos( sourceLine, column, line, dev );
	}
	else
	{
		glbs.Printf( "file '%s', source pos %d line %d column %d:\n", Filename().c_str(), sourcePos->sourcePos, sourcePos->line, sourcePos->column );
	}
}

void AbstractScript::PrintSourcePos( size_t sourcePos, bool dev )
{
	int line;
	int column;
	str sourceLine;

	if( GetSourceAt( sourcePos, sourceLine, column, line ) )
	{
		PrintSourcePos( sourceLine, column, line, dev );
	}
	else
	{
		glbs.Printf( "file '%s', source pos %d:\n", Filename().c_str(), sourcePos );
	}
}

void AbstractScript::PrintSourcePos( unsigned char *m_pCodePos, bool dev )
{
	if( !m_ProgToSource )
	{
		return;
	}

	sourceinfo_t *codePos = m_ProgToSource->findKeyValue( m_pCodePos );

	if( !codePos ) {
		return;
	}

	PrintSourcePos( codePos, dev );
}

void AbstractScript::PrintSourcePos( str sourceLine, int column, int line, bool dev )
{
	int i;
	str markerLine = "";

	for( i = 0; i < column; i++ )
	{
		markerLine.append( sourceLine[ i ] );
	}

	markerLine.append( "^" );

	glbs.Printf( "(%s, %d):\n%s\n%s\n", Filename().c_str(), line, sourceLine.c_str(), markerLine.c_str() );
}

AbstractScript::AbstractScript()
{
	m_ProgToSource = NULL;
	m_SourceBuffer = NULL;
	m_SourceLength = 0;
}

StateScript::StateScript()
{
	m_Parent = NULL;
}

template<>
void Entry< const_str, script_label_t >::Archive( Archiver& arc )
{
	unsigned int offset;

	Director.ArchiveString( arc, key );

	if( arc.Saving() )
	{
		offset = value.codepos - current_progBuffer;
		arc.ArchiveUnsigned( &offset );
	}
	else
	{
		arc.ArchiveUnsigned( &offset );
		value.codepos = current_progBuffer + offset;
		value.key = key;
	}

	arc.ArchiveBool( &value.isprivate );
}

void StateScript::Archive( Archiver& arc )
{
	label_list.Archive( arc );
}

bool StateScript::AddLabel( const_str label, unsigned char *pos, bool private_section )
{
	if( label_list.findKeyValue( label ) )
	{
		return true;
	}

	script_label_t &s = label_list.addKeyValue( label );

	s.codepos = pos;
	s.key = label;
	s.isprivate = private_section;

	if( !label_list.findKeyValue( 0 ) )
	{
		label_list.addKeyValue( 0 ) = s;
	}

	reverse_label_list.AddObject( &s );

	return false;
}

bool StateScript::AddLabel( str label, unsigned char *pos, bool private_section )
{
	return AddLabel( Director.AddString( label ), pos, private_section );
}

unsigned char *StateScript::FindLabel( const_str label )
{
	script_label_t *s;
	ScriptClass *scriptClass;
	GameScript *script;

	s = label_list.findKeyValue( label );

	if( s )
	{
		// check if the label is a private function
		if( s->isprivate )
		{
			scriptClass = Director.CurrentScriptClass();

			if( scriptClass )
			{
				script = scriptClass->GetScript();

				// now check if the label's statescript matches this statescript
				if( ( &script->m_State ) != this )
				{
					ScriptError( "Cannot call a private function." );
					return NULL;
				}
			}
		}

		return s->codepos;
	}
	else
	{
		return NULL;
	}
}

unsigned char *StateScript::FindLabel( str label )
{
	return FindLabel( Director.AddString( label ) );
}

const_str StateScript::NearestLabel( unsigned char *pos )
{
	unsigned int offset = pos - m_Parent->m_ProgBuffer;
	unsigned int bestOfs = 0;
	const_str label = 0;

	for( int i = 1; i <= reverse_label_list.NumObjects(); i++ )
	{
		script_label_t *l = reverse_label_list.ObjectAt( i );

		if( ( l->codepos - m_Parent->m_ProgBuffer ) >= bestOfs )
		{
			bestOfs = l->codepos - m_Parent->m_ProgBuffer;

			if( bestOfs > offset )
			{
				break;
			}

			label = l->key;
		}
	}

	return label;
}

GameScript::GameScript()
{
	m_Filename = 0;
	successCompile = false;

	m_ProgBuffer = NULL;
	m_ProgLength = 0;
	m_bPrecompiled = false;

	requiredStackSize = 0;

	m_State.m_Parent = this;
}

GameScript::GameScript( const char *filename )
{
	m_Filename = Director.AddString( filename );
	successCompile = false;

	m_ProgBuffer = NULL;
	m_ProgLength = 0;
	m_ProgToSource = NULL;
	m_bPrecompiled = false;

	m_SourceBuffer = NULL;
	m_SourceLength = 0;

	requiredStackSize = 0;

	m_State.m_Parent = this;
}

GameScript::~GameScript()
{
	Close();
}

struct pfixup_t {
	bool			isString;
	unsigned int	*ptr;
};

static Container< const_str > archivedEvents;
static Container< const_str > archivedStrings;
static Container< pfixup_t * > archivedPointerFixup;

void ArchiveOpcode( Archiver& arc, unsigned char *code )
{
	unsigned int index;

	arc.ArchiveByte( code );

	switch( *code )
	{
	case OP_STORE_NIL:
	case OP_STORE_NULL:
	case OP_DONE:
		break;

	case OP_EXEC_CMD_COUNT1:
	case OP_EXEC_CMD_METHOD_COUNT1:
	case OP_EXEC_METHOD_COUNT1:
		arc.ArchiveByte( code + 1 );
		goto __exec;

	case OP_EXEC_CMD0:
	case OP_EXEC_CMD1:
	case OP_EXEC_CMD2:
	case OP_EXEC_CMD3:
	case OP_EXEC_CMD4:
	case OP_EXEC_CMD5:
	case OP_EXEC_CMD_METHOD0:
	case OP_EXEC_CMD_METHOD1:
	case OP_EXEC_CMD_METHOD2:
	case OP_EXEC_CMD_METHOD3:
	case OP_EXEC_CMD_METHOD4:
	case OP_EXEC_CMD_METHOD5:
	case OP_EXEC_METHOD0:
	case OP_EXEC_METHOD1:
	case OP_EXEC_METHOD2:
	case OP_EXEC_METHOD3:
	case OP_EXEC_METHOD4:
	case OP_EXEC_METHOD5:
		code--;
__exec:
		if( !arc.Loading() )
		{
			index = archivedEvents.AddUniqueObject( *reinterpret_cast< unsigned int * >( code + 2 ) );
		}

		arc.ArchiveUnsigned( &index );

		if( arc.Loading() )
		{
			pfixup_t *p = new pfixup_t;

			p->isString = false;
			p->ptr = reinterpret_cast< unsigned int * >( code + 2 );

			*reinterpret_cast< unsigned int * >( code + 2 ) = index;
			archivedPointerFixup.AddObject( p );
		}
		break;

	case OP_LOAD_FIELD_VAR:
	case OP_LOAD_GAME_VAR:
	case OP_LOAD_GROUP_VAR:
	case OP_LOAD_LEVEL_VAR:
	case OP_LOAD_LOCAL_VAR:
	case OP_LOAD_OWNER_VAR:
	case OP_LOAD_PARM_VAR:
	case OP_LOAD_SELF_VAR:
	case OP_LOAD_STORE_GAME_VAR:
	case OP_LOAD_STORE_GROUP_VAR:
	case OP_LOAD_STORE_LEVEL_VAR:
	case OP_LOAD_STORE_LOCAL_VAR:
	case OP_LOAD_STORE_OWNER_VAR:
	case OP_LOAD_STORE_PARM_VAR:
	case OP_LOAD_STORE_SELF_VAR:
	case OP_STORE_FIELD:
	case OP_STORE_FIELD_REF:
	case OP_STORE_GAME_VAR:
	case OP_STORE_GROUP_VAR:
	case OP_STORE_LEVEL_VAR:
	case OP_STORE_LOCAL_VAR:
	case OP_STORE_OWNER_VAR:
	case OP_STORE_PARM_VAR:
	case OP_STORE_SELF_VAR:
	case OP_STORE_STRING:
		if( !arc.Loading() )
		{
			index = archivedStrings.AddUniqueObject( *reinterpret_cast< unsigned int * >( code + 1 ) );
		}

		arc.ArchiveUnsigned( &index );

		if( arc.Loading() )
		{
			pfixup_t *p = new pfixup_t;

			p->isString = true;
			p->ptr = reinterpret_cast< unsigned int * >( code + 1 );

			*reinterpret_cast< unsigned int * >( code + 1 ) = index;
			archivedPointerFixup.AddObject( p );
		}
		break;

	default:
		if( OpcodeLength( *code ) > 1 )
			arc.ArchiveRaw( code + 1, OpcodeLength( *code ) - 1 );
	}
}

template<>
void Entry < unsigned char *, sourceinfo_t >::Archive( Archiver& arc )
{
	unsigned int offset;

	if( arc.Loading() )
	{
		arc.ArchiveUnsigned( &offset );
		key = current_progBuffer + offset;
	}
	else
	{
		offset = key - current_progBuffer;
		arc.ArchiveUnsigned( &offset );
	}

	arc.ArchiveUnsigned( &value.sourcePos );
	arc.ArchiveInteger( &value.column );
	arc.ArchiveInteger( &value.line );
}

void GameScript::Archive( Archiver& arc )
{
	int count = 0, i;
	unsigned char *p, *code_pos, *code_end;
	const_str s;
	command_t *c, cmd;

	arc.ArchiveSize( ( long * )&m_ProgLength );

	if( arc.Saving() )
	{
		p = m_ProgBuffer;
		current_progBuffer = m_ProgBuffer;

		// archive opcodes
		while( *p != OP_DONE )
		{
			ArchiveOpcode( arc, p );

			p += OpcodeLength( *p );
		}

		ArchiveOpcode( arc, p );

		// archive string dictionary list
		i = archivedStrings.NumObjects();
		arc.ArchiveInteger( &i );

		for( ; i > 0; i-- )
		{
			Director.ArchiveString( arc, archivedStrings.ObjectAt( i ) );
		}

		// archive event list
		i = archivedEvents.NumObjects();
		arc.ArchiveInteger( &i );

		for( ; i > 0; i-- )
		{
			c = Event::GetEventInfo( archivedEvents.ObjectAt( i ) );

			arc.ArchiveString( &c->command );
			arc.ArchiveInteger( &c->flags );
			arc.ArchiveByte( &c->type );
		}
	}
	else
	{
		m_ProgBuffer = ( unsigned char * )glbs.Malloc( m_ProgLength );
		code_pos = m_ProgBuffer;
		code_end = m_ProgBuffer + m_ProgLength;

		current_progBuffer = m_ProgBuffer;

		do
		{
			ArchiveOpcode( arc, code_pos );

			code_pos += OpcodeLength( *code_pos );
		} while( *code_pos != OP_DONE && arc.NoErrors() );

		if( !arc.NoErrors() )
		{
			return;
		}

		// retrieve the string dictionary list
		arc.ArchiveInteger( &i );
		archivedStrings.Resize( i + 1 );

		for( ; i > 0; i-- )
		{
			Director.ArchiveString( arc, s );
			archivedStrings.AddObjectAt( i, s );
		}

		// retrieve the event list
		arc.ArchiveInteger( &i );
		archivedEvents.Resize( i + 1 );

		for( ; i > 0; i-- )
		{
			arc.ArchiveString( &cmd.command );
			arc.ArchiveInteger( &cmd.flags );
			arc.ArchiveByte( &cmd.type );

			archivedEvents.AddObjectAt( i, Event::GetEventWithFlags( cmd.command, cmd.flags, cmd.type ) );
		}

		// fix program string/event pointers
		for( i = archivedPointerFixup.NumObjects(); i > 0; i-- )
		{
			pfixup_t *fixup = archivedPointerFixup.ObjectAt( i );

			if( fixup->isString )
			{
				*fixup->ptr = archivedStrings.ObjectAt( *fixup->ptr );
			}
			else
			{
				*fixup->ptr = archivedEvents.ObjectAt( *fixup->ptr );
			}

			delete fixup;
		}

		successCompile = true;
	}

	// cleanup
	archivedStrings.FreeObjectList();
	archivedEvents.FreeObjectList();
	archivedPointerFixup.FreeObjectList();

	if( !arc.Loading() )
	{
		if( m_ProgToSource )
		{
			count = m_ProgToSource->size();
			arc.ArchiveInteger( &count );

			m_ProgToSource->Archive( arc );
		}
		else
		{
			arc.ArchiveInteger( &count );
		}
	}
	else
	{
		arc.ArchiveInteger( &count );

		if( count )
		{
			m_ProgToSource = new con_set < unsigned char *, sourceinfo_t >;
			m_ProgToSource->Archive( arc );
		}
	}

	arc.ArchiveUnsigned( &requiredStackSize );
	arc.ArchiveBool( &m_bPrecompiled );

	if( !m_bPrecompiled && arc.Loading() )
	{
		fileHandle_t filehandle = NULL;

		m_SourceLength = glbs.FS_ReadFile( Filename().c_str(), ( void ** )&m_SourceBuffer, true );

		if( m_SourceLength > 0 )
		{
			m_SourceBuffer = ( char * )glbs.Malloc( m_SourceLength );

			glbs.FS_Read( m_SourceBuffer, m_SourceLength, filehandle );
			glbs.FS_FCloseFile( filehandle );
		}
	}

	m_State.Archive( arc );

	current_progBuffer = NULL;
}

void GameScript::Archive( Archiver& arc, GameScript *& scr )
{
	str filename;

	if( !arc.Saving() )
	{
		arc.ArchiveString( &filename );

		if( filename != "" )
		{
			scr = Director.GetScript( filename );
		}
		else
		{
			scr = NULL;
		}
	}
	else
	{
		filename = scr->Filename();

		arc.ArchiveString( &filename );
	}
}

void GameScript::ArchiveCodePos( Archiver& arc, unsigned char **codePos )
{
	int pos = 0;
	str filename;

	if( !arc.Saving() )
	{
		pos = *codePos - m_ProgBuffer;
		if( pos >= 0 && pos < m_ProgLength )
		{
			filename = Filename();
		}
	}

	arc.ArchiveInteger( &pos );
	arc.ArchiveString( &filename );

	if( arc.Loading() )
	{
		if( Filename() == filename )
		{
			*codePos = m_ProgBuffer + pos;
		}
	}
}

void GameScript::Close( void )
{
	for( int i = m_CatchBlocks.NumObjects(); i > 0; i-- )
	{
		delete m_CatchBlocks.ObjectAt( i );
	}

	m_CatchBlocks.FreeObjectList();

	if( m_ProgToSource )
	{
		delete m_ProgToSource;
		m_ProgToSource = NULL;
	}

	if( m_ProgBuffer )
	{
		glbs.Free( m_ProgBuffer );
		m_ProgBuffer = NULL;
	}

	if( m_SourceBuffer )
	{
		glbs.Free( m_SourceBuffer );
		m_SourceBuffer = NULL;
	}

	m_ProgLength = 0;
	m_SourceLength = 0;
	m_bPrecompiled = false;
}

void GameScript::Load( const void *sourceBuffer, size_t sourceLength )
{
	size_t nodeLength;
	char *m_PreprocessedBuffer;

	m_SourceBuffer = ( char * )glbs.Malloc( sourceLength + 1 );
	m_SourceLength = sourceLength;

	m_SourceBuffer[ sourceLength ] = 0;

	memcpy( m_SourceBuffer, sourceBuffer, sourceLength );

	Compiler.Reset();

	m_PreprocessedBuffer = Compiler.Preprocess( m_SourceBuffer );
	nodeLength = Compiler.Parse( this, m_PreprocessedBuffer );
	Compiler.Preclean( m_PreprocessedBuffer );

	if( !nodeLength )
	{
		glbs.Printf( "^~^~^ Script file compile error:  Couldn't parse '%s'\n", Filename().c_str() );
		return Close();
	}

	m_ProgBuffer = ( unsigned char * )glbs.Malloc( nodeLength );
	m_ProgLength = Compiler.Compile( this, m_ProgBuffer );

	if( !m_ProgLength )
	{
		glbs.Printf( "^~^~^ Script file compile error:  Couldn't compile '%s'\n", Filename().c_str() );
		return Close();
	}

	requiredStackSize = Compiler.m_iInternalMaxVarStackOffset + 9 * Compiler.m_iMaxExternalVarStackOffset + 1;

	successCompile = true;
}

bool GameScript::GetCodePos( unsigned char *codePos, str& filename, int& pos )
{
	pos = codePos - m_ProgBuffer;

	if( pos >= 0 && pos < m_ProgLength )
	{
		filename = Filename();
		return true;
	}
	else
	{
		return false;
	}
}

bool GameScript::SetCodePos( unsigned char *&codePos, str& filename, int pos )
{
	if( Filename() == filename )
	{
		codePos = m_ProgBuffer + pos;
		return true;
	}
	else
	{
		return false;
	}
}

unsigned int GameScript::GetRequiredStackSize( void )
{
	return requiredStackSize;
}

qboolean GameScript::labelExists( const char *name )
{
	str				labelname;
	
	// if we got passed a NULL than that means just run the script so of course it exists
	if ( !name )
	{
		return true;
	}

	if( m_State.FindLabel( name ) )
	{
		return true;
	}

	return false;
}

StateScript *GameScript::CreateCatchStateScript( unsigned char *try_begin_code_pos, unsigned char *try_end_code_pos )
{
	CatchBlock *catchBlock = new CatchBlock;

	catchBlock->m_TryStartCodePos = try_begin_code_pos;
	catchBlock->m_TryEndCodePos = try_end_code_pos;

	m_CatchBlocks.AddObject( catchBlock );

	return &catchBlock->m_StateScript;
}

StateScript *GameScript::CreateSwitchStateScript( void )
{
	return new StateScript;
}

StateScript *GameScript::GetCatchStateScript( unsigned char *in, unsigned char *&out )
{
	CatchBlock *catchBlock;
	CatchBlock *bestCatchBlock = NULL;

	for( int i = m_CatchBlocks.NumObjects(); i > 0; i-- )
	{
		catchBlock = m_CatchBlocks.ObjectAt( i );

		if( in >= catchBlock->m_TryStartCodePos && in < catchBlock->m_TryEndCodePos )
		{
			if( !bestCatchBlock || catchBlock->m_TryEndCodePos < bestCatchBlock->m_TryEndCodePos )
			{
				bestCatchBlock = catchBlock;
			}
		}
	}

	if( bestCatchBlock )
	{
		out = bestCatchBlock->m_TryEndCodePos;

		return &bestCatchBlock->m_StateScript;
	}
	else
	{
		return NULL;
	}
}

ScriptThreadLabel::ScriptThreadLabel()
{
	m_Script = NULL;
	m_Label = STRING_EMPTY;
}

ScriptThread *ScriptThreadLabel::Create( Listener *listener )
{
	if( !m_Script ) {
		return NULL;
	}

	ScriptClass *scriptClass = new ScriptClass( m_Script, listener );
	ScriptThread *thread = Director.CreateScriptThread( scriptClass, m_Label );

	return thread;
}

void ScriptThreadLabel::Execute( Listener *listener )
{
	if( !m_Script ) {
		return;
	}

	ScriptThread *thread = Create( listener );

	if( thread )
	{
		thread->Execute();
	}
}

void ScriptThreadLabel::Execute( Listener *listener, Event &ev )
{
	Execute( listener, &ev );
}

void ScriptThreadLabel::Execute( Listener *listener, Event *ev )
{
	if( !m_Script ) {
		return;
	}

	ScriptThread *thread = Create( listener );

	if( thread )
	{
		thread->Execute( ev );
	}
}

void ScriptThreadLabel::Set( const char *label )
{
	str script;
	char buffer[ 1023 ];
	char *p = buffer;
	bool foundLabel = false;

	if( !label || !*label )
	{
		m_Script = NULL;
		m_Label = STRING_EMPTY;
		return;
	}

	strcpy( buffer, label );

	while( *p != '\0' )
	{
		if( p[ 0 ] == ':' && p[ 1 ] == ':' )
		{
			*p = '\0';

			script = buffer;
			m_Label = Director.AddString( &p[ 2 ] );
			foundLabel = true;

			break;
		}

		p++;
	}

	if( !foundLabel )
	{
		script = level.m_mapscript;
		m_Label = Director.AddString( buffer );
	}

	m_Script = Director.GetGameScript( script );

	if( !m_Script->m_State.FindLabel( m_Label ) )
	{
		str l = Director.GetString( m_Label );

		m_Script = NULL;
		m_Label = STRING_EMPTY;

		ScriptError( "^~^~^ Could not find label '%s' in '%s'", l.c_str(), script.c_str() );
	}
}

void ScriptThreadLabel::SetScript( const ScriptVariable& label )
{
	if( label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING )
	{
		m_Script = Director.GetGameScript( label.stringValue() );
		m_Label = STRING_EMPTY;
	}
	else if( label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1 )
	{
		ScriptVariable *script = label[ 1 ];
		ScriptVariable *labelname = label[ 2 ];

		m_Script = Director.GetGameScript( script->stringValue() );
		m_Label = labelname->constStringValue();

		if( !m_Script->m_State.FindLabel( m_Label ) )
		{
			m_Script = NULL;
			m_Label = STRING_EMPTY;

			ScriptError( "^~^~^ Could not find label '%s' in '%s'", labelname->stringValue().c_str(), script->stringValue().c_str() );
		}
	}
	else
	{
		ScriptError( "ScriptThreadLabel::SetScript: bad label type '%s'", label.GetTypeName() );
	}
}

void ScriptThreadLabel::SetScript( const char *label )
{
	Set( label );
}

void ScriptThreadLabel::SetThread( const ScriptVariable& label )
{
	ScriptThread *thread = NULL;

	if( label.GetType() == VARIABLE_STRING || label.GetType() == VARIABLE_CONSTSTRING )
	{
		m_Script = Director.CurrentScriptClass()->GetScript();
		m_Label = label.constStringValue();
	}
	else if( label.GetType() == VARIABLE_CONSTARRAY && label.arraysize() > 1 )
	{
		ScriptVariable *script = label[ 1 ];
		ScriptVariable *labelname = label[ 2 ];

		m_Script = Director.GetGameScript( script->stringValue() );
		m_Label = labelname->constStringValue();

		if( !m_Script->m_State.FindLabel( m_Label ) )
		{
			m_Script = NULL;
			m_Label = STRING_EMPTY;

			ScriptError( "^~^~^ Could not find label '%s' in '%s'", labelname->stringValue().c_str(), script->stringValue().c_str() );
		}
	}
	else
	{
		ScriptError( "ScriptThreadLabel::SetThread bad label type '%s'", label.GetTypeName() );
	}
}

bool ScriptThreadLabel::TrySet( const char *label )
{
	try
	{
		Set( label );
	}
	catch( const char *string )
	{
		Com_Printf( "%s\n", string );

		return false;
	}

	return true;
}

bool ScriptThreadLabel::TrySet( const_str label )
{
	return TrySet( Director.GetString( label ) );
}

bool ScriptThreadLabel::TrySetScript( const char *label )
{
	try
	{
		SetScript( label );
	}
	catch( const char *string )
	{
		Com_Printf( "%s\n", string );

		return false;
	}

	return true;
}

bool ScriptThreadLabel::TrySetScript( const_str label )
{
	return TrySetScript( Director.GetString( label ) );
}

bool ScriptThreadLabel::IsSet( void )
{
	return m_Script != NULL ? true : false;
}

void ScriptThreadLabel::Archive( Archiver& arc )
{
	m_Script->Archive( arc );
	Director.ArchiveString( arc, m_Label );
}
