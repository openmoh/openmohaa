/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
#include "tr_local.h"



/*
=================
R_CullTriSurf

Returns true if the grid is completely culled away.
Also sets the clipped hint bit in tess
=================
*/
static qboolean	R_CullTriSurf( srfTriangles_t *cv ) {
	int 	boxCull;

	boxCull = R_CullLocalBox( cv->bounds );

	if ( boxCull == CULL_OUT ) {
		return qtrue;
	}
	return qfalse;
}

/*
=================
R_CullGrid

Returns true if the grid is completely culled away.
Also sets the clipped hint bit in tess
=================
*/
static qboolean	R_CullGrid( srfGridMesh_t *cv ) {
	int 	boxCull;
	int 	sphereCull;

	if ( r_nocurves->integer ) {
		return qtrue;
	}

	if (!tr.refdef.render_terrain) {
		return qtrue;
	}

	if ( tr.currentEntityNum != ENTITYNUM_WORLD ) {
		sphereCull = R_CullLocalPointAndRadius( cv->localOrigin, cv->meshRadius );
	} else {
		sphereCull = R_CullPointAndRadius( cv->localOrigin, cv->meshRadius );
	}
	boxCull = CULL_OUT;
	
	// check for trivial reject
	if ( sphereCull == CULL_OUT )
	{
		tr.pc.c_sphere_cull_patch_out++;
		return qtrue;
	}
	// check bounding box if necessary
	else if ( sphereCull == CULL_CLIP )
	{
		tr.pc.c_sphere_cull_patch_clip++;

		boxCull = R_CullLocalBox( cv->meshBounds );

		if ( boxCull == CULL_OUT ) 
		{
			tr.pc.c_box_cull_patch_out++;
			return qtrue;
		}
		else if ( boxCull == CULL_IN )
		{
			tr.pc.c_box_cull_patch_in++;
		}
		else
		{
			tr.pc.c_box_cull_patch_clip++;
		}
	}
	else
	{
		tr.pc.c_sphere_cull_patch_in++;
	}

	return qfalse;
}


/*
================
R_CullSurface

Tries to back face cull surfaces before they are lighted or
added to the sorting list.

This will also allow mirrors on both sides of a model without recursion.
================
*/
static qboolean	R_CullSurface( surfaceType_t *surface, shader_t *shader ) {
	srfSurfaceFace_t *sface;
	float			d;

	if ( r_nocull->integer ) {
		return qfalse;
	}

	if ( *surface == SF_GRID ) {
		return R_CullGrid( (srfGridMesh_t *)surface );
	}

	if ( *surface == SF_TRIANGLES ) {
		return R_CullTriSurf( (srfTriangles_t *)surface );
	}

	if ( *surface != SF_FACE ) {
		return qfalse;
	}

	if ( shader->cullType == CT_TWO_SIDED ) {
		return qfalse;
	}

	// face culling
	if ( !r_facePlaneCull->integer ) {
		return qfalse;
	}

	sface = ( srfSurfaceFace_t * ) surface;
	d = DotProduct (tr.ori.viewOrigin, sface->plane.normal);

	// don't cull exactly on the plane, because there are levels of rounding
	// through the BSP, ICD, and hardware that may cause pixel gaps if an
	// epsilon isn't allowed here 
	if ( shader->cullType == CT_FRONT_SIDED ) {
		if ( d < sface->plane.dist - 8 ) {
			return qtrue;
		}
	} else {
		if ( d > sface->plane.dist + 8 ) {
			return qtrue;
		}
	}

	return qfalse;
}

/*
======================
R_FastDlightFace
======================
*/
static int R_FastDlightFace( srfSurfaceFace_t *face, int dlightBits ) {
	float		d;
	int			i;
	dlight_t	*dl;

	for ( i = 0 ; i < tr.refdef.num_dlights ; i++ ) {
		if ( ! ( dlightBits & ( 1 << i ) ) ) {
			continue;
		}
		dl = &tr.refdef.dlights[i];
		d = DotProduct( dl->transformed, face->plane.normal ) - face->plane.dist;
		if ( d < -dl->radius || d > dl->radius ) {
			// dlight doesn't reach the plane
			dlightBits &= ~( 1 << i );
		}
	}

	if ( !dlightBits ) {
		tr.pc.c_dlightSurfacesCulled++;
	}

	face->dlightBits[ tr.smpFrame ] = dlightBits;
	return dlightBits;
}

