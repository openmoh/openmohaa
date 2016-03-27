#include "glb_local.h"
#include "script/clientgamecommand.h"
#include "cg_hud.h"
#include "cg_viewmodelanim.h"

#include "script/cplayer.h"
#include "scriptvariable.h"
#include "scriptmaster.h"

#define MAX_IMPACTS 64

// wall impacts
static vec3_t *wall_impact_pos = ( vec3_t * )0x302659E8;
static vec3_t *wall_impact_norm = ( vec3_t * )0x30265CE8;
static int *wall_impact_large = ( int * )0x302655E8;
static int *wall_impact_type = ( int * )0x30266568;
static int *wall_impact_count = ( int * )0x30269A74;

// flesh impacts
static vec3_t *flesh_impact_pos = ( vec3_t * )0x302656E8;
static vec3_t *flesh_impact_norm = ( vec3_t * )0x30266768;
static int *flesh_impact_large = ( int * )0x30266668;
static int *flesh_impact_count = ( int * )0x30269A78;

static int *current_entity_number = ( int * )0x3007C68C;

typedef void ( *CG_MakeBulletTracer_f )( float *i_vBarrel, float *i_vStart,
		float (*i_vEnd)[3], int i_iNumBullets, qboolean iLarge,
		int iTracerVisible, qboolean bIgnoreEntities );
CG_MakeBulletTracer_f CG_MakeBulletTracer = ( CG_MakeBulletTracer_f )0x300313F6;

typedef void ( *CG_MakeBubbleTrail_f )( float *i_vStart, float *i_vEnd, int iLarge );
CG_MakeBubbleTrail_f CG_MakeBubbleTrail = ( CG_MakeBubbleTrail_f )0x3002E233;

typedef void ( *CG_MeleeImpact_f )( float *vStart, float *vEnd );
CG_MeleeImpact_f CG_MeleeImpact = ( CG_MeleeImpact_f )0x30035AD4;

typedef void ( *CG_MakeExplosionEffect_f )( float *vPos, int iType );
CG_MakeExplosionEffect_f CG_MakeExplosionEffect = ( CG_MakeExplosionEffect_f )0x3002FD7B;

typedef void ( *CG_SpawnEffectModel_f )( const char *model, vec3_t origin, vec3_t axis[3] );
CG_SpawnEffectModel_f CG_SpawnEffectModel = ( CG_SpawnEffectModel_f )0x302A52E8;

int ReadNumber()
{
	if( reborn ) {
		return cgi.MSG_ReadShort();
	} else {
		return cgi.MSG_ReadByte();
	}
}

