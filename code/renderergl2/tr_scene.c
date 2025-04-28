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
along with Quake III Arena source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#include "tr_local.h"

int			r_firstSceneDrawSurf;

int			r_numdlights;
int			r_firstSceneDlight;

int			r_numentities;
int			r_firstSceneEntity;

int			r_numpolys;
int			r_firstScenePoly;

int			r_numpolyverts;

//
// OPENMOHAA-specific stuff
//=========================

int			r_firstSceneSpriteSurf;

int			r_numsprites;
int			r_firstSceneSprite;

int			r_numtermarks;
int			r_firstSceneTerMark;

//=========================

/*
====================
R_InitNextFrame

====================
*/
void R_InitNextFrame( void ) {
	backEndData->commands.used = 0;

	r_firstSceneDrawSurf = 0;

	r_numdlights = 0;
	r_firstSceneDlight = 0;

	r_numentities = 0;
	r_firstSceneEntity = 0;

	r_numpolys = 0;
	r_firstScenePoly = 0;

	r_numpolyverts = 0;

    //
    // OPENMOHAA-specific stuff
    //

    r_firstSceneSpriteSurf = 0;

	r_numsprites = 0;
	r_firstSceneSprite = 0;

	r_numtermarks = 0;
	r_firstSceneTerMark = 0;
}


/*
====================
RE_ClearScene

====================
*/
void RE_ClearScene( void ) {
	r_firstSceneDlight = r_numdlights;
	r_firstSceneEntity = r_numentities;
	r_firstScenePoly = r_numpolys;
}

/*
===========================================================================

DISCRETE POLYS

===========================================================================
*/

/*
=====================
R_AddPolygonSurfaces

Adds all the scene's polys into this view's drawsurf list
=====================
*/
void R_AddPolygonSurfaces( void ) {
	int			i;
	shader_t	*sh;
	srfPoly_t	*poly;
	int		fogMask;

	tr.currentEntityNum = REFENTITYNUM_WORLD;
	tr.shiftedEntityNum = tr.currentEntityNum << QSORT_REFENTITYNUM_SHIFT;
	fogMask = -((tr.refdef.rdflags & RDF_NOFOG) == 0);

	for ( i = 0, poly = tr.refdef.polys; i < tr.refdef.numPolys ; i++, poly++ ) {
		sh = R_GetShaderByHandle( poly->hShader );
		R_AddDrawSurf( ( void * )poly, sh, poly->fogIndex & fogMask, qfalse, qfalse, 0 /*cubeMap*/  );
	}
}

