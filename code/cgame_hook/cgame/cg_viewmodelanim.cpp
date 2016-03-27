#include "glb_local.h"
#include "cg_viewmodelanim.h"
#include "script/cplayer.h"
#include "script/clientgamecommand.h"

cvar_t *cg_fakepredict;

float max_z = 94.0f;
int viewHeight = 82;
qboolean modheightfloat_used = false;
float viewHeight_mult = 1.0f;

char vm_anim_name[MAX_STRING_CHARS] = "idle";
qboolean vm_fullanim = false;
int currentviewModelAnim = 0;
int currentviewModelAnimChanged = 0;

qboolean fNoMove = 0;

vec3_t currentDeltaVMPosOffset;
vec3_t currentDeltaVMPosOffset2;

vec3_t currentVMAnglesOffset;

vec3_t oldAngles;
vec3_t deltaAngles;
vec3_t newAngles;

vec3_t eyeOldAngles;
vec3_t eyeNormalAngles;
vec3_t eyeDeltaAngles;

vec3_t eyeStartAngles;

vec3_t previousVelocity;

qboolean fIsReloading;

int landHeight = 0;
qboolean fLanding, fFell;
qboolean fLandingUp;

float landDownValue = 0.0f, landDownDelayScale = 0.0f, landDownViewScale = 0.0f;

vec3_t deltaPosition;

qboolean move_godown_z = false;

double bobMovementMultiplier = 1.0; // default 1.0

int internal_bobOffset = 1;

float *bobValue = (float*)0x303EBFF8;

float time_step = 0;
float viewbob = 0;

float vm_time = 0.0f;

Container< vma_t * > vmalist;
vma_t *current_vma = NULL;

/**********************************************
 *
 * CG_ViewInit
 * Client game hook initialization
 *
 **********************************************/
void CG_ViewInit()
{
	cg_fakepredict = cgi.Cvar_Get("cg_fakepredict", "0", 0);

	VM_VMInit();
}

/**********************************************
 *
 * CG_MatchEyesBone
 * Set the player's viewangles depending on the
 * current FPS animation's eyes bone
 *
 **********************************************/

vec3_t eyesDeltaStart = { 0, 0, 0 };
vec3_t eyesDeltaStart2 = { 0, 0, 0 };
float eyesCurrentCrossTime = 0.0f;
float eyesCurrentTime = 0.0f;

void CG_MatchEyesBone( refEntity_t * model )
{
	dtiki_t * tiki;
	int tagNum;
	orientation_t orientation, orientation_head;
	vec3_t eyeAngles, eyeStartAngles, refdefAngles, local;
	int i;
	qboolean bFirstTime = false;
	float ratio = 1.0f;
	int vmAnim = cgi.anim->lastVMAnim;

	VectorSubtract( cg->refdefViewAngles, player.eyesDeltaOld, cg->refdefViewAngles );
	VectorCopy( player.eyesDelta, player.eyesDeltaOld );

	tiki = model->tiki;

	if( tiki == NULL )
	{
		cgi.Printf( "Invalid tiki\n" );
		return;
	}

	/*if( vmAnim == VMA_IDLE ||
		vmAnim == VMA_FIRE ||
		vmAnim == VMA_FIRE_SECONDARY ||
		vmAnim == VMA_CHARGE )
	{
		float crossblend = cgi.Anim_CrossblendTime( tiki, cgi.anim->vmFrameInfo[ cgi.anim->currentVMAnimSlot ].index ) * 1000.0f;

		if( crossblend ) {
			ratio = eyesCurrentTime/crossblend;
		} else {
			ratio = 1.0f;
		}

		if( ratio > 1.0f ) {
			ratio = 1.0f;
		}

		eyesCurrentTime += cg->frametime;

		player.eyesDelta[ 0 ] = eyesDeltaStart[ 0 ] + ( 0 - eyesDeltaStart[ 0 ] ) * ratio;
		player.eyesDelta[ 1 ] = eyesDeltaStart[ 1 ] + ( 0 - eyesDeltaStart[ 1 ] ) * ratio;
		player.eyesDelta[ 2 ] = eyesDeltaStart[ 2 ] + ( 0 - eyesDeltaStart[ 2 ] ) * ratio;

		VectorCopy( player.eyesDelta, eyesDeltaStart2 );

		return;
	}
	else
	{
		eyesCurrentTime = 0.0f;
		VectorCopy( player.eyesDelta, eyesDeltaStart );
	}*/

	eyesCurrentTime = 0.0f;
	VectorCopy( player.eyesDelta, eyesDeltaStart );

	/*if( !fIsReloading )
	{
		fIsReloading = true;
		bFirstTime = true;
	}*/

	tagNum = cgi.Tag_NumForName( tiki, "eyes bone" );

	if( tagNum == -1 )
	{
		cgi.Printf( "Can't find 'eyes bone' for player\n" );
		return;
	}

	if( player.GetNumber() != model->entityNumber ) {
		player.AssignNumber( model->entityNumber );
	}

	orientation = ri.TIKI_OrientationInternal( tiki, model->entityNumber, tagNum, 1.0f );

	MatrixToEulerAngles( orientation.axis, eyeAngles );
	MatrixToEulerAngles( cg->refdef.viewAxis, refdefAngles );

	if( cgi.anim->crossBlending )
	{
		float crossblend = cgi.Anim_CrossblendTime( tiki, cgi.anim->vmFrameInfo[ cgi.anim->currentVMAnimSlot ].index ) * 1000.0f;

		ratio = eyesCurrentCrossTime/crossblend;

		if( ratio > 1.0f ) {
			ratio = 1.0f;
		}

		eyesCurrentCrossTime += cg->frametime;
	} else {
		eyesCurrentCrossTime = 0.0f;
	}

	eyeAngles[0] -= ( refdefAngles[0] + 8.0f );

	local[0] = eyesDeltaStart2[0] + ( -eyeAngles[0] - eyesDeltaStart2[0] ) * ratio;
	local[1] = eyesDeltaStart2[1] + ( -eyeAngles[1] - eyesDeltaStart2[1] ) * ratio;
	local[2] = eyesDeltaStart2[2] + ( -eyeAngles[2] - eyesDeltaStart2[2] ) * ratio;

	VectorCopy( local, player.eyesDelta );
}

