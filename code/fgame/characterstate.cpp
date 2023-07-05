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

// characterstate.cpp:
// 

#include "characterstate.h"
#include "animate.h"
#include "scriptmaster.h"

static const char *MoveControl_Names[] =
{
	"none",				// MOVECONTROL_NONE
	"user",				// MOVECONTROL_USER
	"legs",				// MOVECONTROL_LEGS
	"user_moveanim",	// MOVECONTROL_USER_MOVEANIM
	"anim",				// MOVECONTROL_ANIM
	"absolute",			// MOVECONTROL_ABSOLUTE
	"hanging",			// MOVECONTROL_HANGING
	"rope_grab",		// MOVECONTROL_ROPE_GRAB
	"rope_release",		// MOVECONTROL_ROPE_RELEASE
	"rope_move",		// MOVECONTROL_ROPE_MOVE
	"pickupenemy",		// MOVECONTROL_PICKUPENEMY
	"push",				// MOVECONTROL_PUSH
	"climbwall",		// MOVECONTROL_CLIMBWALL
	"useanim",			// MOVECONTROL_USEANIM
	"crouch",			// MOVECONTROL_CROUCH
	"loopuseanim",		// MOVECONTROL_LOOPUSEANIM
	"useobject",		// MOVECONTROL_USEOBJECT
	"coolobject",		// MOVECONTROL_COOLOBJECT
	NULL
};

static const char *Camera_Names[] =
   {
   "topdown",        // CAMERA_TOPDOWN
   "behind",         // CAMERA_BEHIND
   "front",          // CAMERA_FRONT
   "side",           // CAMERA_SIDE
   "behind_fixed",   // CAMERA_BEHIND_FIXED
   "side_left",      // CAMERA_SIDE_LEFT
   "side_right",     // CAMERA_SIDE_RIGHT
   "behind_nopitch", // CAMERA_BEHIND_NOPITCH
   NULL
   };

Conditional::Conditional
   (
   Condition<Class> &cond
   ) : condition( cond )

   {
   result = false;
   previous_result = false;
   checked = false;
   }

Conditional::Conditional()
{
	result = false;
	previous_result = false;
	checked = false;
}

Expression::Expression()
   {
   }

Expression::Expression
   (
   Expression &exp
   )
   
   {
   int i;

   value = exp.value;

   for( i = 1; i <= exp.conditions.NumObjects(); i++ )
      {
      conditions.AddObject( exp.conditions.ObjectAt( i ) );
      }
   }

Expression::Expression
   (
   Script &script,
   State &state
   )

   {
   str token;
   condition_t condition;
   int start;

   value = script.GetToken( true );

   if ( !script.TokenAvailable( false ) || Q_stricmp( script.GetToken( false ), ":" ) )
      {
      gi.Error( ERR_DROP, "%s: Expecting ':' on line %d.\n", script.Filename(), script.GetLineNumber() );
      }

   while( script.TokenAvailable( false ) )
      {
      token = script.GetToken( true );

      switch( token[ 0 ] )
         {
         case '!' :
            condition.test = TC_ISFALSE;
            start = 1;
            break;

         case '+' :
            condition.test = TC_EDGETRUE;
            start = 1;
            break;

         case '-' :
            condition.test = TC_EDGEFALSE;
            start = 1;
            break;

         default :
            condition.test = TC_ISTRUE;
            start = 0;
         }

      if ( token.length() <= start )
         {
         gi.Error( ERR_DROP, "%s: Illegal syntax '%s' on line %d.\n", script.Filename(), &token, script.GetLineNumber() );
         condition.condition_index = 0;
         continue;
         }

      condition.condition_index = state.addCondition( &token[ start ], script );
      if ( !condition.condition_index )
         {
         gi.Error( ERR_DROP, "%s: Unknown condition '%s' on line %d.\n", script.Filename(), &token[ start ], script.GetLineNumber() );
         }

      conditions.AddObject( condition );
      }
   }