/*
======================
R_FastDlightGrid
======================
*/
static int R_FastDlightGrid( srfGridMesh_t *grid, int dlightBits ) {
	int			i;
	dlight_t	*dl;

	for ( i = 0 ; i < tr.refdef.num_dlights ; i++ ) {
		if ( ! ( dlightBits & ( 1 << i ) ) ) {
			continue;
		}
		dl = &tr.refdef.dlights[i];
		if ( dl->origin[0] - dl->radius > grid->meshBounds[1][0]
			|| dl->origin[0] + dl->radius < grid->meshBounds[0][0]
			|| dl->origin[1] - dl->radius > grid->meshBounds[1][1]
			|| dl->origin[1] + dl->radius < grid->meshBounds[0][1]
			|| dl->origin[2] - dl->radius > grid->meshBounds[1][2]
			|| dl->origin[2] + dl->radius < grid->meshBounds[0][2] ) {
			// dlight doesn't reach the bounds
			dlightBits &= ~( 1 << i );
		}
	}

	if ( !dlightBits ) {
		tr.pc.c_dlightSurfacesCulled++;
	}

	grid->dlightBits[ tr.smpFrame ] = dlightBits;
	return dlightBits;
}

/*
======================
R_FastDlightTerrain
======================
*/
static int R_FastDlightTerrain(cTerraPatchUnpacked_t* srf, int dlightBits) {
    dlight_t* dl;
    float* origin;
    int i;

    for (i = 0; i < tr.refdef.num_dlights; i++) {
        dl = &tr.refdef.dlights[i];
        origin = dl->transformed;

        if (srf->x0 - origin[0] > dl->radius
            || srf->x0 - origin[0] < -512.0 - dl->radius
            || srf->y0 - origin[1] > dl->radius
            || srf->y0 - origin[1] < -512.0 - dl->radius
            || srf->z0 - origin[2] > dl->radius
            || srf->z0 - origin[2] < -srf->zmax - dl->radius
            ) {
            // dlight doesn't reach the bounds
            dlightBits &= ~(1 << i);
        }
    }

    if (!dlightBits) {
        tr.pc.c_dlightSurfacesCulled++;
    }

    srf->drawinfo.dlightBits[tr.smpFrame] = dlightBits;
    return dlightBits;
}

/*
======================
R_DlightTrisurf
======================
*/
static int R_DlightTrisurf( srfTriangles_t *surf, int dlightBits ) {
	// FIXME: more dlight culling to trisurfs...
	surf->dlightBits[ tr.smpFrame ] = dlightBits;
	return dlightBits;
#if 0
	int			i;
	dlight_t	*dl;

	for ( i = 0 ; i < tr.refdef.num_dlights ; i++ ) {
		if ( ! ( dlightBits & ( 1 << i ) ) ) {
			continue;
		}
		dl = &tr.refdef.dlights[i];
		if ( dl->origin[0] - dl->radius > grid->meshBounds[1][0]
			|| dl->origin[0] + dl->radius < grid->meshBounds[0][0]
			|| dl->origin[1] - dl->radius > grid->meshBounds[1][1]
			|| dl->origin[1] + dl->radius < grid->meshBounds[0][1]
			|| dl->origin[2] - dl->radius > grid->meshBounds[1][2]
			|| dl->origin[2] + dl->radius < grid->meshBounds[0][2] ) {
			// dlight doesn't reach the bounds
			dlightBits &= ~( 1 << i );
		}
	}

	if ( !dlightBits ) {
		tr.pc.c_dlightSurfacesCulled++;
	}

	grid->dlightBits[ tr.smpFrame ] = dlightBits;
	return dlightBits;
#endif
}

