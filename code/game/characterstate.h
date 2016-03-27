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

// characterstate.h: Character state

#ifndef __CHARACTERSTATE_H__
#define __CHARACTERSTATE_H__

#include "g_local.h"
#include "entity.h"
#include "script.h"

enum testcondition_t
   {
   TC_ISTRUE,     // no prefix
   TC_ISFALSE,    // !
   TC_EDGETRUE,   // +
   TC_EDGEFALSE   // -
   };

enum movecontrol_t 
{
	MOVECONTROL_NONE,
	MOVECONTROL_USER,			// Quake style
	MOVECONTROL_LEGS,			// Quake style, legs state system active
	MOVECONTROL_USER_MOVEANIM,
	MOVECONTROL_ANIM,			// move based on animation, with full collision testing
	MOVECONTROL_ABSOLUTE,		// move based on animation, with full collision testing but no turning
	MOVECONTROL_HANGING,		// move based on animation, with full collision testing, hanging
	MOVECONTROL_ROPE_GRAB,
	MOVECONTROL_ROPE_RELEASE,
	MOVECONTROL_ROPE_MOVE,
	MOVECONTROL_PICKUPENEMY,
	MOVECONTROL_PUSH,
	MOVECONTROL_CLIMBWALL,
	MOVECONTROL_USEANIM,
	MOVECONTROL_CROUCH,
	MOVECONTROL_LOOPUSEANIM,
	MOVECONTROL_USEOBJECT,
	MOVECONTROL_COOLOBJECT,
};

enum cameratype_t 
{
	CAMERA_TOPDOWN,
	CAMERA_BEHIND,
	CAMERA_FRONT,
	CAMERA_SIDE,
	CAMERA_BEHIND_FIXED,
	CAMERA_SIDE_LEFT,
	CAMERA_SIDE_RIGHT,
	CAMERA_BEHIND_NOPITCH
};

#define DEFAULT_MOVETYPE   MOVECONTROL_NONE
#define DEFAULT_CAMERA     CAMERA_BEHIND

class Conditional;

template< class Type >
struct Condition
	{
	const char     *name;
   qboolean       ( Type::*func )( Conditional &condition );
   };

class Conditional : public Class
   {
   private :
      qboolean                   result;
      qboolean                   previous_result;
      bool                       checked;

   public :
	   CLASS_PROTOTYPE( Conditional );

		Condition<Class>           condition;
		Container<str>					parmList;

      bool                       getResult( testcondition_t test, Entity &ent );
      const char                 *getName( void );

								Conditional( Condition<Class> &condition );
								Conditional();
      
		const char						*getParm( int number );
      void                       addParm( str parm );
		int								numParms( void );
		void								clearCheck( void );
		void								clearPrevious( void );
   };

inline void Conditional::addParm
   (
   str parm
   )

   {
   parmList.AddObject( parm );
   }

inline const char *Conditional::getParm
   (
   int number
   )

   {
   if ( ( number < 1 ) || ( number > parmList.NumObjects() ) )
      {
      gi.Error( ERR_DROP, "Parm #%d out of range on %s condition\n", number, condition.name );
      }
   return parmList.ObjectAt( number ).c_str();
   }

inline int Conditional::numParms
   (
   void
   )

   {
   return parmList.NumObjects();
   }

inline void Conditional::clearCheck
   (
   void
   )

   {
   checked = false;
   }

inline void Conditional::clearPrevious
   (
   void
   )

   {
   previous_result = 0;
   }

inline const char *Conditional::getName
   (
   void
   )

   {
   return condition.name;
   }

inline bool Conditional::getResult
   (
   testcondition_t test, 
   Entity &ent
   )

{
	if ( condition.func && !checked )
	{
		checked = true;
		previous_result = result;
	
		result = ( ent.*condition.func )( *this );
	}

	switch( test )
	{
		case TC_ISFALSE :
			return !result;

		case TC_EDGETRUE:
			return result && !previous_result;

		case TC_EDGEFALSE:
			return !result && previous_result;

		case TC_ISTRUE :
		default:
			return result != false;
	}
}

class State;
class StateMap;

class Expression : public Class
   {
   private :
      struct condition_t
         {
         testcondition_t      test;
         int			         condition_index;
         };

      str                     value;
      Container<condition_t>  conditions;

   public :
                              Expression();
                              Expression( Expression &exp );
                              Expression( Script &script, State &state );

      void		               operator=( const Expression &exp );

      bool                    getResult( State &state, Entity &ent, Container<Conditional *> *sent_conditionals );
      const char              *getValue( void );
   };

