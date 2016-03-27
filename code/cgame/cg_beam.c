/*
===========================================================================
Copyright (C) 2000 by Ritual Entertainment, Inc.
Copyright (C) 2011 by su44

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
//
// cg_beams.c - beam effects

#include "cg_local.h"

#define BEAM_LIGHTNING_EFFECT   (1<<0)
#define BEAM_USEMODEL           (1<<1)
#define BEAM_PERSIST_EFFECT     (1<<2)
#define BEAM_SPHERE_EFFECT      (1<<3)
#define BEAM_RANDOM_DELAY       (1<<4)
#define BEAM_TOGGLE             (1<<5)
#define BEAM_RANDOM_TOGGLEDELAY (1<<6)
#define BEAM_WAVE_EFFECT        (1<<7)
#define BEAM_USE_NOISE          (1<<8)
#define BEAM_PARENT             (1<<9)
#define BEAM_TILESHADER         (1<<10)
#define BEAM_OFFSET_ENDPOINTS   (1<<11)

#define MAX_BEAMS 256

typedef struct beam_s {
	int		   entity;
	qhandle_t   hModel;
	int		   endtime;
	vec3_t	   start, end;
	float       scale;
	float       alpha;
	int         flags;
	int         parent;
	float       max_offset;
	float       min_offset;
	int         numSubdivisions;
	int         overlap;
	int         beamshader;
	byte        shaderRGBA[4];
	int         update_time;
	int         delay;
	float       life;
	int         numspherebeams;
	float       sphereradius;
	int         toggletime;
	int         toggledelay;
	qboolean    active;
	float       alphastep;
	int         renderfx;
	char        name[128];
} beam_t;

void CG_InitBeam(beam_t *b) {
	memset(b,0,sizeof(beam_t));
	b->parent=ENTITYNUM_NONE;
}

static beam_t cg_beams[MAX_BEAMS];

static int seed = 100; // chosen by fair dice roll

#define MAX_BEAM_BACKUP   6
#define MAX_BEAM_SEGMENTS 32

typedef struct beamSegment_s {
	polyVert_t points[4];
} beamSegment_t;

typedef struct beamList_s {
	int time;
	int updatetime;
	int numsegments;
	beamSegment_t segments[MAX_BEAM_SEGMENTS];
} beamList_t;

typedef struct beamEnt_s {
	int owner;
	int numbeams;
	int life;
	int renderfx;
	beamList_t beamlist[MAX_BEAM_BACKUP];
	struct beamEnt_s *next, *prev;
} beamEnt_t;

#define MAX_BEAM_ENTITIES 512

static beamEnt_t beamEnts[MAX_BEAM_ENTITIES];
static beamEnt_t cg_activeBeamEnts;	// double linked list
static beamEnt_t *cg_freeBeamEnts;	// single linked list

void CG_InitBeams() {
	int		i;

	memset( beamEnts, 0, sizeof( beamEnts ) );
	cg_activeBeamEnts.next = &cg_activeBeamEnts;
	cg_activeBeamEnts.prev = &cg_activeBeamEnts;
	cg_freeBeamEnts = beamEnts;
	for ( i = 0 ; i < MAX_BEAM_ENTITIES - 1 ; i++ ) {
		beamEnts[i].next = &beamEnts[i+1];
	}
}

static beamEnt_t *CG_AllocBeamEnt() {
	beamEnt_t	*be;

	if ( !cg_freeBeamEnts ) {
		CG_Printf("CG_AllocBeamEnt: no free beamEnts\n");
		return 0;
	}

	be = cg_freeBeamEnts;
	cg_freeBeamEnts = cg_freeBeamEnts->next;

	memset( be, 0, sizeof( *be ) );

	// link into the active list
	be->next = cg_activeBeamEnts.next;
	be->prev = &cg_activeBeamEnts;
	cg_activeBeamEnts.next->prev = be;
	cg_activeBeamEnts.next = be;
	return be;
}

static void CG_FreeBeamEnt(beamEnt_t *be) {
	if ( !be->prev ) {
		CG_Error( "CG_FreeBeamEnt: not active" );
	}

	// remove from the doubly linked active list
	be->prev->next = be->next;
	be->next->prev = be->prev;

	// the free list is only singly linked
	be->next = cg_freeBeamEnts;
	cg_freeBeamEnts = be;
}

static void RemoveBeamList(int owner) {
	beamEnt_t	*be, *next;
	be = cg_activeBeamEnts.next;
	for ( ; be != &cg_activeBeamEnts ; be = next ) {
		// grab next now, so if the event is freed we
		// still have it
		next = be->next;
		if ( owner == be->owner ) {
			// free
			CG_FreeBeamEnt(be);
			continue;
		}
	}

}

static beamEnt_t *FindBeamList( int owner ) {
	beamEnt_t	*be, *next;
	be = cg_activeBeamEnts.next;
	for ( ; be != &cg_activeBeamEnts ; be = next ) {
		// grab next now, so if the event is freed we
		// still have it
		next = be->next;
		if ( owner == be->owner ) {
			return be;
		}
	}
	return NULL;
}

static int CreateNewBeamEntity(int owner, float life) {
	beamEnt_t   *be;
	int         i,oldest,oldest_time;

	be = FindBeamList( owner );

	if ( !be ) {
		be = CG_AllocBeamEnt();

		if ( !be )
			CG_Error("Could not allocate memory for beamEnt.\n" );

		memset ( be, 0, sizeof( beamEnt_t ) );
		memset ( be->beamlist, 0, sizeof( beamList_t ) * MAX_BEAM_BACKUP );
		be->owner = owner;
		be->life  = life;


	}

	// find the oldest beam and overwrite it.
	oldest      = -1;
	oldest_time = 999999999;

	for ( i=0; i<MAX_BEAM_BACKUP; i++ )	{
		// Check for update time
		float t = be->beamlist[ i ].time;

		if ( !t ) {
			oldest = i;
			break;
		}

		if ( t < oldest_time ) {
			oldest = i;
			oldest_time = t;
		}
	}

	// Use the oldest beam for the next beam.
	be->beamlist[ oldest ].numsegments = 0;
	be->beamlist[ oldest ].time        = cg.time;
	be->beamlist[ oldest ].updatetime  = cg.time + be->life;
	return oldest;
}

static void RemoveBeamEntity(int owner) {
	RemoveBeamList( owner );
}

static void AddBeamSegmentToList(int owner, polyVert_t points[4], int beamnum,
	int segnum, int renderfx) {
	beamEnt_t *be;

	be = FindBeamList( owner );

	if ( !be ) {
		CG_Printf( "Could not find beam entity for owner:%d\n", owner );
		return;
	}

	if ( segnum >= MAX_BEAM_SEGMENTS )
		return;
	
	be->renderfx = renderfx;

	// Copy the 4 points
	memcpy( &be->beamlist[beamnum].segments[segnum].points, points, 4 * sizeof( polyVert_t ) );

	// Increase the segment counter
	be->beamlist[beamnum].numsegments++;
}

static void CG_AddBeamsFromList(int owner, int beamshader) {
	int         i,j,k,l;
	float       frac, fade;
	beamEnt_t   *be = FindBeamList( owner );
	polyVert_t  newpoints[4];

	if ( !be ) {
		return;
	}

	for ( i=0; i<MAX_BEAM_BACKUP; i++ )	{
		beamList_t  *bl = &be->beamlist[i];

		if ( !bl->time )
			continue;

		// Calculate the blend factor for fading 
		frac   =  (float)( cg.time - bl->time ) / (float)be->life;
		fade   =  1.0f - frac;

		if ( fade <= 0 ) {
			bl->time = 0; // RemoveBeamList( owner );
			continue;
		}

		// Go through each segment and draw it with the new modulate
		assert( bl->numsegments < MAX_BEAM_SEGMENTS );

		for ( j=0; j<bl->numsegments; j++ )	{
			memcpy( newpoints, bl->segments[j].points, 4 * sizeof( polyVert_t ) );

			for ( k=0; k<4; k++ ) {
				for ( l=0; l<4; l++ ) {
					newpoints[k].modulate[l] = bl->segments[j].points[k].modulate[l] * fade; 
				}
			}

			cgi.R_AddPolyToScene( beamshader, 4, newpoints/*, be->renderfx*/ );
		}
	}
}

