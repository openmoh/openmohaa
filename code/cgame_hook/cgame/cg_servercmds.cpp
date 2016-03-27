#include "glb_local.h"
#include "cg_viewmodelanim.h"
#include "cl_sound.h"
#include "script/earthquake.h"
#include "script/clientservercommand.h"

void CG_AddViewModelPrefix(const char * weapon_name, const char * prefix);

void CG_ServerCommand()
{
	/*const char *command = cgi.Argv( 0 );
	char *param1 = cgi.Argv( 1 );
	char *param2 = cgi.Argv( 2 );
	char *param3 = cgi.Argv( 3 );
	char *param4 = cgi.Argv( 4 );
	char *param5 = cgi.Argv( 5 );
	char *param6 = cgi.Argv( 6 );
	char *param7 = cgi.Argv( 7 );
	char *param8 = cgi.Argv( 8 );
	char *param9 = cgi.Argv( 9 );
	char *param10 = cgi.Argv( 10 );*/

	char *buffer[ MAX_STRING_CHARS ];
	int arguments = cgi.Argc();

	for( int i = 0; i < arguments; i++ ) {
		buffer[ i ] = cgi.Argv( i );
	}

	svcommandManager.ExecuteCommand( buffer, arguments );

	/*if(strcmp(command, "viewmodelanim") == 0)
	{
		vm_anim_name = param1;

		if(param2)
			vm_fullanim = atoi(param2);
		else
			vm_fullanim = 0;
	}
	else if(strcmp(command, "viewmodelprefix") == 0)
		CG_AddViewModelPrefix(param1, param2);
	else if(strcmp(command, "modheightfloat") == 0)
	{
		__try
		{
			modheightfloat_used = qtrue;
			viewHeight = atoi(param1);
			max_z = (float)atof(param2);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
	else if(strcmp(command, "setviewbobspeed") == 0)
	{
		__try
		{
			bobMovementMultiplier = atof(param1);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}*/

	/*if(strcmp(command, "visioneffect") == 0)
	{
		char * name = param1;
		float fadetime, intensity, red, green, blue;
		float parameters[2];
		vec3_t colors;
		qboolean effectAdd;

		effectAdd = atoi(param2);
		fadetime = (float)atof(param3);
		intensity = (float)atof(param4);
		red = (float)atof(param5);
		green = (float)atof(param6);
		blue = (float)atof(param7);
		parameters[0] = (float)atof(param8);
		parameters[1] = (float)atof(param9);

		colors[0] = red;
		colors[1] = green;
		colors[2] = blue;

		R_PostProcessEffect(name, intensity, colors, fadetime, effectAdd, parameters);
	}
	else if(strcmp(command, "fadesound2") == 0)
	{
		float time;
		float min_vol;

		time = (float)atof(param1);
		min_vol = (float)atof(param2);

		CL_FadeSound2(time, min_vol);
	}
	else if(strcmp(command, "restoresound") == 0)
	{
		float time;
		float max_vol;

		time = (float)atof(param1);
		max_vol = (float)atof(param2);

		CL_RestoreSound(time, max_vol);
	}
	else if( strcmp( command, "eq" ) == 0 )
	{
		float duration;
		float magnitude;
		qboolean no_rampup;
		qboolean no_rampdown;
		str location;
		float radius;

		duration = ( float )atof( param1 );
		magnitude = ( float )atof( param2 );
		no_rampup = atoi( param3 );
		no_rampdown = atoi( param4 );

		if( param5 && param6 && param7 )
		{
			location = param5;
			location += str( " " ) + param6;
			location += str( " " ) + param7;

			radius = ( float )atof( param8 );
		}

		Earthquake * earthquake = new Earthquake;

		earthquake->SetDuration( duration );
		earthquake->SetMagnitude( magnitude );
		earthquake->SetNoRampup( no_rampup );
		earthquake->SetNoRampdown( no_rampdown );

		if( location )
		{
			earthquake->SetLocation( Vector( location.c_str() ) );
			earthquake->SetRadius( radius );
		}

		earthquake->Enable();
	} else if ( strcmp( command, "vsn" ) == 0 ) {
		R_ProcessVision( param1, atof( param2 ) );
	}
	else if( strcmp( command, "setshader" ) == 0 )
	{
		int ent_num = -1;
		const char * shadername = NULL;
		qboolean fReset = false;

		ent_num = atoi( param1 );
		shadername = param2;
		fReset = atoi( param3 );

		R_SetShader( ent_num, shadername, fReset );

	} else {
		cgi.Printf( "Unknown client game command: %s\n", command );
	}*/
}
