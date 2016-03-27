
//
// b_nav.c
//

//FIXME make botInfo, etc visible here too and get rid of all the mutliple dereferences like bot->bot->

#include "b_local.h"


#define NAVF_EDGEZONE	0x00000001

#define INFINITE 1000000

#define BOTAI_PUSHED (1<<0)

cvar_t      *nav_showsectors;

char			*navFileData;

int				surfaceCount;
nsurface_t		*surface;
int				neighborCount;
nneighbor_t		*neighbor;
byte			*route;

//#if 0

static int spawnpadModelIndex;


int Nav_SurfaceUnderPlayer( gentity_t *player ) {
	vec3_t		start;
	vec3_t		end;
	vec3_t		p;
	trace_t		tr;
	float		bestDist;
	int			bestSurf;
	vec3_t		v;
	int			n;
	float		dist;

	VectorCopy( player->s.origin, start );
	VectorCopy( player->s.origin, end );
	end[2] -= 4096;

	gi.trace ( &tr, start, player->mins, player->maxs, end, player->s.number, MASK_DEADSOLID, true );

//	p[0] = ((int)tr.endpos[0] + 8) & (~16);
//	p[1] = ((int)tr.endpos[1] + 8) & (~16);
	p[0] = tr.endpos[0];
	p[1] = tr.endpos[1];
	p[2] = floor(tr.endpos[2]+player->mins[2]);

	bestDist = INFINITE;
	bestSurf = -1;

	for ( n = 0; n < surfaceCount; n++ ) {
		if ( Q_fabs( surface[n].origin[2] - p[2] ) > 24 ) {
			continue;
		}

		VectorSubtract( p, surface[n].origin, v );
		dist = VectorLength( v );
		if ( dist < bestDist ) {
			bestDist = dist;
			bestSurf = n;
		}

		if ( surface[n].origin[2] != p[2] ) {
			continue;
		}
		if ( surface[n].absmin[0] > p[0] ) {
			continue;
		}
		if ( surface[n].absmax[0] < p[0] ) {
			continue;
		}
		if ( surface[n].absmin[1] > p[1] ) {
			continue;
		}
		if ( surface[n].absmax[1] < p[1] ) {
			continue;
		}
		return n;
	}

//	gi.Printf( "guess for %s at %s\n", ent->classname, vtos( p ) );
	return bestSurf;
}


/*
=============
Nav_GroundSurfaceNumber

Returns the surface number for where an entity is currently located.
If the entity is not on the ground, it returns -1.

FIXME we can make this more efficient
	right now surfaces are in Z sorted order
	we could make a Z index and binary search it to get to right z group fast
=============
*/
int Nav_GroundSurfaceNumber( gentity_t *ent ) {
	vec3_t		p;
	vec3_t		v;
	int			n;
	float		dist;
	float		bestDist;
	int			bestSurf;
	gentity_t	*groundEntity;

	// if ent is not on the ground it is not on a surface
	if ( ent->s.groundEntityNum == -1 ) {
		return -1;
	}

//	p[0] = ((int)ent->s.origin[0] + 8) & (~16);
//	p[1] = ((int)ent->s.origin[1] + 8) & (~16);
	p[0] = ent->s.origin[0];
	p[1] = ent->s.origin[1];
	p[2] = floor(ent->s.origin[2]+ent->mins[2]);

	// if ground is not the world we need to handle it differently.
	if ( ent->s.groundEntityNum != ENTITYNUM_WORLD ) {
		groundEntity = &g_entities[ent->s.groundEntityNum];

		// check for sitting on a spawn pad
		if ( !groundEntity->bmodel && groundEntity->s.modelindex == spawnpadModelIndex ) {
			p[2] -= 8.0;
		}
		// check for plats
		/* else if ( groundEntity->bmodel && Q_stricmp( groundEntity->classname, "func_plat" ) == 0 ) {
			// if at the top the return PLATHIGH surface number, otherwise return PLATLOW surface number
			if ( VectorCompare( groundEntity->currentOrigin, groundEntity->pos2 ) ) {
				return surface[groundEntity->navSurfaceNum].parm;
			}
			return groundEntity->navSurfaceNum;
		} */
	}

	bestDist = INFINITE;
	bestSurf = -1;

	for ( n = 0; n < surfaceCount; n++ ) {
		if ( Q_fabs( surface[n].origin[2] - p[2] ) > 24 ) {
			continue;
		}

		VectorSubtract( p, surface[n].origin, v );
		dist = VectorLength( v );
		if ( dist < bestDist ) {
			bestDist = dist;
			bestSurf = n;
		}

		if ( surface[n].origin[2] != p[2] ) {
			continue;
		}
		if ( surface[n].absmin[0] > p[0] ) {
			continue;
		}
		if ( surface[n].absmax[0] < p[0] ) {
			continue;
		}
		if ( surface[n].absmin[1] > p[1] ) {
			continue;
		}
		if ( surface[n].absmax[1] < p[1] ) {
			continue;
		}
		return n;
	}

//	gi.Printf( "guess for %s at %s\n", ent->classname, vtos( p ) );
	return bestSurf;
}