/**********************************************
 *
 * CG_FakePredict
 * Fake prediction like jump, fire, crouch, etc.
 *
 **********************************************/

qboolean attackprimary_up;
qboolean movedown_up;
qboolean moveup_up;

void CG_FakePredict()
{
	kbutton_t *attackprimary;
	kbutton_t *movedown;
	kbutton_t *moveup;

	if(!cg_fakepredict->integer)
		return;

	attackprimary = (kbutton_t*)0x012F70E0;
	movedown = (kbutton_t*)0x012F6FC0;
	moveup = (kbutton_t*)0x012F6FA0;

	if(attackprimary->active)
	{
		if(cgi.anim->lastVMAnim != VMA_IDLE)
			goto _skip_attackprimary;

		if(attackprimary_up)
			goto _skip_attackprimary;

		attackprimary_up = true;

		cg->predictedPlayerState.iViewModelAnim = VMA_FIRE;
		cg->predictedPlayerState.iViewModelAnimChanged = ( cg->predictedPlayerState.iViewModelAnimChanged + 1 ) & 3;

		cgi.anim->lastVMAnim = VMA_FIRE;
		cgi.anim->lastVMAnimChanged = (cgi.anim->lastVMAnimChanged + 1) & 3;
	}
	else
		attackprimary_up = false;

_skip_attackprimary:

	if(movedown->active)
	{
		if(movedown_up)
			goto _skip_movedown;

		movedown_up = true;

		if( viewHeight == 48 ) // crouching
			viewHeight = 82;
		else if( viewHeight == 82 ) // standing
			viewHeight = 48;
	}
	else
		movedown_up = false;

_skip_movedown:

	if(moveup->active)
	{
		if(moveup_up)
			goto _skip_moveup;

		moveup_up = true;

		if( viewHeight == 48 ) // crouching
			viewHeight = 82; // stand

		else if(viewHeight == 82 && cg->predictedPlayerState.velocity[2] <= 10 && cg->predictedPlayerState.velocity[2] >= -5)
		{
			int i;

			if(cg->snap != NULL)
			{
				cg->snap->ps.velocity[2] += (float)sqrt( 2.0f * 800 * 56 );
				cg->snap->ps.walking = false; // make sure the player leaves the ground
			}

			if(cg->nextSnap != NULL)
			{
				cg->nextSnap->ps.velocity[2] += (float)sqrt( 2.0f * 800 * 56 );
				cg->nextSnap->ps.walking = false;
			}

			for(i=0;i<2;i++)
			{
				cg->activeSnapshots[i].ps.velocity[2] += (float)sqrt( 2.0f * 800 * 56 );
				cg->activeSnapshots[i].ps.walking = false;
			}

			cg->predictedPlayerState.velocity[2] += (float)sqrt( 2.0f * 800 * 56 );
			cg->predictedPlayerState.walking = false;
		}
	}
	else
		moveup_up = false;

_skip_moveup:
	return;
}

#if 0
void __stdcall CG_CalcViewBob( float value )
{
	float multiplier;
	float speed_scale;
	trace_t results;
	float val = value;

	//if(value > 250.0f)

	if( value > 10.0f ) {
		value = 235.0f;
	}

	multiplier = value * 0.001500000013038516f + 0.8999999761581421f;

	speed_scale = ( float )( cg->frametime * bobMovementMultiplier );

	*bobValue -= ( speed_scale * 0.001000000047497451f + speed_scale * 0.001000000047497451f )
				* M_PI
				* multiplier;

	viewbob += ( speed_scale * 0.001000000047497451f + speed_scale * 0.001000000047497451f )
				* M_PI
				* multiplier;

	//if(*bobValue < -4)
	//	*bobValue = 0.0f;

	//*bobValue = (float)((int)*bobValue % 4);

	//time_step += speed_scale * 0.001000000047497451f;

	if( viewbob > 3.1f ) // time_step > 0.37f )
	{
		str string;
		vec3_t mins;
		vec3_t maxs;
		vec3_t start;
		vec3_t end;
		float *height = (float*)0x303EBC6C;
		const char * snd_name;
		int flags;

		//time_step = 0.0f;
		viewbob = 0;

		if( val < 10.0f || cent == NULL )
			return;

		memcpy( start, (const void *)0x303EBBD0, sizeof(vec3_t) );
		memcpy( end, (const void *)0x303EBBD0, sizeof(vec3_t) );

		//start[2] += *height;
		end[2] -= 1000;

		//VectorSet( mins, -6.0f, -6.0f, -6.0f );
		//VectorSet( maxs, 6.0f, 6.0f, 6.0f );

		memcpy( mins, ( const void * )0x30077554, sizeof( vec3_t ) );
		memcpy( maxs, ( const void * )0x30077560, sizeof( vec3_t ) );

		CG_Trace( &results, start, mins, maxs, end, cg->activeSnapshots[0].ps.clientNum, MASK_VISIBLE, 0, 1, "CG_CalcViewBob" );

		flags = results.surfaceFlags;

		if( flags & SURF_PAPER )
			snd_name = "snd_step_paper";
		else if( flags & SURF_WOOD )
			snd_name = "snd_step_wood";
		else if( flags & SURF_METAL)
			snd_name = "snd_step_metal";
		else if( flags & SURF_STONE)
			snd_name = "snd_step_stone";
		else if( flags & SURF_DIRT )
			snd_name = "snd_step_dirt";
		else if( flags & SURF_METALGRILL )
			snd_name = "snd_step_grill";
		else if( flags & SURF_GRASS )
			snd_name = "snd_step_grass";
		else if( flags & SURF_MUD )
			snd_name = "snd_step_mud";
		else if( flags & SURF_PUDDLE )
			snd_name = "snd_step_puddle";
		else if( flags & SURF_GLASS )
			snd_name = "snd_step_glass";
		else if( flags & SURF_GRAVEL )
			snd_name = "snd_step_gravel";
		else if( flags & SURF_SAND )
			snd_name = "snd_step_sand";
		else if( flags & SURF_FOLIAGE )
			snd_name = "snd_step_foliage";
		else if( flags & SURF_SNOW )
			snd_name = "snd_step_snow";
		else if( flags & SURF_CARPET )
			snd_name = "snd_step_carpet";
		else
			snd_name = "snd_step_stone";

		if( results.contents & CONTENTS_WATER )
			snd_name = "snd_step_water";

		string = snd_name;

		CG_PlaySound( ( char * )string.data, cent->currentState.origin, 5, -1.0, -1.0, -1.0, 1);
	}
}
#endif