static void RenderSegment(vec3_t pt1a, vec3_t pt1b, vec3_t pt2a, vec3_t pt2b,   
   byte   modulate[4], int beamshader, int renderfx) {
	int         i,j;
	polyVert_t  points[4];

	VectorCopy( pt1a, points[0].xyz );
	VectorCopy( pt2a, points[1].xyz );
	VectorCopy( pt2b, points[2].xyz );
	VectorCopy( pt1b, points[3].xyz );

	points[0].st[0] = 1;   points[0].st[1] = 1;
	points[1].st[0] = 0;   points[1].st[1] = 1;
	points[2].st[0] = 0;   points[2].st[1] = 0;
	points[3].st[0] = 1;   points[3].st[1] = 0;

	// Set the color of the verts
	for ( i=0; i<4; i++ ) {
		for ( j=0; j<4; j++ ) {
			points[i].modulate[j] = modulate[j];
		}
	}

	// Add a segment to the list
	cgi.R_AddPolyToScene( beamshader, 4, points/*, renderfx */);
}

#define MAX_SUBPOINTS 256
static vec3_t subpoints[MAX_SUBPOINTS];
static int ptctr=0;
/*
===============
CG_Subdivide

a, b, and c are control points.
the subdivided sequence will be: a, out1, out2, out3, c
===============
*/
static void CG_Subdivide( vec3_t a, vec3_t b, vec3_t c, vec3_t out1, vec3_t out2, vec3_t out3 ) {
	VectorLerp(a,b,0.5,out1);
	VectorLerp(b,c,0.5,out3);
	VectorLerp(out1,out3,0.5,out2);
}

