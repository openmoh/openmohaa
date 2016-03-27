/*
* Level.cpp
*
* DESCRIPTION : Global level info
*/

#include "level.h"

Event EV_Level_testf
(
	"testf",
	EV_DEFAULT,
	"i",
	"test_int",
	"test",
	EV_SETTER
);

Level level;

Level::Level()
{
	prespawned = false;
	spawned = false;
}

Level::~Level()
{

}

void Level::setTime( int levelTime )
{
	svsTime = levelTime;
	inttime = levelTime - svsStartTime;
	svsFloatTime = levelTime / 1000.0f;
	time = inttime / 1000.0f;
}

void Level::setFrametime( int frametime )
{
	intframetime = frametime;
	this->frametime = frametime / 1000.0f;
}

void Level::Archive( Archiver &arc )
{
	Listener::Archive( arc );
}

void Level::TestEvent( Event *ev )
{
	int integer = ev->GetInteger( 1 );

	cgi.Printf( "this = %p\n", this );

	cgi.Printf( "SCRIPT: integer = %d\n", integer );
}

CLASS_DECLARATION( Listener, Level, NULL )
{
	{ &EV_Level_testf,		&Level::TestEvent },
	{ NULL, NULL }
};
