/*
===========================================================================
Copyright (C) 2023 the OpenMoHAA team

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

// DESCRIPTION:
// Beam effects

#include "cg_local.h"
#include "vector.h"
#include "container.h"
#include "cg_commands.h"

class beam_t : public Class
   {
   public:
               beam_t();

	int		   entity;
	qhandle_t   hModel;
	int		   endtime;
	Vector	   start, end;
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
   str         name;
   };

beam_t::beam_t()
   {
	entity=0;
	hModel=0;
	endtime=0;
   scale=0;
   alpha=0;
   flags=0;
   parent=ENTITYNUM_NONE;
   max_offset=0;
   min_offset=0;
   numSubdivisions=0;
   overlap=0;
   beamshader=0;
   update_time=0;
   delay=0;
   life=0;
   numspherebeams=0;
   sphereradius=0;
   toggletime=0;
   toggledelay=0;
   active=0;
   alphastep=0;
   renderfx=0;
   memset( shaderRGBA, 0, 4 );
   }

beam_t		cl_beams[MAX_BEAMS];

static int seed = 100;

// Recursive beam builder - I don't use it anymore
/*
void CG_BuildRenderBeam_r
   (
   Vector start,
   Vector end,
   float angleVar,
   int   numSubdivisions,
   int   maxSubdivisions
   )

   {   
   if ( numSubdivisions == maxSubdivisions )
		{
		return;
		}
	else
		{
		// subdivide line and call on both halves
		numSubdivisions += 1;

      mid = ( p1 * 0.5 ) + ( p2 * 0.5 );
      
      int seed = 100;

      mid[0] += Q_crandom( &seed ) * angleVar;
      mid[1] += Q_crandom( &seed ) * angleVar;
      mid[2] += Q_crandom( &seed ) * angleVar;

		CG_BuildRendererBeam( p1, mid, angleVar, numSubdivisions, maxSubdivisions, color, beamshader, scale );
		CG_BuildRendererBeam( mid, p2, angleVar, numSubdivisions, maxSubdivisions, color, beamshader, scale );
		}      
   }
*/

#define MAX_BEAM_BACKUP   6
#define MAX_BEAM_SEGMENTS 32

typedef struct beamSegment_t
   {
   polyVert_t points[4];
   } beamSegment_t;

typedef struct beamList_t
   {
   int time;
   int updatetime;
   int numsegments;
   beamSegment_t segments[MAX_BEAM_SEGMENTS];
   } beamList_t;

typedef struct beamEnt_t
   {
   int owner;
   int numbeams;
   int life;
   int renderfx;
   beamList_t beamlist[MAX_BEAM_BACKUP];
   } beamEnt_t;

Container<beamEnt_t *> beamManager;

void RemoveBeamList
   (
   int owner 
   )

   {
   int i,num;

   num = beamManager.NumObjects();

   for ( i=1; i<=num; i++ )
      {
      beamEnt_t *be = beamManager.ObjectAt( i );

      if ( be->owner == owner )
         {
         beamManager.RemoveObjectAt( i );
         delete be;
         return;
         }
      }
   }

beamEnt_t *FindBeamList
   (
   int owner
   )

   {
   int i,num;

   num = beamManager.NumObjects();

   for ( i=1; i<=num; i++ )
      {
      beamEnt_t *be = beamManager.ObjectAt( i );

      if ( be->owner == owner )
         return be;
      }
   return NULL;
   }

