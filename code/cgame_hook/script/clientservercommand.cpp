/*
* ClientServerCommandManager.cpp
*
* DESCRIPTION : Client Server Command Manager. Processes server commands as event (if one).
*/

#include "clientservercommand.h"
#include "earthquake.h"
#include "cgame/cl_sound.h"

ClientServerCommandManager svcommandManager;

Event EV_ServerCommand_Earthquake
(
	"eq",
	EV_SERVERCMD,
	"ffbbVF",
	"duration magnitude no_rampup no_rampdown location radius",
	"Earthquake",
	EV_NORMAL
);

Event EV_ServerCommand_FadeSound
(
	"fadesound2",
	EV_SERVERCMD,
	"ff",
	"time min_vol",
	"Fade the sound",
	EV_NORMAL
);

Event EV_ServerCommand_RestoreSound
(
	"restoresound",
	EV_SERVERCMD,
	"ff",
	"time max_vol",
	"Restore the sound",
	EV_NORMAL
);

Event EV_ServerCommand_SetClientFlag
(
	"cf",
	EV_SERVERCMD,
	"s",
	"name",
	"Client call back flag.",
	EV_NORMAL
);

Event EV_ServerCommand_SetShader
(
	"setshader",
	EV_SERVERCMD,
	"isb",
	"ent_num shader fReset",
	"Set entity shader",
	EV_NORMAL
);

Event EV_ServerCommand_VisionSetBlur
(
	"vsb",
	EV_SERVERCMD,
	"ff",
	"blur_level transition_time",
	"Specific server command for the naked blur vision",
	EV_NORMAL
);

Event EV_ServerCommand_VisionSetNaked
(
	"vsn",
	EV_SERVERCMD,
	"sff",
	"vision transition_time phase",
	"Specific server command for the naked vision",
	EV_NORMAL
);

void ClientServerCommandManager::Archive( Archiver &arc )
{

}

void ClientServerCommandManager::ExecuteCommand( char *arguments[], int num_arguments )
{
	const char *command = arguments[ 0 ];
	const char *formatspec;
	int k;

	char **args = arguments + 1;

	Event *ev = new Event( command, EV_NORMAL );

	if( ev->eventnum == 0 )
	{
		delete ev;
		return;
	}

	formatspec = ev->GetFormat();

	for( int i = 0, k = 0; k < num_arguments - 1; k++ )
	{
		const char *argument = args[ i ];
		str vec;

		if( formatspec[ k ] == '\0' ) {
			break;
		}

		if( argument == NULL || *argument == '\0' ) {
			continue;
		}

		switch( formatspec[ k ] )
		{
		case 'b':
		case 'B':
			ev->AddInteger( atoi( argument ) );
			i++;
			break;

		case 's':
		case 'S':
			ev->AddString( argument );
			i++;
			break;

		case 'i':
		case 'I':
			ev->AddInteger( atoi( argument ) );
			i++;
			break;

		case 'f':
		case 'F':
			ev->AddFloat( ( float )atof( argument ) );
			i++;
			break;

		case 'v':
		case 'V':
			vec = args[ i ];
			vec += str( " " ) + args[ i + 1 ];
			vec += str( " " ) + args[ i + 2 ];

			ev->AddVector( Vector( vec.c_str() ) );
			i += 3;
			break;

		default:
			ev->AddString( argument );
			i++;
		}
	}

	try
	{
		ProcessEvent( ev );
	}
	catch( ScriptException exc )
	{
		cgi.Printf( exc.string );
	}
}


////==== EVENTS ====////

void ClientServerCommandManager::EventEarthquake( Event *ev )
{
	float duration = ev->GetFloat( 1 );
	float magnitude = ev->GetFloat( 2 );
	qboolean no_rampup = ev->GetBoolean( 3 );
	qboolean no_rampdown = ev->GetBoolean( 4 );
	Vector org;
	float radius;

	if( ev->NumArgs() > 4 )
	{
		org = ev->GetVector( 5 );

		if( ev->NumArgs() > 5 ) {
			radius = ev->GetFloat( 6 );
		}
	}

	Earthquake * earthquake = new Earthquake;

	earthquake->SetDuration( duration );
	earthquake->SetMagnitude( magnitude );
	earthquake->SetNoRampup( no_rampup );
	earthquake->SetNoRampdown( no_rampdown );

	if( ev->NumArgs() > 4 )
	{
		earthquake->SetLocation( org );

		if( ev->NumArgs() > 5 ) {
			earthquake->SetRadius( radius );
		}
	}

	earthquake->Enable();
}

void ClientServerCommandManager::EventFadeSound( Event *ev )
{
	float time = ev->GetFloat( 1 );
	float min_vol = ev->GetFloat( 2 );

	CL_FadeSound2( time, min_vol );
}

void ClientServerCommandManager::EventRestoreSound( Event *ev )
{
	float time = ev->GetFloat( 1 );
	float max_vol = ev->GetFloat( 2 );

	CL_RestoreSound( time, max_vol );
}

void ClientServerCommandManager::EventSetClientFlag( Event *ev )
{
	/*str name = ev->GetString( 1 );
	ClientScript *clientScript;

	clientScript = new ClientScript;

	clientScript->ExecuteCallback( name );

	if( clientScript->ScriptFailed() ) {
		delete clientScript;
	}*/
}

void ClientServerCommandManager::EventSetShader( Event *ev )
{
	int ent_num = ev->GetInteger( 1 );
	str shadername = ev->GetString( 2 );
	qboolean fReset = ev->GetBoolean( 3 );

	R_SetShader( ent_num, shadername, fReset );
}

void ClientServerCommandManager::EventVisionSetBlur( Event *ev )
{
	float blur_level = ev->GetFloat( 1 );
	float transition_time;

	if( ev->NumArgs() > 1 ) {
		transition_time = ev->GetFloat( 2 );
	} else {
		transition_time = 0.0f;
	}

	R_ProcessBlur( blur_level, transition_time );
}

void ClientServerCommandManager::EventVisionSetNaked( Event *ev )
{
	str vision_name = ev->GetString( 1 );
	float transition_time;
	float phase;

	if( ev->NumArgs() > 1 ) {
		transition_time = ev->GetFloat( 2 );
	} else {
		transition_time = 0.0f;
	}

	if( ev->NumArgs() > 2 ) {
		phase = ev->GetFloat( 3 );
	}
	else {
		phase = 0.0f;
	}

	R_ProcessVision( vision_name, transition_time, phase );
}

CLASS_DECLARATION( Listener, ClientServerCommandManager, NULL )
{
	{ &EV_ServerCommand_Earthquake,					&ClientServerCommandManager::EventEarthquake },
	{ &EV_ServerCommand_FadeSound,					&ClientServerCommandManager::EventFadeSound },
	{ &EV_ServerCommand_RestoreSound,				&ClientServerCommandManager::EventRestoreSound },
	{ &EV_ServerCommand_SetClientFlag,				&ClientServerCommandManager::EventSetClientFlag },
	{ &EV_ServerCommand_SetShader,					&ClientServerCommandManager::EventSetShader },
	{ &EV_ServerCommand_VisionSetBlur,				&ClientServerCommandManager::EventVisionSetBlur },
	{ &EV_ServerCommand_VisionSetNaked,				&ClientServerCommandManager::EventVisionSetNaked },
	{ NULL, NULL }
};