inline void Expression::operator=
	(
   const Expression &exp
   )
   
   {
   int i;

   value = exp.value;

   conditions.FreeObjectList();
   for( i = 1; i <= exp.conditions.NumObjects(); i++ )
      {
      conditions.AddObject( exp.conditions.ObjectAt( i ) );
      }
   }

inline const char *Expression::getValue
   (
   void
   )

   {
   return value.c_str();
   }

class State : public Class
   {
   private :
      Container<int>					condition_indexes;

      StateMap                   &statemap;

      str                        name;

      str                        nextState;
      movecontrol_t              movetype;
      cameratype_t               cameratype;

		str								behaviorName;
		Container<str>					behaviorParmList;

		float								minTime;
		float								maxTime;

      Container<Expression>			legAnims;
	  Container<Expression>			m_actionAnims;
	  int							m_iActionAnimType;

      Container<Expression>      states;
      Container<str>             entryCommands;
      Container<str>             exitCommands;

      void                       readNextState( Script &script );
      void                       readMoveType( Script &script );
      void                       readCamera( Script &script );
      void                       readLegs( Script &script );
      void                       readAction( Script &script );
		void                       readBehavior( Script &script );
		void                       readTime( Script &script );
      void                       readStates( Script &script );
      void                       readCommands( Script &script, Container<str> &container );

      void                       ParseAndProcessCommand( str command, Entity *target );

   public :
                                 State( const char *name, Script &script, StateMap &map );

      State                      *Evaluate( Entity &ent, Container<Conditional *> *ent_conditionals );
      int								addCondition( const char *name, Script &script );
      void                       CheckStates( void );

      const char                 *getName( void );

      const char                 *getLegAnim( Entity &ent, Container<Conditional *> *sent_conditionals );
      const char                 *getActionAnim( Entity &ent, Container<Conditional *> *sent_conditionals, int *piAnimType = NULL );
		const char                 *getBehaviorName( void );
      State                      *getNextState( void );
      movecontrol_t              getMoveType( void );
      cameratype_t               getCameraType( void );
      qboolean                   setCameraType( str ctype );

		const char						*getBehaviorParm( int number=1 );
      void                       addBehaviorParm( str parm );
		int								numBehaviorParms( void );

		float								getMinTime( void );
		float								getMaxTime( void );
      void                       ProcessEntryCommands( Entity *target );
      void                       ProcessExitCommands( Entity *target );
      void                       GetLegAnims( Container<const char *> *c );
      void                       GetActionAnims( Container<const char *> *c );
   };

inline void State::addBehaviorParm
   (
   str parm
   )

   {
   behaviorParmList.AddObject( parm );
   }

inline const char *State::getBehaviorParm
   (
   int number
   )

   {
   return behaviorParmList.ObjectAt( number ).c_str();
   }

inline int State::numBehaviorParms
   (
   void
   )

   {
   return behaviorParmList.NumObjects();
   }

class StateMap : public Class
   {
   private :
      Container<State *>         stateList;
      Condition<Class>           *current_conditions;
      Container<Conditional *>   *current_conditionals;
		str								filename;
      
   public :
                                 StateMap( const char *filename, Condition<Class> *conditions, Container<Conditional *> *conditionals );
                                 ~StateMap();

      Condition<Class>           *getCondition( const char *name );
      int								findConditional( Conditional *condition );
      int	                     addConditional( Conditional *condition );
      Conditional                *getConditional( const char *name );
      void                       GetAllAnims( Container<const char *> *c );
      State                      *FindState( const char *name );
		const char						*Filename();
   };

inline const char *StateMap::Filename
   (
   void
   )

   {
   return filename.c_str();
   }

inline const char *State::getName
   (
   void
   )

   {
   return name.c_str();
   }

inline State *State::getNextState
   (
   void
   )

   {
   return statemap.FindState( nextState.c_str() );
   }

inline movecontrol_t State::getMoveType
   (
   void
   )

   {
   return movetype;
   }

inline cameratype_t State::getCameraType
   (
   void
   )

   {
   return cameratype;
   }

void ClearCachedStatemaps( void );
StateMap *GetStatemap( str filename, Condition<Class> *conditions,  Container<Conditional *> *conditionals, qboolean reload, qboolean cache_only = false );
void CacheStatemap( str filename, Condition<Class> *conditions	);

#endif /* !__CHARACTERSTATE_H__ */