/*
====================
R_DlightSurface

The given surface is going to be drawn, and it touches a leaf
that is touched by one or more dlights, so try to throw out
more dlights if possible.
====================
*/
static int R_DlightSurface( msurface_t *surf, int dlightBits ) {
	int dlightMap;

	dlightMap = 0;

	if ( *surf->data == SF_FACE ) {
		if (!r_fastdlights->integer) {
			if (surf->shader->lightmapIndex >= 0) {
				dlightMap = R_RealDlightFace((srfSurfaceFace_t*)surf->data, dlightBits);
			} else {
				((srfSurfaceFace_t*)surf->data)->dlightMap[tr.smpFrame] = 0;
				dlightMap = 0;
			}
			return dlightMap;
        } else {
			dlightBits = R_FastDlightFace((srfSurfaceFace_t*)surf->data, dlightBits);
		}
	} else if ( *surf->data == SF_GRID ) {
		if (!r_fastdlights->integer) {
			if (surf->shader->lightmapIndex >= 0) {
				dlightMap = R_RealDlightPatch((srfGridMesh_t*)surf->data, dlightBits);
			} else {
				((srfGridMesh_t*)surf->data)->dlightMap[tr.smpFrame] = 0.f;
				dlightMap = 0;
            }
            return dlightMap;
        } else {
            dlightBits = R_FastDlightGrid((srfGridMesh_t*)surf->data, dlightBits);
		}
	} else {
		return dlightMap;
	}

	if (dlightBits) {
		tr.pc.c_dlightSurfaces++;
		return 1;
	}

	return dlightMap;
}

/*
======================
R_DlightTerrain
======================
*/
int R_DlightTerrain(cTerraPatchUnpacked_t* surf, int dlightBits)
{
	int dlightMap;

    dlightMap = 0;

    if (r_fastdlights->integer)
    {
        if (R_FastDlightTerrain(surf, dlightBits)) {
            ++tr.pc.c_dlightSurfaces;
			dlightMap = 1;
        }
    } else if (surf->shader->lightmapIndex < 0) {
        surf->drawinfo.dlightMap[tr.smpFrame] = 0;
    } else {
		dlightMap = R_RealDlightTerrain(surf, dlightBits);
    }

    return dlightMap;
}

/*
======================
R_CheckDlightSurface
======================
*/
static int R_CheckDlightSurface(msurface_t* surf, int dlightBits)
{
    if (dlightBits && surf->frameCount != tr.frameCount)
    {
        surf->frameCount = tr.frameCount;
        return R_DlightSurface(surf, dlightBits);
    }

	if (dlightBits)
	{
		//
		// Added in 2.0
		//  Return the existing dlight map
		if (*surf->data == SF_FACE)
		{
			return ((srfSurfaceFace_t*)surf->data)->dlightMap[tr.smpFrame];
		}
		else if (*surf->data == SF_GRID)
		{
			return ((srfGridMesh_t*)surf->data)->dlightMap[tr.smpFrame];
		}
	}
	else
	{
		if (*surf->data == SF_FACE)
		{
			((srfSurfaceFace_t*)surf->data)->dlightMap[tr.smpFrame] = 0;
			((srfSurfaceFace_t*)surf->data)->dlightBits[tr.smpFrame] = 0;
		}
		else if (*surf->data == SF_GRID)
		{
			((srfGridMesh_t*)surf->data)->dlightMap[tr.smpFrame] = 0;
			((srfGridMesh_t*)surf->data)->dlightBits[tr.smpFrame] = 0;
		}
	}

    return 0;
}

/*
======================
R_CheckDlightTerrain
======================
*/
int R_CheckDlightTerrain(cTerraPatchUnpacked_t* surf, int dlightBits)
{
	if (surf->frameCount == tr.frameCount) {
		return surf->drawinfo.dlightMap[tr.smpFrame];
	}

	surf->frameCount = tr.frameCount;
	if (dlightBits) {
		return R_DlightTerrain(surf, dlightBits);
	}

	surf->drawinfo.dlightMap[tr.smpFrame] = 0;
	surf->drawinfo.dlightBits[tr.smpFrame] = 0;
	return 0;
}