bool Expression::getResult
   (
   State &state,
   Entity &ent,
	Container<Conditional *> *sent_conditionals
   )

   {
   int i;
   condition_t *cond;
	Conditional *conditional;

   for( i = 1; i <= conditions.NumObjects(); i++ )
      {
      cond = &conditions.ObjectAt( i );
		conditional = sent_conditionals->ObjectAt( cond->condition_index );

      if ( !conditional || !conditional->getResult( cond->test, ent ) )
         {
         return false;
         }
      }

   return true;
   }

void State::readNextState
   (
   Script &script
   )

   {
   nextState = script.GetToken( false );
   }

void State::readMoveType
   (
   Script &script
   )

   {
   str token;
   const char **name;
   int i;

   token = script.GetToken( false );

   for( i = 0, name = MoveControl_Names; *name != NULL; name++, i++ )
      {
      if ( !token.icmp( *name ) )
         {
         break;
         }
      }

   if ( *name == NULL )
      {
      gi.Error( ERR_DROP, "%s: Unknown movetype '%s' on line %d.\n", script.Filename(), token.c_str(), script.GetLineNumber() );
      }
   else
      {
      movetype = ( movecontrol_t )i;
      }
   }

qboolean State::setCameraType
   (
   str ctype
   )

   {
   const char **name;
   int i;

   for( i = 0, name = Camera_Names; *name != NULL; name++, i++ )
      {
      if ( !ctype.icmp( *name ) )
         {
         cameratype = ( cameratype_t )i;
         return qtrue;
         }
      }
   return qfalse;
   }


void State::readCamera
   (
   Script &script
   )

   {
   str token;

   token = script.GetToken( false );

   if ( !setCameraType( token ) )
      {
      gi.Error( ERR_DROP, "%s: Unknown camera type '%s' on line %d.\n", script.Filename(), token.c_str(), script.GetLineNumber() );
      }
   }

void State::readLegs
   (
   Script &script
   )

   {
   str token;

   if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
      {
      gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
      }

   while( script.TokenAvailable( true ) )
      {
      token = script.GetToken( true );
      if ( !Q_stricmp( token.c_str(), "}" ) )
         {
         break;
         }

      script.UnGetToken();
      legAnims.AddObject( Expression( script, *this ) );
      }
   }

void State::readAction
	(
	Script &script
	)
{
	str token;

	m_iActionAnimType = 0;

	if( script.TokenAvailable( false ) )
	{
		token = script.GetToken( false );
		if( token.icmp( "aim" ) ) {
			m_iActionAnimType = 1;
		}
	}

	if( !script.TokenAvailable( true ) )
	{
		gi.Error( ERR_DROP, "%s: EOF, expected state body on line %d.\n", script.Filename(), script.GetLineNumber() );
	}

	if( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}

	while( script.TokenAvailable( true ) )
	{
		token = script.GetToken( true );
		if( !Q_stricmp( token.c_str(), "}" ) )
		{
			return;
		}

		script.UnGetToken();
		m_actionAnims.AddObject( Expression( script, *this ) );
	}

	gi.Error( ERR_DROP, "%s: EOF, expected '}' on line %d.\n", script.Filename(), script.GetLineNumber() );
}

void State::readBehavior
   (
   Script &script
   )

   {
	str token;

   if ( !script.TokenAvailable( true ) )
      {
      gi.Error( ERR_DROP, "%s: Expecting behavior name on line %d.\n", script.Filename(), script.GetLineNumber() );
      }

   behaviorName = script.GetToken( true );
   if ( !getClass( behaviorName ) )
      {
      gi.Error( ERR_DROP, "%s: Unknown behavior '%s' on line %d.\n", script.Filename(), behaviorName.c_str(), script.GetLineNumber() );
      }

	// Read in the behavior arguments if there are any

	while ( script.TokenAvailable( false ) && script.AtString( false ) )
		{
		token = script.GetToken( false );
		addBehaviorParm( token );
		}
   }

void State::readTime
   (
   Script &script
   )

   {
	str token;


	if ( script.TokenAvailable( false ) && script.AtString( false ) )
		{
		token   = script.GetToken( false );
		minTime = atof( token.c_str() );
		}

	if ( script.TokenAvailable( false ) && script.AtString( false ) )
		{
		token   = script.GetToken( false );
		maxTime = atof( token.c_str() );
		}
	else
		{
		maxTime = minTime;
		}
   }