/*
=====================
RE_AddPolyToScene

=====================
*/
void RE_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys ) {
	srfPoly_t	*poly;
	int			i, j;
	int			fogIndex;
	fog_t		*fog;
	vec3_t		bounds[2];

	if ( !tr.registered ) {
		return;
	}

	if ( !hShader ) {
		// This isn't a useful warning, and an hShader of zero isn't a null shader, it's
		// the default shader.
		//ri.Printf( PRINT_WARNING, "WARNING: RE_AddPolyToScene: NULL poly shader\n");
		//return;
	}

	for ( j = 0; j < numPolys; j++ ) {
		if ( r_numpolyverts + numVerts > max_polyverts || r_numpolys >= max_polys ) {
      /*
      NOTE TTimo this was initially a PRINT_WARNING
      but it happens a lot with high fighting scenes and particles
      since we don't plan on changing the const and making for room for those effects
      simply cut this message to developer only
      */
			ri.Printf( PRINT_DEVELOPER, "WARNING: RE_AddPolyToScene: r_max_polys or r_max_polyverts reached\n");
			return;
		}

		poly = &backEndData->polys[r_numpolys];
		poly->surfaceType = SF_POLY;
		poly->hShader = hShader;
		poly->numVerts = numVerts;
		poly->verts = &backEndData->polyVerts[r_numpolyverts];
		
		Com_Memcpy( poly->verts, &verts[numVerts*j], numVerts * sizeof( *verts ) );

		if ( glConfig.hardwareType == GLHW_RAGEPRO ) {
			poly->verts->modulate[0] = 255;
			poly->verts->modulate[1] = 255;
			poly->verts->modulate[2] = 255;
			poly->verts->modulate[3] = 255;
		}
		// done.
		r_numpolys++;
		r_numpolyverts += numVerts;

		// if no world is loaded
		if ( tr.world == NULL ) {
			fogIndex = 0;
		}
		// see if it is in a fog volume
		else if ( tr.world->numfogs == 1 ) {
			fogIndex = 0;
		} else {
			// find which fog volume the poly is in
			VectorCopy( poly->verts[0].xyz, bounds[0] );
			VectorCopy( poly->verts[0].xyz, bounds[1] );
			for ( i = 1 ; i < poly->numVerts ; i++ ) {
				AddPointToBounds( poly->verts[i].xyz, bounds[0], bounds[1] );
			}
			for ( fogIndex = 1 ; fogIndex < tr.world->numfogs ; fogIndex++ ) {
				fog = &tr.world->fogs[fogIndex]; 
				if ( bounds[1][0] >= fog->bounds[0][0]
					&& bounds[1][1] >= fog->bounds[0][1]
					&& bounds[1][2] >= fog->bounds[0][2]
					&& bounds[0][0] <= fog->bounds[1][0]
					&& bounds[0][1] <= fog->bounds[1][1]
					&& bounds[0][2] <= fog->bounds[1][2] ) {
					break;
				}
			}
			if ( fogIndex == tr.world->numfogs ) {
				fogIndex = 0;
			}
		}
		poly->fogIndex = fogIndex;
	}
}


//=================================================================================


/*
=====================
RE_AddRefEntityToScene

=====================
*/
void RE_AddRefEntityToScene( const refEntity_t *ent ) {
	vec3_t cross;

	if ( !tr.registered ) {
		return;
	}
	if ( r_numentities >= MAX_REFENTITIES ) {
		ri.Printf(PRINT_DEVELOPER, "RE_AddRefEntityToScene: Dropping refEntity, reached MAX_REFENTITIES\n");
		return;
	}
	if ( Q_isnan(ent->origin[0]) || Q_isnan(ent->origin[1]) || Q_isnan(ent->origin[2]) ) {
		static qboolean firstTime = qtrue;
		if (firstTime) {
			firstTime = qfalse;
			ri.Printf( PRINT_WARNING, "RE_AddRefEntityToScene passed a refEntity which has an origin with a NaN component\n");
		}
		return;
	}
	if ( (int)ent->reType < 0 || ent->reType >= RT_MAX_REF_ENTITY_TYPE ) {
		ri.Error( ERR_DROP, "RE_AddRefEntityToScene: bad reType %i", ent->reType );
	}

	backEndData->entities[r_numentities].e = *ent;
	backEndData->entities[r_numentities].lightingCalculated = qfalse;

	CrossProduct(ent->axis[0], ent->axis[1], cross);
	backEndData->entities[r_numentities].mirrored = (DotProduct(ent->axis[2], cross) < 0.f);

	r_numentities++;
}


/*
=====================
RE_AddDynamicLightToScene

=====================
*/
void RE_AddDynamicLightToScene( const vec3_t org, float intensity, float r, float g, float b, int additive ) {
	dlight_t	*dl;

	if ( !tr.registered ) {
		return;
	}
	if ( r_numdlights >= MAX_DLIGHTS ) {
		return;
	}
	if ( intensity <= 0 ) {
		return;
	}
	// these cards don't have the correct blend mode
	if ( glConfig.hardwareType == GLHW_RIVA128 || glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		return;
	}
	dl = &backEndData->dlights[r_numdlights++];
	VectorCopy (org, dl->origin);
	dl->radius = intensity;
	dl->color[0] = r;
	dl->color[1] = g;
	dl->color[2] = b;
	dl->additive = additive;
}

/*
=====================
RE_AddLightToScene

=====================
*/
void RE_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	RE_AddDynamicLightToScene( org, intensity, r, g, b, qfalse );
}