/*
=============
Nav_ItemSurfaceNumber

Returns the surface number for where an item entity is currently located.
If the entity is not on the ground, it returns -1.  This is a specialized
copy of Nav_GroundSurfaceNumber for items that caches the result.
=============
*/
int Nav_ItemSurfaceNumber( gentity_t *ent ) {
	if ( !VectorCompare( ent->s.origin, ent->navOrigin ) && level.time > ent->navTime ) {
		VectorCopy( ent->s.origin, ent->navOrigin );
		ent->navTime = level.time;
		ent->navSurfaceNum = Nav_GroundSurfaceNumber( ent );
	}
	return ent->navSurfaceNum;
}


/*
=============
Nav_EntitySurfaceNumber
=============
*/
int Nav_EntitySurfaceNumber( gentity_t *ent ) {
	if ( ent->s.eType == ET_ITEM ) {
		return Nav_ItemSurfaceNumber( ent );
	}

	/*if ( ent->classname&& strcmp( ent->classname, "bot_goal" ) == 0 ) {
		return Nav_SurfaceUnderPlayer( ent );
	}*/

	return Nav_GroundSurfaceNumber( ent );
}


/*
=============
PathEdge
=============
*/

static nneighbor_t *PathEdge( int sourceSurfNum, int destSurfNum ) {
	int		base;
	int		n;

	base = surface[sourceSurfNum].neighborIndex;
	for ( n = 0; n < surface[sourceSurfNum].neighborCount; n++ ) {
		if ( neighbor[base+n].surfaceNum == destSurfNum ) {
			return &neighbor[base+n];
		}
	}
	return NULL;
}

/*
=============
PointIsInEdgeRegion
=============
*/

static qboolean PointIsInEdgeRegion( vec3_t p, nneighbor_t *n ) {
	if ( p[0] < n->absmin[0] ) {
		return qfalse;
	}
	if ( p[0] > n->absmax[0] ) {
		return qfalse;
	}
	if ( p[1] < n->absmin[1] ) {
		return qfalse;
	}
	if ( p[1] > n->absmax[1] ) {
		return qfalse;
	}
	return qtrue;
}



/*
=============
Nav_MoveToGoal
=============
*/