uintptr_t CG_OffsetFirstPersonView_f = 0;
uintptr_t CG_OffsetThirdPersonView_f = 0;
uintptr_t CG_ModelAnim_f = 0;

void CG_OffsetFirstPersonViewReal( refEntity_t *model, int a2 )
{
	JMPTO( CG_OffsetFirstPersonView_f );
}

void CG_OffsetThirdPersonViewReal()
{
	JMPTO( CG_OffsetThirdPersonView_f );
}

void CG_ModelAnimReal( centity_t *ent, int un1 )
{
	JMPTO( CG_ModelAnim_f );
}

void CG_ProcessDeltaAngles( qboolean isDead )
{
	vec3_t angles;

	if( isDead ) {
		MatrixToEulerAngles( cg->refdef.viewAxis, angles );
	} else {
		VectorCopy( cg->refdefViewAngles, angles );
	}

	// Process delta angles (shock, etc)
	for( int i = 0; i < MAX_DELTAANGLES; i++ )
	{
		vec3_t angles2;

		VectorCopy( player.deltaAnglesOld[ i ], angles2 );

		/*if( isDead )
		{
			angles2[ 0 ] *= 0.035f;
			angles2[ 1 ] *= 0.035f;
			angles2[ 2 ] *= 0.035f;
		}*/

		VectorSubtract( angles, angles2, angles );
		VectorCopy( player.deltaAngles[ i ], player.deltaAnglesOld[ i ] );
	}

	if( isDead ) {
		AnglesToAxis( angles, cg->refdef.viewAxis );
	} else {
		VectorCopy( angles, cg->refdefViewAngles );
	}
}

void CG_OffsetFirstPersonView( refEntity_t *model, int a2 )
{
	int old_frametime = cg->frametime;
	float speed_scale;
	trace_t results;
	vec3_t modelAngles;
	float sv_crouchspeedmult;
	int sv_runspeed;

	CG_ProcessDeltaAngles( false );

	player.DebugCamera();

	if( cg_debugview->integer ) {
		return;// CG_OffsetFirstPersonViewReal( model, a2 );
	}

	model->renderfx &= ~RF_INVISIBLE;
	model->renderfx &= ~RF_DONTDRAW;

	CG_MoveWeapon( cg->frametime );
	//CG_MatchEyesBone( model );

	float speed = sqrt( cg->predictedPlayerState.velocity[0] * cg->predictedPlayerState.velocity[0]
						+ cg->predictedPlayerState.velocity[1] * cg->predictedPlayerState.velocity[1]
						+ cg->predictedPlayerState.velocity[2] * cg->predictedPlayerState.velocity[2] );

	if( reborn )
	{
		sv_crouchspeedmult = atof( CG_GetServerVariable( "sv_crouchspeedmult" ) );
		sv_runspeed = atoi( CG_GetServerVariable( "sv_runspeed" ) );
	}
	else
	{
		sv_crouchspeedmult = 1.0f;
		sv_runspeed = ( int )speed;
	}

	if( cg->predictedPlayerState.viewheight == 48 ) {
		sv_runspeed = ( int )( ( float )sv_runspeed * sv_crouchspeedmult );
	}

	if( sv_runspeed <= 1 )
		sv_runspeed = 1;

	bobMovementMultiplier = speed / sv_runspeed;

	if( speed > 15.0f ) {
		speed = 235.0f;
	}

	cg->predictedPlayerState.viewheight = ( int )( ( float )cg->predictedPlayerState.viewheight * viewHeight_mult );

	if( cg->predictedPlayerState.walking )
	{
		float multiplier = cg->frametime * 0.001000000047497451 * bobMovementMultiplier;

		cg->currentViewBobPhase += ( multiplier + multiplier )
									* M_PI
									* ( speed * 0.001500000013038516 + 0.8999999761581421 );

		viewbob += ( multiplier + multiplier )
					* M_PI
					* ( speed * 0.001500000013038516 + 0.8999999761581421 );


		cg->predictedPlayerState.velocity[0] *= bobMovementMultiplier;
		cg->predictedPlayerState.velocity[1] *= bobMovementMultiplier;
		cg->predictedPlayerState.velocity[2] *= bobMovementMultiplier;

		cg->currentViewBobAmp = ( 1.0 - fabs( cg->refdefViewAngles[ 0 ] ) * 0.01111111138015985 * 0.5 ) * 0.5 * 0.75;
	}

	CG_OffsetFirstPersonViewReal( model, a2 );

	if( viewbob > M_PI )
	{
		vec3_t mins;
		vec3_t maxs;
		Vector start;
		Vector end;
		float *height = (float*)0x303EBC6C;
		const char * snd_name;
		int flags;

		viewbob = 0;

		if( speed < 10.0f || cent == NULL || !cl_run->integer ) {
			return;
		}

		//memcpy( start, (const void *)0x303EBBD0, sizeof(vec3_t) );
		//memcpy( end, (const void *)0x303EBBD0, sizeof(vec3_t) );

		start = cg->predictedPlayerState.origin;
		start.z += cg->predictedPlayerState.viewheight;

		end = cg->predictedPlayerState.origin;
		end.z -= 1000;

		//VectorSet( mins, -6.0f, -6.0f, -6.0f );
		//VectorSet( maxs, 6.0f, 6.0f, 6.0f );

		memcpy( mins, ( const void * )0x30077554, sizeof( vec3_t ) );
		memcpy( maxs, ( const void * )0x30077560, sizeof( vec3_t ) );

		CG_Trace( &results, start, mins, maxs, end, model->entityNumber, MASK_BEAM, 0, 1, "CG_CalcViewBob" );

		flags = results.surfaceFlags;

		if( flags & SURF_PAPER )
			snd_name = "snd_step_paper";
		else if( flags & SURF_WOOD )
			snd_name = "snd_step_wood";
		else if( flags & SURF_METAL)
			snd_name = "snd_step_metal";
		else if( flags & SURF_ROCK )
			snd_name = "snd_step_stone";
		else if( flags & SURF_DIRT )
			snd_name = "snd_step_dirt";
		else if( flags & SURF_GRILL )
			snd_name = "snd_step_grill";
		else if( flags & SURF_GRASS )
			snd_name = "snd_step_grass";
		else if( flags & SURF_MUD )
			snd_name = "snd_step_mud";
		else if( flags & SURF_PUDDLE )
			snd_name = "snd_step_puddle";
		else if( flags & SURF_GLASS )
			snd_name = "snd_step_glass";
		else if( flags & SURF_GRAVEL )
			snd_name = "snd_step_gravel";
		else if( flags & SURF_SAND )
			snd_name = "snd_step_sand";
		else if( flags & SURF_FOLIAGE )
			snd_name = "snd_step_foliage";
		else if( flags & SURF_SNOW )
			snd_name = "snd_step_snow";
		else if( flags & SURF_CARPET )
			snd_name = "snd_step_carpet";
		else
			snd_name = "snd_step_stone";

		if( results.contents & CONTENTS_WATER )
			snd_name = "snd_step_water";

		commandManager->PlaySound( snd_name, cent->currentState.origin, 5, -1.0, -1.0, -1.0, 1 );
	}
}