void CG_ParseCGMessage2() {
	int msgtype;
	vec3_t vecStart, vecTmp, vecEnd, vecArray[64];
	vec3_t axis[3];
	int iCount, iLarge;
	int i, iTemp;
	const char *s;
	qboolean loop;
	hdElement_t *hde;

	int index;
	const char *name;
	Hud *hud;
	float col_x, col_y, col_z;
	const char *weapname, *prefix;
	const char *hint;
	qboolean bFullAnim;
	float alpha;
	float time, fade_at_time;
	float rate;
	qboolean time_up;
	short width, height;
	float speed;
	float volume;

	Vector vector_or_offset;
	short ent_num;
	qboolean always_show;
	qboolean has_depth;

	do {
		msgtype = cgi.MSG_ReadBits( 6 );

		//if(cg_debugCGMessages.integer) {
		//	Com_Printf( "CG_ParseCGMessage: command type %i\n", msgtype );
		//}
		switch ( msgtype ) {
			case 1: // BulletTracer (visible?)
				vecTmp[0] = cgi.MSG_ReadCoord();
				vecTmp[1] = cgi.MSG_ReadCoord();
				vecTmp[2] = cgi.MSG_ReadCoord();
			case 2: // BulletTracer (invisible?)
			case 5: // BubbleTrail
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				if ( msgtype != 1 ) {
					vecTmp[0] = vecStart[0];
					vecTmp[1] = vecStart[1];
					vecTmp[2] = vecStart[2];
				}

				vecArray[0][0] = cgi.MSG_ReadCoord();
				vecArray[0][1] = cgi.MSG_ReadCoord();
				vecArray[0][2] = cgi.MSG_ReadCoord();

				iLarge = cgi.MSG_ReadBits( 1 );

				if( msgtype==1 ) {
					CG_MakeBulletTracer( vecTmp, vecStart, vecArray, 1, iLarge, 1, 1 );
				} else if( msgtype==2 ) {
					CG_MakeBulletTracer( vecTmp, vecStart, vecArray, 1, iLarge, 0, 1 );
				} else {
					CG_MakeBubbleTrail( vecStart, vecArray[0], iLarge );
				}
				break;
			case 3: // BulletTracer multiple times
				vecTmp[0] = cgi.MSG_ReadCoord();
				vecTmp[1] = cgi.MSG_ReadCoord();
				vecTmp[2] = cgi.MSG_ReadCoord();
				iTemp = cgi.MSG_ReadBits( 6 );
			case 4: // BulletTracer multiple times (shotgun shot)
				if ( msgtype == 4 ) {
					iTemp = 0;
				}

				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				iLarge = cgi.MSG_ReadBits( 1 );
				iCount = cgi.MSG_ReadBits( 6 );

				// this check is missing in MOHAA code, so this has buffer overflow risk in AA
				if ( iCount > 64 ) {
					cgi.Error( ERR_DROP, "CG message type 4 sent too many data.\n" );
				}

				for (i=0;i<iCount;i++) {
					vecArray[i][0] = cgi.MSG_ReadCoord();
					vecArray[i][1] = cgi.MSG_ReadCoord();
					vecArray[i][2] = cgi.MSG_ReadCoord();
				}
				CG_MakeBulletTracer( vecTmp, vecStart, vecArray, iCount, iLarge, iTemp, 1 );
				break;
			case 6: // wall impact
			case 7: // flesh impact
			case 8: // flesh impact (?)
			case 9: // wall impact (?)
			case 10: // wall impact (?)
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				cgi.MSG_ReadDir( vecEnd );
				iLarge = cgi.MSG_ReadBits( 1 );

				switch ( msgtype ) {
					case 6: // a window was hit
						if( *wall_impact_count < MAX_IMPACTS ) {
							VectorCopy( vecStart, wall_impact_pos[*wall_impact_count] );
							VectorCopy( vecEnd, wall_impact_norm[*wall_impact_count] );
							wall_impact_large[*wall_impact_count] = iLarge;
							( *wall_impact_count )++;
						}
						break;
					case 7:
						if( *flesh_impact_count < MAX_IMPACTS ) {
							// inverse hit normal, I dont know why,
							// but that's done by MoHAA
							VectorInverse( vecEnd );
							VectorCopy( vecStart,flesh_impact_pos[*flesh_impact_count] );
							VectorCopy( vecEnd,flesh_impact_norm[*flesh_impact_count] );
							flesh_impact_large[*flesh_impact_count] = iLarge;
							( *flesh_impact_count )++;
						}
						break;
					case 8:
						if( *flesh_impact_count < MAX_IMPACTS ) {
							// same here?
							VectorInverse( vecEnd );
							VectorCopy( vecStart, flesh_impact_pos[*flesh_impact_count] );
							VectorCopy( vecEnd, flesh_impact_norm[*flesh_impact_count] );
							flesh_impact_large[*flesh_impact_count] = iLarge;
							( *flesh_impact_count )++;
						}
						break;
					case 9:
						if( *wall_impact_count < MAX_IMPACTS ) {
							VectorCopy( vecStart, wall_impact_pos[*wall_impact_count] );
							VectorCopy( vecEnd, wall_impact_norm[*wall_impact_count] );
							wall_impact_large[*wall_impact_count] = iLarge;
							if(iLarge) {
								wall_impact_type[*wall_impact_count] = 3;
							} else {
								wall_impact_type[*wall_impact_count] = 2;
							}
							( *wall_impact_count )++;
						}
						break;
					case 10:
						if( *wall_impact_count < MAX_IMPACTS ) {
							VectorCopy( vecStart, wall_impact_pos[*wall_impact_count] );
							VectorCopy( vecEnd, wall_impact_norm[*wall_impact_count] );
							wall_impact_large[*wall_impact_count] = iLarge;
							if(iLarge) {
								wall_impact_type[*wall_impact_count] = 5;
							} else {
								wall_impact_type[*wall_impact_count] = 4;
							}
							( *wall_impact_count )++;
						}
						break;
					default:
						break;
				}
				break;
			case 11:
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();
				vecEnd[0] = cgi.MSG_ReadCoord();
				vecEnd[1] = cgi.MSG_ReadCoord();
				vecEnd[2] = cgi.MSG_ReadCoord();
				CG_MeleeImpact(vecStart,vecEnd);
				break;
			case 12: // m1 frag/stiel grenade explosion
			case 13: // bazooka/panzershrek projectile explosion
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();
				CG_MakeExplosionEffect(vecStart,msgtype);
				break;
			default: //unknown message
				cgi.Error( ERR_DROP, "CG_ParseCGMessage: Unknown CG Message %i", msgtype );
				break;
			case 14:
			case 15: // MakeEffect
			case 16:
			case 17:
			case 18:
			case 19: // oil barrel effect
			case 20:
			case 21: // oil barrel effect top
			case 22: // oil barrel effect top - first hit
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();
				cgi.MSG_ReadDir( vecEnd );
				sfxManager->MakeEffect_Normal( msgtype + 67, vecStart, vecEnd );
				break;
			case 23: // broke crate
			case 24: // broke glass window
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				i = cgi.MSG_ReadByte();

				if( msgtype == 23 ) {
					s = va( "models/fx/crates/debris_%i.tik", i );
				} else {
					s = va( "models/fx/windows/debris_%i.tik", i );
				}

				VectorSet( axis[0], 0.f, 0.f, 1.f );
				VectorSet( axis[1], 0.f, 1.f, 0.f );
				VectorSet( axis[2], 1.f, 0.f, 0.f );

				cgi.R_SpawnEffectModel( s, vecStart, axis );
				break;
			case 25: // Bullet tracer
				vecTmp[0] = cgi.MSG_ReadCoord();
				vecTmp[1] = cgi.MSG_ReadCoord();
				vecTmp[2] = cgi.MSG_ReadCoord();
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				vecArray[0][0] = cgi.MSG_ReadCoord();
				vecArray[0][1] = cgi.MSG_ReadCoord();
				vecArray[0][2] = cgi.MSG_ReadCoord();

				iLarge = cgi.MSG_ReadBits( 1 );

				CG_MakeBulletTracer(vecTmp,vecStart,vecArray,1,iLarge,0,1);
				break;
			case 26: // Bullet tracer
				vecTmp[0] = 0;
				vecTmp[1] = 0;
				vecTmp[2] = 0;
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				vecArray[0][0] = cgi.MSG_ReadCoord();
				vecArray[0][1] = cgi.MSG_ReadCoord();
				vecArray[0][2] = cgi.MSG_ReadCoord();

				iLarge = cgi.MSG_ReadBits( 1 );
				CG_MakeBulletTracer(vecTmp,vecStart,vecArray,1,iLarge,0,1);
				break;
			case CGM_HUDDRAW_SHADER:
				index = ReadNumber();

				name = cgi.MSG_ReadString();

				hud = Hud::FindOrCreate( index );
				hud->SetShader( name, -1 , -1 );

				break;

			case CGM_HUDDRAW_ALIGN:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );

				hud->SetAlignX( ( hudAlign_t )cgi.MSG_ReadBits( 2 ) );
				hud->SetAlignY( ( hudAlign_t )cgi.MSG_ReadBits( 2 ) );

				break;

			case CGM_HUDDRAW_RECT:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );

				hud->SetRectX( cgi.MSG_ReadShort() );
				hud->SetRectY( cgi.MSG_ReadShort() );
				hud->SetRectWidth( cgi.MSG_ReadShort() );
				hud->SetRectHeight( cgi.MSG_ReadShort() );

				break;

			case CGM_HUDDRAW_VIRTUALSIZE:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );

				hud->SetVirtualSize( cgi.MSG_ReadBits( 1 ) );

				break;

			case CGM_HUDDRAW_COLOR:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );

				col_x = cgi.MSG_ReadByte() / 255.0f;
				col_y = cgi.MSG_ReadByte() / 255.0f;
				col_z = cgi.MSG_ReadByte() / 255.0f;

				hud->SetColor( Vector( col_x, col_y, col_z ) );

				break;

			case CGM_HUDDRAW_ALPHA:
				index = ReadNumber();

				alpha = cgi.MSG_ReadByte() / 255.0f;

				hud = Hud::FindOrCreate( index );
				hud->SetAlpha( alpha );

				break;

			case CGM_HUDDRAW_STRING:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );

				hud->SetText( cgi.MSG_ReadString() );

				break;

			case CGM_HUDDRAW_FONT:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );

				hud->SetFont( cgi.MSG_ReadString() );

				break;
			case 35:
			case 36:

				*current_entity_number = cg->snap->ps.clientNum;

				if( msgtype == 36 )
				{
					s = "dm_kill_notify";
				}
				else
				{
					s = "dm_hit_notify";
				}

				commandManager->PlaySound( ( const char * )s,
							Vector( 0, 0, 0 ),
							5,
							2.0,
							-1.0,
							-1.0,
							1 );
				//CG_PlaySound( s, 0, 5, 2.0, -1.0, vecStart, 1 );
				break;
			case 37: // voicechat message (squad command, taunt, etc...)
				vecStart[0] = cgi.MSG_ReadCoord();
				vecStart[1] = cgi.MSG_ReadCoord();
				vecStart[2] = cgi.MSG_ReadCoord();

				iTemp = cgi.MSG_ReadBits( 1 );
				// read client index
				i = cgi.MSG_ReadBits( 6 );
				// read voicechat sound alias
				s = cgi.MSG_ReadString();

				if( iTemp ) {
					*current_entity_number = i;

				}
				//CG_Printf("Case 37: iTemp %i, i %i, s %s\n",iTemp,i,s);
				// play an aliased sound from uberdialog.scr
				// wombat: only if there is a sound to be played
				if ( *s ) {
					commandManager->PlaySound( s, vecStart, 5, -1.0, -1.0, -1.0, 0 );
				}

				break;

			case CGM_VIEWMODELANIM:
				name = cgi.MSG_ReadStringLine();
				bFullAnim = cgi.MSG_ReadByte();

				CG_ViewModelAnim(( char * )name, bFullAnim );
				break;

			case CGM_VIEWMODELPREFIX:
				weapname = cgi.MSG_ReadStringLine();
				prefix = cgi.MSG_ReadString();

				VM_AddViewModelPrefix( weapname, prefix );
				break;

			case CGM_MODHEIGHTFLOAT:
				modheightfloat_used = true;
				viewHeight = cgi.MSG_ReadLong();
				max_z = cgi.MSG_ReadFloat();
				break;

			case CGM_SETVMASPEED:
				name = cgi.MSG_ReadStringLine();
				speed = cgi.MSG_ReadFloat();

				CG_SetVMASpeed( name, speed );
				break;

			case CGM_HUDDRAW_TIMER:
				index = ReadNumber();

				time = cgi.MSG_ReadFloat();
				fade_at_time = cgi.MSG_ReadFloat();
				time_up = cgi.MSG_ReadBits( 1 );

				hud = Hud::FindOrCreate( index );
				if( !time_up ) {
					hud->SetTimer( time, fade_at_time );
				} else {
					hud->SetTimerUp( time, fade_at_time );
				}

				break;

			case CGM_HUDDRAW_3D:
				index = ReadNumber();

				vector_or_offset[ 0 ] = cgi.MSG_ReadCoord();
				vector_or_offset[ 1 ] = cgi.MSG_ReadCoord();
				vector_or_offset[ 2 ] = cgi.MSG_ReadCoord();

				ent_num = cgi.MSG_ReadShort();
				always_show = cgi.MSG_ReadBits( 1 );
				has_depth = cgi.MSG_ReadBits( 1 );

				hud = Hud::FindOrCreate( index );
				hud->Set3D( vector_or_offset, always_show, has_depth, ent_num );

				break;

			case CGM_HUDDRAW_BREAK3D:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );
				hud->SetNon3D();

				break;

			case CGM_HUDDRAW_FADE:
				index = ReadNumber();

				time = cgi.MSG_ReadFloat();

				hud = Hud::FindOrCreate( index );
				hud->FadeOverTime( time );

				break;

			case CGM_HUDDRAW_MOVE:
				index = ReadNumber();

				time = cgi.MSG_ReadFloat();

				hud = Hud::FindOrCreate( index );
				hud->MoveOverTime( time );

				break;

			case CGM_HUDDRAW_SCALE:
				index = ReadNumber();

				time = cgi.MSG_ReadFloat();
				width = cgi.MSG_ReadShort();
				height = cgi.MSG_ReadShort();

				hud = Hud::FindOrCreate( index );
				hud->ScaleOverTime( time, width, height );

				break;

			case CGM_HINTSTRING: // approaching an entity which have an hint string
				hint = cgi.MSG_ReadStringLine();

				CG_ShowHint( hint );
				break;

			case CGM_PLAYLOCALSOUND:
				s = cgi.MSG_ReadStringLine();
				loop = cgi.MSG_ReadBits( 1 );
				time = cgi.MSG_ReadFloat();
				volume = cgi.MSG_ReadFloat();

				commandManager->PlayLocalSound( s, loop, volume, time );
				break;

			case CGM_STOPLOCALSOUND:
				s = cgi.MSG_ReadStringLine();
				time = cgi.MSG_ReadFloat();

				commandManager->StopLocalSound( s, time );
				break;

			case CGM_SETLOCALSOUNDRATE:
				s = cgi.MSG_ReadStringLine();
				rate = cgi.MSG_ReadFloat();
				time = cgi.MSG_ReadFloat();

				commandManager->SetLocalSoundRate( s, rate, time );
				break;

			case CGM_HUDDRAW_RECTX:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );
				hud->SetRectX( cgi.MSG_ReadShort() );
				break;

			case CGM_HUDDRAW_RECTY:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );
				hud->SetRectY( cgi.MSG_ReadShort() );
				break;

			case CGM_HUDDRAW_RECTWH:
				index = ReadNumber();

				hud = Hud::FindOrCreate( index );
				hud->SetRectWidth( cgi.MSG_ReadShort() );
				hud->SetRectHeight( cgi.MSG_ReadShort() );
				break;

			case CGM_REPLICATION:
				index = cgi.MSG_ReadBits( 4 );
				name = cgi.MSG_ReadString();

				{
					ScriptVariable var;

					var.SetKey( Director.AddString( name ) );

					switch( index )
					{
					case VARIABLE_CHAR:
						var.setCharValue( cgi.MSG_ReadChar() );
						player.replication.ReplicateVariable( var );
						break;

					case VARIABLE_CONSTSTRING:
					case VARIABLE_STRING:
						var.setStringValue( cgi.MSG_ReadString() );
						player.replication.ReplicateVariable( var );
						break;

					case VARIABLE_FLOAT:
						var.setFloatValue( cgi.MSG_ReadFloat() );
						player.replication.ReplicateVariable( var );
						break;

					case VARIABLE_INTEGER:
						var.setIntValue( cgi.MSG_ReadLong() );
						player.replication.ReplicateVariable( var );
						break;

					case VARIABLE_VECTOR:
						vecTmp[ 0 ] = cgi.MSG_ReadCoord();
						vecTmp[ 1 ] = cgi.MSG_ReadCoord();
						vecTmp[ 2 ] = cgi.MSG_ReadCoord();

						var.setVectorValue( ( Vector )vecTmp );
						player.replication.ReplicateVariable( var );
						break;
					}
				}
				break;

			case CGM_HUDDRAW_DELETE:
				hud = Hud::Find( cgi.MSG_ReadShort() );

				if( hud )
				{
					delete hud;
				}
				break;
		}
	} while( cgi.MSG_ReadBits( 1 ) );
}