int Nav_MoveToGoal( gentity_t *bot, vec3_t dir, int *flags ) {
	int			currentSurf;
	int			nextSurf;
	int			thirdSurf;
	int			goalSurf;
	int			routeIndex;
	nneighbor_t	*edge;
	nneighbor_t	*nextEdge;
	//gentity_t	*ent;
	//float		dist;

	*flags = 0;
	VectorCopy( bot->navDir, dir );

	currentSurf = bot->currentSurface;

	// if bot is airborne, just keep heading the same
	if ( currentSurf == -1 ) {
		if ( bot->aiFlags & BOTAI_PUSHED ) {
			//gi.Printf( "bot was bounced\n" );
			*flags |= NAVF_HOLD;
		}
		//gi.Printf( "not on ground\n" );
		return 0;
	}
	if ( bot->pushedTime < level.time ) {
		bot->aiFlags &= ~BOTAI_PUSHED;
	}

	if ( !bot->goalEntity ) {
//		gi.Printf( ERROR "Nav_MoveToGoal called with no goalEntity set\n" );
		return -1;
	}

	goalSurf = Nav_EntitySurfaceNumber( bot->goalEntity );
	if ( goalSurf == -1 ) {
		return -1;
	}

	// if we've changed surfaces, the surface to surface navigation flags and timer need to be cleared
	if ( currentSurf != bot->lastSurface ) {
		bot->navFlags = 0;
		bot->navTime = 0;
		//gi.Printf( "surface changed from %i to %i\n", bot->bot->lastSurface, bot->bot->currentSurface );
	}

	if ( currentSurf == goalSurf ) {
		//gi.Printf( "On target surface\n" );
		VectorSubtract( bot->goalEntity->s.origin, bot->s.origin, dir );
		//VectorCopy( dir, bot->bot->navDir );
		VectorCopy( dir, bot->navDir );
		return 0;
	}

	routeIndex = route[currentSurf * surfaceCount + goalSurf];
	if ( routeIndex == 255 ) {
		//gi.Printf( "Nav_MoveToGoal - no known route from %i to %i\n", currentSurf, goalSurf );
		return -1;
	}
	if ( routeIndex >= surface[currentSurf].neighborCount ) {
		gi.Printf( ERROR "Nav_MoveToGoal - bad routeIndex\n" );
		return -1;
	}
	nextSurf = neighbor[surface[currentSurf].neighborIndex + routeIndex].surfaceNum;

	edge = PathEdge( currentSurf, nextSurf );
	if ( !edge ) {
		gi.Printf( ERROR "Nav_MoveToGoal - %i does not have %i as a neighbor\n", currentSurf, nextSurf );
		VectorClear( dir );
		return -1;
	}

	if ( ! ( bot->navFlags & NAVF_EDGEZONE ) ) {
		if ( PointIsInEdgeRegion( bot->s.origin, edge ) ) {
			//gi.Printf( "hit edge\n" );
			bot->navFlags |= NAVF_EDGEZONE;
			bot->navTime = level.time;
		}
	}

	// if we are in the edge zone
	if ( bot->navFlags & NAVF_EDGEZONE ) {
		// if we're trying to get onto a plat, we must make sure it's there
		/*if ( surface[nextSurf].flags & SF_PLATLOW ) {
			ent = &g_entities[surface[surface[nextSurf].parm].parm];	//FIXME this works for now, but I don't like it
			if ( VectorCompare( ent->currentOrigin, ent->pos1 ) == 0 ) {
				*flags |= NAVF_HOLD;
				//gi.Printf(" wait for plat2\n" );
			}
		}*/

		// if we're riding up on a plat, we don't need to move
		if ( surface[nextSurf].flags & SF_PLATHIGH ) {
			*flags |= NAVF_HOLD;
			//gi.Printf(" hold on plat\n" );
		}

		// if the next surface contains the goalEntity, head towards it
		if ( nextSurf == goalSurf ) {
			//gi.Printf( "next surf has goal - targeting directly\n" );
			VectorSubtract( bot->goalEntity->s.origin, bot->s.origin, dir );
			//VectorCopy( dir, bot->bot->navDir );
			VectorCopy( dir, bot->navDir );
		}
		// start heading towards the next edge
		else {
			routeIndex = route[nextSurf * surfaceCount + goalSurf];
			if ( routeIndex == 255 ) {
				gi.Printf( ERROR "Nav_MoveToGoal - no known route from %i to %i\n", nextSurf, goalSurf );
				return -1;
			}
			if ( routeIndex >= surface[nextSurf].neighborCount ) {
				gi.Printf( ERROR "Nav_MoveToGoal - bad routeIndex\n" );
				return -1;
			}
			thirdSurf = neighbor[surface[nextSurf].neighborIndex + routeIndex].surfaceNum;
			nextEdge = PathEdge( nextSurf, thirdSurf );
			if ( !nextEdge ) {
				gi.Printf( ERROR "Nav_MoveToGoal - %i does not have %i as a neighbor\n", nextSurf, thirdSurf );
				VectorClear( dir );
				return -1;
			}
			//gi.Printf( "targeting next edge\n" );
			if ( surface[nextSurf].flags & SF_PLATHIGH ) {
				VectorSubtract( nextEdge->origin, surface[nextSurf].origin, dir );
			}
			else {
				VectorSubtract( nextEdge->origin, bot->s.origin, dir );
			}
			//VectorCopy( dir, bot->bot->navDir );
			VectorCopy( dir, bot->navDir );
		}

		if ( edge->flags & NF_DUCK ) {
			*flags |= NAVF_DUCK;
		}
		if ( edge->flags & NF_JUMP ) {
			*flags |= NAVF_JUMP;
		}
	}
	else {
		VectorSubtract( edge->origin, bot->s.origin, dir );
		//VectorCopy( dir, bot->bot->navDir );
		VectorCopy( dir, bot->navDir );

		/*if ( surface[nextSurf].flags & SF_PLATLOW ) {
			ent = &g_entities[surface[surface[nextSurf].parm].parm];	//FIXME this works for now, but I don't like it
			if ( VectorCompare( ent->currentOrigin, ent->pos1 ) == 0 ) {
				dist = VectorLength( dir );
				if ( dist > 64 ) {
					*flags |= NAVF_SLOW;
					//gi.Printf(" slow for plat\n" );
				}
				else {
					*flags |= NAVF_HOLD;
					//gi.Printf(" wait for plat\n" );
				}
			}
		}*/
	}

	return 0;
}