/*
=====================
RE_AddAdditiveLightToScene

=====================
*/
void RE_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	RE_AddDynamicLightToScene( org, intensity, r, g, b, qtrue );
}


void RE_BeginScene(const refdef_t *fd)
{
	Com_Memcpy( tr.refdef.text, fd->text, sizeof( tr.refdef.text ) );

	tr.refdef.x = fd->x;
	tr.refdef.y = fd->y;
	tr.refdef.width = fd->width;
	tr.refdef.height = fd->height;
	tr.refdef.fov_x = fd->fov_x;
	tr.refdef.fov_y = fd->fov_y;

	VectorCopy( fd->vieworg, tr.refdef.vieworg );
	VectorCopy( fd->viewaxis[0], tr.refdef.viewaxis[0] );
	VectorCopy( fd->viewaxis[1], tr.refdef.viewaxis[1] );
	VectorCopy( fd->viewaxis[2], tr.refdef.viewaxis[2] );

	tr.refdef.time = fd->time;
	tr.refdef.rdflags = fd->rdflags;

	// copy the areamask data over and note if it has changed, which
	// will force a reset of the visible leafs even if the view hasn't moved
	tr.refdef.areamaskModified = qfalse;
	if ( ! (tr.refdef.rdflags & RDF_NOWORLDMODEL) ) {
		int		areaDiff;
		int		i;

		// compare the area bits
		areaDiff = 0;
		for (i = 0 ; i < MAX_MAP_AREA_BYTES/4 ; i++) {
			areaDiff |= ((int *)tr.refdef.areamask)[i] ^ ((int *)fd->areamask)[i];
			((int *)tr.refdef.areamask)[i] = ((int *)fd->areamask)[i];
		}

		if ( areaDiff ) {
			// a door just opened or something
			tr.refdef.areamaskModified = qtrue;
		}
	}

	tr.refdef.sunDir[3] = 0.0f;
	tr.refdef.sunCol[3] = 1.0f;
	tr.refdef.sunAmbCol[3] = 1.0f;

	VectorCopy(tr.sunDirection, tr.refdef.sunDir);
	if ( (tr.refdef.rdflags & RDF_NOWORLDMODEL) || !(r_depthPrepass->value) ){
		VectorSet(tr.refdef.sunCol, 0, 0, 0);
		VectorSet(tr.refdef.sunAmbCol, 0, 0, 0);
	}
	else
	{
		float scale = (1 << r_mapOverBrightBits->integer) / 255.0f;

		if (r_forceSun->integer)
			VectorScale(tr.sunLight, scale * r_forceSunLightScale->value, tr.refdef.sunCol);
		else
			VectorScale(tr.sunLight, scale, tr.refdef.sunCol);

		if (r_sunlightMode->integer == 1)
		{
			tr.refdef.sunAmbCol[0] =
			tr.refdef.sunAmbCol[1] =
			tr.refdef.sunAmbCol[2] = r_forceSun->integer ? r_forceSunAmbientScale->value : tr.sunShadowScale;
		}
		else
		{
			if (r_forceSun->integer)
				VectorScale(tr.sunLight, scale * r_forceSunAmbientScale->value, tr.refdef.sunAmbCol);
			else
				VectorScale(tr.sunLight, scale * tr.sunShadowScale, tr.refdef.sunAmbCol);
		}
	}

	if (r_forceAutoExposure->integer)
	{
		tr.refdef.autoExposureMinMax[0] = r_forceAutoExposureMin->value;
		tr.refdef.autoExposureMinMax[1] = r_forceAutoExposureMax->value;
	}
	else
	{
		tr.refdef.autoExposureMinMax[0] = tr.autoExposureMinMax[0];
		tr.refdef.autoExposureMinMax[1] = tr.autoExposureMinMax[1];
	}

	if (r_forceToneMap->integer)
	{
		tr.refdef.toneMinAvgMaxLinear[0] = pow(2, r_forceToneMapMin->value);
		tr.refdef.toneMinAvgMaxLinear[1] = pow(2, r_forceToneMapAvg->value);
		tr.refdef.toneMinAvgMaxLinear[2] = pow(2, r_forceToneMapMax->value);
	}
	else
	{
		tr.refdef.toneMinAvgMaxLinear[0] = pow(2, tr.toneMinAvgMaxLevel[0]);
		tr.refdef.toneMinAvgMaxLinear[1] = pow(2, tr.toneMinAvgMaxLevel[1]);
		tr.refdef.toneMinAvgMaxLinear[2] = pow(2, tr.toneMinAvgMaxLevel[2]);
	}

	// Makro - copy exta info if present
	if (fd->rdflags & RDF_EXTRA) {
		const refdefex_t* extra = (const refdefex_t*) (fd+1);

		tr.refdef.blurFactor = extra->blurFactor;

		if (fd->rdflags & RDF_SUNLIGHT)
		{
			VectorCopy(extra->sunDir,    tr.refdef.sunDir);
			VectorCopy(extra->sunCol,    tr.refdef.sunCol);
			VectorCopy(extra->sunAmbCol, tr.refdef.sunAmbCol);
		}
	} 
	else
	{
		tr.refdef.blurFactor = 0.0f;
	}

	// derived info

	tr.refdef.floatTime = tr.refdef.time * 0.001;

	tr.refdef.numDrawSurfs = r_firstSceneDrawSurf;
	tr.refdef.drawSurfs = backEndData->drawSurfs;

	tr.refdef.num_entities = r_numentities - r_firstSceneEntity;
	tr.refdef.entities = &backEndData->entities[r_firstSceneEntity];

	tr.refdef.num_dlights = r_numdlights - r_firstSceneDlight;
	tr.refdef.dlights = &backEndData->dlights[r_firstSceneDlight];

	tr.refdef.numPolys = r_numpolys - r_firstScenePoly;
	tr.refdef.polys = &backEndData->polys[r_firstScenePoly];

	tr.refdef.num_pshadows = 0;
	tr.refdef.pshadows = &backEndData->pshadows[0];

	// turn off dynamic lighting globally by clearing all the
	// dlights if it needs to be disabled or if vertex lighting is enabled
	if ( r_dynamiclight->integer == 0 ||
		 r_vertexLight->integer == 1 ||
		 glConfig.hardwareType == GLHW_PERMEDIA2 ) {
		tr.refdef.num_dlights = 0;
	}

	// a single frame may have multiple scenes draw inside it --
	// a 3D game view, 3D status bar renderings, 3D menus, etc.
	// They need to be distinguished by the light flare code, because
	// the visibility state for a given surface may be different in
	// each scene / view.
	tr.frameSceneNum++;
	tr.sceneCount++;

	//
	// OPENMOHAA-specific stuff
    //=========================
    TIKI_Reset_Caches();

    // copy the sky data
    tr.refdef.sky_alpha = fd->sky_alpha;
    tr.refdef.sky_portal = fd->sky_portal;

    VectorCopy(fd->sky_origin, tr.refdef.sky_origin);
    VectorCopy(fd->sky_axis[0], tr.refdef.sky_axis[0]);
    VectorCopy(fd->sky_axis[1], tr.refdef.sky_axis[1]);
    VectorCopy(fd->sky_axis[2], tr.refdef.sky_axis[2]);

    tr.refdef.numSpriteSurfs = r_firstSceneSpriteSurf;
    tr.refdef.spriteSurfs = backEndData->spriteSurfs;

    tr.refdef.num_sprites = r_numsprites - r_firstSceneSprite;
    tr.refdef.sprites = &backEndData->sprites[r_firstSceneSprite];

    tr.refdef.numTerMarks = r_numtermarks - r_firstSceneTerMark;
    tr.refdef.terMarks = &backEndData->terMarks[r_firstSceneTerMark];

    backEndData->staticModelData = tr.refdef.staticModelData;
	
    tr.skyRendered = qfalse;
    tr.portalRendered = qfalse;
	//=========================
}


