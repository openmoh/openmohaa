/*
* CAnimate.cpp
*
* DESCRIPTION : Client animate. Enable entities animations
*/

#include "canimate.h"
#include "clientgamecommand.h"

Event EV_Animate_Anim
(
	"anim",
	EV_DEFAULT,
	"s",
	"animName",
	"Exec anim commands on client.",
	EV_NORMAL
);

Animate::Animate()
{

}

Animate::~Animate()
{

}

void Animate::Archive( Archiver &arc )
{
	Entity::Archive( arc );
}

void Animate::Think()
{
	int num_playing = 0;

	if( !tiki ) {
		return Entity::Think();
	}

	for( int i = 0; i < GetNumFrames(); i++ )
	{
		float max_time = cgi.Anim_Time( tiki, frame[ i ].index );

		if( frame[ i ].time < max_time )
		{
			frame[ i ].time += cg->frametime;
			num_playing++;
		}
		else
		{
			frame[ i ].time = max_time;
		}
	}

	// Optimize performances
	if( !num_playing && this->classinfo() == Animate::classinfostatic() ) {
		turnThinkOff();
	}

	Entity::Think();
}

void Animate::PlayAnim( const char *name, int index )
{
	int num = cgi.Anim_NumForName( tiki, name );

	if( num != -1 ) {
		PlayAnim( num, index );
	}
}

void Animate::PlayAnim( int num, int index )
{
	float frametime;

	if( index >= MAX_FRAMEINFOS ) {
		return;
	}

	if( index >= numframes )
	{
		numframes++;
		frame = ( frameInfo_t * )realloc( frame, sizeof( frameInfo_t ) * numframes );
	}

	frame[ index ].index = num;
	frame[ index ].time = 0.0f;
	frame[ index ].weight = 1.0f;

	frametime = cgi.Anim_Frametime( tiki, num );

	if( cgi.Anim_HasCommands( tiki, num ) )
	{
		float time = -frametime;

		// Exec anim commands
		// They will be executed in the context of the commandManager

		for( int i = 0; i < cgi.Anim_NumFrames( tiki, num ); i++ )
		{
			tiki_cmd_t cmds;
			Event *ev;

			time += frametime;

			if( !cgi.Frame_Commands( tiki, num, i, &cmds ) ) {
				continue;
			}

			for( int k = 0; k < cmds.num_cmds; k++ )
			{
				cgi.Printf( "args = '%s'\n", cmds.cmds[ k ].args[ 0 ] );
				ev = new Event( cmds.cmds[ k ].args[ 0 ] );

				for( int c = 1; c < cmds.cmds[ k ].num_args; c++ )
				{
					const char *argument = cmds.cmds[ k ].args[ c ];

					ev->AddString( cmds.cmds[ k ].args[ c ] );
				}

				commandManager->PostEvent( ev, frametime, 0 );
			}
		}
	}
}

void Animate::SetTime( int index, float time )
{
	if( index >= numframes ) {
		return;
	}

	frame[ index ].time = time;

	turnThinkOn();
}

//=== SCRIPT ===\\

void Animate::ForwardExec( Event *ev )
{
	str name = ev->GetString( 1 );

	PlayAnim( name, 0 );
	SetTime( 0, 0.0f );
}

CLASS_DECLARATION( Entity, Animate, NULL )
{
	{ &EV_Animate_Anim,			&Animate::ForwardExec },
	{ NULL, NULL },
};
