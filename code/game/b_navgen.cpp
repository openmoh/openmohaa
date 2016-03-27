//
// b_navgen.c
//

/*
Path costs - time is the cost basis
	For normal paths: take dist(sSurf.origin->edge + pmoveEndSpot->tSurf.origin) and
		divide by speed to get time.  Adjust for duck/wade/swim.  Add on time used for
		the pmoves to get from edge to pmoveEndspot.
	Teleporter: fixed cost of teleport delay (currently 160ms, should be a shared define)
	Plats: take vertical distance and divide by plat speed to get move time; add on
		any delays (this assumes plats are always at bottom waiting for us)
	Trains: ???
*/

/*
FIXME for Q3MAP
	When done in game, plates are seen at their low position.  Once this is in Q3MAP they will
	be seen in their high position requiring a bit of rework of that code.  I've noted most of it.
*/


#include <stdio.h>
#include "b_local.h"
#include "mover.h"


// these defines are placeholders for when this code moves into Q3MAP and is multithreaded
#define ThreadLock()
#define ThreadUnlock()

#define INFINITE	1000000

// these defines MUST match those in bg_pmove!!!
#define	MIN_WALK_NORMAL		0.7
#define	STEPSIZE			18
#define DEFAULT_VIEWHEIGHT	26
#define CROUCH_VIEWHEIGHT	12

#define	DUCKSCALE			0.25
#define	SWIMSCALE			0.50
#define	WADESCALE			0.70

// these defines MUST match the values used in the game dll
#define DEFAULT_MINS_0		-48
#define DEFAULT_MINS_1		-48
#define DEFAULT_MINS_2		0
#define DEFAULT_MAXS_0		48
#define DEFAULT_MAXS_1		48
#define DEFAULT_MAXS_2		76
#define CROUCH_MAXS_2		49

// although this is a derived value, it needs to match the game
#define MAX_JUMPHEIGHT		32

// these are used to drive the pmove based reachability function
#define MAX_PMOVES		64
#define PMOVE_MSEC		50
#define PLAYER_SPEED	320

// defines for ranges and granularity of the spot testing
#define XY_MIN	-4080
#define XY_MAX	4080
#define XY_STEP 16
#define Z_MIN	-4096
#define Z_MAX	4064
#define Z_STEP	1

#define MAX_SPOTS		65536
#define MAX_SPOTINDEX (((XY_MAX - XY_MIN) / XY_STEP) + 2 )

typedef struct {
	unsigned	flags;
	int			surfaceNum;
	vec3_t		origin;
	int			parm;
} nspot_t;

static nspot_t		*spot;
static int			spotCount;

static int		sortIndex[MAX_SPOTS];
static int		spotIndex[MAX_SPOTINDEX];
static int		spotIndexCount;


#if 0	//Q3MAP
nsurface_t	*surface;
int			surfaceCount;


nneighbor_t	*neighbor;
int			neighborCount;

byte		*route;
#endif

/*
=============
VectorToString

This is just a convenience function
for printing vectors
=============
*/
char	*vtos( const vec3_t v ) {
	static	int		index;
	static	char	str[8][32];
	char	*s;

	// use an array so that multiple vtos won't collide
	s = str[index];
	index = (index + 1)&7;

	Com_sprintf (s, 32, "(%i %i %i)", (int)v[0], (int)v[1], (int)v[2]);

	return s;
}

//
// FindSpots
//

static vec3_t		mins;
static vec3_t		duckMaxs;
static vec3_t		standMaxs;

//FIXME this should NOT be global
static gentity_t	*test;

static void FindSpotsInY( int num ) {
	trace_t		tr;
	trace_t		tr2;
	vec3_t		origin;
	vec3_t		point2;
	vec3_t		testpoint;
	qboolean	roof;
	float		viewheight;
	int			cont;
	float		sample1;
	float		sample2;
	int			waterlevel;
	//int			numEnts;
	//gentity_t	*ents[MAX_GENTITIES];
	gentity_t	*ent;
	qboolean	n;
	vec3_t		absmin;
	vec3_t		absmax;
	int			spotNum;
	float		height;
	//gentity_t	*target;
	//gentity_t	*destTarget;

	origin[0] = XY_MIN + num * XY_STEP;

	for ( origin[1] = XY_MIN; origin[1] <= XY_MAX; origin[1] += XY_STEP ) {
		origin[2] = Z_MAX;
		tr.endpos[2] = Z_MAX + Z_STEP;
		point2[0] = origin[0];
		point2[1] = origin[1];
		point2[2] = Z_MIN;
		roof = qtrue;
		height = Z_STEP;

//		gi.Printf( "begin %i %i\n", (int)origin[0], (int)origin[1] );
		while ( origin[2] > Z_MIN ) {
			origin[2] = tr.endpos[2] - height;
			height = Z_STEP;

			gi.trace( &tr, origin, mins, duckMaxs, point2, ENTITYNUM_NONE, MASK_DEADSOLID, true );

			// did we fall out the bottom? yes = this xy finished
			if ( tr.endpos[2] <= point2[2] ) {
//				gi.Printf( "  done - fell out\n" );
				break;
			}

			// were we in solid the whole time? yes = this xy finished
			if ( tr.allsolid ) {
//				gi.Printf( "  done - all solid\n" );
				break;
			}

			// did we end up inside a solid?
			gi.trace( &tr2, tr.endpos, mins, duckMaxs, tr.endpos, ENTITYNUM_NONE, MASK_DEADSOLID, true );
			if ( tr2.startsolid ) {
//				gi.Printf( "  in solid @ %f\n", tr.endpos[2] - 24.0 );
				continue;
			}

			// did we land on the roof of the level? yes = move z down a step from point of contact
			if ( roof ) {
//				gi.Printf( "  fell onto level @ %f\n", tr.endpos[2] - 24.0 );
				roof = qfalse;
				continue;
			}

			// is this too steep to stand on? yes = move z down a step from point of contact
			if ( tr.plane.normal[2] < MIN_WALK_NORMAL ) {
//				gi.Printf( "  spot too steep @ %f\n", tr.endpos[2] - 24.0 );
				continue;
			}

// FIXME rule out any we want ignored here
			// see if we are on a special entity
			ent = &g_entities[tr.entityNum];
			if ( ent->s.number != ENTITYNUM_WORLD ) {
            if ( ent->entity && ent->entity->isSubclassOf( Mover ) ) {
               continue;
            }
#if 0
				if ( Q_stricmp( ent->entname, "func_plat" ) == 0 ) {
					//FIXME what do we need to do if we are
            }
#endif
			}

			// see if we are outside the world
			VectorCopy( tr.endpos, test->s.origin );
			gi.linkentity( test );
			n = test->linked;
			gi.unlinkentity( test );
			if ( n == qfalse ) {
//				gi.Printf( "  outside world c @ %s\n", vtos( tr.endpos ) );
				continue;
			}

			if ( spotCount == MAX_SPOTS ) {
				gi.Printf( "MAX_SPOTS exceeded\n" );
				//FIXME in Q3MAP this should be an exit()
				return;
			}

			// we found a valid standing location, add it to spot list
			ThreadLock();
			spotNum = spotCount++;
			ThreadUnlock();

//			gi.Printf( "  found spot @ %f %f %f\n", tr.endpos[0], tr.endpos[1], tr.endpos[2] - 24.0);
//			VectorCopy( tr.endpos, spot[spotNum].origin );
			spot[spotNum].origin[0] = tr.endpos[0];
			spot[spotNum].origin[1] = tr.endpos[1];
			spot[spotNum].origin[2] = COORDINATE_TO_FLOAT * (int)(tr.endpos[2]*FLOAT_TO_COORDINATE);
			spot[spotNum].flags = 0;
			spot[spotNum].surfaceNum = -1;

			// see if we are on a special entity
			ent = &g_entities[tr.entityNum];
			if ( ent->s.number != ENTITYNUM_WORLD ) {
#if 0
				if ( Q_stricmp( ent->entname, "func_plat" ) == 0 ) {
					spot[spotNum].flags |= SF_PLATLOW;	// Q3MAP: SF_PLATHIGH
					height = Z_STEP;					// Q3MAP: ent->pos2[2] - ent->pos1[2];
					spot[spotNum].parm = tr.entityNum;
				}
//				else if ( Q_stricmp( ent->entname, "func_train" ) == 0 ) {
//					spot[spotNum].flags |= SF_TRAIN;
//				}
#endif
			}

         // check if we must be ducked
			VectorAdd( tr.endpos, mins, absmin );
			gi.trace( &tr2, tr.endpos, mins, standMaxs, tr.endpos, ENTITYNUM_NONE, MASK_DEADSOLID, true );
			if ( tr2.allsolid ) {
				spot[spotNum].flags |= SF_DUCK;
				viewheight = CROUCH_VIEWHEIGHT;
				VectorAdd( tr.endpos, duckMaxs, absmax );
			}
			else {
				viewheight = DEFAULT_VIEWHEIGHT;
				VectorAdd( tr.endpos, standMaxs, absmax );
			}

			// check water depth and type (depends on duck state to set viewheight)
			waterlevel = 0;
			testpoint[0] = tr.endpos[0];
			testpoint[1] = tr.endpos[1];
			testpoint[2] = tr.endpos[2] + mins[2] + 1;	
			cont = gi.pointcontents( testpoint, 0 );

			if ( cont & MASK_WATER ) {
				sample2 = viewheight - mins[2];
				sample1 = sample2 / 2;

				if ( cont & (CONTENTS_LAVA | CONTENTS_SLIME) ) {
					spot[spotNum].flags |= SF_PAIN;
				}

				waterlevel = 1;
				testpoint[2] = tr.endpos[2] + mins[2] + sample1;
				cont = gi.pointcontents( testpoint, 0 );
				if (cont & MASK_WATER) {
					waterlevel = 2;
					testpoint[2] = tr.endpos[2] + mins[2] + sample2;
					cont = gi.pointcontents( testpoint, 0 );
					if (cont & MASK_WATER) {
						waterlevel = 3;
					}
				}
			}
			if ( waterlevel & 1) {
				spot[spotNum].flags |= SF_WATERLEVEL1;
			}
			if ( waterlevel & 2) {
				spot[spotNum].flags |= SF_WATERLEVEL2;
			}

			// if water depth 3, see if there is air above us
			if ( waterlevel == 3) {
				if ( spot[spotNum].flags & SF_DUCK ) {
					spot[spotNum].flags |= SF_WATER_NOAIR;
				}
				else {
					gi.trace( &tr2, tr.endpos, mins, standMaxs, origin, ENTITYNUM_NONE, MASK_DEADSOLID, true );
					testpoint[0] = tr2.endpos[0];
					testpoint[1] = tr2.endpos[1];
					testpoint[2] = tr2.endpos[2] + DEFAULT_VIEWHEIGHT;
					cont = gi.pointcontents( testpoint, 0 );
					if (cont & MASK_WATER) {
						spot[spotNum].flags |= SF_WATER_NOAIR;
					}
				}
			}
#if 0
			// check if this spot is in a trigger we care about
			numEnts = gi.EntitiesInBox( absmin, absmax, ents, MAX_GENTITIES );
			for ( n = 0; n < numEnts; n++ ) {
				ent = ents[n];
//				if ( Q_stricmp ( ent->entname, "misc_teleporter" ) == 0 ) {
				if ( Q_stricmp ( ent->entname, "trigger_teleport" ) == 0 ) {
					spot[spotNum].flags |= SF_TELEPORTER;
					destTarget = NULL;
					destTarget = G_Find( destTarget, FOFS( targetname ), ent->target );
					if ( !destTarget ) {
						gi.Printf( "target_teleporter with no target\n" );
						spot[spotNum].parm = 0;
					}
					else {
						spot[spotNum].parm = destTarget->s.number;
					}
				}
				if ( !( ent->contents & CONTENTS_TRIGGER ) ) {
					continue;
				}
				if ( Q_stricmp ( ent->entname, "trigger_hurt" ) == 0 ) {
					spot[spotNum].flags |= SF_PAIN;
				}
				if ( Q_stricmp ( ent->entname, "trigger_push" ) == 0 ) {
					spot[spotNum].flags |= SF_PUSH;
					spot[spotNum].parm = ent->s.number;
				}
				if ( Q_stricmp ( ent->entname, "trigger_multiple" ) == 0 ) {
					target = NULL;
					while ( (target = G_Find( target, FOFS( targetname ), ent->target ) ) != NULL ) {
						if ( target == ent ) {
							continue;
						}
						if ( Q_stricmp ( target->entname, "target_teleporter" ) == 0 ) {
							spot[spotNum].flags |= SF_TELEPORTER;
							destTarget = NULL;
							destTarget = G_Find( destTarget, FOFS( targetname ), target->target );
							if ( !destTarget ) {
								gi.Printf( "target_teleporter with no target\n" );
								spot[spotNum].parm = 0;
							}
							else {
								spot[spotNum].parm = destTarget->s.number;
							}
						}
						if ( Q_stricmp ( target->entname, "target_push" ) == 0 ) {
							spot[spotNum].flags |= SF_PUSH;
							spot[spotNum].parm = target->s.number;
						}
					}
				}
			}
#endif
		}
	}
}