static void CG_MultiBeamBegin() {
	ptctr = 0;   
}

static void CG_MultiBeamAddPoints(vec3_t start, vec3_t end, int numsegments, int flags,
	float minoffset, float maxoffset, qboolean addstartpoint) {
	vec3_t   delta,dir,randdir;
	float    length;
	int      i;

	if ( ptctr > MAX_SUBPOINTS )
		return;

	if ( addstartpoint ) {
		_VectorCopy(start, subpoints[ ptctr++ ]);
	}

	VectorSubtract(end,start,delta);
	length = VectorLength(delta);
	length /= numsegments;

	// get the dir of beam
	VectorCopy(delta,dir);
	VectorNormalize(dir);

	for ( i=1; i<numsegments; i++ ) {
		vec3_t newpt;
		vec3_t tmp;

		if ( ptctr > MAX_SUBPOINTS )
			return;

		randdir[0] = crandom();
		randdir[1] = crandom();
		randdir[2] = crandom();

		VectorScale(dir,i * length,newpt);
		VectorAdd(newpt,start,newpt);

		VectorScale(randdir,minoffset,tmp);
		VectorAdd(newpt,tmp,newpt);

		VectorScale(randdir,maxoffset,tmp);
		VectorAdd(newpt,tmp,newpt);

		_VectorCopy(newpt,  subpoints[ ptctr++ ]);       
	}
	_VectorCopy(end,  subpoints[ ptctr++ ]);
}

void CG_MultiBeamEnd( float scale, int renderfx, const char *beamshadername, 
		byte modulate[4], int flags, int owner, float life) {
	vec3_t         prevpt,currpt;
	vec3_t         v1,v2,up,currpt1,currpt2,prevpt1,prevpt2;
	qboolean       prevptvalid=qfalse;
	int            i,beamshader;

	beamshader  = cgi.R_RegisterShader( beamshadername );

	VectorCopy(subpoints[0],prevpt);
	prevptvalid = qfalse;

	for ( i=1; i<ptctr; i++ ) {
		vec3_t tmp;
		VectorCopy(subpoints[i],currpt);
		// Generate the up vector
		VectorSubtract(prevpt,cg.refdef.vieworg,v1);
		VectorSubtract(currpt,cg.refdef.vieworg,v2);

#if 0
		cgi.R_DebugLine( prevpt, currpt, 1,1,1,1);
		vec3_t pt = prevpt + up * 5;
		cgi.R_DebugLine( prevpt, pt, 0,0,1,1);
#endif

		CrossProduct(v1,v2,up);
		VectorNormalize(up);

		// Calculate the first points
		VectorScale(up,scale,tmp);
		VectorAdd(currpt,tmp,currpt1);

		VectorScale(up,-scale,tmp);
		VectorAdd(currpt,tmp,currpt2);

		if ( !prevptvalid ) {
			VectorScale(up,scale,tmp);
			VectorAdd(prevpt,tmp,prevpt1);	

			VectorScale(up,-scale,tmp);
			VectorAdd(prevpt,tmp,prevpt2);		
			prevptvalid = qtrue;
		}
#if 1
		RenderSegment( currpt1, currpt2, prevpt1, prevpt2, modulate, beamshader, renderfx );
#endif

		VectorCopy(currpt,prevpt);
		VectorCopy(currpt1,prevpt1);
		VectorCopy(currpt2,prevpt2);
	}

	if ( flags & BEAM_PERSIST_EFFECT )
		CG_AddBeamsFromList( owner, beamshader );
}


static void CG_MultiBeamSubdivide(centity_t *cent) {
	vec3_t    pt1, pt2, pt3;
	vec3_t    out1, out2, out3, out4, out5, out6, out7, out8, out9;
	centity_t *current;

	ptctr = 0;
	current = cent;

	// Multibeam requires at least 3 points to start with

	// Get pt1
	if ( current->currentState.tag_num == ENTITYNUM_NONE ) {
		CG_Printf( "CG_MultiBeamSubdivide : Multi beam entity does not have a child\n" );
		return;
	}
	VectorCopy(current->lerpOrigin, pt1);

	// Get pt2
	current = &cg_entities[ current->currentState.tag_num ];
	// Make sure that child is a multibeam
	if ( current->currentState.eType != ET_MULTIBEAM )
		return;

	if ( current->currentState.tag_num == ENTITYNUM_NONE ) {
		CG_Printf( "CG_MultiBeamSubdivide : Multi beam entity does not have a child\n" );
		return;
	}

	VectorCopy(current->lerpOrigin,pt2);

	// Get pt3
	current = &cg_entities[ current->currentState.tag_num ];
	// Make sure that child is a multibeam
	if ( current->currentState.eType != ET_MULTIBEAM ) {
		return;
	}

	if ( current->currentState.tag_num == ENTITYNUM_NONE ) {
		CG_Printf( "CG_MultiBeamSubdivide : Multi beam entity does not have a child\n" );
		return;
	}
	VectorCopy(current->lerpOrigin,pt3);

	// First point into the subdivided points
	_VectorCopy(pt1, subpoints[ptctr++]);

	while ( 1 ) {
		// Do the subdivide
		CG_Subdivide( pt1, pt2, pt3, out1, out2, out3 );
		CG_Subdivide( pt1, out1, out2, out4, out5, out6 );

		if ( ( ptctr + 4 ) > MAX_SUBPOINTS ) 
			break;

		// Save the points
		_VectorCopy(out4,subpoints[ptctr++]);
		_VectorCopy(out5,subpoints[ptctr++]);
		_VectorCopy(out6,subpoints[ptctr++]);
		_VectorCopy(out2,subpoints[ptctr++]);

		// end condition
		if ( ( current->currentState.tag_num == ENTITYNUM_NONE ) || ( !current->currentValid ) ) {
			CG_Subdivide( out2, out3, pt3, out7, out8, out9 );

			_VectorCopy(out7,subpoints[ptctr++]);
			_VectorCopy(out8,subpoints[ptctr++]);
			_VectorCopy(out9,subpoints[ptctr++]);
			_VectorCopy(pt3,subpoints[ptctr++]);


			break;
		}

		// Advance to next ent
		current = &cg_entities[ current->currentState.tag_num ];

		if ( !current->currentValid ) {
			break;
		}

		// Advance the points down the line
		VectorCopy(out2,pt1);
		VectorCopy(pt3,pt2);
		VectorCopy(current->lerpOrigin,pt3);
	}
}