void CG_OffsetThirdPersonView()
{
	CG_ProcessDeltaAngles( false );

	player.DebugCamera();

	if( cg_debugview->integer ) {
		return;
	}

	CG_OffsetThirdPersonViewReal();
}

void CG_ModelAnim( centity_t *ent, int un1 )
{
	qboolean isDead = ( ent->currentState.eFlags & EF_DEAD );

	if( isDead )
	{
		CG_ProcessDeltaAngles( isDead );
		player.DebugCamera();
	}

	CG_ModelAnimReal( ent, un1 );
}

/**********************************************
 *
 * CG_MoveWeapon
 * Make the arms always animated
 *
 **********************************************/

void CG_MoveWeapon(int frametime)
{
	vec3_t offsetAngles;
	vec3_t ratio;
	float limit = 32.0f;

	CG_FakePredict();

	if( fNoMove ) {
		return;
	}

	//if( cg->predictedPlayerState.pm_type == PM_NOCLIP ) {
	//	cgi.anim->currentVMPosOffset[2] = vm_offset_air_up->value;
	//}

	internal_bobOffset = ( int )( frametime * bobMovementMultiplier );

	//if(fLanding)
	//	goto _falling;

	/* FIXME: This can be improved */

	VectorSubtract( cgi.anim->currentVMPosOffset, currentDeltaVMPosOffset2, cgi.anim->currentVMPosOffset );
	VectorAdd( cgi.anim->currentVMPosOffset, currentDeltaVMPosOffset, cgi.anim->currentVMPosOffset );

	VectorCopy( currentDeltaVMPosOffset, currentDeltaVMPosOffset2 );

	VectorSet( currentDeltaVMPosOffset, 0, 0, 0 );

	/* Get the delta angles before getting the old view angles */
	deltaAngles[0] = ( cg->predictedPlayerState.viewangles[0] - oldAngles[0] );
	deltaAngles[1] = ( cg->predictedPlayerState.viewangles[1] - oldAngles[1] );
	deltaAngles[2] = ( cg->predictedPlayerState.viewangles[2] - oldAngles[2] );

	/* Set the old angles */
	oldAngles[0] = cg->predictedPlayerState.viewangles[0];
	oldAngles[1] = cg->predictedPlayerState.viewangles[1];
	oldAngles[2] = cg->predictedPlayerState.viewangles[2];

	/*if( deltaAngles[0] > limit ) {
		deltaAngles[0] = limit;
	} else if ( deltaAngles[0] < -limit ) {
		deltaAngles[0] = -limit;
	}

	if( deltaAngles[1] > limit ) {
		deltaAngles[1] = limit;
	} else if ( deltaAngles[1] < -limit ) {
		deltaAngles[1] = -limit;
	}

	if( deltaAngles[2] > limit ) {
		deltaAngles[2] = limit;
	} else if ( deltaAngles[2] < -limit ) {
		deltaAngles[2] = -limit;
	}*/

	ratio[0] = abs( deltaAngles[0]/limit );
	ratio[1] = abs( deltaAngles[1]/limit );
	ratio[2] = abs( deltaAngles[2]/limit );

	/*deltaAngles[0] /= limit;
	deltaAngles[1] /= limit;
	deltaAngles[2] /= limit;*/

	/* Set the new angles for the viewmodel */
	newAngles[0] = -( deltaAngles[0] / 12 );
	newAngles[1] = -( deltaAngles[1] / 24 );
	newAngles[2] = -( deltaAngles[2] / 48 );

	/* Get the future angles */
	offsetAngles[0] = currentVMAnglesOffset[0] + newAngles[0];
	offsetAngles[1] = currentVMAnglesOffset[1] + newAngles[1];
	offsetAngles[2] = currentVMAnglesOffset[2] + newAngles[2];

	/* Set the viewmodel offset with a limit to prevent it
	 * from getting ugly
	 */
	if( offsetAngles[0] < 3.0 && offsetAngles[0] > -3.0 ) {
		cgi.anim->currentVMPosOffset[0] += newAngles[0] / 2;
	}

	if( offsetAngles[1] < 2.0 && offsetAngles[1] > -2.3 ) {
		cgi.anim->currentVMPosOffset[1] += newAngles[1] / 2;
	}

	if( offsetAngles[2] < 2.0 && offsetAngles[2] > -2.0 ) {
		cgi.anim->currentVMPosOffset[2] += newAngles[2] / 2;
	}

	/* If we're idling, animate the viewmodel by
	 * moving it
	 */
	if((cg->predictedPlayerState.velocity[0] == 0 &&
		cg->predictedPlayerState.velocity[1] == 0 &&
		cg->predictedPlayerState.velocity[2] == 0) ||
		cg->predictedPlayerState.pm_type == PM_NOCLIP)
	{
		if(deltaPosition[1] > 0.0005f)
			deltaPosition[1] = 0.0005f;
		else if(deltaPosition[1] < -0.0005f)
			deltaPosition[1] = -0.0005f;

		if(deltaPosition[2] > 0.0002f)
			deltaPosition[2] = 0.0002f;
		else if(deltaPosition[2] < -0.0002f)
			deltaPosition[2] = -0.0002f;

		if(cgi.anim->currentVMPosOffset[2] < 0.3 && !move_godown_z)
		{
			if(cgi.anim->currentVMPosOffset[2] <= 0.15)
			{
				deltaPosition[1] -= 0.0000004f * frametime;
				deltaPosition[2] += 0.0000001f * frametime;
			}
			else
			{
				deltaPosition[1] += 0.0000004f * frametime;
				deltaPosition[2] -= 0.00000015f * frametime;
			}
			cgi.anim->currentVMPosOffset[1] += deltaPosition[1] * frametime;
			cgi.anim->currentVMPosOffset[2] += deltaPosition[2] * frametime;
		}
		else
		{
			move_godown_z = true;

			if(cgi.anim->currentVMPosOffset[2] > -0.3)
			{
				if(cgi.anim->currentVMPosOffset[2] >= -0.15)
				{
					deltaPosition[1] += 0.0000004f * frametime;
					deltaPosition[2] -= 0.0000001f * frametime;
				}
				else
				{
					deltaPosition[1] -= 0.0000004f * frametime;
					deltaPosition[2] += 0.00000015f * frametime;
				}
				cgi.anim->currentVMPosOffset[1] -= deltaPosition[1] * frametime;
				cgi.anim->currentVMPosOffset[2] -= -deltaPosition[2] * frametime; //0.0001f * frametime;
			}
			else
				move_godown_z = false;
		}
	}

//_falling:
	if( !cg->fpsOnGround && !fLanding && !fFell ) // &&
		//cg->predictedPlayerState.pm_type == PM_NORMAL)
	{
		fLanding = true;
	}
	else if( ( cg->fpsOnGround || fFell ) && fLanding ) // && cg->predictedPlayerState.pm_type == PM_NORMAL )
	{
		if(landDownValue == 0)
		{
			if(previousVelocity[2] > -100)
			{
				landHeight = viewHeight;
				goto _landCleanup;
			}
			else if(previousVelocity[2] <= -230 && previousVelocity[2] > -570)
			{
				landDownValue = -3.00f;
				landDownDelayScale = 1.0f;
				landDownViewScale = 0.16f;
			}
			else if(previousVelocity[2] <= -570 && previousVelocity[2] > -800)
			{
				landDownValue = -4.25f;
				landDownDelayScale = 1.25f;
				landDownViewScale = 0.25f;
			}
			else if(previousVelocity[2] <= -800)
			{
				landDownValue = -6.5f;
				landDownDelayScale = 2.0f;
				landDownViewScale = 0.33f;
			}
		}

		if(!fLandingUp)
		{
			if(landHeight == 0)
				landHeight = viewHeight;

			// The weapon/view is going down

			fFell = true;
			//cg->refdef.viewOrg[2] -= 0.009f * frametime;
			/*viewHeight -= (int)((landDownViewScale + 0.02) * (float)frametime);

			if(viewHeight < 5)
				viewHeight = 5;*/

			viewHeight_mult -= ( ( landDownViewScale + 0.02f ) * 0.005f ) * frametime;

			//CG_SetViewHeight( viewHeight * viewHeight_mult );
			cgi.anim->currentVMPosOffset[2] -= 0.016f * frametime * landDownDelayScale;

			if(cgi.anim->currentVMPosOffset[2] < landDownValue)
				fLandingUp = true;
		}
		else
		{
			//cg->refdef.viewOrg[2] += 0.009f * frametime;
			/*viewHeight += (int)(landDownViewScale * (float)frametime);

			if(viewHeight > 82)
				viewHeight = 82;*/

			viewHeight_mult += ( landDownViewScale * 0.005f ) * frametime;

			//CG_SetViewHeight( viewHeight * viewHeight_mult );
			cgi.anim->currentVMPosOffset[2] += 0.011f * frametime * landDownDelayScale;

			if(cgi.anim->currentVMPosOffset[2] > 0.0f)
				goto _landCleanup;
		}
	}

	previousVelocity[0] = cg->predictedPlayerState.velocity[0];
	previousVelocity[1] = cg->predictedPlayerState.velocity[1];
	previousVelocity[2] = cg->predictedPlayerState.velocity[2];

	return;

_landCleanup:
	fLanding = false;
	fLandingUp = false;
	fFell = false;
	cgi.anim->currentVMPosOffset[2] = 0.0f;
	//viewHeight = landHeight;
	viewHeight_mult = 1.0f;
	landHeight = 0;
	landDownValue = 0.0f;
	landDownDelayScale = 0.0f;
	landDownViewScale = 0.0f;
}