static void FindSpots( void ) {
	int			n;
	//gentity_t	*ent;
	//gentity_t	*target;
	//int			spotNum;

	spot =  ( nspot_t * )gi.TagMalloc ( MAX_SPOTS * sizeof(spot[0]), TAG_GAME );
	spotCount = 0;

	for ( n = 0; n < MAX_SPOTS; n++ ) {
		spot[n].flags = 0;
	}

	VectorSet( mins, DEFAULT_MINS_0, DEFAULT_MINS_1, DEFAULT_MINS_2 );
	VectorSet( standMaxs, DEFAULT_MAXS_0, DEFAULT_MAXS_1, DEFAULT_MAXS_2 );
	VectorSet( duckMaxs, DEFAULT_MAXS_0, DEFAULT_MAXS_1, CROUCH_MAXS_2 );

	test = G_Spawn();
	test->s.eType = ET_GENERAL;
	VectorCopy( mins, test->mins );
	VectorCopy( duckMaxs, test->maxs );

   //	RunThreadsOn( (XY_MAX - XY_MIN) / XY_STEP, qtrue, FindSpotsInY );
	for ( n = 0; n < ((XY_MAX - XY_MIN) / XY_STEP); n++ ) {
      gi.DebugPrintf( "FindSpotsInY %d/%d\n", n, ((XY_MAX - XY_MIN) / XY_STEP) );
		FindSpotsInY( n );
	}

	G_FreeEdict( test );


#if 0
	//FIXME add spots for trigger_teleport
	for ( n = 0; n < globals.num_entities; n++ ) {
		ent = &g_entities[n];
		if ( !ent->inuse ) {
			continue;
		}
		if ( Q_stricmp ( ent->entname, "misc_teleporter" ) == 0 ) {
		if ( Q_stricmp ( ent->entname, "trigger_teleport" ) == 0 ) {
			target = NULL;
			target = G_Find( target, FOFS( targetname ), ent->target );
			if ( !target ) {
				continue;
			}
			spotNum = spotCount++;
			spot[spotNum].origin[0] = ent->s.origin[0];
			spot[spotNum].origin[1] = ent->s.origin[1];
			spot[spotNum].origin[2] = COORDINATE_TO_FLOAT * (int)(ent->s.origin[2]*FLOAT_TO_COORDINATE);
			spot[spotNum].flags = SF_TELEPORTER;
			spot[spotNum].surfaceNum = -1;
			spot[spotNum].parm = target - &g_entities[0];
		}
	}

#endif
	gi.Printf( "    %i spots\n", spotCount );
}


//
// FindSurfaces
//

#define MAX_SURFACES	4096

static int	zstart;
static int	zend;
//static int	surfaceSizes[10];

static int CompareSpotsByZ( const void *a, const void *b ) {
	nspot_t	*s1 = &spot[*(int *)a];
	nspot_t	*s2 = &spot[*(int *)b];

	// z
	if ( s1->origin[2] > s2->origin[2] ) {
		return -1;
	}
	if ( s1->origin[2] < s2->origin[2] ) {
		return 1;
	}

	// x
	if ( s1->origin[0] < s2->origin[0] ) {
		return -1;
	}
	if ( s1->origin[0] > s2->origin[0] ) {
		return 1;
	}

	// y
	if ( s1->origin[1] < s2->origin[1] ) {
		return -1;
	}
	if ( s1->origin[1] > s2->origin[1] ) {
		return 1;
	}

	return 0;
}

static void BuildSpotIndexByZ( void ) {
	int		n;
	int		i;
	float	z;
	float	cz;

	for ( n = 0; n < spotCount; n++ ) {
		sortIndex[n] = n;
	}
	for ( n = 0; n < (((XY_MAX - XY_MIN) / XY_STEP) + 2); n++ ) {
		spotIndex[n] = 0;
	}
	qsort( sortIndex, spotCount, sizeof(sortIndex[0]), CompareSpotsByZ );

	i = 0;
	cz = -8192;
	for ( n = 0; n < spotCount; n++ ) {
		z = spot[sortIndex[n]].origin[2];
		if ( z != cz ) {
			spotIndex[i++] = n;
			cz = z;
         if ( i >= ( MAX_SPOTINDEX - 1 ) )
         {
		      gi.Printf( "MAX_SPOTINDEX exceeded\n" );
		      //FIXME in Q3MAP this should be an exit()
		      break;
         }
		}
	}
	spotIndexCount = i;
	spotIndex[i] = n;
}

static int SpotIndex( vec3_t origin, int flags ) {
	int		n;

//	gi.Printf( "    SpotIndex( %s )\n", vtos( origin ) );

	for ( n = zstart; n < zend; n++ ) {
		if ( spot[sortIndex[n]].origin[0] < origin[0] ) {
			continue;
		}
		if ( spot[sortIndex[n]].origin[0] > origin[0] ) {
			return -1;
		}
		if ( spot[sortIndex[n]].origin[1] < origin[1] ) {
			continue;
		}
		if ( spot[sortIndex[n]].origin[1] > origin[1] ) {
			return -1;
		}
		if ( spot[sortIndex[n]].surfaceNum != -1 ) {
			return -1;
		}
		if ( spot[sortIndex[n]].flags != flags ) {
			return -1;
		}
		return sortIndex[n];
	}
	return -1;
}

static void BuildSurface( int index ) {
	int			surfaceNum;
	int			plusx;
	int			minusx;
	int			plusy;
	int			minusy;
	qboolean	plusxLock;
	qboolean	minusxLock;
	qboolean	plusyLock;
	qboolean	minusyLock;
	float		xmin, xmax, x;
	float		ymin, ymax, y;
	vec3_t		test;
	int			n;
	int			count;
	int			tbm[256];
	int			sCount;
	nsurface_t	*surf;
	//int			surfaceNum2;
	//gentity_t	*ent;
	//float		height;

	if ( surfaceCount == MAX_SURFACES ) {
		gi.Printf( "MAX_SURFACES exceeded\n" );
		//FIXME in Q3MAP this should be an exit()
		return;
	}

	ThreadLock();
	surfaceNum = surfaceCount++;
	ThreadUnlock();

	spot[index].surfaceNum = surfaceNum;

	plusx = 0;
	minusx = 0;
	plusy = 0;
	minusy = 0;
	plusxLock = qfalse;
	minusxLock = qfalse;
	plusyLock = qfalse;
	minusyLock = qfalse;
	test[2] = spot[index].origin[2];
	sCount = 1;

	while ( !plusxLock || !minusxLock || !plusyLock || !minusyLock ) {
		if ( !plusxLock ) {
			plusx++;
			count = 0;
			test[0] = spot[index].origin[0] + plusx * XY_STEP;
			ymin = spot[index].origin[1] - minusy * XY_STEP;
			ymax = spot[index].origin[1] + plusy * XY_STEP;
			for ( y = ymin; y <= ymax; y += XY_STEP ) {
				test[1] = y;
				n = SpotIndex( test, spot[index].flags );
				if ( n == -1 ) {
					plusx--;
					plusxLock = qtrue;
					break;
				}
				tbm[count++] = n;
			}
			for ( n = 0; n < count; n ++ ) {
				spot[tbm[n]].surfaceNum = surfaceNum;
			}
			sCount += count;
		}

		if ( !minusxLock ) {
			minusx++;
			count = 0;
			test[0] = spot[index].origin[0] - minusx * XY_STEP;
			ymin = spot[index].origin[1] - minusy * XY_STEP;
			ymax = spot[index].origin[1] + plusy * XY_STEP;
			for ( y = ymin; y <= ymax; y += XY_STEP ) {
				test[1] = y;
				n = SpotIndex( test, spot[index].flags );
				if ( n == -1 ) {
					minusx--;
					minusxLock = qtrue;
					break;
				}
				tbm[count++] = n;
			}
			for ( n = 0; n < count; n ++ ) {
				spot[tbm[n]].surfaceNum = surfaceNum;
			}
			sCount += count;
		}

		if ( !plusyLock ) {
			plusy++;
			count = 0;
			test[1] = spot[index].origin[1] + plusy * XY_STEP;
			xmin = spot[index].origin[0] - minusx * XY_STEP;
			xmax = spot[index].origin[0] + plusx * XY_STEP;
			for ( x = xmin; x <= xmax; x += XY_STEP ) {
				test[0] = x;
				n = SpotIndex( test, spot[index].flags );
				if ( n == -1 ) {
					plusy--;
					plusyLock = qtrue;
					break;
				}
				tbm[count++] = n;
			}
			for ( n = 0; n < count; n ++ ) {
				spot[tbm[n]].surfaceNum = surfaceNum;
			}
			sCount += count;
		}

		if ( !minusyLock ) {
			minusy++;
			count = 0;
			test[1] = spot[index].origin[1] - minusy * XY_STEP;
			xmin = spot[index].origin[0] - minusx * XY_STEP;
			xmax = spot[index].origin[0] + plusx * XY_STEP;
			for ( x = xmin; x <= xmax; x += XY_STEP ) {
				test[0] = x;
				n = SpotIndex( test, spot[index].flags );
				if ( n == -1 ) {
					minusy--;
					minusyLock = qtrue;
					break;
				}
				tbm[count++] = n;
			}
			for ( n = 0; n < count; n ++ ) {
				spot[tbm[n]].surfaceNum = surfaceNum;
			}
			sCount += count;
		}
	}

#if 0
	if ( sCount <= 1 ) {
		surfaceSizes[0]++;
	}
	else if ( sCount <= 2 ) {
		surfaceSizes[1]++;
	}
	else if ( sCount <= 4 ) {
		surfaceSizes[2]++;
	}
	else if ( sCount <= 8 ) {
		surfaceSizes[3]++;
	}
	else if ( sCount <= 16 ) {
		surfaceSizes[4]++;
	}
	else if ( sCount <= 32 ) {
		surfaceSizes[5]++;
	}
	else if ( sCount <= 64 ) {
		surfaceSizes[6]++;
	}
	else if ( sCount <= 128 ) {
		surfaceSizes[7]++;
	}
	else if ( sCount <= 256 ) {
		surfaceSizes[8]++;
	}
	else {
		surfaceSizes[9]++;
	}
#endif

	surf = &surface[surfaceNum];
	surf->flags = spot[index].flags;

	surf->absmin[0] = spot[index].origin[0] - ( minusx * XY_STEP );
	surf->absmin[1] = spot[index].origin[1] - ( minusy * XY_STEP );

	surf->absmax[0] = spot[index].origin[0] + ( plusx * XY_STEP );
	surf->absmax[1] = spot[index].origin[1] + ( plusy * XY_STEP );

	surf->origin[0] = ( surf->absmin[0] + surf->absmax[0] ) * 0.5;
	surf->origin[1] = ( surf->absmin[1] + surf->absmax[1] ) * 0.5;
	surf->origin[2] = spot[index].origin[2];

   gi.DebugPrintf( "Spot %d : origin %s\n", index, vtos( surf->origin ) );

//	gi.Printf( "Surface from %s ", vtos( surf->absmin ) );
//	gi.Printf( "to %s", vtos( surf->absmax) );
//	gi.Printf( "at %s\n", vtos( surf->origin) );
#if 0
	// if this is a plat high position, build to plat low surface
	// Q#MAP: numerous changes in here
	if ( spot[index].flags & SF_PLATLOW ) {	//Q3MAP: SF_PLATHIGH
		// grab another surface
		ThreadLock();
		surfaceNum2 = surfaceCount++;
		ThreadUnlock();

		// make a copy of the other plat surface
		memcpy( &surface[surfaceNum2], &surface[surfaceNum], sizeof( nsurface_t ) );

//		// switch the flag from high to low
//		surface[surfaceNum2].flags &= ~SF_PLATHIGH;
//		surface[surfaceNum2].flags |= SF_PLATLOW;
		// switch the flag from low to high
		surface[surfaceNum2].flags &= ~SF_PLATLOW;
		surface[surfaceNum2].flags |= SF_PLATHIGH;

		// adjust the z value
		ent = &g_entities[spot[index].parm];
		height = ent->pos2[2] - ent->pos1[2];
//		surface[surfaceNum2].origin[2] -= height;
		surface[surfaceNum2].origin[2] += height;

		// set low plat surface parm to high plat surface number
//		surface[surfaceNum2].parm = surfaceNum;
		surface[surfaceNum].parm = surfaceNum2;

//		// set high plat surface parm plat cost
//		surface[surfaceNum].parm = (height / ent->speed) * 1000;
//		surface[surfaceNum2].parm = (height / ent->speed) * 1000;
		// set high plat surface parm to plat entity number
//		surface[surfaceNum].parm = spot[index].parm;
		surface[surfaceNum2].parm = spot[index].parm;
	}
#endif
	if ( spot[index].flags & SF_PUSH ) {
		surf->parm = spot[index].parm;
	}

	if ( spot[index].flags & SF_TELEPORTER ) {
		surf->parm = spot[index].parm;
	}
}

static void FindSurfacesAtZ( int zIndex ) {
	int		n;

	zstart = spotIndex[zIndex];
	zend = spotIndex[zIndex + 1];

	for ( n = zstart; n < zend; n++ ) {
		if ( spot[sortIndex[n]].surfaceNum == -1 ) {
			BuildSurface( sortIndex[n] );
		}
	}
}

static void FindSurfaces( void ) {
	int		n;

	surface = ( nsurface_t * )gi.TagMalloc ( MAX_SURFACES * sizeof(surface[0]), TAG_GAME );
	surfaceCount = 0;

//	memset( surfaceSizes, 0, sizeof( surfaceSizes ) );
	BuildSpotIndexByZ();

//	RunThreadsOn( spotIndexCount, qtrue, FindSurfacesAtZ );
	for ( n = 0; n < spotIndexCount; n++ ) {
		FindSurfacesAtZ( n );
	}

	gi.TagFree( spot );

#if 0
	gi.Printf( "%3i @ 1\n", surfaceSizes[0] );
	gi.Printf( "%3i @ 2\n", surfaceSizes[1] );
	gi.Printf( "%3i @ 3-4\n", surfaceSizes[2] );
	gi.Printf( "%3i @ 5-8\n", surfaceSizes[3] );
	gi.Printf( "%3i @ 9-16\n", surfaceSizes[4] );
	gi.Printf( "%3i @ 17-32\n", surfaceSizes[5] );
	gi.Printf( "%3i @ 33-64\n", surfaceSizes[6] );
	gi.Printf( "%3i @ 65-128\n", surfaceSizes[7] );
	gi.Printf( "%3i @ 129-256\n", surfaceSizes[8] );
	gi.Printf( "%3i @ 257+\n", surfaceSizes[9] );
#endif

	gi.Printf( "    %i surfaces\n", surfaceCount );
}

int FindSpot( vec3_t origin, int flags, float min_height )
{
	int n;

	for ( n = 0; n < spotCount ; n++ ) 
	{
		if ( spot[sortIndex[n]].origin[0] < origin[0] ) {
			continue;
		}
		if ( spot[sortIndex[n]].origin[0] > origin[0] ) {
			return -1;
		}
		if ( spot[sortIndex[n]].origin[1] < origin[1] ) {
			continue;
		}
		if ( spot[sortIndex[n]].origin[1] > origin[1] ) {
			return -1;
		}
		if ( spot[sortIndex[n]].origin[2] <= min_height ) {
			continue;
		}
		if ( spot[sortIndex[n]].surfaceNum != -1 ) {
			return -1;
		}
		if ( spot[sortIndex[n]].flags != flags ) {
			return -1;
		}
		return sortIndex[n];
	}
	return -1;
}

int FindUsedSpot( vec3_t origin, float min_height )
{
	int n;

	for ( n = 0; n < spotCount ; n++ ) 
	{
		if ( spot[sortIndex[n]].origin[0] < origin[0] ) {
			continue;
		}
		if ( spot[sortIndex[n]].origin[0] > origin[0] ) {
			return -1;
		}
		if ( spot[sortIndex[n]].origin[1] < origin[1] ) {
			continue;
		}
		if ( spot[sortIndex[n]].origin[1] > origin[1] ) {
			return -1;
		}
		if ( spot[sortIndex[n]].origin[2] <= min_height ) {
			continue;
		}
		return sortIndex[n];
	}
	return -1;
}

int FindSpotInSurface( vec3_t origin, int surfaceNum )
{
	int index;
	float current_height;

	
	current_height = Z_MIN;

	while( 1 )
	{
		index = FindUsedSpot( origin, current_height );

		if (index == -1)
			return -1;

		if (spot[index].surfaceNum == surfaceNum)
			return index;

		current_height = spot[index].origin[2];
	}

	return -1;
}

qboolean IsConvex( int surfaceNum, vec3_t check_point, int n )
{
	float i, j;
	float x1, y1;
	float x2, y2;
	float step;
	int test_y;
	float y_diff;
	vec3_t test_point;
	int test = 0;


	if (check_point[0] < spot[n].origin[0])
	{	
		x1 = check_point[0];
		y1 = check_point[1];

		x2 = spot[n].origin[0];
		y2 = spot[n].origin[1];
	}
	else
	{
		x1 = spot[n].origin[0];
		y1 = spot[n].origin[1];

		x2 = check_point[0];
		y2 = check_point[1];
	}

	if (x2 - x1 != 0)
		step = (y2 - y1) / (x2 - x1) / XY_STEP;
	else
		step = 0;

	j = 0;

	for ( i = x1 ; i < x2 ; i += XY_STEP )
	{
		// Test points here

		test_point[0] = i;

		test_y = j + .5;

		y_diff = j - test_y;

		if (0) //((y_diff < .05) && (y_diff > -.05))
		{
			test_point[1] = y1 + ((int)j) * XY_STEP;
			
			if (FindSpotInSurface( test_point, surfaceNum ) == -1)
				//return qfalse;
				test++;
			else
				test = 0;
			
		}
		else
		{
			
			test_point[1] = y1 + ((int)j) * XY_STEP;

			if (FindSpotInSurface( test_point, surfaceNum ) == -1)
			{
				if (j > 0)
					test_point[1] = y1 + ((int)(j + 1)) * XY_STEP;
				else
					test_point[1] = y1 + ((int)(j - 1)) * XY_STEP;

				if (FindSpotInSurface( test_point, surfaceNum ) == -1)
					//return qfalse;
					test++;
				else
					test = 0;
			}
			else
				test = 0;
		}

		if (test == 3)
			return qfalse;

		j += step;
	}

	return qtrue;
}

qboolean CanAddPoint1( int surfaceNum, int n )
{
	int i, j;
	float z_diff;
	int index;
	vec3_t check_point;
	float current_height;
	int has_neighbor_point;


	// Make sure can walk to nearest points in this surface

	has_neighbor_point = qfalse;

	for ( i = -1 ; i <= 1 ; i++ )
	{
		check_point[0] = spot[n].origin[0] + i * XY_STEP;

		for ( j = -1 ; j <= 1 ; j++ )
		{
			check_point[1] = spot[n].origin[1] + j * XY_STEP;

			if (! ((i == 0) && (j == 0)) )
			{
				// Find point in same surface

				current_height = Z_MIN;

				while( 1 )
				{
					index = FindUsedSpot( check_point, current_height );

					if (index == -1)
						break;

					if (spot[index].surfaceNum == surfaceNum) 
					{
						has_neighbor_point = qtrue;

						z_diff = spot[index].origin[2] - spot[n].origin[2];
						if (z_diff < 0)
							z_diff = -z_diff;

						if (z_diff > STEPSIZE)
							return qfalse;
						
						break;
					}

					current_height = spot[index].origin[2];
				}
			}
		}
	}

	// Make sure it was next to at least one other point in this surface

	if (!has_neighbor_point)
		return qfalse;

	return qtrue;
}

qboolean CanAddPoint2( int surfaceNum, int n )
{
	int i, j;
	vec3_t check_point;
	vec2_t absmin;
	vec2_t absmax;
	qboolean skip_top, skip_left, skip_right, skip_bottom;


	// Setup ansolute mins and maxs correctly

	if (spot[n].origin[0] < surface[surfaceNum].absmin[0])
	{
		absmin[0] = spot[n].origin[0];
		skip_left = qtrue;
	}
	else 
	{
		absmin[0] = surface[surfaceNum].absmin[0];
		skip_left = qfalse;
	}

	if (spot[n].origin[0] > surface[surfaceNum].absmax[0])
	{
		absmax[0] = spot[n].origin[0];
		skip_right = qtrue;
	}
	else 
	{
		absmax[0] = surface[surfaceNum].absmax[0];
		skip_right = qfalse;
	}

	if (spot[n].origin[1] < surface[surfaceNum].absmin[1])
	{
		absmin[1] = spot[n].origin[1];
		skip_top = qtrue;
	}
	else 
	{
		absmin[1] = surface[surfaceNum].absmin[1];
		skip_top = qfalse;
	}

	if (spot[n].origin[1] > surface[surfaceNum].absmax[1])
	{
		absmax[1] = spot[n].origin[1];
		skip_bottom = qtrue;
	}
	else 
	{
		absmax[1] = surface[surfaceNum].absmax[1];
		skip_bottom = qfalse;
	}

	// Test top and bottom points for convexness

	for( i = absmin[0] ; i <= absmax[0] ; i += XY_STEP )
	{
		check_point[0] = i;

		// Find top point in this column

		if (!skip_top)
		{
			for( j = absmin[1] ; j <= absmax[1] ; j += XY_STEP )
			{
				check_point[1] = j;

				if (FindSpotInSurface( check_point, surfaceNum ) != -1)
				{
					// Test for convexness

					if (!IsConvex( surfaceNum, check_point, n ))
						return qfalse;

					break;
				}
			}
		}

		// Find bottom point in this column

		if (!skip_bottom)
		{
			for( j = absmax[1] ; j >= absmin[1] ; j -= XY_STEP )
			{
				check_point[1] = j;

				if (FindSpotInSurface( check_point, surfaceNum ) != -1)
				{
					// Test for convexness

					if (!IsConvex( surfaceNum, check_point, n ))
						return qfalse;

					break;
				}
			}
		}
	}

	// Test left and right points for convexness

	for( j = absmin[1] ; j <= absmax[1] ; j += XY_STEP )
	{
		check_point[1] = j;

		// Find left point in this row

		if (!skip_left)
		{
			for( i = absmin[0] ; i <= absmax[0] ; i += XY_STEP )
			{
				check_point[0] = i;

				if (FindSpotInSurface( check_point, surfaceNum ) != -1)
				{
					// Test for convexness

					if (!IsConvex( surfaceNum, check_point, n ))
						return qfalse;

					break;
				}
			}
		}

		// Find right point in this row

		if (!skip_right)
		{
			for( i = absmax[0] ; i >= absmin[0] ; i -= XY_STEP )
			{
				check_point[0] = i;

				if (FindSpotInSurface( check_point, surfaceNum ) != -1)
				{
					// Test for convexness

					if (!IsConvex( surfaceNum, check_point, n ))
						return qfalse;

					break;
				}
			}
		}
	}

	
	/* for ( i = -1 ; i <= 1 ; i++ )
	{
		check_point[0] = spot[n].origin[0] + i * XY_STEP;

		for ( j = -1 ; j <= 1 ; j++ )
		{
			check_point[1] = spot[n].origin[1] + j * XY_STEP;

			if (! ((i == 0) && (j == 0)) )
			{
				// Find point in same surface

				current_height = Z_MIN;

				while( 1 )
				{
					index = FindUsedSpot( check_point, current_height );

					if (index == -1)
						break;

					if (spot[index].surfaceNum == surfaceNum)
						break;

					current_height = spot[index].origin[2];
				}

				if (index == -1)
				{
					current_height = Z_MIN;

					while( 1 )
					{
						index = FindUsedSpot( check_point, current_height );

						if (index == -1)
							break;

						if (spot[index].surfaceNum == surfaceNum)
							break;

						current_height = spot[index].origin[2];
					}

					if (index != -1)
						return qfalse;
				}
			}
		}
	} */

	return qtrue;
}

void AddPointToSurface( int surfaceNum, int n )
{
	spot[n].surfaceNum = surfaceNum;

	if (spot[n].origin[0] < surface[surfaceNum].absmin[0])
		surface[surfaceNum].absmin[0] = spot[n].origin[0];

	if (spot[n].origin[0] > surface[surfaceNum].absmax[0])
		surface[surfaceNum].absmax[0] = spot[n].origin[0];

	if (spot[n].origin[1] < surface[surfaceNum].absmin[1])
		surface[surfaceNum].absmin[1] = spot[n].origin[1];

	if (spot[n].origin[1] > surface[surfaceNum].absmax[1])
		surface[surfaceNum].absmax[1] = spot[n].origin[1];
}

static void BuildConvexSurface( int index ) 
{
	int			surfaceNum;
	int			plusx;
	int			minusx;
	int			plusy;
	int			minusy;
	qboolean	plusxLock;
	qboolean	minusxLock;
	qboolean	plusyLock;
	qboolean	minusyLock;
	qboolean plusxhold, minusxhold, plusyhold, minusyhold;
	qboolean plusxalreadyheld, minusxalreadyheld, plusyalreadyheld, minusyalreadyheld;
	float		xmin, xmax, x;
	float		ymin, ymax, y;
	vec3_t		test;
	int			n;
	int			count;
	int			tbm[256];
	int			sCount;
	nsurface_t	*surf;
	qboolean valid_direction;
	float current_height;
	int i;



	if ( surfaceCount == MAX_SURFACES )
	{
		gi.Printf( "MAX_SURFACES exceeded\n" );
		return;
	}

	//if (surfaceCount > 100)
	//	return;

	ThreadLock();
	surfaceNum = surfaceCount++;
	ThreadUnlock();

	spot[index].surfaceNum = surfaceNum;

	surface[surfaceNum].absmin[0] = spot[index].origin[0];
	surface[surfaceNum].absmax[0] = spot[index].origin[0];
	surface[surfaceNum].absmin[1] = spot[index].origin[1];
	surface[surfaceNum].absmax[1] = spot[index].origin[1];

	plusx  = 0;
	minusx = 0;
	plusy  = 0;
	minusy = 0;

	plusxLock  = qfalse;
	minusxLock = qfalse;
	plusyLock  = qfalse;
	minusyLock = qfalse;

	plusxhold  = qfalse;
	minusxhold = qfalse;
	plusyhold  = qfalse;
	minusyhold = qfalse;

	plusxalreadyheld  = qfalse;
	minusxalreadyheld = qfalse;
	plusyalreadyheld  = qfalse;
	minusyalreadyheld = qfalse;

	test[2] = spot[index].origin[2];
	sCount = 1;

	while ( !plusxLock || !minusxLock || !plusyLock || !minusyLock ) 
	{
		if ( ( plusxLock || plusxhold ) && ( minusxLock || minusxhold ) && ( plusyLock || plusyhold ) && ( minusyLock || minusyhold ) )
		{
			// Remove all of the holds

			plusxhold  = qfalse;
			minusxhold = qfalse;
			plusyhold  = qfalse;
			minusyhold = qfalse;

			plusxalreadyheld  = qtrue;
			minusxalreadyheld = qtrue;
			plusyalreadyheld  = qtrue;
			minusyalreadyheld = qtrue;
		}

		if ( !plusxLock && !plusxhold ) 
		{
			plusx++;
			count = 0;
			test[0] = spot[index].origin[0] + plusx * XY_STEP;
			ymin = spot[index].origin[1] - minusy * XY_STEP;
			ymax = spot[index].origin[1] + plusy * XY_STEP;
			valid_direction = qfalse;

			for ( y = ymin; y <= ymax; y += XY_STEP ) 
			{
				// Find this spot

				test[1] = y;
				current_height = Z_MIN;
				n = FindSpot( test, spot[index].flags, current_height );

				while( n != -1 )
				{
					// See if this point can be added and the surface stay convex
						
					if ( CanAddPoint1( surfaceNum, n ) )
					{
						// Add this point to this surface

						spot[n].surfaceNum = surfaceNum;
						tbm[count] = n;
						count++;

						break;
					}

					// Get the next spot at this x, y

					current_height = spot[n].origin[2];
					n = FindSpot( test, spot[index].flags, current_height );
				}
			}

			if ( plusxalreadyheld )
			{
				for( i = 0 ; i < count ; i++ )
				{
					if ( CanAddPoint2( surfaceNum, tbm[i] ) )
					{
						AddPointToSurface( surfaceNum, tbm[i] );
						valid_direction = qtrue;
					}
					else
						spot[tbm[i]].surfaceNum = -1;
				}
			}
			else
			{
				valid_direction = qtrue;

				if (count == (ymax - ymin) / XY_STEP + 1) 
				{
					for( i = 0 ; i < count ; i++ )
						AddPointToSurface( surfaceNum, tbm[i] );
				}
				else
				{
					for( i = 0 ; i < count ; i++ )
						spot[tbm[i]].surfaceNum = -1;
					plusxhold = qtrue;
					plusx--;
				}
			}

			if ( !valid_direction )
			{
				plusx--;
				plusxLock = qtrue;
			}
		}

		if ( !minusxLock && !minusxhold ) 
		{
			minusx++;
			count = 0;
			test[0] = spot[index].origin[0] - minusx * XY_STEP;
			ymin = spot[index].origin[1] - minusy * XY_STEP;
			ymax = spot[index].origin[1] + plusy * XY_STEP;
			valid_direction = qfalse;

			for ( y = ymin; y <= ymax; y += XY_STEP ) 
			{
				// Find this spot

				test[1] = y;
				current_height = Z_MIN;
				n = FindSpot( test, spot[index].flags, current_height );

				while( n != -1 )
				{
					// See if this point can be added and the surface stay convex
						
					if ( CanAddPoint1( surfaceNum, n ) )
					{
						// Add this point to this surface

						spot[n].surfaceNum = surfaceNum;
						tbm[count] = n;
						count++;

						break;
					}

					// Get the next spot at this x, y

					current_height = spot[n].origin[2];
					n = FindSpot( test, spot[index].flags, current_height );
				}
			}

			if ( minusxalreadyheld )
			{
				for( i = 0 ; i < count ; i++ )
				{
					if ( CanAddPoint2( surfaceNum, tbm[i] ) )
					{
						AddPointToSurface( surfaceNum, tbm[i] );
						valid_direction = qtrue;
					}
					else
						spot[tbm[i]].surfaceNum = -1;
				}
			}
			else
			{
				valid_direction = qtrue;
				
				if (count == (ymax - ymin) / XY_STEP + 1) 
				{
					for( i = 0 ; i < count ; i++ )
						AddPointToSurface( surfaceNum, tbm[i] );
				}
				else
				{
					for( i = 0 ; i < count ; i++ )
						spot[tbm[i]].surfaceNum = -1;
					minusxhold = qtrue;
					minusx--;
				}
			}

			if ( !valid_direction )
			{
				minusx--;
				minusxLock = qtrue;
			}
		}

		if ( !plusyLock && !plusyhold ) 
		{
			plusy++;
			count = 0;
			test[1] = spot[index].origin[1] + plusy * XY_STEP;
			xmin = spot[index].origin[0] - minusx * XY_STEP;
			xmax = spot[index].origin[0] + plusx * XY_STEP;
			valid_direction = qfalse;

			for ( x = xmin; x <= xmax; x += XY_STEP )
			{
				// Find this spot

				test[0] = x;
				current_height = Z_MIN;
				n = FindSpot( test, spot[index].flags, current_height );

				while( n != -1 )
				{
					// See if this point can be added and the surface stay convex
						
					if ( CanAddPoint1( surfaceNum, n ) )
					{
						// Add this point to this surface

						spot[n].surfaceNum = surfaceNum;
						tbm[count] = n;
						count++;

						break;
					}

					// Get the next spot at this x, y

					current_height = spot[n].origin[2];
					n = FindSpot( test, spot[index].flags, current_height );
				}
			}

			if ( plusyalreadyheld )
			{
				for( i = 0 ; i < count ; i++ )
				{
					if ( CanAddPoint2( surfaceNum, tbm[i] ) )
					{
						AddPointToSurface( surfaceNum, tbm[i] );
						valid_direction = qtrue;
					}
					else
						spot[tbm[i]].surfaceNum = -1;
				}
			}
			else
			{
				valid_direction = qtrue;
			
				if (count == (xmax - xmin) / XY_STEP + 1) 
				{
					for( i = 0 ; i < count ; i++ )
						AddPointToSurface( surfaceNum, tbm[i] );
				}
				else
				{
					for( i = 0 ; i < count ; i++ )
						spot[tbm[i]].surfaceNum = -1;
					plusyhold = qtrue;
					plusy--;
				}
			}

			if ( !valid_direction )
			{
				plusy--;
				plusyLock = qtrue;
			}
		}

		if ( !minusyLock && !minusyhold ) 
		{
			minusy++;
			count = 0;
			test[1] = spot[index].origin[1] - minusy * XY_STEP;
			xmin = spot[index].origin[0] - minusx * XY_STEP;
			xmax = spot[index].origin[0] + plusx * XY_STEP;
			valid_direction = qfalse;

			for ( x = xmin; x <= xmax; x += XY_STEP )
			{
				// Find this spot

				test[0] = x;
				current_height = Z_MIN;
				n = FindSpot( test, spot[index].flags, current_height );

				while( n != -1 )
				{
					// See if this point can be added and the surface stay convex
						
					if ( CanAddPoint1( surfaceNum, n ) )
					{
						// Add this point to this surface

						spot[n].surfaceNum = surfaceNum;
						tbm[count] = n;
						count++;

						break;
					}

					// Get the next spot at this x, y

					current_height = spot[n].origin[2];
					n = FindSpot( test, spot[index].flags, current_height );
				}
			}

			if ( minusyalreadyheld )
			{
				for( i = 0 ; i < count ; i++ )
				{
					if ( CanAddPoint2( surfaceNum, tbm[i] ) )
					{
						AddPointToSurface( surfaceNum, tbm[i] );
						valid_direction = qtrue;
					}
					else
						spot[tbm[i]].surfaceNum = -1;
				}
			}
			else
			{
				valid_direction = qtrue;

				if (count == (xmax - xmin) / XY_STEP + 1) 
				{
					for( i = 0 ; i < count ; i++ )
						AddPointToSurface( surfaceNum, tbm[i] );
				}
				else
				{
					for( i = 0 ; i < count ; i++ )
						spot[tbm[i]].surfaceNum = -1;
					minusyhold = qtrue;
					minusy--;
				}
			}

			if ( !valid_direction )
			{
				minusy--;
				minusyLock = qtrue;
			}
		}
	}

	// Setup surface info

	surf = &surface[surfaceNum];
	surf->flags = spot[index].flags;

	surf->absmin[0] = spot[index].origin[0] - ( minusx * XY_STEP );
	surf->absmin[1] = spot[index].origin[1] - ( minusy * XY_STEP );

	surf->absmax[0] = spot[index].origin[0] + ( plusx * XY_STEP );
	surf->absmax[1] = spot[index].origin[1] + ( plusy * XY_STEP );

	surf->origin[0] = ( surf->absmin[0] + surf->absmax[0] ) * 0.5;
	surf->origin[1] = ( surf->absmin[1] + surf->absmax[1] ) * 0.5;
	surf->origin[2] = spot[index].origin[2];

   gi.DebugPrintf( "surface #%d - Spot %d : origin %s\n", surfaceNum, index, vtos( surf->origin ) );
}

static int CompareSpots( const void *a, const void *b ) {
	nspot_t	*s1 = &spot[*(int *)a];
	nspot_t	*s2 = &spot[*(int *)b];

	// x
	if ( s1->origin[0] < s2->origin[0] ) {
		return -1;
	}
	if ( s1->origin[0] > s2->origin[0] ) {
		return 1;
	}

	// y
	if ( s1->origin[1] < s2->origin[1] ) {
		return -1;
	}
	if ( s1->origin[1] > s2->origin[1] ) {
		return 1;
	}

	// z
	if ( s1->origin[2] > s2->origin[2] ) {
		return -1;
	}
	if ( s1->origin[2] < s2->origin[2] ) {
		return 1;
	}

	return 0;
}

static void SortSpots( void ) 
{
	int		n;
	//int		i;
	//float	z;
	//float	cz;

	for ( n = 0; n < spotCount; n++ )
		sortIndex[n] = n;

	for ( n = 0; n < (((XY_MAX - XY_MIN) / XY_STEP) + 2); n++ )
		spotIndex[n] = 0;

	qsort( sortIndex, spotCount, sizeof(sortIndex[0]), CompareSpots );
}

static void FindConvexSurfaces( void ) 
{
	int n;

	surface = ( nsurface_t * )gi.TagMalloc ( MAX_SURFACES * sizeof(surface[0]), TAG_GAME );
	surfaceCount = 0;

	SortSpots();

	for ( n = 0; n < spotCount; n++ ) 
	{
		if ( spot[sortIndex[n]].surfaceNum == -1 )
			BuildConvexSurface( sortIndex[n] );
	}

	gi.TagFree( spot );

	gi.Printf( "    %i surfaces\n", surfaceCount );
}

//
// FindNeighbors
//

// stats on point to edge reachability testing
typedef struct {
	int		count;
	int		accept;
	int		reject1;
	int		reject2;
	int		reject3;
	int		pmoveCount;
} reachStats_t;

// stats on surface to surface testing
typedef struct {
	int		count;
	int		accept;
	int		reject;
	int		treject1;
	int		treject2;
	int		treject3;
	int		treject4;
	int		zeroNeighbors;
} surfaceStats_t;

// per source surface info
//
// The reason for the local neighbors array and MAX_NEIGHBORS_PER_SURFACE is so FindSurfaceNeighbors
// can be run on many surfaces at once when multithreaded.

#define MAX_NEIGHBORS_PER_SURFACE	256

typedef struct {
	int			number;
	nsurface_t	*surface;
	int			neighborCount;
	nneighbor_t	neighbor[MAX_NEIGHBORS_PER_SURFACE];
} sourceSurfaceInfo_t;

// per target surface info
typedef struct {
	int			number;
	nsurface_t	*surface;
	qboolean	edgeActive;
	vec2_t		edgeStart;
	vec2_t		edgeEnd;
	int			edgeFlags;
	float		cost;
} targetSurfaceInfo_t;

// per thread neighbor testing info block
typedef struct {
	reachStats_t			rStats;
	surfaceStats_t			sStats;
	sourceSurfaceInfo_t		sSurf;
	targetSurfaceInfo_t		tSurf;
} surfaceThreadInfo_t;

// global (per map) instances of stats; used to sum all the per thread stats
static reachStats_t		reachStats;
static surfaceStats_t	surfStats;


#define	FLOAT_TO_VELOCITY		16.0
#define	VELOCITY_TO_FLOAT		(1.0/FLOAT_TO_VELOCITY)

static int SurfaceNumberAtPoint( vec3_t point ) {
	vec3_t	p;
	int		n;

	p[0] = point[0];
	p[1] = point[1];
	p[2] = floor( point[2] );

	for ( n = 0; n < surfaceCount; n++ ) {
		if ( floor( surface[n].origin[2] ) != p[2] ) {
			continue;
		}
		if ( ( surface[n].absmin[0] - 16 ) > p[0] ) {
			continue;
		}
		if ( ( surface[n].absmax[0] + 16 ) < p[0] ) {
			continue;
		}
		if ( ( surface[n].absmin[1] - 16 ) > p[1] ) {
			continue;
		}
		if ( ( surface[n].absmax[1] + 16 ) < p[1] ) {
			continue;
		}
		return n;
	}

	return -1;
}


static qboolean PointIsOnSurfaceNumber( vec3_t point, int n ) {
	vec3_t	p;

	p[0] = point[0];
	p[1] = point[1];
	p[2] = floor( point[2] );

	if ( floor( surface[n].origin[2] ) != p[2] ) {
		return qfalse;
	}
	if ( ( surface[n].absmin[0] - 16 ) > p[0] ) {
		return qfalse;
	}
	if ( ( surface[n].absmax[0] + 16 ) < p[0] ) {
		return qfalse;
	}
	if ( ( surface[n].absmin[1] - 16 ) > p[1] ) {
		return qfalse;
	}
	if ( ( surface[n].absmax[1] + 16 ) < p[1] ) {
		return qfalse;
	}
	return qtrue;
}

static void ReachTrace(trace_t *result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contents, qboolean cylinder )
   {
	trace_t	tr;

	gi.trace ( &tr, start, mins, maxs, end, passEntityNum, contents, cylinder );
	
   //FIXME this is FUGLY
	while ( tr.entityNum && ( g_entities[tr.entityNum].s.eType == ET_MOVER ) )
      {
		if ( Q_stricmp( g_entities[tr.entityNum].entname, "func_plat" ) == 0 )
         {
			*result = tr;
         return;
		   }
		gi.trace ( &tr, tr.endpos, mins, maxs, end, tr.entityNum, contents, cylinder );
		if ( tr.fraction == 0.0 ) 
         {
         *result = tr;
			return;
		   }
	   }
   *result = tr;
   return;
   }

//FIXME try start running, start walking, and start motionless (special flags set if slower speed required)
static qboolean CanReach( surfaceThreadInfo_t *info, vec3_t start, vec3_t dir, int *flags ) {
	pmove_t			pm;
	playerState_t	ps;
	int				n;
	int				currentSurface;
	vec3_t			lastOrigin;

	info->rStats.count++;

	*flags = 0;

	VectorNormalize( dir );
	VectorCopy( start, lastOrigin );

	memset( &pm, 0, sizeof( pm ) );
	memset( &ps, 0, sizeof( ps ) );
	pm.ps = &ps;
	pm.cmd.msec = PMOVE_MSEC;
	pm.cmd.buttons = 0;
	pm.cmd.weapon = 0;
	pm.cmd.angles[0] = 0;
	pm.cmd.angles[1] = 0;
	pm.cmd.angles[2] = 0;
	pm.cmd.forwardmove = 127;
	pm.cmd.rightmove = 0;
	pm.cmd.upmove = 0;
	pm.tracemask = MASK_DEADSOLID;
	pm.noFootsteps = qtrue;
	pm.trace = ReachTrace;
	pm.pointcontents = gi.pointcontents;

	ps.pm_type = PM_NORMAL;
	ps.gravity = sv_gravity->value;
	ps.speed = PLAYER_SPEED;
	VectorCopy( start, ps.origin );
//	VectorClear( ps.velocity );
	VectorScale( dir, ps.speed, ps.velocity );
	ps.delta_angles[0] = 0;
	ps.delta_angles[1] = ANGLE2SHORT ( vectoyaw ( dir ) );
	ps.delta_angles[2] = 0;
	ps.groundEntityNum = 0;
	ps.clientNum = 0;						// FIXME? was ent->s.number;

	// do we need to start ducked?
	if ( info->sSurf.surface->flags & SF_DUCK ) {
		ps.pm_flags |= PMF_DUCKED;
	}

	// does the target surface require us to be ducked
	if ( info->tSurf.surface->flags & SF_DUCK ) {
		ps.pm_flags |= PMF_DUCKED;
		pm.cmd.upmove = -127;
	}

	// do we need to jump up?
	if ( ( info->tSurf.surface->origin[2] - info->sSurf.surface->origin[2] ) > STEPSIZE ) {
		// if we're ducked we can't jump so it is unreachable
		if ( ps.pm_flags & PMF_DUCKED ) {
			return qfalse;
		}
		pm.cmd.upmove = 127;
		*flags |= NF_JUMP;
	}

	// junk we need to fill in so PMove behaves correctly
	for( n = 0; n < MAX_STATS; n++ ) {
		ps.stats[n] = 0;
	}
	ps.stats[STAT_HEALTH] = 100;
	for( n = 0; n < PW_NUM_POWERUPS; n++ ) {
		ps.powerups[n] = 0;
	}

	for ( n = 0; n < MAX_PMOVES; n++ ) {

		Pmove( &pm );
		info->rStats.pmoveCount++;

		if( VectorCompare( ps.origin, lastOrigin ) != 0 ) {
			info->rStats.reject1++;
			return qfalse;
		}
		VectorCopy( ps.origin, lastOrigin );

		if ( ps.groundEntityNum == -1 ) {
			continue;
		}

		currentSurface = SurfaceNumberAtPoint( ps.origin );

		// FIXME - this happens at the spot under a door
		if ( currentSurface == -1 ) {
			continue;
		}

		if ( pm.pmoveEvent == EV_FALL_FAR ) {
			*flags |= NF_FALL2;
		}
		else if ( pm.pmoveEvent == EV_FALL_MEDIUM ) {
			*flags |= NF_FALL1;
		}

		if ( PointIsOnSurfaceNumber( ps.origin, info->tSurf.number ) ) {
			info->rStats.accept++;
			return qtrue;
		}

		if ( !PointIsOnSurfaceNumber( ps.origin, info->sSurf.number ) ) {
			info->rStats.reject2++;
			return qfalse;
		}
	}

	info->rStats.reject3++;
	return qfalse;
}


static int FindDestination_Push( surfaceThreadInfo_t *info, gentity_t *pushEnt, int *flags ) {
   return -1;
#if 0
	vec3_t			dir;
	pmove_t			pm;
	playerState_t	ps;
	int				n;
	int				currentSurface;
	qboolean		leftGround;
	vec3_t			pushVel;
	vec3_t			absmin;
	vec3_t			absmax;
	int				numEnts;
	gentity_t		*ents[MAX_GENTITIES];

	*flags = 0;
	VectorCopy( pushEnt->movedir, dir );
	VectorCopy( dir, pushVel );
	VectorNormalize( dir );

	memset( &pm, 0, sizeof( pm ) );
	memset( &ps, 0, sizeof( ps ) );
	pm.ps = &ps;
	pm.cmd.msec = PMOVE_MSEC;
	pm.cmd.buttons = 0;
	pm.cmd.weapon = 0;
	pm.cmd.angles[0] = 0;
	pm.cmd.angles[1] = 0;
	pm.cmd.angles[2] = 0;
	pm.cmd.forwardmove = 0;
	pm.cmd.rightmove = 0;
	pm.cmd.upmove = 0;
	pm.tracemask = MASK_DEADSOLID;
	pm.noFootsteps = qtrue;
	pm.trace = ReachTrace;
	pm.pointcontents = gi.pointcontents;

	ps.pm_type = PM_NORMAL;
	ps.gravity = g_gravity->value;
	ps.speed = PLAYER_SPEED;
	VectorCopy( info->sSurf.surface->origin, ps.origin );
	VectorCopy( pushVel, ps.velocity );
	ps.delta_angles[0] = 0;
	ps.delta_angles[1] = ANGLE2SHORT ( vectoyaw ( dir ) );
	ps.delta_angles[2] = 0;
	ps.groundEntityNum = 0;
	ps.playernum = 1;

	// junk we need to fill in so PMove behaves correctly
	for( n = 0; n < MAX_STATS; n++ ) {
		ps.stats[n] = 0;
	}
	ps.stats[STAT_HEALTH] = 100;
	for( n = 0; n < PW_NUM_POWERUPS; n++ ) {
		ps.powerups[n] = 0;
	}

	leftGround = qfalse;
	for ( n = 0; n < MAX_PMOVES; n++ ) {
		VectorAdd( ps.origin, mins, absmin );
		VectorAdd( ps.origin, standMaxs, absmax );
		numEnts = gi.EntitiesInBox( absmin, absmax, ents, MAX_GENTITIES );
		for ( n = 0; n < numEnts; n++ ) {
			if ( ents[n] == pushEnt ) {
				VectorCopy( pushVel, ps.velocity );
			}
		}

		Pmove( &pm );
		if ( ps.groundEntityNum == -1 ) {
			leftGround = qtrue;
			continue;
		}

		currentSurface = SurfaceNumberAtPoint( ps.origin );

		if ( currentSurface == info->sSurf.number && !leftGround ) {
			continue;
		}

		if ( pm.pmoveEvent == EV_FALL_FAR ) {
			*flags |= NF_FALL2;
		}
		else if ( pm.pmoveEvent == EV_FALL_MEDIUM ) {
			*flags |= NF_FALL1;
		}

		info->tSurf.cost = n * PMOVE_MSEC;

		return currentSurface;
	}

	return -1;
#endif
}


//	TeleportPlayer( activator, dest->s.origin, dest->s.angles );
//	AngleVectors( angles, player->client->ps.velocity, NULL, NULL );
//	VectorScale( player->client->ps.velocity, 400, player->client->ps.velocity );

static int FindDestination_Teleport( surfaceThreadInfo_t *info, gentity_t *destEnt, int *flags ) {
	pmove_t			pm;
	playerState_t	ps;
	int				n;
	int				currentSurface;
	vec3_t			lastOrigin;
	vec3_t			angles;

	*flags = 0;

	VectorCopy( destEnt->s.origin, lastOrigin );
	lastOrigin[2] += 1;

	AngleVectors( destEnt->s.angles, angles, NULL, NULL );

	memset( &pm, 0, sizeof( pm ) );
	memset( &ps, 0, sizeof( ps ) );
	pm.ps = &ps;
	pm.cmd.msec = PMOVE_MSEC;
	pm.cmd.buttons = 0;
	pm.cmd.weapon = 0;
	pm.cmd.angles[0] = 0;
	pm.cmd.angles[1] = 0;
	pm.cmd.angles[2] = 0;
	pm.cmd.forwardmove = 0;
	pm.cmd.rightmove = 0;
	pm.cmd.upmove = 0;
	pm.tracemask = MASK_DEADSOLID;
	pm.noFootsteps = qtrue;
	pm.trace = ReachTrace;
	pm.pointcontents = gi.pointcontents;

	ps.pm_type = PM_NORMAL;
	ps.gravity = sv_gravity->value;
	ps.speed = PLAYER_SPEED;
	VectorCopy( lastOrigin, ps.origin );
	VectorScale( angles, 400, ps.velocity );
	ps.delta_angles[0] = 0;
	ps.delta_angles[1] = ANGLE2SHORT ( angles[YAW] );
	ps.delta_angles[2] = 0;
	ps.groundEntityNum = 0;
	ps.clientNum = 0;

	// junk we need to fill in so PMove behaves correctly
	for( n = 0; n < MAX_STATS; n++ ) {
		ps.stats[n] = 0;
	}
	ps.stats[STAT_HEALTH] = 100;
	for( n = 0; n < PW_NUM_POWERUPS; n++ ) {
		ps.powerups[n] = 0;
	}

	n = 0;
	while ( 1 ) {
		n++;
		Pmove( &pm );
		info->rStats.pmoveCount++;

		if( VectorCompare( ps.origin, lastOrigin ) != 0 ) {
			break;
		}
		VectorCopy( ps.origin, lastOrigin );

		if ( ps.groundEntityNum == -1 ) {
			continue;
		}

		currentSurface = SurfaceNumberAtPoint( ps.origin );

		if ( pm.pmoveEvent == EV_FALL_FAR ) {
			*flags |= NF_FALL2;
		}
		else if ( pm.pmoveEvent == EV_FALL_MEDIUM ) {
			*flags |= NF_FALL1;
		}

		info->tSurf.cost = n * PMOVE_MSEC;

		return currentSurface;
	}

	return -1;
}


static vec3_t	testDir[] = { {0, 1, 0}, {-1, 0, 0}, {0, -1, 0}, {1, 0, 0}, {-0.5, 0.5, 0}, {0.5, 0.5, 0}, {-0.5, -0.5, 0}, {0.5, -0.5, 0} };

static void EdgeOpen( surfaceThreadInfo_t *info, vec3_t spot, int flags ) {
	info->tSurf.edgeActive = qtrue;
	info->tSurf.edgeStart[0] = spot[0];
	info->tSurf.edgeStart[1] = spot[1];
	info->tSurf.edgeEnd[0] = spot[0];
	info->tSurf.edgeEnd[1] = spot[1];
	info->tSurf.edgeFlags = flags;
}

static void EdgeClose( surfaceThreadInfo_t *info ) {
	nneighbor_t	*n;
	vec3_t		vec;
	float		d;

	n = &info->sSurf.neighbor[info->sSurf.neighborCount];

	n->surfaceNum = info->tSurf.number;
	if ( info->tSurf.edgeStart[0] <= info->tSurf.edgeEnd[0] ) {
		n->absmin[0] = info->tSurf.edgeStart[0];
		n->absmax[0] = info->tSurf.edgeEnd[0];
	}
	else {
		n->absmin[0] = info->tSurf.edgeEnd[0];
		n->absmax[0] = info->tSurf.edgeStart[0];
	}
	if ( info->tSurf.edgeStart[1] <= info->tSurf.edgeEnd[1] ) {
		n->absmin[1] = info->tSurf.edgeStart[1];
		n->absmax[1] = info->tSurf.edgeEnd[1];
	}
	else {
		n->absmin[1] = info->tSurf.edgeEnd[1];
		n->absmax[1] = info->tSurf.edgeStart[1];
	}

	n->origin[0] = (n->absmin[0] + n->absmax[0]) * 0.5;
	n->origin[1] = (n->absmin[1] + n->absmax[1]) * 0.5;
	n->origin[2] = info->sSurf.surface->origin[2];

	n->flags = info->tSurf.edgeFlags;

	// calc path cost

	if ( info->tSurf.cost ) {
		n->cost = info->tSurf.cost;
	}
	else {
		// take dist from source origin to point
		VectorSubtract( info->sSurf.surface->origin, n->origin, vec );
		d = VectorLength( vec );

		// add any movement weights
		if ( ( info->sSurf.surface->flags & (SF_WATERLEVEL1 | SF_WATERLEVEL2 ) ) == (SF_WATERLEVEL1 | SF_WATERLEVEL2 ) ) {
			d /= SWIMSCALE;
		}
		else if ( info->sSurf.surface->flags & SF_DUCK ) {
			d /= DUCKSCALE;
		}
		else if ( info->sSurf.surface->flags & (SF_WATERLEVEL1 | SF_WATERLEVEL2 ) ) {
			d /= WADESCALE;
		}

		// save that as first segment distance
		n->cost = d;

		// take dist from point to target origin
		VectorSubtract( info->tSurf.surface->origin, n->origin, vec );
		d = VectorLength( vec );

		// add any movement weights
		if ( ( info->tSurf.surface->flags & (SF_WATERLEVEL1 | SF_WATERLEVEL2 ) ) == (SF_WATERLEVEL1 | SF_WATERLEVEL2 ) ) {
			d /= SWIMSCALE;
		}
		else if ( info->tSurf.surface->flags & SF_DUCK ) {
			d /= DUCKSCALE;
		}
		else if ( info->tSurf.surface->flags & (SF_WATERLEVEL1 | SF_WATERLEVEL2 ) ) {
			d /= WADESCALE;
		}

		// add that to first distance
		n->cost += d;

		// convert distance to time
		n->cost = ( n->cost / (float)PLAYER_SPEED ) * 1000.0;

		// possibly add a jump cost
		if ( n->flags & NF_JUMP ) {
			n->cost += 250.0;	//FIXME what is a good value to use here? track actual time used for transitional pmoves!
		}
	}

	if ( info->tSurf.surface->flags & SF_DUCK ) {
		n->flags |= NF_DUCK;
	}

//	gi.Printf( "s%i to s%i: edge %f,%f to %f,%f f=%i\n", info->sSurf.number, info->tSurf.number, edgeStart[0], edgeStart[1], edgeEnd[0], edgeEnd[1], *edgeFlags );
//	gi.Printf( "s%i to s%i\n", info->sSurf.number, info->tSurf.number );
	info->tSurf.edgeActive = qfalse;

	info->sSurf.neighborCount++;
	if ( info->sSurf.neighborCount == MAX_NEIGHBORS_PER_SURFACE ) {
		gi.Printf( "MAX_NEIGHBORS_PER_SURFACE exceeded\n" );
	}
}

static void EdgeUpdate( surfaceThreadInfo_t *info, qboolean reachable, vec3_t spot, int flags ) {
//	gi.Printf( "s1=%i s2=%i spot=%s result=%i f=%i\n", info->sSurf.number, info->tSurf.number, vtos( spot ), reachable, flags );
	if ( reachable ) {
		if ( info->tSurf.edgeActive ) {
			if ( flags == info->tSurf.edgeFlags ) {
				// continue edge
				info->tSurf.edgeEnd[0] = spot[0];
				info->tSurf.edgeEnd[1] = spot[1];
			}
			else {
				EdgeClose( info );
				EdgeOpen( info, spot, flags );
			}
		}
		else {
			EdgeOpen( info, spot, flags );
		}
	}
	else {
		if ( info->tSurf.edgeActive ) {
			EdgeClose( info );
		}
	}
}


static void TestReachSurface( surfaceThreadInfo_t *info ) {
	int			edgeCode;
	float		xStart, xEnd;
	float		yStart, yEnd;
	float		x, y, z;
	vec3_t		spot;
	int			flags;
	qboolean	reachable;

	// determine which edges of s1 are facing s2
	edgeCode = 0;

	if ( info->tSurf.surface->absmin[1] > info->sSurf.surface->absmax[1] ) {
		edgeCode |= 1;
	}
	if ( info->tSurf.surface->absmax[0] < info->sSurf.surface->absmin[0] ) {
		edgeCode |= 2;
	}
	if ( info->tSurf.surface->absmin[0] > info->sSurf.surface->absmax[0] ) {
		edgeCode |= 4;
	}
	if ( info->tSurf.surface->absmax[1] < info->sSurf.surface->absmin[1] ) {
		edgeCode |= 8;
	}

	if ( info->tSurf.surface->absmax[1] > info->sSurf.surface->absmax[1] && info->tSurf.surface->absmin[0] <= info->sSurf.surface->absmax[0] && info->tSurf.surface->absmax[0] >= info->sSurf.surface->absmin[0] ) {
		edgeCode |= 1;
	}
	if ( info->tSurf.surface->absmin[0] < info->sSurf.surface->absmin[0] && info->tSurf.surface->absmin[1] <= info->sSurf.surface->absmax[1] && info->tSurf.surface->absmax[1] >= info->sSurf.surface->absmin[1] ) {
		edgeCode |= 2;
	}
	if ( info->tSurf.surface->absmax[0] > info->sSurf.surface->absmax[0] && info->tSurf.surface->absmin[1] <= info->sSurf.surface->absmax[1] && info->tSurf.surface->absmax[1] >= info->sSurf.surface->absmin[1] ) {
		edgeCode |= 4;
	}
	if ( info->tSurf.surface->absmin[1] < info->sSurf.surface->absmin[1] && info->tSurf.surface->absmin[0] <= info->sSurf.surface->absmax[0] && info->tSurf.surface->absmax[0] >= info->sSurf.surface->absmin[0] ) {
		edgeCode |= 8;
	}

	if ( edgeCode == 0 ) {
		gi.Printf( "bad edgeCode\n" );
		return;
	}

	// get coordinates for corners
	xStart = info->sSurf.surface->absmin[0];
	xEnd   = info->sSurf.surface->absmax[0];
	yStart = info->sSurf.surface->absmin[1];
	yEnd   = info->sSurf.surface->absmax[1];
	z      = info->sSurf.surface->origin[2];

	// run through the spots on the relevant edges
	// for each spot, see if the target surface is reachable from there
	info->tSurf.edgeActive = qfalse;

	if ( edgeCode & 1 ) {
		for ( x = xStart; x <= xEnd; x += XY_STEP ) {
			VectorSet( spot, x, yEnd, z );
			reachable = CanReach( info, spot, testDir[0], &flags );
			EdgeUpdate( info, reachable, spot, flags  );
		}
	}
	if ( info->tSurf.edgeActive ) {
		EdgeClose( info  );
	}

	if ( edgeCode & 2 ) {
		for ( y = yStart; y <= yEnd; y += XY_STEP ) {
			VectorSet( spot, xStart, y, z );
			reachable = CanReach( info, spot, testDir[1], &flags );
			EdgeUpdate( info, reachable, spot, flags  );
		}
	}
	if ( info->tSurf.edgeActive ) {
		EdgeClose( info  );
	}

	if ( edgeCode & 4 ) {
		for ( y = yStart; y <= yEnd; y += XY_STEP ) {
			VectorSet( spot, xEnd, y, z );
			reachable = CanReach( info, spot, testDir[3], &flags );
			EdgeUpdate( info, reachable, spot, flags  );
		}
	}
	if ( info->tSurf.edgeActive ) {
		EdgeClose( info  );
	}

	if ( edgeCode & 8 ) {
		for ( x = xStart; x <= xEnd; x += XY_STEP ) {
			VectorSet( spot, x, yStart, z );
			reachable = CanReach( info, spot, testDir[2], &flags );
			EdgeUpdate( info, reachable, spot, flags  );
		}
	}
	if ( info->tSurf.edgeActive ) {
		EdgeClose( info  );
	}

	// special checks for corners
	if ( ( edgeCode & 3 ) == 3 ) {
		VectorSet( spot, xStart, yEnd, z );
		reachable = CanReach( info, spot, testDir[4], &flags );
		EdgeUpdate( info, reachable, spot, flags  );
	}
	if ( info->tSurf.edgeActive ) {
		EdgeClose( info  );
	}

	if ( ( edgeCode & 5 ) == 5 ) {
		VectorSet( spot, xEnd, yEnd, z );
		reachable = CanReach( info, spot, testDir[5], &flags );
		EdgeUpdate( info, reachable, spot, flags  );
	}
	if ( info->tSurf.edgeActive ) {
		EdgeClose( info  );
	}

	if ( ( edgeCode & 10 ) == 10 ) {
		VectorSet( spot, xStart, yStart, z );
		reachable = CanReach( info, spot, testDir[6], &flags );
		EdgeUpdate( info, reachable, spot, flags  );
	}
	if ( info->tSurf.edgeActive ) {
		EdgeClose( info  );
	}

	if ( ( edgeCode & 12 ) == 12 ) {
		VectorSet( spot, xEnd, yStart, z );
		reachable = CanReach( info, spot, testDir[7], &flags );
		EdgeUpdate( info, reachable, spot, flags  );
	}
	if ( info->tSurf.edgeActive ) {
		EdgeClose( info  );
	}
}


static void TestReachSurface_Push( surfaceThreadInfo_t *info, gentity_t *pushEnt ) {
	int			flags;
	int			destSurf;

	//Q3MAP we will need to calculate movedir the same way the game dll does

	destSurf = FindDestination_Push( info, pushEnt, &flags );
	if ( destSurf == -1 ) {
		gi.Printf( "pusher at %s never hit ground?\n", vtos( info->sSurf.surface->origin ) );
	}
	else if ( destSurf != info->sSurf.number ) {
		info->tSurf.number = destSurf;
		info->tSurf.surface = &surface[destSurf];
		info->tSurf.edgeActive = qfalse;
		EdgeUpdate( info, qtrue, info->sSurf.surface->origin, flags  );
		EdgeClose( info  );
		gi.Printf( "pusher at %s ", vtos( info->sSurf.surface->origin ) );
		gi.Printf( "hits surface at %s ", vtos( info->tSurf.surface->origin ) );
		gi.Printf( "(%i,%i)-(%i,%i)\n", (int)info->tSurf.surface->absmin[0], (int)info->tSurf.surface->absmin[1], (int)info->tSurf.surface->absmax[0], (int)info->tSurf.surface->absmax[1] );
	}
	else {
		gi.Printf( WARNING "bad pusher at %s\n", vtos( info->sSurf.surface->origin ) );
	}
}


static void TestReachSurface_Teleport( surfaceThreadInfo_t *info, gentity_t *teleportEnt ) {
	int			flags;
	int			destSurf;

	destSurf = FindDestination_Teleport( info, teleportEnt, &flags );
	if ( destSurf != -1 ) {
		info->tSurf.number = destSurf;
		info->tSurf.surface = &surface[destSurf];
		info->tSurf.edgeActive = qfalse;
		EdgeUpdate( info, qtrue, info->sSurf.surface->origin, flags  );
		EdgeClose( info  );
	}
}


static void CreatePlatNeighbor( surfaceThreadInfo_t *info ) {
#if 0
   nneighbor_t	*n;
	gentity_t	*ent;

	n = &info->sSurf.neighbor[info->sSurf.neighborCount];
	ent = &g_entities[info->tSurf.surface->parm];

	n->surfaceNum = info->tSurf.number;

	n->absmin[0] = info->sSurf.surface->absmin[0] + 8;
	n->absmin[1] = info->sSurf.surface->absmin[1] + 8;

	n->absmax[0] = info->sSurf.surface->absmax[0] - 8;
	n->absmax[1] = info->sSurf.surface->absmax[1] - 8;

	VectorCopy( info->sSurf.surface->origin, n->origin );
	n->flags = NF_PLAT;
	n->cost = ( ( ent->pos2[2] - ent->pos1[2] ) / (float)PLAYER_SPEED ) * 1000.0;
	info->sSurf.neighborCount++;
#endif
}


static void FindSurfaceNeighbors( int surfaceNum ) {
	int						n;
	int						oldCount;
	surfaceThreadInfo_t		info;
	gentity_t				*ent;

	memset( &info, 0, sizeof( info ) );
	info.sSurf.number = surfaceNum;
	info.sSurf.surface = &surface[surfaceNum];
	info.sSurf.neighborCount = 0;

	//FIXME we must handle special surface cases here - push, teleport
	//  they don't need to check every other surface, where player will wind
	//	up is predetermined (roughly at least)

	// teleporter special case
	if ( info.sSurf.surface->flags & SF_TELEPORTER ) {
		ent = &g_entities[info.sSurf.surface->parm];
		TestReachSurface_Teleport( &info, ent );
		goto updateGlobals;
	}

	// trigger_push special case
	if ( info.sSurf.surface->flags & SF_PUSH ) {
		ent = &g_entities[info.sSurf.surface->parm];
		TestReachSurface_Push( &info, ent );
		goto updateGlobals;
	}
#if 0
	// Q3MAP: reverse this
	// if this is a PLATHIGH surface, we need to temporarily relink the plat to the high position
	if ( info.sSurf.surface->flags & SF_PLATHIGH ) {
		ent = &g_entities[info.sSurf.surface->parm];
		VectorCopy( ent->pos2, ent->s.origin );
		VectorCopy( ent->pos2, ent->currentOrigin );
		gi.linkentity( ent );
	}
#endif
	for ( n = 0; n < surfaceCount; n++ ) {
		if ( n == info.sSurf.number ) {
			info.sStats.treject1++;
			continue;
		}

		info.tSurf.number = n;
		info.tSurf.surface = &surface[n];

		// plat high surfaces are a special case as a target
		// only the corresponding plat low should have them as a neighbor
		if ( info.tSurf.surface->flags & SF_PLATHIGH ) {
			if ( info.sSurf.surface->flags & SF_PLATLOW ) {
				CreatePlatNeighbor( &info );
				info.sStats.accept++;
			}
			continue;
		}

		// reject if surf is too high to be reached
		if ( ( info.tSurf.surface->origin[2] - info.sSurf.surface->origin[2] ) > MAX_JUMPHEIGHT ) {
			info.sStats.treject2++;
			continue;
		}

		// reject is surf XY range is a subset of our own (meaning it is underneath us and not directly reachable)
		if ( ( info.tSurf.surface->absmin[0] >= info.sSurf.surface->absmin[0] ) && ( info.tSurf.surface->absmin[1] >= info.sSurf.surface->absmin[1] ) &&
			( info.tSurf.surface->absmax[0] <= info.sSurf.surface->absmax[0] ) && ( info.tSurf.surface->absmax[1] <= info.sSurf.surface->absmax[1] ) ) {
			info.sStats.treject3++;
			continue;
		}

		// reject if surf origin is not in PVS
		if ( !gi.inPVSIgnorePortals( info.tSurf.surface->origin, info.sSurf.surface->origin ) ) {
			info.sStats.treject4++;
			continue;
		}

		// see if it's a neighbor
		info.sStats.count++;
		oldCount = info.sSurf.neighborCount;
		TestReachSurface( &info );
		if ( info.sSurf.neighborCount != oldCount ) {
			info.sStats.accept++;
		}
		else {
			info.sStats.reject++;
		}
	}

#if 0
   // Q3MAP: reverse this
	// if this is a PLATHIGH surface, we need to restore the plat to the low position
	if ( info.sSurf.surface->flags & SF_PLATHIGH ) {
		ent = &g_entities[info.sSurf.surface->parm];
		VectorCopy( ent->pos1, ent->s.origin );
		VectorCopy( ent->pos1, ent->currentOrigin );
		gi.linkentity( ent );
	}
#endif
	if ( neighborCount + info.sSurf.neighborCount > MAX_NEIGHBORS ) {
		gi.Printf( "MAX_NEIGHBORS exceeded\n" );
		return;
	}

updateGlobals:
	// grab the critical section lock to do global variable updates
	ThreadLock();

	// reserve the block of neighbors we need
	surface[info.sSurf.number].neighborIndex = neighborCount;
	neighborCount += info.sSurf.neighborCount;

	// update global stats
	surfStats.count += info.sStats.count;
	surfStats.accept += info.sStats.accept;
	surfStats.reject += info.sStats.reject;
	surfStats.treject1 += info.sStats.treject1;
	surfStats.treject2 += info.sStats.treject2;
	surfStats.treject3 += info.sStats.treject3;
	surfStats.treject4 += info.sStats.treject4;
	if ( info.sSurf.neighborCount == 0 ) {
		surfStats.zeroNeighbors++;
	}

	reachStats.count += info.rStats.count;
	reachStats.accept += info.rStats.accept;
	reachStats.reject1 += info.rStats.reject1;
	reachStats.reject2 += info.rStats.reject2;
	reachStats.reject3 += info.rStats.reject3;
	reachStats.pmoveCount += info.rStats.pmoveCount;

	// release the lock
	ThreadUnlock();

	// now we can fill in the block of neighbors from our cache
	surface[info.sSurf.number].neighborCount = info.sSurf.neighborCount;
	if ( info.sSurf.neighborCount ) {
		memcpy( &neighbor[surface[info.sSurf.number].neighborIndex], info.sSurf.neighbor, info.sSurf.neighborCount * sizeof( info.sSurf.neighbor[0] ) );
	}

}


static void FindNeighbors( void ) {
	int n;

	neighbor = ( nneighbor_t * )gi.TagMalloc ( MAX_NEIGHBORS * sizeof(neighbor[0]), TAG_GAME );
	neighborCount = 0;

	memset( &reachStats, 0, sizeof( reachStats ) );
	memset( &surfStats, 0, sizeof( surfStats ) );

//	RunThreadsOn( spotIndexCount, qtrue, FindSurfaceNeighbors );
//#if 0
	for ( n = 0; n < surfaceCount; n++ ) {
      gi.DebugPrintf( "FindSurfaceNeighbors %d/%d\n", n, surfaceCount );
		FindSurfaceNeighbors( n );
	}
//#endif

	gi.Printf( "  %i neighbors (avg %.2f per surf)\n", neighborCount, (float)neighborCount/(float)surfaceCount );
	gi.Printf( "  %i surfaces with 0 neighbors\n", surfStats.zeroNeighbors );

	gi.Printf( "  surface to surface testing stats:\n" );
	gi.Printf( "    %i combinations\n", surfaceCount * surfaceCount );
	gi.Printf( "    %i trivial rejects (%i %i %i %i)\n", surfStats.treject1 + surfStats.treject2 + surfStats.treject3 + surfStats.treject4, surfStats.treject1, surfStats.treject2, surfStats.treject3, surfStats.treject4 );
	gi.Printf( "    %i full tests (%.2f avg per surf)\n", surfStats.count, (float)surfStats.count / (float)surfaceCount );
	gi.Printf( "    %i accepted, %i rejected\n", surfStats.accept, surfStats.reject );

	gi.Printf( "  spot to surface testing stats:\n" );
	gi.Printf( "    %i tests (%.2f avg per surf)\n", reachStats.count, (float)reachStats.count / (float)surfaceCount );
	gi.Printf( "    %i accepted, %i rejected (%i %i %i)\n", reachStats.accept, reachStats.reject1 + reachStats.reject2 + reachStats.reject3, reachStats.reject1, reachStats.reject2, reachStats.reject3 );
	gi.Printf( "    %.2f avg pmoves per test\n", (float)reachStats.pmoveCount / (float)reachStats.count );
}


//
// InflateSurfaces
//

static void InflateSurfaces( void ) {
	int		n;

	// inflate surfaces by 16 along both axis
	// also drop the Z value by mins
	for ( n = 0; n < surfaceCount; n++ ) {
		surface[n].absmin[0] -= 0;
		surface[n].absmin[1] -= 0;
		surface[n].absmax[0] += 0;
		surface[n].absmax[1] += 0;
		surface[n].origin[2] = floor( surface[n].origin[2] + mins[2] );
	}

	// inflate neighbor edges by 16 along the fixed axis
	for ( n = 0; n < neighborCount; n++ ) {
		if ( neighbor[n].absmin[0] == neighbor[n].absmax[0] ) {
			neighbor[n].absmin[0] -= 0;
			neighbor[n].absmax[0] += 0;
		}
		if ( neighbor[n].absmin[1] == neighbor[n].absmax[1] ) {
			neighbor[n].absmin[1] -= 0;
			neighbor[n].absmax[1] += 0;
		}
		neighbor[n].origin[2] = floor( neighbor[n].origin[2] + mins[2] );
	}
}


//
// CalculateAllRoutes
//

typedef enum {
	nodestate_estimate,
	nodestate_final
} nodeState_t;

typedef struct {
	float		cost;
	nodeState_t	state;
	int			predecessor;
} nodeInfo_t;

static nodeInfo_t		*nodeInfo;


static int NeighborIndex( int sourceSurfNum, int destSurfNum ) {
	int		base;
	int		n;

	if ( destSurfNum == -1 ) {
		return -1;
	}
	base = surface[sourceSurfNum].neighborIndex;
	for ( n = 0; n < surface[sourceSurfNum].neighborCount; n++ ) {
		if ( neighbor[base+n].surfaceNum == destSurfNum ) {
			return n;
		}
	}
	return -1;
}


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
SurfaceDistance

Returns the cost of moving from sourceSurfNum to targetSurfNum, 0 if there
is no direct path.
=============
*/

static float SurfaceDistance( int sourceSurfNum, int destSurfNum ) {
	nneighbor_t	*n;

	n = PathEdge( sourceSurfNum, destSurfNum );
	if ( n ) {
		return n->cost;
	}
	return 0;
}


/*
=============
CalculateRoutes

Takes the surfaces and neighbors tablea and uses Dijkstra's algorithm to
determine the shortest route from one surface to another.
=============
*/

static void CalculateRoutes( int rootSurfaceNum ) {
	int			n;
	int			currentNode;
	int			testNode;
	float		bestCost;
	int			bestNode;
	float		d;

	// initialize the node info
	for ( n = 0; n < surfaceCount; n++ ) {
		nodeInfo[n].cost = INFINITE;
		nodeInfo[n].state = nodestate_estimate;
		nodeInfo[n].predecessor = -1;
	}

	// prime thing to get the loop started
	currentNode = rootSurfaceNum;
	nodeInfo[rootSurfaceNum].cost= 0;

	// calculate the shortest path info
	// we loop surfaceCount times; a new final node is determined each iteration
	n = 0;
	while ( currentNode != -1 ) {

		bestCost = INFINITE;
		bestNode = -1;

		// test each node
		for ( testNode = 0; testNode < surfaceCount; testNode++ ) {
			// do not test it against itself
			if ( testNode == currentNode ) {
				continue;
			}

			// leave final nodes alone
			if ( nodeInfo[testNode].state == nodestate_final ) {
				continue;
			}

			if ( surface[testNode].neighborCount == 0 ) {
				continue;
			}

			// update adjacent nodes
			d =  SurfaceDistance( currentNode, testNode );
			if ( d != 0 ) {
				// see if we can improve the current estimate at the test node
				if ( nodeInfo[currentNode].cost + d < nodeInfo[testNode].cost )
				{
					nodeInfo[testNode].predecessor = currentNode;
					nodeInfo[testNode].cost = nodeInfo[currentNode].cost + d;
				}
			}

			// see if this is our new best estimate
			if ( nodeInfo[testNode].cost < bestCost ) {
				bestCost = nodeInfo[testNode].cost;
				bestNode = testNode;
			}
		}

		// mark current node as final and best node as new current node
		nodeInfo[currentNode].state = nodestate_final;
		currentNode = bestNode;
		n++;
	}

	// now fill in the route info
	for ( n = 0; n < surfaceCount; n++ ) {
		if ( n == rootSurfaceNum ) {
			route[rootSurfaceNum * surfaceCount + n] = 255;
			continue;
		}
		testNode = n;
		while ( ( testNode != -1 ) && ( nodeInfo[testNode].predecessor != rootSurfaceNum ) ) {
			testNode = nodeInfo[testNode].predecessor;
		}
		if ( testNode == -1 ) {
			route[rootSurfaceNum * surfaceCount + n] = 255;
			continue;
		}
		route[rootSurfaceNum * surfaceCount + n] = NeighborIndex( rootSurfaceNum, testNode );
	}
}


static void CalculateAllRoutes( void ) {
	int		n;

	route =  ( byte * )gi.TagMalloc ( surfaceCount * surfaceCount * sizeof( byte ), TAG_GAME );
	nodeInfo =  ( nodeInfo_t * )gi.TagMalloc ( surfaceCount * sizeof( nodeInfo[0] ), TAG_GAME );

	for ( n = 0; n < surfaceCount; n++ ) {
		 CalculateRoutes( n );
	}

	gi.TagFree( nodeInfo );
}



//
// WriteNavigationData
//

static void WriteNavigationData( void ) {
	FILE			*f;
	int				n;
	navheader_t		outHeader;
	nsurface_t		outSurface;
	nneighbor_t		outNeighbor;
	str            filename;
	str            osFilename;

	Swap_Init();

	filename = str( "maps/" ) + level.mapname.c_str() + ".nav";
	gi.Printf( "Writing %s.\n", filename.c_str() );
	osFilename = gi.FS_PrepFileWrite( filename.c_str() );
	f = fopen( osFilename.c_str(), "wb" );
	if ( !f )
	{
		gi.Printf( "Open failed.\n" );
		return;
	}

	// write version header
	outHeader.id = LittleLong( NAVFILE_ID );
	outHeader.version = LittleLong( NAVFILE_VERSION );
	outHeader.surfaceCount = LittleLong( surfaceCount );
	outHeader.neighborCount = LittleLong( neighborCount );
	fwrite( &outHeader, sizeof( outHeader ), 1, f );

	// write surfaces
	for ( n = 0; n < surfaceCount; n++ ) {
		outSurface.origin[0] = LittleFloat( surface[n].origin[0] );
		outSurface.origin[1] = LittleFloat( surface[n].origin[1] );
		outSurface.origin[2] = LittleFloat( surface[n].origin[2] );

		outSurface.absmin[0] = LittleFloat( surface[n].absmin[0] );
		outSurface.absmin[1] = LittleFloat( surface[n].absmin[1] );

		outSurface.absmax[0] = LittleFloat( surface[n].absmax[0] );
		outSurface.absmax[1] = LittleFloat( surface[n].absmax[1] );

		outSurface.flags = LittleLong( surface[n].flags );
		outSurface.neighborCount = LittleLong( surface[n].neighborCount );
		outSurface.neighborIndex = LittleLong( surface[n].neighborIndex );
		outSurface.parm = LittleLong( surface[n].parm );

		fwrite( &outSurface, sizeof( outSurface ), 1, f );

		gi.Printf( "surface%02i f=%04x n=%i@%i z=%i ", n, surface[n].flags, surface[n].neighborCount, surface[n].neighborIndex, (int)surface[n].origin[2] );
		gi.Printf( "(%i,%i)-(%i,%i)\n", (int)surface[n].absmin[0], (int)surface[n].absmin[1], (int)surface[n].absmax[0], (int)surface[n].absmax[1] );
	}

	// write neighbors
	for ( n = 0; n < neighborCount; n++ ) {
		outNeighbor.origin[0] = LittleFloat( neighbor[n].origin[0] );
		outNeighbor.origin[1] = LittleFloat( neighbor[n].origin[1] );
		outNeighbor.origin[2] = LittleFloat( neighbor[n].origin[2] );

		outNeighbor.absmin[0] = LittleFloat( neighbor[n].absmin[0] );
		outNeighbor.absmin[1] = LittleFloat( neighbor[n].absmin[1] );

		outNeighbor.absmax[0] = LittleFloat( neighbor[n].absmax[0] );
		outNeighbor.absmax[1] = LittleFloat( neighbor[n].absmax[1] );

		outNeighbor.surfaceNum = LittleLong( neighbor[n].surfaceNum );
		outNeighbor.flags = LittleLong( neighbor[n].flags );
		outNeighbor.cost = LittleLong( neighbor[n].cost );
		outNeighbor.filler = LittleLong( neighbor[n].filler );

		fwrite( &outNeighbor, sizeof( outNeighbor ), 1, f );

		gi.Printf( "neighbor%03i f=%04x surface=%02i cost=%f\n", n, neighbor[n].flags, neighbor[n].surfaceNum, neighbor[n].cost );
	}

	// write routes
	fwrite( route, surfaceCount * surfaceCount * sizeof( byte ), 1, f );

	fclose( f );
}


//
// Nav_Gen_f
//

void Nav_Gen_f( void ) {
	int			start;
	int			elapsed;
   int         i;
   gentity_t	*ent;

   for( i = 0; i < globals.num_entities; i++ )
      {
	   ent = &g_entities[i];
      if ( ent->entity && ent->entity->isSubclassOf( Mover ) )
         {
         ent->entity->unlink();
         }
      }

	// FindSpots
	gi.Printf( "FindSpots\n" );
	start = gi.Milliseconds();
	FindSpots();
	elapsed = gi.Milliseconds() - start;
	gi.Printf( "  %.2f seconds elapsed\n", (float)elapsed / 1000.0 );

	// FindSurfaces
	gi.Printf( "FindSurfaces\n" );
	start = gi.Milliseconds();
	//FindSurfaces();
	FindConvexSurfaces();
	elapsed = gi.Milliseconds() - start;
	gi.Printf( "  %.2f seconds elapsed\n", (float)elapsed / 1000.0 );

	// FindNeighbors
	gi.Printf( "FindNeighbors\n" );
	start = gi.Milliseconds();
	FindNeighbors();
	elapsed = gi.Milliseconds() - start;
	gi.Printf( "  %.2f seconds elapsed\n", (float)elapsed / 1000.0 );

	// InflateSurfaces
	gi.Printf( "InflateSurfaces\n" );
	start = gi.Milliseconds();
	InflateSurfaces();
	elapsed = gi.Milliseconds() - start;
	gi.Printf( "  %.2f seconds elapsed\n", (float)elapsed / 1000.0 );

	// CalculateAllRoutes
	gi.Printf( "CalculateAllRoutes\n" );
	start = gi.Milliseconds();
	CalculateAllRoutes();
	elapsed = gi.Milliseconds() - start;
	gi.Printf( "  %.2f seconds elapsed\n", (float)elapsed / 1000.0 );

	// WriteNavigationData
	start = gi.Milliseconds();
	WriteNavigationData();
	elapsed = gi.Milliseconds() - start;
	gi.Printf( "  %.2f seconds elapsed\n", (float)elapsed / 1000.0 );

	gi.TagFree( route );
	gi.TagFree( neighbor );
	gi.TagFree( surface );

   for( i = 0; i < globals.num_entities; i++ )
      {
	   ent = &g_entities[i];
      if ( ent->entity && ent->entity->isSubclassOf( Mover ) )
         {
         ent->entity->link();
         }
      }
}