void CG_MultiBeam(centity_t *cent) {
	vec3_t         prevpt,currpt;
	entityState_t  *s1;
	vec3_t         v1,v2,up,currpt1,currpt2,prevpt1,prevpt2;
	const char     *beamshadername;
	int            beamshader;
	byte           modulate[4];
	qboolean       prevptvalid=qfalse;
	int            i;

	s1 = &cent->currentState;

	// If this isn't the parent of the beam, then return
	if ( !s1->surfaces[0] ) {
		return;
	}

	// Subdivide up the segments
	CG_MultiBeamSubdivide( cent );

	// This is the top of the beam ent list, build up a renderer beam based on all the children
	beamshadername = CG_ConfigString( CS_IMAGES + s1->surfaces[1] ); // index for shader configstring
	beamshader     = cgi.R_RegisterShader( beamshadername );
	//beamshader     = cgi.R_RegisterShader( "<default>" );
	for ( i=0;i<4;i++ )
		modulate[i] = cent->color[i] * 255;

	if ( ptctr < 3 ) {
		return;
	}

	VectorCopy(subpoints[0],prevpt);
	prevptvalid = qfalse;

	for ( i=1; i<ptctr; i++ ) {
		vec3_t tmp;
		VectorCopy(subpoints[i],currpt);
		// Generate the up vector
		VectorSubtract(prevpt,cg.refdef.vieworg,v1);
		VectorSubtract(currpt,cg.refdef.vieworg,v2);
#if 0
		cgi.R_DebugLine( prevpt, currpt, 1,1,1,1);
		vec3_t pt = prevpt + up * 5;
		cgi.R_DebugLine( prevpt, pt, 0,0,1,1);
#endif

		CrossProduct(v1,v2,up);
		VectorNormalize(up);

		// Calculate the first points
		VectorScale(up,s1->scale,tmp);
		VectorAdd(currpt,tmp,currpt1);

		VectorScale(up,-s1->scale,tmp);
		VectorAdd(currpt,tmp,currpt2);

		if ( !prevptvalid ) {
			VectorScale(up,s1->scale,tmp);
			VectorAdd(prevpt,tmp,prevpt1);	

			VectorScale(up,-s1->scale,tmp);
			VectorAdd(prevpt,tmp,prevpt2);		
			prevptvalid = qtrue;
		}

		RenderSegment( currpt1, currpt2, prevpt1, prevpt2, modulate, beamshader, s1->renderfx );

		VectorCopy(currpt,prevpt);
		VectorCopy(currpt1,prevpt1);
		VectorCopy(currpt2,prevpt2);
	}      
}
float R_Noise( float x, float y, float z, float t ) {
	return 1.f; // TODO
}
static void CG_BuildRendererBeam( vec3_t start, vec3_t end, float angleVar, int numSubdivisions,
   byte color[4], int beamshader, float scale, float overlap, int owner, float life,
   int flags, float startalpha, float alphastep, int renderfx) {
	vec3_t         p1, p2, v1, v2, dir, prevpt1, prevpt2, delta, up;
	int            i,ii,jj;
	polyVert_t     points[4];
	float          length;
	int            segnum = 0;
	int            beamnum = 0;
	float          alphafactor;
	int            picW;   
	vec3_t		   tmp;

	// Create or increment the number of beams for this owner and check to 
	// see if we should add a new beam
	if ( flags & BEAM_PERSIST_EFFECT ) {
		beamnum = CreateNewBeamEntity( owner, life );
		if ( beamnum < 0 )
			return;
	}

	// For debugging texture coords
	//beamshader = cgi.R_RegisterShader( "<default>" );   

	picW = cgi.R_GetShaderWidth( beamshader );

	// calcluate length of beam segment
	VectorSubtract(end,start,delta);
	length = VectorLength(delta);
	length /= numSubdivisions;

	// get the dir of beam
	VectorCopy(delta,dir);
	VectorNormalize(dir);

	// Calculate the first up vector
	VectorSubtract(start,cg.refdef.vieworg,v1);
	VectorSubtract(end,cg.refdef.vieworg,v2);

	CrossProduct(v1,v2,up);
	VectorNormalize(up);

	// Calculate the first points
	VectorScale(up,scale,tmp);
	VectorAdd(start,tmp,prevpt1);

	VectorScale(up,-scale,tmp);
	VectorAdd(start,tmp,prevpt2);

	VectorCopy(start,p1);

	// go through and calculate each point of the beam and offset it by the anglevar
	for ( i=1; i<=numSubdivisions; i++ ) {
		// Calculate the next point along the beam
		VectorScale(dir,i*length,tmp);
		VectorAdd(start,tmp,p2);

		// Random variance on the next point ( except if it's the last )
		if ( i != numSubdivisions ) {
			if ( flags & BEAM_WAVE_EFFECT ) {
				float phase = p2[0] + p2[1];
				p2[2] += sin( phase + cg.time ) * angleVar;
			} else if ( flags & BEAM_USE_NOISE ) {
				p2[0] += R_Noise( p2[0],p2[1],p2[2],cg.time ) * angleVar;
				p2[1] += R_Noise( p2[0],p2[1],p2[2],cg.time ) * angleVar;
				p2[2] += R_Noise( p2[0],p2[1],p2[2],cg.time ) * angleVar;
			} else {
				p2[0] += Q_crandom( &seed ) * angleVar;
				p2[1] += Q_crandom( &seed ) * angleVar;
				p2[2] += Q_crandom( &seed ) * angleVar;
			}
		}

		// Create the up vec for the beam which is parallel to the viewplane

		VectorSubtract(p1,cg.refdef.vieworg,v1);
		VectorSubtract(p2,cg.refdef.vieworg,v2);
		CrossProduct(v1,v2,up);
		VectorNormalize(up);

		// Build the quad
		VectorMA( p2, scale, up, points[0].xyz );
		VectorCopy( prevpt1, points[1].xyz );
		VectorCopy( prevpt2, points[2].xyz );
		VectorMA( p2, -scale, up, points[3].xyz );

		if ( flags & BEAM_TILESHADER ) {// Tile the shader across the beam
			float startS = ( length * ( i-1 ) ) / (float)picW;
			float endS   = ( length * ( i ) )   / (float)picW;

			points[0].st[0] = startS; points[0].st[1] = 1;
			points[1].st[0] = endS;   points[1].st[1] = 1;
			points[2].st[0] = endS;   points[2].st[1] = 0;
			points[3].st[0] = startS; points[3].st[1] = 0;
		} else {
			points[0].st[0] = 1;   points[0].st[1] = 1;
			points[1].st[0] = 0;   points[1].st[1] = 1;
			points[2].st[0] = 0;   points[2].st[1] = 0;
			points[3].st[0] = 1;   points[3].st[1] = 0;
		}


		if ( !alphastep )
			alphafactor = 1.0f;
		else
			alphafactor = startalpha + (alphastep * i);

		// Set the color of the verts
		for ( ii=0; ii<4; ii++ ) {
			for ( jj=0; jj<4; jj++ ) {
				points[ii].modulate[jj] = color[jj] * alphafactor;
			}
		}

		if ( flags & BEAM_PERSIST_EFFECT ) {
			// Save the segment for backup for drawing faded out
			AddBeamSegmentToList( owner, points, beamnum, segnum++, renderfx );
		} else {            
			// Add it to the ref
			cgi.R_AddPolyToScene( beamshader, 4, points/*, renderfx*/ );
		}


		// Subtract off the overlap
		if ( overlap ) {
			vec3_t tmp;
			VectorScale(dir,-overlap,tmp);
			VectorAdd(p2,tmp,p2);
		}

		// Save off the last point to use as the first point on the next quad
		VectorMA( p2, scale, up, prevpt1 );
		VectorMA( p2, -scale, up, prevpt2 );
		VectorCopy(p2,p1);
	}
}