/**********************************************
*
* CG_SetVMASpeed
* Sets the speed of a view model animation
*
**********************************************/

void CG_SetVMASpeed( const char *name, float speed )
{
	vma_t *vma = CG_GetVMA( name );

	vma->speed = speed;
}

/**********************************************
 *
 * CG_HeightCheck
 *
 **********************************************/
void CG_HeightCheck()
{
	pmove_t *pm = ( pmove_t * )*( uintptr_t * )0x30596AE4;

	pm->ps->speed = ( int )( ( float )cg->activeSnapshots[ 0 ].ps.speed * player.m_fMoveSpeedScale );

	/*pm->ps->velocity[ 0 ] *= player.m_fMoveSpeedScale;
	pm->ps->velocity[ 1 ] *= player.m_fMoveSpeedScale;
	pm->ps->velocity[ 2 ] *= player.m_fMoveSpeedScale;*/

	pm->mins[0] = -15.0f;
	pm->mins[1] = -15.0f;

	pm->maxs[0] = 15.0f;
	pm->maxs[1] = 15.0f;

	pm->mins[2] = 0.0f;

	if ( pm->ps->pm_type == PM_DEAD )
	{
		pm->maxs[2] = 32.0f;
		pm->ps->viewheight = 48;

		max_z = 32.0f;
		viewHeight = 48;
	}
	else if ( ( pm->ps->pm_flags & 3 ) == 3 )
	{
		pm->maxs[2] = 54.0f;
		pm->ps->viewheight = 48;

		max_z = 54.0f;
		viewHeight = 48;
	}
	else if ( pm->ps->pm_flags & 1 )
	{
		pm->maxs[2] = 60.0f;
		pm->ps->viewheight = 48;

		max_z = 60.0f;
		viewHeight = 48;
	}
	else if ( pm->ps->pm_flags & 2 )
	{
		pm->maxs[2] = 20.0f;
		pm->ps->viewheight = 16;

		max_z = 20.0f;
		viewHeight = 16;
	}
	else if ( pm->ps->pm_flags & 0x800 )
	{
		pm->maxs[2] = 94.0f;
		pm->mins[2] = 54.0f;
		pm->ps->viewheight = 82;

		max_z = 94.0f;
		viewHeight = 82;
	}
	else if ( pm->ps->pm_flags & 0x1000 )
	{
		pm->maxs[2] = 94.0f;
		pm->ps->viewheight = 52;

		max_z = 94.0f;
		viewHeight = 52;
	}
	else
	{
		pm->maxs[2] = 94.0f;
		pm->ps->viewheight = 82;

		/*pm->maxs[2] = max_z;
		pm->ps->viewheight = viewHeight;

		CG_SetViewHeight( viewHeight );*/
	}

	/*if( fLanding )
	{
		CG_SetViewHeight( pm->ps->viewheight * viewHeight_mult );
		return;
	}*/
}