void RE_EndScene(void)
{
	// the next scene rendered in this frame will tack on after this one
	r_firstSceneDrawSurf = tr.refdef.numDrawSurfs;
	r_firstSceneEntity = r_numentities;
	r_firstSceneDlight = r_numdlights;
	r_firstScenePoly = r_numpolys;

	//
	// OPENMOHAA-specific stuff
    //

    r_firstSceneSpriteSurf = tr.refdef.numSpriteSurfs;
    r_firstSceneSprite = r_numsprites;
    r_firstSceneTerMark = r_numtermarks;
}

/*
@@@@@@@@@@@@@@@@@@@@@
RE_RenderScene

Draw a 3D view into a part of the window, then return
to 2D drawing.

Rendering a scene may require multiple views to be rendered
to handle mirrors,
@@@@@@@@@@@@@@@@@@@@@
*/
void RE_RenderScene( const refdef_t *fd ) {
	viewParms_t		parms;
	int				startTime;

	if ( !tr.registered ) {
		return;
	}
	GLimp_LogComment( "====== RE_RenderScene =====\n" );

	if ( r_norefresh->integer ) {
		return;
	}

	startTime = ri.Milliseconds();

	if (!tr.world && !( fd->rdflags & RDF_NOWORLDMODEL ) ) {
		ri.Error (ERR_DROP, "R_RenderScene: NULL worldmodel");
	}

	RE_BeginScene(fd);

	// SmileTheory: playing with shadow mapping
	if (!( fd->rdflags & RDF_NOWORLDMODEL ) && tr.refdef.num_dlights && r_dlightMode->integer >= 2)
	{
		R_RenderDlightCubemaps(fd);
	}

	/* playing with more shadows */
	if(glRefConfig.framebufferObject && !( fd->rdflags & RDF_NOWORLDMODEL ) && r_shadows->integer == 4)
	{
		R_RenderPshadowMaps(fd);
	}

	// playing with even more shadows
	if(glRefConfig.framebufferObject && r_sunlightMode->integer && !( fd->rdflags & RDF_NOWORLDMODEL ) && (r_forceSun->integer || tr.sunShadows))
	{
		if (r_shadowCascadeZFar->integer != 0)
		{
			R_RenderSunShadowMaps(fd, 0);
			R_RenderSunShadowMaps(fd, 1);
			R_RenderSunShadowMaps(fd, 2);
		}
		else
		{
			Mat4Zero(tr.refdef.sunShadowMvp[0]);
			Mat4Zero(tr.refdef.sunShadowMvp[1]);
			Mat4Zero(tr.refdef.sunShadowMvp[2]);
		}

		// only rerender last cascade if sun has changed position
		if (r_forceSun->integer == 2 || !VectorCompare(tr.refdef.sunDir, tr.lastCascadeSunDirection))
		{
			VectorCopy(tr.refdef.sunDir, tr.lastCascadeSunDirection);
			R_RenderSunShadowMaps(fd, 3);
			Mat4Copy(tr.refdef.sunShadowMvp[3], tr.lastCascadeSunMvp);
		}
		else
		{
			Mat4Copy(tr.lastCascadeSunMvp, tr.refdef.sunShadowMvp[3]);
		}
	}

	// playing with cube maps
	// this is where dynamic cubemaps would be rendered
	if (0) //(glRefConfig.framebufferObject && !( fd->rdflags & RDF_NOWORLDMODEL ))
	{
		int i, j;

		for (i = 0; i < tr.numCubemaps; i++)
		{
			for (j = 0; j < 6; j++)
			{
				R_RenderCubemapSide(i, j, qtrue);
			}
		}
	}

	// setup view parms for the initial view
	//
	// set up viewport
	// The refdef takes 0-at-the-top y coordinates, so
	// convert to GL's 0-at-the-bottom space
	//
	Com_Memset( &parms, 0, sizeof( parms ) );
	parms.viewportX = tr.refdef.x;
	parms.viewportY = glConfig.vidHeight - ( tr.refdef.y + tr.refdef.height );
	parms.viewportWidth = tr.refdef.width;
	parms.viewportHeight = tr.refdef.height;
	parms.isPortal = qfalse;

	parms.fovX = tr.refdef.fov_x;
	parms.fovY = tr.refdef.fov_y;
	
	parms.stereoFrame = tr.refdef.stereoFrame;

	VectorCopy( fd->vieworg, parms.or.origin );
	VectorCopy( fd->viewaxis[0], parms.or.axis[0] );
	VectorCopy( fd->viewaxis[1], parms.or.axis[1] );
	VectorCopy( fd->viewaxis[2], parms.or.axis[2] );

	VectorCopy( fd->vieworg, parms.pvsOrigin );

	if(!( fd->rdflags & RDF_NOWORLDMODEL ) && r_depthPrepass->value && ((r_forceSun->integer) || tr.sunShadows))
	{
		parms.flags = VPF_USESUNLIGHT;
	}

	//
	// OPENMOHAA-specific stuff
	//=========================

	// copy the farplane data
	parms.farplane_distance = fd->farplane_distance;
	parms.farplane_bias = fd->farplane_bias;
	parms.farplane_color[0] = fd->farplane_color[0];
	parms.farplane_color[1] = fd->farplane_color[1];
	parms.farplane_color[2] = fd->farplane_color[2];
	parms.farplane_cull = fd->farplane_cull;
	parms.renderTerrain = fd->renderTerrain;

	tr.refdef.skybox_farplane = fd->skybox_farplane;
	tr.refdef.render_terrain = parms.renderTerrain;

	if (fd->farclipOverride >= 15900 || fd->farclipOverride <= -0.99) {
		tr.farclip = 0;
	} else {
		tr.farclip = r_farclip->integer;
		if (!tr.farclip && (r_picmip->integer > 1 || r_colorbits->integer == 16)) {
			tr.farclip = 2800;
		}
	}

	if (tr.farclip) {
        if (fd->farclipOverride != 0) {
            parms.farplane_distance = fd->farclipOverride;
		} else {
			parms.farplane_distance = tr.farclip;
		}

		if (fd->farplane_color[0] >= 0 && fd->farplane_color[1] >= 0 && fd->farplane_color[2] >= 0) {
            parms.farplane_color[0] = fd->farplane_color[0];
            parms.farplane_color[1] = fd->farplane_color[1];
            parms.farplane_color[2] = fd->farplane_color[2];
		}

		if (fd->farplaneColorOverride[0] >= 0 && fd->farplaneColorOverride[1] >= 0 && fd->farplaneColorOverride[2] >= 0) {
			parms.farplane_color[0] = fd->farplaneColorOverride[0];
			parms.farplane_color[1] = fd->farplaneColorOverride[1];
			parms.farplane_color[2] = fd->farplaneColorOverride[2];
		}

		parms.farplane_cull = qtrue;

        if (fd->farplane_distance > 0 && fd->farplane_distance < parms.farplane_distance) {
            parms.farplane_distance = fd->farplane_distance;
		} else {
			if (fd->farplane_bias == 0) {
				parms.farplane_bias = parms.farplane_distance * 0.18f;
			} else if (fd->farplane_distance <= 500.f) {
				parms.farplane_bias = parms.farplane_distance * 0.18f;
			} else {
				parms.farplane_bias = parms.farplane_distance / fd->farplane_distance;
			}
		}
	} else if (parms.farplane_bias == 0) {
		parms.farplane_bias = parms.farplane_distance * 0.18f;
	}
	//=========================

	R_RenderView( &parms );

	if(!( fd->rdflags & RDF_NOWORLDMODEL ))
		R_AddPostProcessCmd();

	RE_EndScene();

	tr.frontEndMsec += ri.Milliseconds() - startTime;
}