static void CG_CreateModelBeam(beam_t *b, vec3_t org, vec3_t dist, float total_length,
	vec3_t ndir, vec3_t left, vec3_t up) {
#if 0
	tiki_t      *tiki;
	vec3_t      bounds[2];
	int         single_beam_length;
	refEntity_t ent;
	int         count;
	int         j;
	float       factor[3];
	float       t;
	vec3_t      angles;
	int         i;

	// Find the length of a single beam
	tiki = cgi.TIKIForModel( b->hModel );

	// Calculate the bounds of the model to get it's length
	cgi.TIKI_AppendFrameBoundsAndRadius(tiki,0,0,&t,bounds);

	single_beam_length = bounds[1][0] - bounds[0][0];

	// Create the beam entity
	memset (&ent, 0, sizeof(ent));
	count = 0;

	// Initialize the factors
	for ( j=0; j<3; j++)
	factor[j] = 0.3f * crandom();

	t = 0;

	while ( t >= 0 && t < 1 )
	{
		float       dot;
		vec3_t      pdir;
		float       delta;
		vec3_t      distance_point;

		count++;

		// Set the origin of the current beam using the last calculated org
		VectorCopy( org, ent.origin );  

		// Advance the org one beam length in the new direction ( dist is the newly calculated direction )
		for ( j=0 ; j<3 ; j++ )
			org[j] += dist[j] * ( single_beam_length - b->overlap );

		// Offset the org by a random amount to simulate lightning

		VectorMA( org, single_beam_length * factor[2], up, org ); 
		VectorMA( org, single_beam_length * factor[1], left, org );

		// Calculate (t) - how far this new point is along the overall distance
		VectorSubtract( org, b->start, pdir );
		dot = DotProduct( pdir, ndir );
		t = dot/total_length;

		// Calculate point at current distance along center beam
		VectorMA( b->start, total_length * t, ndir, distance_point );

		// Allow any variations
		if ( t > 0.1 && t < 0.9 )
		{
			for ( j=0; j<3; j++ )
			{
				delta = org[j] - distance_point[j];
				if ( delta > b->max_offset )
				{
					org[j] = distance_point[j] + b->max_offset;
					factor[j] = -0.3 * crandom();
				}
				else if ( delta < -b->max_offset )
				{
					org[j] = distance_point[j] - b->max_offset;
					factor[j] = 0.3 * crandom();
				}
				else
					factor[j] = 0.3 * crandom();
			}
		}
		else // Clamp to mins 
		{
			for ( j=0; j<3; j++ )
			{
				delta = org[j] - distance_point[j];
				if ( delta > b->min_offset )
				{
					org[j] -= 0.4 * single_beam_length;
					factor[j] = -0.2;
				}
				else if ( delta < -b->min_offset )
				{
					org[j] += 0.4 * single_beam_length;
					factor[j] = 0.2;
				}
				else
					factor[j] = 0;
			}
		}

		// Calculate the new dist vector so we can get pitch and yaw for this beam
		VectorSubtract (org, ent.origin, dist);

		// Set the pitch and the yaw based off this new vector
		vectoangles( dist, angles );

		// Fill in the ent fields
		ent.hModel              = b->hModel;
		ent.scale               = b->scale;
		ent.renderfx            = b->renderfx;

		for( i=0; i<4; i++ )
			ent.shaderRGBA[i] = b->shaderRGBA[i];

		VectorCopy(ent.origin, ent.oldorigin);
		AnglesToAxis( angles, ent.axis );

		// Add in this beam to the ref
		cgi.R_AddRefEntityToScene( &ent );
	}
#endif
}