void CG_SetViewHeight(int newHeight)
{
	int i;

	pmove_t * pm = ( pmove_t * )*( uintptr_t * )0x30596AE4;

	if(cg->snap != NULL)
		cg->snap->ps.viewheight = newHeight;

	if(cg->nextSnap != NULL)
		cg->nextSnap->ps.viewheight = newHeight;

	for(i=0;i<2;i++)
		cg->activeSnapshots[i].ps.viewheight = newHeight;

	cg->predictedPlayerState.viewheight = newHeight;
	//viewHeight = newHeight;
	pm->ps->viewheight = newHeight;
}

const char *CG_GetCurrentWeaponName(void)
{
	return CG_ConfigString(cg->activeSnapshots->ps.activeItems[1] + 1748);
}

const char *CG_GetCurrentWeaponPrefix(void)
{
	int index = VM_GetVMAnimPrefixString( ( char * )CG_GetCurrentWeaponName() );

	return itemPrefix[index];
}

vma_t *CG_GetVMA( const char *name )
{
	vma_t *vma;

	for( int i = 0; i < vmalist.NumObjects(); i++ )
	{
		vma = vmalist[ i ];

		if( vma->name.icmp( name ) == 0 ) {
			return vma;
		}
	}

	vma = new vma_t;

	vma->name = name;
	vma->speed = 1.0f;

	vmalist.AddObject( vma );

	return vma;
}

/**********************************************
 *
 * CG_GetVMAnimPrefixString
 *
 **********************************************/

/*int CG_GetVMAnimPrefixString(char * weapon_name)
{
	int i;

	if(weapon_name == NULL)
		return 18; // unarmed

	for(i=1;i<255;i++)
	{
		if(Q_strcmpi(itemName[i], weapon_name) == 0)
			return i;
	}

	return 18;
}*/

/**********************************************
 *
 * CG_GetViewModelAnimName
 *
 **********************************************/

void CG_GetViewModelAnimName(char * Dest, char * Format, char * prefix, char * anim_name)
{
	if(!vm_fullanim)
		sprintf(Dest, Format, prefix, anim_name);
	else
		strcpy(Dest, anim_name);
}

/**********************************************
 *
 * CG_AddViewModelPrefix
 *
 **********************************************/

void CG_AddViewModelPrefix( const char * weapon_name, const char * prefix )
{
	int i;
	char *weapon_name_allocated;
	char *weapon_prefix_allocated;

	for( i = 1; i < 255; i++ )
	{
		/* Don't add this more than once... */
		if( itemPrefix[ i ] != NULL && strcmp( itemPrefix[ i ], prefix ) == 0 && strcmp( itemName[ i ], weapon_name ) == 0 )
			return;
	}

	int weapon_name_length = strnlen( weapon_name, 255 );
	int weapon_prefix_length = strnlen( prefix, 255 );

	/* Allocate the name so that it will be always present in the database */
	weapon_name_allocated = new char[ weapon_name_length + 1 ]; //( char * )malloc( weapon_name_length );
	weapon_prefix_allocated = new char[ weapon_prefix_length + 1 ]; //( char * )malloc( weapon_name_prefix );

	strncpy( weapon_name_allocated, weapon_name, weapon_name_length );
	strncpy( weapon_prefix_allocated, prefix, weapon_prefix_length );

	for( i = 1; i < 255; i++ )
	{
		if( itemName[ i ] == NULL )
		{
			itemName[ i ] = ( char * )weapon_name_allocated;
			itemPrefix[ i ] = ( char * )weapon_prefix_allocated;
			break;
		}
	}
}

int CG_ViewModelAnimNum(char *name, qboolean bFullAnim)
{
	int num;
	char anim_name[255];

	if(!bFullAnim)
		sprintf(anim_name, "%s_%s", CG_GetCurrentWeaponPrefix(), name);

	num = cgi.Anim_NumForName(cg->playerFPSModel, bFullAnim ? name : anim_name);

	if(num == -1)
		cgi.Printf("Couldn't find view model animation %s\n", bFullAnim ? name : anim_name);

	return num;
}