void State::readStates
   (
   Script &script
   )

   {
   str token;

   if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
      {
      gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
      }

   while( script.TokenAvailable( true ) )
      {
      token = script.GetToken( true );
      if ( !Q_stricmp( token.c_str(), "}" ) )
         {
         break;
         }

      script.UnGetToken();
      states.AddObject( Expression( script, *this ) );
      }
   }

void State::ParseAndProcessCommand
   (
   str command,
   Entity *target
   )

   {
   int      argc;
	const    char *argv[ MAX_COMMANDS ];
	char     args[ MAX_COMMANDS ][ SCRIPT_MAXTOKEN ];
   Script   script;
   Event    *event;

   script.Parse( command, command.length(), "CommandString" );

   argc = 0;
	while( script.TokenAvailable( false ) )
		{
		if ( argc >= MAX_COMMANDS )
			{
		   gi.DPrintf( "State:ParseAndProcessCommand : Line exceeds %d command limit", MAX_COMMANDS );
			script.SkipToEOL();
			break;
			}
		strcpy( args[ argc ], script.GetToken( false ) );
		argv[ argc ] = args[ argc ];
		argc++;
		}

	assert( argc > 0 );

   if ( argc <= 0 )
      return;

   event = new Event( args[0] );
	event->AddTokens( argc - 1, &argv[ 1 ] );
   target->ProcessEvent( event );
   }

void State::ProcessEntryCommands
   (
   Entity *target
   )

   {
   int i,count;
   str command;

   assert( target );
   if ( !target )
      {
      return;
      }

   count = entryCommands.NumObjects();
   for( i = 1; i <= count; i++ )
      {
      ParseAndProcessCommand( entryCommands.ObjectAt( i ), target );
      }
   }

void State::ProcessExitCommands
   (
   Entity *target
   )

   {
   int i,count;
   str command;

   assert( target );
   if ( !target )
      {
      return;
      }

   count = exitCommands.NumObjects();
   for( i = 1; i <= count; i++ )
      {
      ParseAndProcessCommand( exitCommands.ObjectAt( i ), target );
      }
   }