// called every frame
void CG_AddBeams() {
	int      i,ii;
	beam_t   *b;
	vec3_t   delta;
	vec3_t   angles;
	vec3_t   forward, left, up;
	float    length;
	byte     color[4];
	float    fade;

	b=cg_beams;
	for (i=0 ; i<MAX_BEAMS ; i++, b++) {
		// If no model is set or the endtime < current time remove the whole beam entity
		if ( !b->hModel || b->endtime < cg.time ) {         
			// Make sure endtime is not 0, and remove the beam entirely
			if ( b->endtime ) {
				RemoveBeamList( b->entity );
				b->entity = ENTITYNUM_NONE;
				b->endtime = 0;
			}
			continue;
		}

		// Fade the beam based on it's life
		fade = (float)( b->endtime - cg.time ) / (float) b->life;

		for ( ii=0; ii<4; ii++ )
		color[ii] = b->shaderRGBA[ii] * fade;

		// Check to see if the beam should be toggled
		if ( b->flags & BEAM_TOGGLE ) {
			if ( cg.time > b->toggletime ) {
				b->active = !b->active;
				if ( b->flags & BEAM_RANDOM_TOGGLEDELAY ) 
					b->toggletime = cg.time + random() * b->toggledelay;
				else
					b->toggletime = cg.time + b->toggledelay;
			}
		}

		if ( !b->active ) {
			CG_AddBeamsFromList( b->entity, b->beamshader );
			continue;
		}

		if ( ( b->flags & BEAM_PERSIST_EFFECT ) && ( b->update_time > cg.time ) ) {
			CG_AddBeamsFromList( b->entity, b->beamshader );
			continue;
		}

		b->update_time = cg.time + b->delay;

		if ( !b->active )
			continue;

		if ( b->flags & BEAM_USEMODEL ) {
			// Calculate the direction
			VectorSubtract( b->start, b->end, delta );

			// Calculate the beam length
			length = VectorLength( delta );

			// Get the perpendicular vectors to this vector
			vectoangles( delta, angles );
			AngleVectors( angles, forward, left, up );
			CG_CreateModelBeam( b, b->start, delta, length, forward, left, up );
		} else {
			// Do a sphere effect
			if ( b->flags & BEAM_SPHERE_EFFECT ) {
				int k;

				// Calculate the direction
				VectorSubtract( b->start, b->end, delta );

				// Calculate the beam length
				length = VectorLength( delta );

				for( k=0; k<b->numspherebeams; k++ ) {
					vec3_t offset;
					vec3_t tmp;
					vec3_t start,end;
					offset[0] = crandom();
					offset[1] = crandom();
					offset[2] = crandom();

					VectorScale(offset,b->sphereradius,tmp);
					VectorAdd(b->start,offset,start);

					VectorScale(offset,length,tmp);
					VectorAdd(b->start,offset,end);

					CG_BuildRendererBeam( start, end, b->max_offset,
						b->numSubdivisions, color, b->beamshader, b->scale, 
						b->overlap, b->entity, b->life, b->flags, b->alpha,
						b->alphastep, b->renderfx );               
				}
			} else {
				//CG_Printf( "%2f %2f %2f\n", b->start[0],b->start[1],b->start[2] );
				CG_BuildRendererBeam( b->start, b->end, b->max_offset,
				b->numSubdivisions, color, b->beamshader, b->scale, 
				b->overlap, b->entity, b->life, b->flags, b->alpha,
				b->alphastep, b->renderfx );
			}
			if ( b->flags & BEAM_PERSIST_EFFECT )
				CG_AddBeamsFromList( b->entity, b->beamshader );
		}
	}	
}