/*
======================
R_AddWorldSurface
======================
*/
static void R_AddWorldSurface( msurface_t *surf, int dlightBits ) {
	//if ( surf->viewCount == tr.viewCount ) {
	//	return;		// already in this view
	//}

	surf->viewCount = tr.viewCount;
	// FIXME: bmodel fog?

	// try to cull before dlighting or adding
	if ( R_CullSurface( surf->data, surf->shader ) ) {
		return;
	}

	// check for dlighting
	dlightBits = R_CheckDlightSurface(surf, dlightBits);
	if (surf->shader && surf->shader->isPortalSky) {
		// Sky portal
		R_Sky_AddSurf(surf);
		return;
	}

	R_AddDrawSurf( surf->data, surf->shader, dlightBits );
}

/*
=============================================================

	BRUSH MODELS

=============================================================
*/

/*
=================
R_AddBrushModelSurfaces
=================
*/
void R_AddBrushModelSurfaces ( trRefEntity_t *ent ) {
	bmodel_t	*bmodel;
	int			clip;
	model_t		*pModel;
	int			i;

	pModel = R_GetModelByHandle( ent->e.hModel );

	bmodel = pModel->d.bmodel;

	clip = R_CullLocalBox( bmodel->bounds );
	if ( clip == CULL_OUT ) {
		return;
	}
	
	R_DlightBmodel( bmodel );

	for ( i = 0 ; i < bmodel->numSurfaces ; i++ ) {
		R_AddWorldSurface( bmodel->firstSurface + i, tr.currentEntity->needDlights );
	}
}


/*
=============================================================

	WORLD MODEL

=============================================================
*/

void R_GetInlineModelBounds(int iIndex, vec3_t vMins, vec3_t vMaxs)
{
	bmodel_t* bmodel;

	bmodel = &tr.world->bmodels[iIndex];
	VectorCopy(bmodel->bounds[0], vMins);
	VectorCopy(bmodel->bounds[1], vMaxs);
}


/*
================
R_RecursiveWorldNode
================
*/
static void R_RecursiveWorldNode( mnode_t *node, int planeBits, int dlightBits ) {

	do {
		// if the node wasn't marked as potentially visible, exit
		if (node->visframe != tr.visCount) {
			return;
		}

		// if the bounding volume is outside the frustum, nothing
		// inside can be visible OPTIMIZE: don't do this all the way to leafs?

		if ( !r_nocull->integer ) {
			int		r;

			if (planeBits & 1) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[0]);
				if (r == 2) {
					return;						// culled
				}
				if (r == 1) {
					planeBits &= ~1;			// all descendants will also be in front
				}
			}

			if (planeBits & 2) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[1]);
				if (r == 2) {
					return;						// culled
				}
				if (r == 1) {
					planeBits &= ~2;			// all descendants will also be in front
				}
			}

			if (planeBits & 4) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[2]);
				if (r == 2) {
					return;						// culled
				}
				if (r == 1) {
					planeBits &= ~4;			// all descendants will also be in front
				}
			}

			if (planeBits & 8) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[3]);
				if (r == 2) {
					return;						// culled
				}
				if (r == 1) {
					planeBits &= ~8;			// all descendants will also be in front
				}
			}

			if (planeBits & 16) {
				r = BoxOnPlaneSide(node->mins, node->maxs, &tr.viewParms.frustum[4]);
				if (r == 2) {
					return;						// culled
				}
				if (r == 1) {
					planeBits &= ~16;			// all descendants will also be in front
				}
			}

		}

		if ( node->contents != -1 ) {
			break;
		}

		// recurse down the children, front side first
		R_RecursiveWorldNode (node->children[0], planeBits, dlightBits);

		// tail recurse
		node = node->children[1];
	} while ( 1 );

	{
		// leaf node, so add mark surfaces
		int			c;
		msurface_t	*surf, **mark;

		tr.pc.c_leafs++;

		// add to z buffer bounds
		if ( node->mins[0] < tr.viewParms.visBounds[0][0] ) {
			tr.viewParms.visBounds[0][0] = node->mins[0];
		}
		if ( node->mins[1] < tr.viewParms.visBounds[0][1] ) {
			tr.viewParms.visBounds[0][1] = node->mins[1];
		}
		if ( node->mins[2] < tr.viewParms.visBounds[0][2] ) {
			tr.viewParms.visBounds[0][2] = node->mins[2];
		}

		if ( node->maxs[0] > tr.viewParms.visBounds[1][0] ) {
			tr.viewParms.visBounds[1][0] = node->maxs[0];
		}
		if ( node->maxs[1] > tr.viewParms.visBounds[1][1] ) {
			tr.viewParms.visBounds[1][1] = node->maxs[1];
		}
		if ( node->maxs[2] > tr.viewParms.visBounds[1][2] ) {
			tr.viewParms.visBounds[1][2] = node->maxs[2];
		}

		tr.portalsky.cntNode = node;

		if (r_drawbrushes->integer) {
			// add the individual surfaces
			mark = node->firstmarksurface;
			c = node->nummarksurfaces;
			while (c--) {
				// the surface may have already been added if it
				// spans multiple leafs
				surf = *mark;
				if (surf->viewCount != tr.viewCount) {
					R_AddWorldSurface(surf, dlightBits);
				}
				mark++;
			}
		}

		if (r_drawterrain->integer && tr.refdef.render_terrain && !tr.viewParms.isPortalSky)
		{
			int i;

			for (i = 0; i < node->numTerraPatches; i++) {
				R_MarkTerrainPatch(tr.world->visTerraPatches[node->firstTerraPatch + i]);
			}
		}

		if (r_drawstaticdecals->integer) {
			if (node->pFirstMarkFragment) {
				R_AddPermanentMarkFragmentSurfaces(node->pFirstMarkFragment, node->iNumMarkFragment);
			}
		}

		if (r_drawstaticmodels->integer) {
			int i;

			for (i = 0; i < node->numStaticModels; i++) {
				tr.world->visStaticModels[node->firstStaticModel + i]->visCount = tr.visCount;
			}
		}
	}

}