void Nav_ShowPath( gentity_t *bot ) {
#if 0
	gentity_t	*tent;
	int			m, n;

	if ( !bot->bot->goalEntity ) {
		return;
	}

	tent = G_TempEntity( bot->s.origin, EV_DEBUG_LINE );
	VectorCopy( bot->bot->currentWaypoint->s.origin, tent->s.origin2 );

	m = bot->bot->currentWaypoint->count;
	for (;;) {
		if ( m == bot->bot->finalWaypoint->count ) {
			break;
		}
		n = route[m*maxwaypoints+bot->bot->finalWaypoint->count];
		if ( n == -1 ) {
			break;
		}
		tent = G_TempEntity( rents[m]->s.origin, EV_DEBUG_LINE );
		VectorCopy( rents[n]->s.origin, tent->s.origin2 );
		m = n;
	}

	if ( bot->bot->finalWaypoint != bot->bot->goalEntity ) {
		tent = G_TempEntity( bot->bot->finalWaypoint->s.origin, EV_DEBUG_LINE );
		VectorCopy( bot->bot->goalEntity->s.origin, tent->s.origin2 );
	}
#endif
	/* gentity_t	*tent;
	int			m, n;
	gentity_t	*player;
	int			pSurf;

   player = &g_entities[1];
	pSurf = Nav_GroundSurfaceNumber( player );

	tent = G_TempEntity( player->s.origin, EV_DEBUG_LINE );
	VectorCopy( surface[pSurf].origin, tent->s.origin2 ); */
}
//#endif


/*
// Init and Shutdown
//
//
*/

static void Nav_Cleanup( void ) {
	if ( navFileData ) {
		gi.FS_FreeFile ( navFileData );
		navFileData = NULL;
	}
	surfaceCount = 0;
	neighborCount = 0;
}

void Nav_InitPreSpawn( void ) {
   nav_showsectors = gi.cvar( "nav_showsectors", "0", 0 );
	Nav_Cleanup();
	Nav_LoadRoutes();
}