void CG_CreateBeam(vec3_t start, vec3_t dir, int owner, qhandle_t hModel, float alpha, 
   float scale, int flags, float length, int life, qboolean create, vec3_t endpointvec,
   int min_offset, int max_offset, int overlap, int numSubdivisions, int delay,
   const char *beamshadername, byte modulate[4], int numspherebeams, float sphereradius,
   int toggledelay, float endalpha, int renderfx, const char *name) {
	int      i;
	beam_t   *b;
	vec3_t   end;
	trace_t  trace;

	// Check to see if endpoint is specified
	if ( endpointvec ) {
		VectorCopy( endpointvec, end );
	} else {
		// Trace to find the endpoint with a shot
		VectorMA( start, length, dir, end );
		CG_Trace( &trace,start, vec3_origin, vec3_origin, end, 0, MASK_SHOT/*, qfalse, qtrue, "Create Beam"*/ );
		VectorCopy( trace.endpos, end );
	}

	// If we aren't creating a beam, then search the beams for this one already active
	if ( !create ) {
		for ( i=0, b=cg_beams ; i< MAX_BEAMS ; i++, b++ ) {
			if ( b->entity == owner ) {
				if ( name && b->name == name ) {
					b->endtime           = cg.time + life;
					b->hModel            = hModel;
					b->scale             = scale;
					b->flags             = flags;            
					b->overlap           = overlap;
					b->min_offset        = min_offset;
					b->max_offset        = max_offset;
					b->alpha             = alpha;
					b->beamshader        = cgi.R_RegisterShader( beamshadername );
					b->numSubdivisions   = numSubdivisions;
					b->delay             = delay;
					b->life              = life;
					b->numspherebeams    = numspherebeams;
					b->sphereradius      = sphereradius;
					b->renderfx          = renderfx;

					// take the alpha from the entity if less than 1, else grab it from the client commands version
					if ( alpha < 1 )
						b->shaderRGBA[ 3 ] = alpha * 255;
					else
						b->shaderRGBA[ 3 ] = modulate[3];

					// Modulation based off the color
					for( i=0; i<3; i++ )
						b->shaderRGBA[ i ] = modulate[ i ] * ( (float)b->shaderRGBA[3] / 255.0f );

					b->alphastep = ( (float)( endalpha - alpha ) / (float)b->numSubdivisions );

					VectorCopy( start, b->start );
					VectorCopy( end, b->end );
					return;
				}
			}
		}
	}

	// find a free beam
	for ( i=0, b=cg_beams ; i< MAX_BEAMS ; i++, b++ ) {
		if ( b->endtime < cg.time ) {
			b->entity            = owner;
			b->endtime           = cg.time + life;
			b->hModel            = hModel;
			b->alpha             = alpha;
			b->scale             = scale;
			b->flags             = flags;
			b->overlap           = overlap;
			b->min_offset        = min_offset;
			b->max_offset        = max_offset;
			b->beamshader        = cgi.R_RegisterShader( beamshadername );
			b->numSubdivisions   = numSubdivisions;
			b->delay             = delay;
			b->update_time       = 0;//cg.time + delay;
			b->life              = life;
			b->numspherebeams    = numspherebeams;
			b->sphereradius      = sphereradius;
			b->active            = qtrue;
			b->toggledelay       = toggledelay;
			b->renderfx          = renderfx;
			strcpy( b->name, name );

			// take the alpha from the entity if less than 1, else grab it from the client commands version
			if ( alpha < 1 )
				b->shaderRGBA[ 3 ] = alpha * 255;
			else
				b->shaderRGBA[ 3 ] = modulate[ 3 ];

			// Modulation based off the color
			for( i=0; i<3; i++ )
				b->shaderRGBA[ i ] = modulate[ i ] * ( (float)b->shaderRGBA[3] / 255.0f );


			b->alphastep = ( (float)( endalpha - alpha ) / (float)b->numSubdivisions );

			VectorCopy( start, b->start );
			VectorCopy( end, b->end );
			return;
		}
	}
	return;
}