int R_SphereInLeafs(const vec3_t p, float r, mnode_t** nodes, int nMaxNodes) {
	mnode_t* nodestack[1024];
	int iNodeStackPos;
	mnode_t* pCurNode;
	int nFoundNodes;

	iNodeStackPos = 0;
	pCurNode = tr.world->nodes;
	nFoundNodes = 0;

	while (1)
    {
        cplane_t* plane;
		float d;

		while (pCurNode->contents == -1)
		{
			plane = pCurNode->plane;
			if (plane->type >= PLANE_NON_AXIAL) {
				d = DotProduct(p, plane->normal) - plane->dist;
			} else {
				d = p[plane->type] - plane->dist;
			}

            if (d < r) {
                if (d > -r) {
                    nodestack[iNodeStackPos++] = pCurNode->children[0];
                }
                pCurNode = pCurNode->children[1];
            } else {
                pCurNode = pCurNode->children[0];
			}
		}

		nodes[nFoundNodes++] = pCurNode;
		if (!iNodeStackPos || nFoundNodes == nMaxNodes) {
			break;
		}

		iNodeStackPos--;
        pCurNode = nodestack[iNodeStackPos];
	}

	return nFoundNodes;
}
/*
===============
R_PointInLeaf
===============
*/
mnode_t *R_PointInLeaf( const vec3_t p ) {
	mnode_t		*node;
	float		d;
	cplane_t	*plane;
	
	if ( !tr.world ) {
		ri.Error (ERR_DROP, "R_PointInLeaf: bad model");
	}
	
	node = tr.world->nodes;
	while( 1 ) {
		if (node->contents != -1) {
			break;
		}
		plane = node->plane;
		d = DotProduct (p,plane->normal) - plane->dist;
		if (d > 0) {
			node = node->children[0];
		} else {
			node = node->children[1];
		}
	}
	
	return node;
}

/*
==============
R_ClusterPVS
==============
*/
static const byte *R_ClusterPVS (int cluster) {
	if (!tr.world || !tr.world->vis || cluster < 0 || cluster >= tr.world->numClusters ) {
		return tr.world->novis;
	}

	return tr.world->vis + cluster * tr.world->clusterBytes;
}