//
// OPENMOHAA-specific stuff
//

/*
=====================
RE_AddLightToScene2

=====================
*/
void RE_AddLightToScene2(const vec3_t org, float intensity, float r, float g, float b, int type) {
    RE_AddDynamicLightToScene(org, intensity, r, g, b, qfalse);
}


/*
=====================
RE_AddRefEntityToScene2

=====================
*/
void RE_AddRefEntityToScene2( const refEntity_t *ent, int parentEntityNumber ) {
	RE_AddRefEntityToScene(ent);
}

void RE_AddRefSpriteToScene(const refEntity_t* ent) {
	refSprite_t* spr;
	int i;

	if (!tr.registered) {
		return;
	}

	if (r_numsprites >= MAX_SPRITES) {
		return;
	}

	spr = &backEndData->sprites[r_numsprites];
	VectorCopy(ent->origin, spr->origin);
	spr->surftype = SF_SPRITE;
    spr->hModel = ent->hModel;
    spr->scale = ent->scale;
    spr->renderfx = ent->renderfx;
    spr->shaderTime = ent->shaderTime;
	AxisCopy(ent->axis, spr->axis);

	for (i = 0; i < 4; ++i) {
		spr->shaderRGBA[i] = ent->shaderRGBA[i];
	}

    ++r_numsprites;
}