int CreateNewBeamEntity
   (
   int   owner,
   float life
   )

   {
   beamEnt_t   *be;
   int         i,oldest,oldest_time;

   be = FindBeamList( owner );

   if ( !be )
      {
      be = new beamEnt_t;

      if ( !be )
         cgi.Error( ERR_DROP, "Could not allocate memory for beamEnt.\n" );

      memset ( be, 0, sizeof( beamEnt_t ) );
      memset ( be->beamlist, 0, sizeof( beamList_t ) * MAX_BEAM_BACKUP );
      be->owner = owner;
      be->life  = life;

      beamManager.AddObject( be );
      }
   
   // find the oldest beam and overwrite it.
   oldest      = -1;
   oldest_time = 999999999;

   for ( i=0; i<MAX_BEAM_BACKUP; i++ )
      {
      // Check for update time
      float t = be->beamlist[ i ].time;

      if ( !t )
         {
         oldest = i;
         break;
         }

      if ( t < oldest_time )
         {
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

void RemoveBeamEntity
   (
   int   owner
   )

   {
   RemoveBeamList( owner );
   }

void AddBeamSegmentToList
   (
   int owner,
   polyVert_t points[4],
   int beamnum,
   int segnum,
   int renderfx
   )

   {
   beamEnt_t *be;

   be = FindBeamList( owner );

   if ( !be )
      {
      cgi.DPrintf( "Could not find beam entity for owner:%d\n", owner );
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

void CG_AddBeamsFromList
   (
   int owner,
   int beamshader
   )

   {
   int         i,j,k,l;
   float       frac, fade;
   beamEnt_t   *be = FindBeamList( owner );
   polyVert_t  newpoints[4];

   if ( !be )
      {
      return;
      }
   
   for ( i=0; i<MAX_BEAM_BACKUP; i++ )
      {
      beamList_t  *bl = &be->beamlist[i];

      if ( !bl->time )
         continue;

      // Calculate the blend factor for fading 
      frac   =  (float)( cg.time - bl->time ) / (float)be->life;
      fade   =  1.0f - frac;

      if ( fade <= 0 )
         {
         bl->time = 0; // RemoveBeamList( owner );
         continue;
         }

      // Go through each segment and draw it with the new modulate
      assert( bl->numsegments < MAX_BEAM_SEGMENTS );

      for ( j=0; j<bl->numsegments; j++ )
         {
         memcpy( newpoints, bl->segments[j].points, 4 * sizeof( polyVert_t ) );
         
         for ( k=0; k<4; k++ )
            {
            for ( l=0; l<4; l++ )
               {
               newpoints[k].modulate[l] = bl->segments[j].points[k].modulate[l] * fade; 
               }
            }

         cgi.R_AddPolyToScene( beamshader, 4, newpoints, be->renderfx );
         }
      }
   }

void RenderSegment
   (
   Vector pt1a,
   Vector pt1b,
   Vector pt2a,
   Vector pt2b,   
   byte   modulate[4],
   int    beamshader,
   int    renderfx
   )

   {
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
   for ( i=0; i<4; i++ )
      {
      for ( j=0; j<4; j++ )
         {
         points[i].modulate[j] = modulate[j];
         }
      }

   // Add a segment to the list
   cgi.R_AddPolyToScene( beamshader, 4, points, renderfx );
   }

const int MAX_SUBPOINTS=256;
static Vector subpoints[MAX_SUBPOINTS];
static int ptctr=0;
/*
===============
CG_Subdivide

a, b, and c are control points.
the subdivided sequence will be: a, out1, out2, out3, c
===============
*/
static void CG_Subdivide( Vector a, Vector b, Vector c, Vector &out1, Vector &out2, Vector &out3 ) 
   {
   out1 = 0.5 * (a + b);
   out3 = 0.5 * (b + c);
	out2 = 0.5 * (out1 + out3);
	}

void CG_MultiBeamBegin
   (
   void
   )

   {
   ptctr = 0;   
   }

void CG_MultiBeamAddPoints
   (
   vec3_t   start,
   vec3_t   end,
   int      numsegments,
   int      flags,
   float    minoffset,
   float    maxoffset,
   qboolean addstartpoint
   )

   {
   Vector   delta,dir,randdir;
   float    length;
   int      i;

   if ( ptctr > MAX_SUBPOINTS )
      return;

   if ( addstartpoint )
      {
      subpoints[ ptctr++ ] = start;
      }
   
   delta  = Vector( end ) - Vector( start );
   length = delta.length();
   length /= numsegments;

   // get the dir of beam
   dir = delta;
   dir.normalize();

   for ( i=1; i<numsegments; i++ )
      {
      Vector newpt;

      if ( ptctr > MAX_SUBPOINTS )
         return;

      randdir = Vector( crandom(), crandom(), crandom() );

      newpt = Vector( start ) + dir * i * length;
      newpt += minoffset * randdir + maxoffset * randdir;
      
      subpoints[ ptctr++ ] = newpt;         
      }
   subpoints[ ptctr++ ] = end;
   }

void CG_MultiBeamEnd
   (
   float       scale,
   int         renderfx,
   const char  *beamshadername,
   byte        modulate[4],
   int         flags,
   int         owner,
   float       life
   )

   {
   Vector         prevpt,currpt;
   Vector         p1,p2,p3,p4,v1,v2,up,currpt1,currpt2,prevpt1,prevpt2;
   qboolean       prevptvalid=false;
   int            i,beamshader;
   
   beamshader  = cgi.R_RegisterShader( beamshadername );

   prevpt      = subpoints[0];
   prevptvalid = false;

   for ( i=1; i<ptctr; i++ )
      {
      currpt = subpoints[i];
      // Generate the up vector
      v1 = prevpt - cg.refdef.vieworg;
      v2 = currpt - cg.refdef.vieworg;

#if 0
      cgi.R_DebugLine( prevpt, currpt, 1,1,1,1);
      Vector pt = prevpt + up * 5;
      cgi.R_DebugLine( prevpt, pt, 0,0,1,1);
#endif

      up.CrossProduct( v1, v2 );
      up.normalize();
   
      // Calculate the first points
      currpt1 = currpt + ( up * scale );
      currpt2 = currpt + ( up * -scale );

      if ( !prevptvalid )
         {
         prevpt1     = prevpt + up * scale;
         prevpt2     = prevpt + up * -scale;
         prevptvalid = true;
         }
#if 1
      RenderSegment( currpt1, currpt2, prevpt1, prevpt2, modulate, beamshader, renderfx );
#endif
      
      prevpt  = currpt;
      prevpt1 = currpt1;
      prevpt2 = currpt2;
      }
   
   if ( flags & BEAM_PERSIST_EFFECT )
      CG_AddBeamsFromList( owner, beamshader );
   }


static void CG_MultiBeamSubdivide
   (
   centity_t *cent
   )

   {
   Vector    pt1, pt2, pt3;
   Vector    out1, out2, out3, out4, out5, out6, out7, out8, out9;
   centity_t *current;

   ptctr = 0;
   current = cent;

   // Multibeam requires at least 3 points to start with

   // Get pt1
   if ( current->currentState.tag_num == ENTITYNUM_NONE )
      {
      cgi.DPrintf( "CG_MultiBeamSubdivide : Multi beam entity does not have a child\n" );
      return;
      }
   pt1 = current->lerpOrigin;
   
   // Get pt2
   current = &cg_entities[ current->currentState.tag_num ];
   // Make sure that child is a multibeam
   if ( current->currentState.eType != ET_MULTIBEAM )
      return;

   if ( current->currentState.tag_num == ENTITYNUM_NONE )
      {
      cgi.DPrintf( "CG_MultiBeamSubdivide : Multi beam entity does not have a child\n" );
      return;
      }
   pt2 = current->lerpOrigin;
   
   // Get pt3
   current = &cg_entities[ current->currentState.tag_num ];
   // Make sure that child is a multibeam
   if ( current->currentState.eType != ET_MULTIBEAM )
      {
      return;
      }

   if ( current->currentState.tag_num == ENTITYNUM_NONE ) 
      {
      cgi.DPrintf( "CG_MultiBeamSubdivide : Multi beam entity does not have a child\n" );
      return;
      }
   pt3 = current->lerpOrigin;

   // First point into the subdivided points
   subpoints[ptctr++] = pt1;

   while ( 1 )
      {
      // Do the subdivide
      CG_Subdivide( pt1, pt2, pt3, out1, out2, out3 );
      CG_Subdivide( pt1, out1, out2, out4, out5, out6 );

      if ( ( ptctr + 4 ) > MAX_SUBPOINTS ) 
         break;

      // Save the points
      subpoints[ptctr++] = out4;
      subpoints[ptctr++] = out5;      
      subpoints[ptctr++] = out6;
      subpoints[ptctr++] = out2;

      // end condition
      if ( ( current->currentState.tag_num == ENTITYNUM_NONE ) || ( !current->currentValid ) )
         {
         CG_Subdivide( out2, out3, pt3, out7, out8, out9 );
         subpoints[ptctr++] = out7;
         subpoints[ptctr++] = out8;      
         subpoints[ptctr++] = out9;
         subpoints[ptctr++] = pt3;
         break;
         }

      // Advance to next ent
      current = &cg_entities[ current->currentState.tag_num ];

      if ( !current->currentValid )
         {
         break;
         }

      // Advance the points down the line
      pt1 = out2;
      pt2 = pt3;
      pt3 = current->lerpOrigin;
      }
   }


void CG_MultiBeam
   (
   centity_t *cent
   )

   {
   Vector         prevpt,currpt;
   entityState_t  *s1;
   Vector         p1,p2,p3,p4,v1,v2,up,currpt1,currpt2,prevpt1,prevpt2;
   const char     *beamshadername;
   int            beamshader;
   byte           modulate[4];
   qboolean       prevptvalid=false;
   int            i;
   
	s1 = &cent->currentState;

   // If this isn't the parent of the beam, then return
   if ( !s1->surfaces[0] )
      {
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

   if ( ptctr < 3 )
      {
      return;
      }

   prevpt      = subpoints[0];
   prevptvalid = false;

   for ( i=1; i<ptctr; i++ )
      {
      currpt = subpoints[i];
      // Generate the up vector
      v1 = prevpt - cg.refdef.vieworg;
      v2 = currpt - cg.refdef.vieworg;

#if 0
      cgi.R_DebugLine( prevpt, currpt, 1,1,1,1);
      Vector pt = prevpt + up * 5;
      cgi.R_DebugLine( prevpt, pt, 0,0,1,1);
#endif

      up.CrossProduct( v1, v2 );
      up.normalize();
   
      // Calculate the first points
      currpt1 = currpt + ( up * s1->scale );
      currpt2 = currpt + ( up * -s1->scale );

      if ( !prevptvalid )
         {
         prevpt1     = prevpt + up * s1->scale;
         prevpt2     = prevpt + up * -s1->scale;
         prevptvalid = true;
         }

      RenderSegment( currpt1, currpt2, prevpt1, prevpt2, modulate, beamshader, s1->renderfx );
      
      prevpt  = currpt;
      prevpt1 = currpt1;
      prevpt2 = currpt2;
      }      
   }

void CG_BuildRendererBeam
   (
   Vector start,
   Vector end,
   float  angleVar,
   int    numSubdivisions,
   byte   color[4],
   int    beamshader,
   float  scale,
   float  overlap,
   int    owner,
   float  life,
   int    flags,
   float  startalpha,
   float  alphastep,
   int    renderfx
   )
	{
	Vector         p1, p2, v1, v2, dir, prevpt1, prevpt2, nextpt, mid, delta, up;
   int            i,ii,jj;
   polyVert_t     points[4];
   float          length;
   int            segnum = 0;
   int            beamnum = 0;
   float          alphafactor;
   int            picW;   

   // Create or increment the number of beams for this owner and check to 
   // see if we should add a new beam
   if ( flags & BEAM_PERSIST_EFFECT )
      {
      beamnum = CreateNewBeamEntity( owner, life );
      if ( beamnum < 0 )
         return;
      }

   // For debugging texture coords
   //beamshader = cgi.R_RegisterShader( "<default>" );   

   picW = cgi.R_GetShaderWidth( beamshader );

   // calcluate length of beam segment
   delta  = end-start;
   length = delta.length();
   length /= numSubdivisions;

   // get the dir of beam
   dir = delta;
   dir.normalize();

   // Calculate the first up vector
   v1 = start - cg.refdef.vieworg;
   v2 = end   - cg.refdef.vieworg;
   up.CrossProduct( v1, v2 );
   up.normalize();
   
   // Calculate the first points
   prevpt1 = start + ( up * scale );
   prevpt2 = start + ( up * -scale );
   p1      = start;

   // go through and calculate each point of the beam and offset it by the anglevar
   for ( i=1; i<=numSubdivisions; i++ )
      {
      // Calculate the next point along the beam
      p2 = start + ( dir * i * length );

      // Random variance on the next point ( except if it's the last )
      if ( i != numSubdivisions )
         {
         if ( flags & BEAM_WAVE_EFFECT )
            {
            float phase = p2.x + p2.y;
            p2.z += sin( phase + cg.time ) * angleVar;
            }
         else if ( flags & BEAM_USE_NOISE )
            {
            p2.x += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
            p2.y += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
            p2.z += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
            }
         else
            {
            p2.x += Q_crandom( &seed ) * angleVar;
            p2.y += Q_crandom( &seed ) * angleVar;
            p2.z += Q_crandom( &seed ) * angleVar;
            }
         }

      // Create the up vec for the beam which is parallel to the viewplane
      v1 = p1 - cg.refdef.vieworg;
      v2 = p2 - cg.refdef.vieworg;
      up.CrossProduct( v1, v2 );
      up.normalize();
      
      // Build the quad
      VectorMA( p2, scale, up, points[0].xyz );
      VectorCopy( prevpt1, points[1].xyz );
      VectorCopy( prevpt2, points[2].xyz );
      VectorMA( p2, -scale, up, points[3].xyz );

      if ( flags & BEAM_TILESHADER ) // Tile the shader across the beam
         {
         float startS = ( length * ( i-1 ) ) / (float)picW;
         float endS   = ( length * ( i ) )   / (float)picW;
         
         points[0].st[0] = startS; points[0].st[1] = 1;
         points[1].st[0] = endS;   points[1].st[1] = 1;
         points[2].st[0] = endS;   points[2].st[1] = 0;
         points[3].st[0] = startS; points[3].st[1] = 0;
         }
      else
         {
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
      for ( ii=0; ii<4; ii++ )
         {
         for ( jj=0; jj<4; jj++ )
            {
            points[ii].modulate[jj] = color[jj] * alphafactor;
            }
         }

      if ( flags & BEAM_PERSIST_EFFECT )
         {
         // Save the segment for backup for drawing faded out
         AddBeamSegmentToList( owner, points, beamnum, segnum++, renderfx );
         }
      else
         {            
         // Add it to the ref
         cgi.R_AddPolyToScene( beamshader, 4, points, renderfx );
         }


      // Subtract off the overlap
      if ( overlap )
         {
         p2 = p2 + ( dir * -overlap );
         }

      // Save off the last point to use as the first point on the next quad
      VectorMA( p2, scale, up, prevpt1 );
      VectorMA( p2, -scale, up, prevpt2 );
      p1      = p2;
      }
	}

void CG_CreateModelBeam
   (
   beam_t   *b,
   vec3_t   org,
   vec3_t   dist,
   float    total_length,
   vec3_t   ndir,
   vec3_t   left,
   vec3_t   up
   )

{
    dtiki_t* tiki;
    vec3_t      mins, maxs;
    int         single_beam_length;
    refEntity_t ent;
    int         count;
    int         j;
    float       factor[3];
    float       t;
    vec3_t      angles;
    int         i;

    // Find the length of a single beam
    tiki = cgi.R_Model_GetHandle(b->hModel);

    // Calculate the bounds of the model to get it's length
    cgi.TIKI_CalculateBounds(tiki, 1.0, mins, maxs);
    single_beam_length = maxs[0] - mins[0];

    // Create the beam entity
    memset(&ent, 0, sizeof(ent));
    count = 0;

    // Initialize the factors
    for (j = 0; j < 3; j++)
        factor[j] = 0.3f * crandom();

    t = 0;

    while (t >= 0 && t < 1)
    {
        float       dot;
        vec3_t      pdir;
        float       delta;
        vec3_t      distance_point;

        count++;

        // Set the origin of the current beam using the last calculated org
        VectorCopy(org, ent.origin);

        // Advance the org one beam length in the new direction ( dist is the newly calculated direction )
        for (j = 0; j < 3; j++)
            org[j] += dist[j] * (single_beam_length - b->overlap);

        // Offset the org by a random amount to simulate lightning

        VectorMA(org, single_beam_length * factor[2], up, org);
        VectorMA(org, single_beam_length * factor[1], left, org);

        // Calculate (t) - how far this new point is along the overall distance
        VectorSubtract(org, b->start, pdir);
        dot = DotProduct(pdir, ndir);
        t = dot / total_length;

        // Calculate point at current distance along center beam
        VectorMA(b->start, total_length * t, ndir, distance_point);

        // Allow any variations
        if (t > 0.1 && t < 0.9)
        {
            for (j = 0; j < 3; j++)
            {
                delta = org[j] - distance_point[j];
                if (delta > b->max_offset)
                {
                    org[j] = distance_point[j] + b->max_offset;
                    factor[j] = -0.3 * crandom();
                }
                else if (delta < -b->max_offset)
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
            for (j = 0; j < 3; j++)
            {
                delta = org[j] - distance_point[j];
                if (delta > b->min_offset)
                {
                    org[j] -= 0.4 * single_beam_length;
                    factor[j] = -0.2f;
                }
                else if (delta < -b->min_offset)
                {
                    org[j] += 0.4 * single_beam_length;
                    factor[j] = 0.2f;
                }
                else
                    factor[j] = 0;
            }
        }

        // Calculate the new dist vector so we can get pitch and yaw for this beam
        VectorSubtract(org, ent.origin, dist);

        // Set the pitch and the yaw based off this new vector
        vectoangles(dist, angles);

        // Fill in the ent fields
        ent.hModel = b->hModel;
        ent.scale = b->scale;
        ent.renderfx = b->renderfx;

        for (i = 0; i < 4; i++)
            ent.shaderRGBA[i] = b->shaderRGBA[i];

        VectorCopy(ent.origin, ent.oldorigin);
        AnglesToAxis(angles, ent.axis);

        // Add in this beam to the ref
        cgi.R_AddRefEntityToScene(&ent);
    }
}

void CG_AddBeams
   (
   void 
   )

   {
   int      i,ii;
   beam_t   *b;
   vec3_t   delta;
   vec3_t   angles;
   vec3_t   forward, left, up;
   float    length;
   byte     color[4];
   float    fade;

	for (i=0, b=cl_beams ; i<MAX_BEAMS ; i++, b++)
	   {
      // If no model is set or the endtime < current time remove the whole beam entity
		if ( !b->hModel || b->endtime < cg.time )
         {         
         // Make sure endtime is not 0, and remove the beam entirely
         if ( b->endtime )
            {
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
      if ( b->flags & BEAM_TOGGLE ) 
         {
         if ( cg.time > b->toggletime )
            {
            b->active = !b->active;
            if ( b->flags & BEAM_RANDOM_TOGGLEDELAY ) 
               b->toggletime = cg.time + random() * b->toggledelay;
            else
               b->toggletime = cg.time + b->toggledelay;
            }
         }
         
      if ( !b->active )
         {
         CG_AddBeamsFromList( b->entity, b->beamshader );
         continue;
         }

      if ( ( b->flags & BEAM_PERSIST_EFFECT ) && ( b->update_time > cg.time ) )
         {
         CG_AddBeamsFromList( b->entity, b->beamshader );
         continue;
         }

      b->update_time = cg.time + b->delay;

      if ( !b->active )
         continue;

      if ( b->flags & BEAM_USEMODEL )
         {
         // Calculate the direction
         VectorSubtract( b->start, b->end, delta );

		   // Calculate the beam length
         length = VectorLength( delta );

         // Get the perpendicular vectors to this vector
         vectoangles( delta, angles );
         AngleVectors( angles, forward, left, up );
         CG_CreateModelBeam( b, b->start, delta, length, forward, left, up );
         }
      else
         {
         // Do a sphere effect
         if ( b->flags & BEAM_SPHERE_EFFECT )
            {
            int k;

            // Calculate the direction
            VectorSubtract( b->start, b->end, delta );

		      // Calculate the beam length
            length = VectorLength( delta );

            for( k=0; k<b->numspherebeams; k++ )
               {
               Vector offset( crandom(), crandom(), crandom() );
               Vector start( b->start + offset * b->sphereradius );
               Vector end( b->start + offset * length );
               
               CG_BuildRendererBeam( start,
                                     end,
                                     b->max_offset,
                                     b->numSubdivisions,
                                     color,
                                     b->beamshader, 
                                     b->scale, 
                                     b->overlap,
                                     b->entity,
                                     b->life,
                                     b->flags,
                                     b->alpha,
                                     b->alphastep,
                                     b->renderfx
                                    );               

               }
            }
         else
            {
            //cgi.DPrintf( "%2f %2f %2f\n", b->start[0],b->start[1],b->start[2] );
            CG_BuildRendererBeam( b->start,
                                  b->end,
                                  b->max_offset,
                                  b->numSubdivisions,
                                  color,
                                  b->beamshader, 
                                  b->scale, 
                                  b->overlap,
                                  b->entity,
                                  b->life,
                                  b->flags,
                                  b->alpha,
                                  b->alphastep,
                                  b->renderfx
                                );
            }
         if ( b->flags & BEAM_PERSIST_EFFECT )
            CG_AddBeamsFromList( b->entity, b->beamshader );
         }
	   }	
   }

void CG_CreateBeam
   (
   vec3_t         start,
   vec3_t         dir, 
   int            owner,
   qhandle_t      hModel,
   float          alpha, 
   float          scale, 
   int            flags, 
   float          length,
   int            life,
   qboolean       create,
   vec3_t         endpointvec,
   int            min_offset,
   int            max_offset,
   int            overlap,
   int            numSubdivisions,
   int            delay,
   const char     *beamshadername,
   float          modulate[4],
   int            numspherebeams,
   float          sphereradius,
   int            toggledelay,
   float          endalpha,
   int            renderfx,
   const char     *name
   )

   {
   int      i;
   beam_t   *b;
   vec3_t   end;
   trace_t  trace;

   // Check to see if endpoint is specified
   if ( endpointvec )
      {
      VectorCopy( endpointvec, end );
      }
   else
      {
      // Trace to find the endpoint with a shot
      VectorMA( start, length, dir, end );
      CG_Trace( &trace,start, vec3_origin, vec3_origin, end, 0, MASK_SHOT, false, true, "Create Beam" );
      VectorCopy( trace.endpos, end );
      }

   // If we aren't creating a beam, then search the beams for this one already active
   if ( !create )
      {
	   for ( i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++ )
         {
         if ( b->entity == owner )             
            {
            if ( name && b->name == name )
		         {
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
	for ( i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++ )
	   {
		if ( b->endtime < cg.time )
		   {
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
         b->active            = true;
         b->toggledelay       = toggledelay;
         b->renderfx          = renderfx;
         b->name              = name;

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

void CG_KillBeams
   (
   int entity_number
   )

   {
   int i; 
   beam_t *b;

   for ( i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++ )
      {
      if ( b->entity == entity_number )
         {
         b->entity  = ENTITYNUM_NONE;
         b->endtime = 0;
         }
      }
   }

void CG_RestartBeams
   (
   int timedelta
   )

   {
   int i; 
   beam_t *b;

   for ( i=0, b=cl_beams ; i< MAX_BEAMS ; i++, b++ )
      {
      if ( b->active && ( b->update_time > cg.time ) )
         {
         b->endtime -= timedelta;
         b->update_time -= timedelta;
         if ( b->toggletime )
            b->toggletime -= timedelta;
         }
      }
   }


void CG_Rope
   (
   centity_t *cent
   )

   {
   Vector         prevpt,currpt;
   entityState_t  *s1;
   Vector         top, mid, bottom, up, v1, v2;
   Vector         currpt1, currpt2, prevpt1, prevpt2;
   const char     *beamshadername;
   int            beamshader;
   byte           modulate[4];
   float          picH, length, endT;
   int            i,j;
   polyVert_t     points[4];

   
	s1 = &cent->currentState;

   top = s1->origin2;
   mid = cent->lerpOrigin;
   bottom = cent->lerpOrigin;
   bottom.z -= s1->alpha;

   // This is the top of the beam ent list, build up a renderer beam based on all the children
   beamshadername = CG_ConfigString( CS_IMAGES + s1->surfaces[0] ); // index for shader configstring
   beamshader     = cgi.R_RegisterShader( beamshadername );

   picH = cgi.R_GetShaderHeight( beamshader );

   for ( i=0;i<4;i++ )
      modulate[i] = cent->color[i] * 255;

   // Generate the up vector
   v1 = top - cg.refdef.vieworg;
   v2 = bottom - cg.refdef.vieworg;

   up.CrossProduct( v1, v2 );
   up.normalize();

   // Set the color of the verts
   for ( i=0; i<4; i++ )
      {
      for ( j=0; j<4; j++ )
         {
         points[i].modulate[j] = modulate[j];
         }
      }
   // set the s coordinates
   points[0].st[0] = 1;
   points[1].st[0] = 1;
   points[2].st[0] = 0;
   points[3].st[0] = 0;

   // Calculate the first points
   prevpt1 = top + ( up * s1->scale );
   prevpt2 = top + ( up * -s1->scale );

   // draw the top section
   currpt1 = mid + ( up * s1->scale );
   currpt2 = mid + ( up * -s1->scale );

   length = Vector( mid - top ).length();

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
   cgi.R_AddPolyToScene( beamshader, 4, points, s1->renderfx );

   if ( s1->alpha > 0 )
      {
      // draw the bottom section
      prevpt1 = currpt1;
      prevpt2 = currpt2;
      currpt1 = bottom + ( up * s1->scale );
      currpt2 = bottom + ( up * -s1->scale );
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
      cgi.R_AddPolyToScene( beamshader, 4, points, s1->renderfx );
      }
   }