/*void __stdcall CG_ParseCGMessage( int msgtype )
{
	int index;
	float duration, fade_out_time;
	hdElement_t *hde;
	hdExtend_t *hdex;
	const char *weap_name;
	const char *prefix;
	const char *name;
	qboolean bFullAnim;
	vec3_t vector;
	int ent_num;
	qboolean bAlwaysShow, bDepth;
	const char * hintstring;

	float col_x, col_y, col_z;

	Hud *hud;

	switch(msgtype)
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
			break;

		case CGM_HUDDRAW_SHADER:
			index = cgi.MSG_ReadByte();
			name = cgi.MSG_ReadString();

			hud = Hud::FindOrCreate( index );
			hud->SetShader( name, -1 , -1 );

			break;

		case CGM_HUDDRAW_ALIGN:
			index = cgi.MSG_ReadByte();

			hud = Hud::FindOrCreate( index );

			hud->SetAlignX( ( hudAlign_t )cgi.MSG_ReadBits( 2 ) );
			hud->SetAlignY( ( hudAlign_t )cgi.MSG_ReadBits( 2 ) );

			break;

		case CGM_HUDDRAW_RECT:
			index = cgi.MSG_ReadByte();

			hud = Hud::FindOrCreate( index );

			hud->SetRectX( cgi.MSG_ReadShort() );
			hud->SetRectY( cgi.MSG_ReadShort() );
			hud->SetRectHeight( cgi.MSG_ReadShort() );
			hud->SetRectWidth( cgi.MSG_ReadShort() );

			break;

		case CGM_HUDDRAW_VIRTUALSIZE:
			index = cgi.MSG_ReadByte();

			hud = Hud::FindOrCreate( index );

			hud->SetVirtualSize( cgi.MSG_ReadBits( 1 ) );

			break;

		case CGM_HUDDRAW_COLOR:
			index = cgi.MSG_ReadByte();

			hud = Hud::FindOrCreate( index );

			col_x = cgi.MSG_ReadByte() / 255.0f;
			col_y = cgi.MSG_ReadByte() / 255.0f;
			col_z = cgi.MSG_ReadByte() / 255.0f;

			hud->SetColor( Vector( col_x, col_y, col_z ) );

			break;

		case CGM_HUDDRAW_ALPHA:
			index = cgi.MSG_ReadByte();

			hud = Hud::FindOrCreate( index );

			hud->SetAlpha( cgi.MSG_ReadByte() / 255.0f );

			break;

		case CGM_HUDDRAW_STRING:
			index = cgi.MSG_ReadByte();

			hud = Hud::FindOrCreate( index );

			hud->SetText( cgi.MSG_ReadString() );

			break;

		case CGM_HUDDRAW_FONT:
			index = cgi.MSG_ReadByte();

			hud = Hud::FindOrCreate( index );

			hud->SetFont( cgi.MSG_ReadString() );

			break;

		case 35:
		case 36:
		case 37:
			break;

		case CGM_HUDDRAW_TIMER: // huddraw_timer
			index = cgi.MSG_ReadByte();

			duration = cgi.MSG_ReadFloat();
			fade_out_time = cgi.MSG_ReadFloat();

			hde = cgi.hudDrawElements + index;
			hdex = huddrawExtends + index;

			hde->shader = 0;
			memset(hde->shaderName, 0, sizeof(hde->shaderName));

			if(duration <= 0.0f)
				memset(&hdex->timer, 0, sizeof(hdTimer_t));
			else
			{
				hdex->timer.bEnabled = true;
				hdex->timer.duration = duration;
				hdex->timer.current = duration;
				hdex->timer.fade_out_time = fade_out_time * 1000.0f;
				hdex->timer.alpha = hde->color[3];

				strcpy(hde->string, hdex->timer.string);
			}
			break;

		case CGM_VIEWMODELANIM: // viewmodelanim
			name = cgi.MSG_ReadStringLine();
			bFullAnim = cgi.MSG_ReadByte();

			CG_ViewModelAnim((char*)name, bFullAnim);
			break;

		case CGM_VIEWMODELPREFIX: // viewmodelprefix
			weap_name = cgi.MSG_ReadStringLine();
			prefix = cgi.MSG_ReadString();

			VM_AddViewModelPrefix( weap_name, prefix );
			break;

		case CGM_MODHEIGHTFLOAT: // modheightfloat
			modheightfloat_used = true;
			viewHeight = cgi.MSG_ReadLong();
			max_z = cgi.MSG_ReadFloat();
			break;

		case CGM_SETBOBSPEED: // setviewbobspeed
			bobMovementMultiplier = cgi.MSG_ReadFloat();
			break;

		case CGM_HUDDRAW_3D: // huddraw_3d
			index = cgi.MSG_ReadByte();

			vector[0] = cgi.MSG_ReadCoord();
			vector[1] = cgi.MSG_ReadCoord();
			vector[2] = cgi.MSG_ReadCoord();

			ent_num = cgi.MSG_ReadShort();

			bAlwaysShow = cgi.MSG_ReadByte();
			bDepth = cgi.MSG_ReadByte();

			hde = cgi.hudDrawElements + index;
			hdex = huddrawExtends + index;

			hdex->h3d.bEnabled = true;
			VectorCopy( vector, hdex->h3d.vector );
			VectorCopy( vector, hdex->h3d.currentOrg );
			hdex->h3d.ent_num = ent_num;

			hdex->h3d.bAlwaysShow = bAlwaysShow;
			hdex->h3d.bDepth = bDepth;

			hde->x = hdex->h3d.x;
			hde->y = hdex->h3d.y;

			break;

		case CGM_HINTSTRING: // approaching an entity which have an hint string
			hintstring = cgi.MSG_ReadStringLine();

			CG_ShowHint( hintstring );
			break;

		case CGM_HUDDRAW_FADE:
		case CGM_HUDDRAW_MOVE:
		case CGM_HUDDRAW_SCALE:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
			break;

		default:
			cgi.Error( ERR_DROP, "CG_ParseCGMessage: Unknown CGM message type" );
	}
}*/