/*
=====================
RE_AddPolyToScene

=====================
*/
qboolean RE_AddPolyToScene2(qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx) {
	srfPoly_t	*poly;

	if ( !tr.registered ) {
		return qfalse;
	}

	if (numVerts + r_numpolyverts > max_polyverts || r_numpolys >= max_polys) {
		ri.Printf(PRINT_WARNING, "Exceeded MAX POLYS\n");
		return qfalse;
	}

	poly = &backEndData->polys[r_numpolys];
	poly->surfaceType = SF_POLY;
	poly->hShader = hShader;
	poly->numVerts = numVerts;
	poly->verts = &backEndData->polyVerts[r_numpolyverts];
	poly->renderfx = renderfx;

	Com_Memcpy(poly->verts, verts, sizeof(polyVert_t) * numVerts);
	++r_numpolys;
	r_numpolyverts += numVerts;

	return qtrue;
}

/*
=====================
R_AddTerrainMarkSurfaces
=====================
*/
void R_AddTerrainMarkSurfaces(void) {
    srfMarkFragment_t* terMark;
    int j;
    shader_t* shader;

    for (j = 0; j < tr.refdef.numTerMarks; j++)
    {
        terMark = &tr.refdef.terMarks[j];

        shader = R_GetShaderByHandle(terMark->surfaceType);
        terMark->surfaceType = SF_MARK_FRAG;
        R_AddDrawSurf(&terMark->surfaceType, shader, 0, 0, 0, 0);
    }
}