void CG_ViewModelAnim(char *name, qboolean bFullAnim)
{
	/*if(CG_ViewModelAnimNum(name, bFullAnim) == -1)
	{
		int i;

		if(_strcmpi(name, vm_anim_name) == 0)
		{
			currentviewModelAnim = VMA_IDLE;
			currentviewModelAnimChanged = (currentviewModelAnimChanged + 1) & 3;

			strcpy(vm_anim_name, "idle");

			vm_fullanim = false;
		}

		if(cg->snap != NULL)
		{
			cg->snap->ps.iViewModelAnim = currentviewModelAnim;
			cg->snap->ps.iViewModelAnimChanged = currentviewModelAnimChanged;
		}

		if(cg->nextSnap != NULL)
		{
			cg->nextSnap->ps.iViewModelAnim = currentviewModelAnim;
			cg->nextSnap->ps.iViewModelAnimChanged = currentviewModelAnimChanged;
		}

		for(i=0;i<MAX_ACTIVESNAPSHOTS;i++)
		{
			cg->activeSnapshots[i].ps.iViewModelAnim = currentviewModelAnim;
			cg->activeSnapshots[i].ps.iViewModelAnimChanged = currentviewModelAnimChanged;
		}

		cg->predictedPlayerState.iViewModelAnim = currentviewModelAnim;
		cg->predictedPlayerState.iViewModelAnimChanged = currentviewModelAnimChanged;

		cgi.anim->lastVMAnim = currentviewModelAnim;
		cgi.anim->lastVMAnimChanged = currentviewModelAnimChanged;

		return;
	}*/

	currentviewModelAnim = cg->predictedPlayerState.iViewModelAnim;
	currentviewModelAnimChanged = cg->predictedPlayerState.iViewModelAnimChanged;

	strncpy(vm_anim_name, name, sizeof(vm_anim_name));

	if(bFullAnim)
		vm_fullanim = true;
	else
		vm_fullanim = false;
}

/**********************************************
 *
 * CG_ViewModelUninitialize
 *
 **********************************************/

void CG_ViewModelUninitialize(void)
{
	int i;

	/* Destroy the name to free some mem */
	for( i = 19; i < 255; i++ )
	{
		if( itemName[ i ] != NULL )
		{
			delete itemName[ i ];
			itemName[ i ] = NULL;
		}

		if( itemPrefix[ i ] != NULL )
		{
			delete itemPrefix[ i ];
			itemPrefix[ i ] = NULL;
		}
	}
}

/**********************************************
 *
 * CG_ViewModelAnimCheck
 *
 **********************************************/