void State::readCommands
   (
   Script            &script,
   Container<str>    &container
   )

   {
   str token;
   str command;

   if ( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
      {
      gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
      }

   while( script.TokenAvailable( true ) )
      {
      while( script.TokenAvailable( false ) )
         {
         token = script.GetToken( true );   
         if ( !Q_stricmp( token.c_str(), "}" ) )
            {
            goto out;
            }
			if ( token.length() )
            {
				if ( strstr( token.c_str(), " " ) == NULL )
					{
					command.append( token );
					}
				else
					{
					command.append( "\"" );
					command.append( token );
					command.append( "\"" );
					}
            }
			else
            {
				command.append( "\"\"" );
            }

         command.append( " " );
         }
      container.AddObject( command );
      command = "";
      }
out:
   return;
   }

State *State::Evaluate
   (
   Entity &ent,
	Container<Conditional *> *sent_conditionals
   )

   {
   int i;
   Expression *exp;
   State *state;
	int index;

	for( i = 1; i <= condition_indexes.NumObjects(); i++ )
	{
		index = condition_indexes.ObjectAt( i );
		sent_conditionals->ObjectAt( index )->clearCheck();
		//conditions.ObjectAt( i )->clearCheck();
	}

   for( i = 1; i <= states.NumObjects(); i++ )
      {
      exp = &states.ObjectAt( i );
	  if( exp->getResult( *this, ent, sent_conditionals ) )
         {
         state = statemap.FindState( exp->getValue() );
		 return state;
         }
      }

   return NULL;
   }

const char *State::getLegAnim
	(
	Entity &ent,
	Container<Conditional *> *sent_conditionals
	)
{
	int i;
	Expression *exp;
	int index;

	for( i = 1; i <= condition_indexes.NumObjects(); i++ )
	{
		index = condition_indexes.ObjectAt( i );
		sent_conditionals->ObjectAt( index )->clearCheck();
		//conditions.ObjectAt( i )->clearCheck();
	}

	for( i = 1; i <= legAnims.NumObjects(); i++ )
	{
		exp = &legAnims.ObjectAt( i );
		if( exp->getResult( *this, ent, sent_conditionals ) )
		{
			return exp->getValue();
		}
	}

	return "";
}

const char *State::getActionAnim
	(
	Entity &ent,
	Container<Conditional *> *sent_conditionals,
	int *piAnimType
	)
{
	int i;
	Expression *exp;
	int index;

	for( i = 1; i <= condition_indexes.NumObjects(); i++ )
	{
		index = condition_indexes.ObjectAt( i );
		sent_conditionals->ObjectAt( index )->clearCheck();
	}

	for( i = 1; i <= m_actionAnims.NumObjects(); i++ )
	{
		exp = &m_actionAnims.ObjectAt( i );
		if( exp->getResult( *this, ent, sent_conditionals ) )
		{
			if( piAnimType ) {
				*piAnimType = m_iActionAnimType;
			}

			return exp->getValue();
		}
	}

	if( piAnimType ) {
		*piAnimType = 0;
	}

	return "";
}

const char *State::getBehaviorName
   (
   void
   )

   {
   return behaviorName.c_str();
   }

float State::getMinTime
   (
   void
   )

   {
	return minTime;
   }

float State::getMaxTime
   (
   void
   )

   {
	return maxTime;
   }

int State::addCondition
   (
   const char *name,
   Script &script
   )

   {
   Conditional       *condition;
   Condition<Class>  *cond;
	int index;

	str token;

   condition = NULL;
   cond = statemap.getCondition( name );
   if ( !cond )
      {
      return 0;
      }

   condition = new Conditional( *cond );

   // Get the paramaters
   while ( script.TokenAvailable( false ) && script.AtString( false ) )
      {
      token = script.GetToken( false );
		condition->addParm( token );
      }

   // only add a new conditional if a similar on doesn't exist
   index = statemap.findConditional( condition );

   if ( index )
      {
      // delete the one we just made
      delete condition;
      }
   else
      {
      index = statemap.addConditional( condition );
      }

   condition_indexes.AddUniqueObject( index );

   return index;
   }

void State::CheckStates
   (
   void
   )

   {
   const char *value;
   int i;

   if ( !statemap.FindState( nextState.c_str() ) )
      {
      gi.Error( ERR_DROP, "Unknown next state '%s' referenced in state '%s'.\n", nextState.c_str(), getName() );
      }

   for( i = 1; i <= states.NumObjects(); i++ )
      {
      value = states.ObjectAt( i ).getValue();
      if ( !statemap.FindState( value ) )
         {
         gi.Error( ERR_DROP, "Unknown state '%s' referenced in state '%s'.\n", value, getName() );
         }
      }
   }

void State::GetLegAnims
   (
   Container<const char *> *c
   )

   {
   int i,j;
   qboolean addobj = true;

   for ( i=1; i<=legAnims.NumObjects(); i++ )
      {
      const char *value = legAnims.ObjectAt( i ).getValue();
      addobj = true;

      // Check to see if it's already in there
      for ( j=1; j<=c->NumObjects(); j++ )
         {
         if ( !Q_stricmp( c->ObjectAt( j ), value ) )
            {
            addobj = false;
            break;
            }
         }
      if ( addobj )
         c->AddObject( value );
      }
   }

void State::GetActionAnims
   (
   Container<const char *> *c
   )

   {
   int i,j;
   qboolean addobj = true;

   for ( i=1; i<=m_actionAnims.NumObjects(); i++ )
      {
      const char *value = m_actionAnims.ObjectAt( i ).getValue();
      addobj = true;

      // Check to see if it's already in there
      for ( j=1; j<=c->NumObjects(); j++ )
         {
         if ( !Q_stricmp( c->ObjectAt( j ), value ) )
            {
            addobj = false;
            break;
            }
         }
      if ( addobj )
         c->AddObject( value );
      }
   }

State::State
	(
	const char *statename,
	Script &script,
	StateMap &map
	) : statemap( map )
{
	str cmd;

	name = statename;
	nextState = statename;
	movetype = DEFAULT_MOVETYPE;
	cameratype = DEFAULT_CAMERA;
	behaviorName = "idle";

	minTime = -1.0;
	maxTime = -1.0;

	if( !script.TokenAvailable( true ) || Q_stricmp( script.GetToken( true ), "{" ) )
	{
		gi.Error( ERR_DROP, "%s: Expecting '{' on line %d.\n", script.Filename(), script.GetLineNumber() );
	}

	while( script.TokenAvailable( true ) )
	{
		cmd = script.GetToken( true );
		if( !cmd.icmp( "nextstate" ) )
		{
			readNextState( script );
		}
		else if( !cmd.icmp( "movetype" ) )
		{
			readMoveType( script );
		}
		else if( !cmd.icmp( "camera" ) )
		{
			readCamera( script );
		}
		else if( !cmd.icmp( "legs" ) )
		{
			readLegs( script );
		}
		else if( !cmd.icmp( "action" ) )
		{
			readAction( script );
		}
		else if( !cmd.icmp( "behavior" ) )
		{
			readBehavior( script );
		}
		else if( !cmd.icmp( "time" ) )
		{
			readTime( script );
		}
		else if( !cmd.icmp( "states" ) )
		{
			readStates( script );
		}
		else if( !cmd.icmp( "entrycommands" ) )
		{
			readCommands( script, entryCommands );
		}
		else if( !cmd.icmp( "exitcommands" ) )
		{
			readCommands( script, exitCommands );
		}
		else if( !cmd.icmp( "}" ) )
		{
			break;
		}
		else
		{
			gi.Error( ERR_DROP, "%s: Unknown command '%s' on line %d.\n", script.Filename(), cmd.c_str(), script.GetLineNumber() );
		}
	}
}

StateMap::StateMap
   (
   const char *file_name,
   Condition<Class> *conditions, 
	Container<Conditional *> *conditionals
   )

   {
   str cmd;
   str statename;
   State *state;
   int i;
   Script script;

   assert( file_name );

	filename = file_name;

	this->current_conditions = conditions;

   this->current_conditionals = conditionals;

   script.LoadFile( filename );

   while( script.TokenAvailable( true ) )
      {
      cmd = script.GetToken( true );
      if ( !cmd.icmp( "state" ) )
         {
         statename = script.GetToken( false );
         if ( FindState( statename.c_str() ) )
            {
            gi.Error( ERR_DROP, "%s: Duplicate definition of state '%s' on line %d.\n", filename.c_str(), statename.c_str(), script.GetLineNumber() );
            }

         // parse the state even if we already have it defined
         state = new State( statename.c_str(), script, *this );
         stateList.AddObject( state );
         }
      else
         {
         gi.Error( ERR_DROP, "%s: Unknown command '%s' on line %d.\n", script.Filename(), cmd.c_str(), script.GetLineNumber() );
         }
      }

   script.Close();

   // Have all the states check themselves to see if they reference any non-existant states.
   for( i = 1; i <= stateList.NumObjects(); i++ )
      {
      stateList.ObjectAt( i )->CheckStates();
      }
   }

StateMap::~StateMap()
   {
   int i,num;

   num = stateList.NumObjects();
   for( i=num; i>0; i-- )
		{
		delete stateList.ObjectAt( i );
		}
   stateList.FreeObjectList();
   }

Condition<Class> *StateMap::getCondition
   (
   const char *name
   )

   {
   Condition<Class> *c;

   if ( current_conditions )
      {
      for( c = current_conditions; c->name; c++ )
         {
         if ( !strcmp( c->name, name ) )
            {
            return c;
            }
         }
      }

   return NULL;
   }

int StateMap::findConditional
   (
   Conditional *condition
   )

   {
   int i;
   int j;
   Conditional *c;
   bool found;


	// Check for the one special case where we don't want to merge the conditionals 

	if ( strcmp( condition->getName(), "CHANCE" ) == 0 )
		return 0;

   for( i = 1; i <= current_conditionals->NumObjects(); i++ )
      {
      c = current_conditionals->ObjectAt( i );
      if ( ( c->getName() == condition->getName() ) && ( c->numParms() == condition->numParms() ) )
         {
         found = true;
         for( j = 1; j <= c->numParms(); j++ )
            {
            if ( strcmp( c->getParm( j ), condition->getParm( j ) ) )
               {
               found = false;
               break;
               }
            }

         if ( found )
            {
            return i;
            }
         }
      }

   return 0;
   }

int StateMap::addConditional
   (
   Conditional *condition
   )

   {
	int index;
   index = current_conditionals->AddObject( condition );

	return index;
   }

Conditional *StateMap::getConditional
   (
   const char *name
   )

   {
   int i;
   Conditional       *c;
   Condition<Class>  *condition;

   for( i = 1; i <= current_conditionals->NumObjects(); i++ )
      {
      c = current_conditionals->ObjectAt( i );
      if ( !strcmp( c->getName(), name ) )
         {
         return c;
         }
      }

   condition = getCondition( name );

   c = new Conditional( *condition );
   current_conditionals->AddObject( c );

   return c;
   }

State *StateMap::FindState
   (
   const char *name
   )

   {
   int i;

   for( i = 1; i <= stateList.NumObjects(); i++ )
      {
      if ( !strcmp( stateList.ObjectAt( i )->getName(), name ) )
         {
         return stateList.ObjectAt( i );
         }
      }

   return NULL;
   }

// Caching statemaps 

struct cached_statemap_t
	{
	StateMap *statemap;
	Container<Conditional *> *conditionals;
	};

Container<cached_statemap_t> cached_statemaps;

StateMap *GetStatemap
	( 
	str filename,
	Condition<Class> *conditions, 
	Container<Conditional *> *conditionals,
	qboolean reload,
	qboolean cache_only
	)

	{
	int i;
	int j;
	cached_statemap_t *cache = NULL;
	cached_statemap_t new_cache;
	qboolean found = false;
	Conditional *new_conditional;
	Conditional *old_conditional;
	Condition<Class>  *cond;

	for( i = 1 ; i <= cached_statemaps.NumObjects() ; i++ )
		{
		cache = &cached_statemaps.ObjectAt( i );

		if ( strcmp( cache->statemap->Filename(), filename.c_str() ) == 0 )
			{
			found = true;
			break;
			}
		}

	if ( found && reload )
		{
		delete cache->statemap;
		delete cache->conditionals;

		cache->conditionals = new Container<Conditional *>;
		cache->statemap = new StateMap( filename, conditions, cache->conditionals );
		}

	if ( !found )
		{
		new_cache.conditionals = new Container<Conditional *>;
		new_cache.statemap = new StateMap( filename, conditions, new_cache.conditionals );

		cached_statemaps.AddObject( new_cache );

		cache = &new_cache;
		}

	// Copy conditionals over

	if ( !cache_only )
		{
		for( i = 1 ; i <= cache->conditionals->NumObjects() ; i++ )
			{
			old_conditional = cache->conditionals->ObjectAt( i );

			cond = cache->statemap->getCondition( old_conditional->condition.name );

			new_conditional = new Conditional( *cond );

			for( j = 1 ; j <= old_conditional->parmList.NumObjects() ; j++ )
				{
				new_conditional->parmList.AddObject( old_conditional->parmList.ObjectAt( j ) );
				}

			conditionals->AddObject( new_conditional );
			}
		}

	return cache->statemap;
	}

void CacheStatemap
	( 
	str filename,
	Condition<Class> *conditions
	)

	{
	GetStatemap( filename, conditions, NULL, false, true );
	}

void StateMap::GetAllAnims
   (
   Container<const char *> *c
   )

   {
   int i;

   for( i = 1; i <= stateList.NumObjects(); i++ )
      {
      stateList.ObjectAt( i )->GetLegAnims( c );
      stateList.ObjectAt( i )->GetActionAnims( c );
      }     
   }

void ClearCachedStatemaps
	(
	void
	)

	{
	int i,j,num2;
	cached_statemap_t *cache;

   num2 = cached_statemaps.NumObjects();

	for( i=num2 ; i>0; i-- )
		{
		cache = &cached_statemaps.ObjectAt( i );

		delete cache->statemap;

      int num = cache->conditionals->NumObjects();
      for ( j=num; j>0; j-- )
         {
         Conditional *cond = cache->conditionals->ObjectAt( j );
         delete cond;
         }
      delete cache->conditionals;
		}

	cached_statemaps.FreeObjectList();
	}