/*
=================
R_inPVS
=================
*/
qboolean R_inPVS( const vec3_t p1, const vec3_t p2 ) {
	mnode_t *leaf;
	byte	*vis;

	leaf = R_PointInLeaf( p1 );
	vis = ri.CM_ClusterPVS( leaf->cluster );
	leaf = R_PointInLeaf( p2 );

	if ( !(vis[leaf->cluster>>3] & (1<<(leaf->cluster&7))) ) {
		return qfalse;
	}
	return qtrue;
}

/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
static void R_MarkLeaves (void) {
	const byte	*vis;
	mnode_t	*leaf, *parent;
	int		i;
	int		cluster;

	// lockpvs lets designers walk around to determine the
	// extent of the current pvs
	if ( r_lockpvs->integer ) {
		return;
	}

	// current viewcluster
	leaf = R_PointInLeaf( tr.viewParms.pvsOrigin );
	cluster = leaf->cluster;

	// if the cluster is the same and the area visibility matrix
	// hasn't changed, we don't need to mark everything again

	// if r_showcluster was just turned on, remark everything 
	if ( tr.viewCluster == cluster && !tr.refdef.areamaskModified 
		&& !r_showcluster->modified ) {
		return;
	}

	if ( r_showcluster->modified || r_showcluster->integer ) {
		r_showcluster->modified = qfalse;
		if ( r_showcluster->integer ) {
			ri.Printf( PRINT_ALL, "cluster:%i  area:%i\n", cluster, leaf->area );
		}
	}

	tr.visCount++;
	tr.viewCluster = cluster;

	if ( r_novis->integer || tr.viewCluster == -1 ) {
		for (i=0 ; i<tr.world->numnodes ; i++) {
			if (tr.world->nodes[i].contents != CONTENTS_SOLID) {
				tr.world->nodes[i].visframe = tr.visCount;
			}
		}
		return;
	}

	vis = R_ClusterPVS (tr.viewCluster);
	
	for (i=0,leaf=tr.world->nodes ; i<tr.world->numnodes ; i++, leaf++) {
		cluster = leaf->cluster;
		if ( cluster < 0 || cluster >= tr.world->numClusters ) {
			continue;
		}

		// check general pvs
		if ( !(vis[cluster>>3] & (1<<(cluster&7))) ) {
			continue;
		}

		// check for door connection
		if ( (tr.refdef.areamask[leaf->area>>3] & (1<<(leaf->area&7)) ) ) {
			continue;		// not visible
		}

		parent = leaf;
		do {
			if (parent->visframe == tr.visCount)
				break;
			parent->visframe = tr.visCount;
			parent = parent->parent;
		} while (parent);
	}
}


/*
=============
R_AddWorldSurfaces
=============
*/
void R_AddWorldSurfaces (void) {
	if (!r_drawworld->integer) {
		return;
	}

	if (tr.refdef.rdflags & RDF_NOWORLDMODEL) {
		return;
	}

	tr.currentEntityNum = ENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_ENTITYNUM_SHIFT;

	if (r_drawterrain->integer && tr.refdef.render_terrain && !tr.viewParms.isPortalSky) {
		R_TerrainPrepareFrame();
	}

	// determine which leaves are in the PVS / areamask
	R_MarkLeaves();

	// clear out the visible min/max
	ClearBounds(tr.viewParms.visBounds[0], tr.viewParms.visBounds[1]);

	// perform frustum culling and add all the potentially visible surfaces
	if (tr.refdef.num_dlights > 32) {
		tr.refdef.num_dlights = 32;
	}
	R_TransformDlights(tr.refdef.num_dlights, tr.refdef.dlights, &tr.viewParms.world);
	R_RecursiveWorldNode(tr.world->nodes, tr.viewParms.fog.extrafrustums ? 31 : 15, (1 << tr.refdef.num_dlights) - 1);

	if (r_drawterrain->integer && tr.refdef.render_terrain && !tr.viewParms.isPortalSky) {
		R_AddTerrainSurfaces();
	}
	if (r_drawstaticmodels->integer) {
		R_AddStaticModelSurfaces();
	}

	if (g_bInfostaticmodels) {
		g_bInfostaticmodels = 0;
		R_PrintInfoStaticModels();
	}

	R_UpdateLevelMarksSystem();
}