static void CG_KillBeams(int entity_number) {
	int i; 
	beam_t *b;

	for ( i=0, b=cg_beams ; i< MAX_BEAMS ; i++, b++ ) {
		if ( b->entity == entity_number ) {
			b->entity  = ENTITYNUM_NONE;
			b->endtime = 0;
		}
	}
}

static void CG_RestartBeams(int timedelta) {
	int i; 
	beam_t *b;

	for ( i=0, b=cg_beams ; i< MAX_BEAMS ; i++, b++ ) {
		if ( b->active && ( b->update_time > cg.time ) ) {
			b->endtime -= timedelta;
			b->update_time -= timedelta;
			if ( b->toggletime )
				b->toggletime -= timedelta;
		}
	}
}

#if 1
void CG_Rope(centity_t *cent) {
	entityState_t  *s1;
	vec3_t         top, mid, bottom, up, v1, v2;
	vec3_t         currpt1, currpt2, prevpt1, prevpt2;
	const char     *beamshadername;
	int            beamshader;
	byte           modulate[4];
	float          picH, length, endT;
	int            i,j;
	polyVert_t     points[4];
	vec3_t tmp;

	s1 = &cent->currentState;

	VectorCopy(s1->origin2,top);
	VectorCopy(cent->lerpOrigin,mid);
	VectorCopy(cent->lerpOrigin,bottom);

	bottom[2] -= s1->alpha;

	// This is the top of the beam ent list, build up a renderer beam based on all the children
	beamshadername = CG_ConfigString( CS_IMAGES + s1->surfaces[0] ); // index for shader configstring
	beamshader     = cgi.R_RegisterShader( beamshadername );

	picH = cgi.R_GetShaderHeight( beamshader );

	for ( i=0;i<4;i++ )
		modulate[i] = cent->color[i] * 255;

	// Generate the up vector
	VectorSubtract(top,cg.refdef.vieworg,v1);
	VectorSubtract(bottom,cg.refdef.vieworg,v2);


	CrossProduct(v1,v2,up);
	VectorNormalize(up);

	// Set the color of the verts
	for ( i=0; i<4; i++ ) {
		for ( j=0; j<4; j++ ) {
			points[i].modulate[j] = modulate[j];
		}
	}
	// set the s coordinates
	points[0].st[0] = 1;
	points[1].st[0] = 1;
	points[2].st[0] = 0;
	points[3].st[0] = 0;

	// Calculate the first points
	VectorScale(up, s1->scale,tmp);
	VectorAdd(top,tmp,prevpt1);	

	VectorScale(up, -s1->scale,tmp);
	VectorAdd(top,tmp,prevpt2);	

	// draw the top section
	VectorScale(up, s1->scale,tmp);
	VectorAdd(mid,tmp,currpt1);	

	VectorScale(up, -s1->scale,tmp);
	VectorAdd(mid,tmp,currpt2);	


	VectorSubtract(mid,top,tmp);


	length = VectorLength(tmp);

	VectorCopy( currpt1, points[0].xyz );
	VectorCopy( prevpt1, points[1].xyz );
	VectorCopy( prevpt2, points[2].xyz );
	VectorCopy( currpt2, points[3].xyz );

	endT = length / picH;
	points[0].st[1] = endT;
	points[3].st[1] = endT;
	points[1].st[1] = 0;
	points[2].st[1] = 0;

	// Add a segment to the list
	cgi.R_AddPolyToScene( beamshader, 4, points/*, s1->renderfx*/ );

	if ( s1->alpha > 0 ) {
		// draw the bottom section
		vec3_t tmp;
		VectorCopy(currpt1,prevpt1);
		VectorCopy(currpt2,prevpt2);
		VectorScale(up,s1->scale,tmp);
		VectorAdd(bottom,tmp,currpt1);

		VectorScale(up,-s1->scale,tmp);
		VectorAdd(bottom,tmp,currpt2);

		VectorCopy( currpt1, points[0].xyz );
		VectorCopy( prevpt1, points[1].xyz );
		VectorCopy( prevpt2, points[2].xyz );
		VectorCopy( currpt2, points[3].xyz );

		// add on the rest of the rope
		length += s1->alpha;

		// use previous T value for the start of this segment
		points[1].st[1] = endT;
		points[2].st[1] = endT;
		endT = length / picH;
		points[0].st[1] = endT;
		points[3].st[1] = endT;

		// Add a segment to the list
		cgi.R_AddPolyToScene( beamshader, 4, points/*, s1->renderfx*/ );
	}
}

#endif