/*
=====================
RE_AddTerrainMarkToScene
=====================
*/
void RE_AddTerrainMarkToScene(int iTerrainIndex, qhandle_t hShader, int numVerts, const polyVert_t* verts, int renderfx) {
    srfMarkFragment_t* terMark;

    if (!tr.registered) {
        return;
    }

    if (numVerts + r_numpolyverts > max_polyverts || r_numtermarks >= max_termarks) {
        ri.Printf(PRINT_WARNING, "Exceeded MAX TERRAIN MARKS\n");
        return;
    }

    terMark = &backEndData->terMarks[r_numtermarks];
    terMark->surfaceType = hShader;
    terMark->iIndex = iTerrainIndex;
    terMark->numVerts = numVerts;
    terMark->verts = &backEndData->polyVerts[r_numpolyverts];
    memcpy(terMark->verts, verts, sizeof(polyVert_t) * numVerts);

    r_numtermarks++;
    r_numpolyverts += numVerts;
}

//=================================================================================

/*
=====================
RE_GetRenderEntity
=====================
*/
refEntity_t* RE_GetRenderEntity(int entityNumber) {
    int i;

    for (i = 0; i < r_numentities; i++) {
        if (backEndData->entities[i].e.entityNumber == entityNumber) {
            return &backEndData->entities[i].e;
        }
    }

    return NULL;
}