void Nav_InitPostSpawn( void ) {
#if 0
	int			n;
	nsurface_t	*s;
	gentity_t	*ent;

	// FIXME resolve targetnames here (like button needed to open a door)

	// get the modelindex for the spawnpad model so we can use it for surface determination
	spawnpadModelIndex = G_ModelIndex( "models/objects/dmspot.md3" );

	// set the navSurface for plats
	for ( n = 0; n < surfaceCount; n++ ) {
		s = &surface[n];
		if ( s->flags & SF_PLATLOW ) {
			ent = &g_entities[surface[s->parm].parm];	//FIXME this works for now, but I don't like it
			ent->navSurfaceNum = n;
		}
	}
#endif
}


void Nav_Shutdown ( void ) {
	Nav_Cleanup();
}

void Nav_Test_f( void ) {
#if 0
	gentity_t	*player;
	gentity_t	*goal;
	char		*goalname;
	int			pSurf;
	int			gSurf;
	int			cSurf;
	int			n;
	gentity_t	*tent;

	player = &g_entities[0];
	pSurf = Nav_GroundSurfaceNumber( player );

	goalname = gi.argv(2);
	if ( !goalname[0] ) {
		gi.Printf( "Player1 is at (%f, %f, %f) on surface %i\n", player->s.origin[0], player->s.origin[1], player->s.origin[2] + player->mins[2], pSurf );
		return;
	}

	goal = NULL;
	goal = G_Find( goal, FOFS( classname ), goalname );
	if ( !goal ) {
		gi.Printf( "no %s on level\n", goalname );
		return;
	}
	gSurf = Nav_EntitySurfaceNumber( goal );


	cSurf = pSurf;
	while ( cSurf != gSurf ) {
		n = route[cSurf * surfaceCount + gSurf];
		if ( n == 255 ) {
			//gi.Printf( "no known route from %i to %i\n", cSurf, gSurf );
			return;
		}
		if ( n >= surface[cSurf].neighborCount ) {
			gi.Printf( ERROR "bad routeIndex\n" );
			return;
		}
		n = neighbor[surface[cSurf].neighborIndex + n].surfaceNum;

		if ( cSurf == pSurf ) {
			tent = G_TempEntity( player->s.origin, EV_DEBUG_LINE );
		}
		else {
			tent = G_TempEntity( surface[cSurf].origin, EV_DEBUG_LINE );
		}
		if ( n == gSurf ) {
			VectorCopy( goal->s.origin, tent->s.origin2 );
		}
		else {
			VectorCopy( surface[n].origin, tent->s.origin2 );
		}

		cSurf = n;
	}
#endif
}

void Nav_Gen_f( void );

void Cmd_Nav_f( void )
{
	char	*cmd;

	cmd = gi.argv(1);

	if ( Q_stricmp ( cmd, "gen" ) == 0 ) {
		Nav_Gen_f();
      Nav_InitPreSpawn();
	}
	else if ( Q_stricmp ( cmd, "test" ) == 0 ) {
		Nav_Test_f();
	}
	else {
		gi.Printf("Unknown nav command '%s'\n", cmd);
	}
}

void Nav_ShowStuff
   (
   void
   )

   {
   int i;
   nsurface_t *surf;

   if ( !nav_showsectors->integer )
      {
      return;
      }

   G_Color4f( 1, 1, 0, 1 );
   for( i = 0; i < surfaceCount; i++ )
      {
      surf = &surface[ i ];

      G_BeginLine();
      G_Vertex( Vector( surf->absmin[ 0 ], surf->absmin[ 1 ], surf->origin[ 2 ] ) );
      G_Vertex( Vector( surf->absmin[ 0 ], surf->absmax[ 1 ], surf->origin[ 2 ] ) );
      G_Vertex( Vector( surf->absmax[ 0 ], surf->absmax[ 1 ], surf->origin[ 2 ] ) );
      G_Vertex( Vector( surf->absmax[ 0 ], surf->absmin[ 1 ], surf->origin[ 2 ] ) );
      G_Vertex( Vector( surf->absmin[ 0 ], surf->absmin[ 1 ], surf->origin[ 2 ] ) );
      G_EndLine();
      }
   }