void CG_ViewModelAnimCheck( clientAnim_t * newAnim )
{
	dtiki_t * tiki;
	const char * item;
	int index = 0, i;
	int num;

	qboolean bAnimCheck = false;
	qboolean bNewWeapon = false;

	char anim_name[ MAX_STRING_TOKENS ] = { 'i', 'd', 'l', 'e' };
	const char * lastVMAnim;
	int slot = cgi.anim->currentVMAnimSlot;

	float weight = 0.0f, crossblend = 0.0f, time = 0.0f;

	if( slot < 0 ) {
		slot = 0;
	}

	tiki = ( dtiki_t * )newAnim->tiki;

	item = CG_ConfigString(cg->snap->ps.activeItems[1] + 1748);

	if( cgi.anim->lastEquippedWeaponStat != cg->snap->ps.stats[ STAT_EQUIPPED_WEAPON ]
		|| ( item != NULL && strcmp( cgi.anim->lastActiveItem, item ) != 0 ) )
	{
		index = VM_GetVMAnimPrefixString( ( char * )item );
		cgi.anim->lastEquippedWeaponStat = cg->snap->ps.stats[ STAT_EQUIPPED_WEAPON ];

		strcpy(cgi.anim->lastActiveItem, item);
		cgi.anim->lastAnimPrefixIndex = index;

		bAnimCheck = true;
		bNewWeapon = true;
	} else {
		index = cgi.anim->lastAnimPrefixIndex;
	}

	if ( cgi.anim->lastVMAnim == -1 )
	{
		sprintf( anim_name, "%s_idle", itemPrefix[index] );

		num = cgi.Anim_Random( tiki, anim_name );

		if ( num == -1 )
		{
			num = cgi.Anim_Random( tiki, "idle" );

			if( num == -1 ) {
				num = 0;
			}

			cgi.Printf("Warning: #1 Couldn't find view model animation %s\n", anim_name);
		}

		cgi.anim->vmFrameInfo[slot].index = num;

		cgi.anim->vmFrameInfo[slot].time = 0.0f;
		cgi.anim->vmFrameInfo[slot].weight = 1.0f;
		cgi.anim->lastVMAnim = 0;
	}

	if ( cg->snap->ps.iViewModelAnimChanged != cgi.anim->lastVMAnimChanged )
	{
		bAnimCheck = true;

		cgi.anim->lastVMAnim = cg->snap->ps.iViewModelAnim;
		cgi.anim->lastVMAnimChanged = cg->snap->ps.iViewModelAnimChanged;
	}

	/*if(CG_ViewModelAnimNum(vm_anim_name, vm_fullanim) == -1)
	{
		cgi.anim->lastVMAnim = VMA_IDLE;
		cgi.anim->lastVMAnimChanged = (cgi.anim->lastVMAnimChanged + 1) & 3;
	}*/

	if( !bAnimCheck ) {
		goto _skip_anim_check;
	}

	switch ( cgi.anim->lastVMAnim )
	{
		case VMA_IDLE:
			lastVMAnim = "idle";
			break;

		case VMA_CHARGE:
			lastVMAnim = "charge";
			break;

		case VMA_FIRE:
			lastVMAnim = "fire";
			break;

		case VMA_FIRE_SECONDARY:
			lastVMAnim = "fire_secondary";
			break;

		case VMA_RECHAMBER:
			lastVMAnim = "rechamber";
			break;

		case VMA_RELOAD:
			lastVMAnim = "reload";
			break;

		case VMA_RELOAD_SINGLE:
			lastVMAnim = "reload_single";
			break;

		case VMA_RELOAD_END:
			lastVMAnim = "reload_end";
			break;

		case VMA_PULLOUT:
			lastVMAnim = "pullout";
			break;

		case VMA_PUTAWAY:
			lastVMAnim = "putaway";
			break;

		case VMA_LADDERSTEP:
			lastVMAnim = "ladderstep";
			break;

		case VMA_ENABLE:
			lastVMAnim = "enable";
			break;

		default:
			lastVMAnim = vm_anim_name;
	}

	if( !vm_fullanim ) {
		sprintf( anim_name, "%s_%s", itemPrefix[ index ], lastVMAnim );
	} else {
		strcpy( anim_name, lastVMAnim );
	}

	current_vma = CG_GetVMA( lastVMAnim );

	crossblend = cgi.Anim_CrossblendTime( tiki, cgi.anim->vmFrameInfo[slot].index );
	time = cgi.anim->currentVMDuration * 0.001000000047497451f;

	if( time < crossblend )
	{
		weight = time / crossblend;

		for ( i = 0; i < MAX_FRAMEINFOS; i++ )
		{
			float w;

			w = cgi.anim->vmFrameInfo[i].weight;

			if( w == 0.0f )
				continue;

			if( i == slot )
				cgi.anim->vmFrameInfo[i].weight = weight;
			else
			{
				w = ( 1.0f - weight ) * cgi.anim->vmFrameInfo[i].weight;
				cgi.anim->vmFrameInfo[i].weight = w;
			}
		}
	}

	slot = ( slot + 1 ) % 16;
	cgi.anim->currentVMAnimSlot = slot;
	num = cgi.Anim_Random( tiki, anim_name );

	if ( num < 0 )
	{
		cgi.Printf( "Warning: #2 Couldn't find view model animation %s\n", anim_name );

		if( !bNewWeapon ) {
			goto _skip_anim_check;
		}
		else
		{
			if(!vm_fullanim) {
				sprintf( anim_name, "%s_idle", itemPrefix[index], lastVMAnim );
			} else {
				strcpy( anim_name, "idle" );
			}

			num = cgi.Anim_Random( tiki, anim_name );

			if( num < 0 ) {
				num = 0;
			}
		}
	}

	cgi.anim->vmFrameInfo[slot].index = num;

	cgi.anim->vmFrameInfo[slot].time = 0.0f;
	cgi.anim->vmFrameInfo[slot].weight = 1.0f;
	cgi.anim->currentVMDuration = 0;

	crossblend = cgi.Anim_CrossblendTime( tiki, cgi.anim->vmFrameInfo[slot].index );

	if ( crossblend == 0.0f )
	{
		for( i = 0; i < MAX_FRAMEINFOS; i++ )
		{
			if ( i != slot )
				cgi.anim->vmFrameInfo[i].weight = 0.0f;
		}

		cgi.anim->crossBlending = 0;
	}
	else
		cgi.anim->crossBlending = 1;

_skip_anim_check:

	cgi.anim->currentVMDuration += cg->frametime;

	if( cgi.anim->crossBlending )
	{
		crossblend = cgi.Anim_CrossblendTime( tiki, cgi.anim->vmFrameInfo[slot].index );
		time = cgi.anim->currentVMDuration * 0.001000000047497451f;

		if( time >= crossblend )
		{
			for( i = 0; i < MAX_FRAMEINFOS; i++ )
			{
				if ( slot != i )
					cgi.anim->vmFrameInfo[ i ].weight = 0.0f;
			}

			cgi.anim->crossBlending = 0;
		}
		else
			weight = time / crossblend;
	}

	for( i = 0; i < MAX_FRAMEINFOS; i++ )
	{
		float w, frametime;

		w = cgi.anim->vmFrameInfo[ i ].weight;

		if ( w == 0.0f )
		{
			newAnim->vmFrameInfo[ i + 7 ].index = 0;
			newAnim->vmFrameInfo[ i + 7 ].time = 0.0f;
			newAnim->vmFrameInfo[ i + 7 ].weight = 0.0f;

			continue;
		}

		frametime = cgi.Anim_Frametime( tiki, cgi.anim->vmFrameInfo[ i ].index );

		float speed;

		if( current_vma != NULL ) {
			speed = current_vma->speed;
		} else {
			speed = 1.0f;
		}

		cgi.anim->vmFrameInfo[ i ].time += ( ( float )cg->frametime * speed ) / 1000.0f; // *0.001000000047497451f;

		/*if( cgi.Anim_FlagsSkel( tiki, cgi.anim->vmFrameInfo[ i ].index ) & 0x20 ) {
			cgi.anim->vmFrameInfo[ i ].time -= frametime;
		} else {
			cgi.anim->vmFrameInfo[ i ].time = frametime;
		}*/

		newAnim->vmFrameInfo[ i + 7 ].index = cgi.anim->vmFrameInfo[ i ].index;
		newAnim->vmFrameInfo[ i + 7 ].time = cgi.anim->vmFrameInfo[ i ].time;

		if ( cgi.anim->crossBlending )
		{
			if ( slot == i )
				newAnim->vmFrameInfo[i + 7].weight = weight;
			else
				newAnim->vmFrameInfo[i + 7].weight = (1.0f - weight) * cgi.anim->vmFrameInfo[i].weight;
		}
		else
			newAnim->vmFrameInfo[i + 7].weight = 1.0f;
	}

	// Loop if needed
	if( cgi.anim->vmFrameInfo[ slot ].time > cgi.Anim_Time( tiki, cgi.anim->vmFrameInfo[slot].index ) &&
		cgi.anim->vmFrameInfo[ slot ].index == 0 &&
		cgi.Anim_Flags( tiki, cgi.anim->vmFrameInfo[ slot ].index ) & 0x20 )
	{
		cgi.anim->lastVMAnimChanged = -1;
	}

	*(uintptr_t *)&newAnim->lastActiveItem[60] = 0x3F800000u;
}
