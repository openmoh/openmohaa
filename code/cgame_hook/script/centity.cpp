/*
* Entity.cpp
*
* DESCRIPTION : Client-side entity
*/

#include "centity.h"
#include "archive.h"

Container< SafePtr< Entity > > s_entities;

Event EV_Entity_Attach
(
	"attach",
	EV_DEFAULT,
	"esIV",
	"parent tagname use_angles offset",
	"attach this entity to the parent's legs tag called tagname",
	EV_NORMAL
);

Event EV_Entity_Detach
(
	"detach",
	EV_DEFAULT,
	NULL,
	NULL,
	"detach this entity from its parent.",
	EV_NORMAL
);

Event EV_Entity_GetModel
(
	"model",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the model.",
	EV_RETURN
);

Event EV_Entity_Getter_GetModel
(
	"model",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the model.",
	EV_GETTER
);

Event EV_Entity_GetScale
(
	"scale",
	EV_DEFAULT,
	NULL,
	NULL,
	"Gets the scale.",
	EV_GETTER
);

Event EV_Entity_SetModel
(
	"model",
	EV_DEFAULT,
	"s",
	"modelName",
	"Sets the model to modelName.",
	EV_NORMAL
);

Event EV_Entity_SetScale
(
	"scale",
	EV_DEFAULT,
	"f",
	"scale",
	"Sets the scale.",
	EV_SETTER
);

Event EV_Entity_Setter_SetModel
(
	"model",
	EV_DEFAULT,
	"s",
	"modelName",
	"Sets the model to modelName.",
	EV_SETTER
);

Event EV_Entity_SetSize
(
	"setsize",
	EV_DEFAULT,
	"vv",
	"mins maxs",
	"Set the bounding box of the entity to mins and maxs.",
	EV_NORMAL
);

Event EV_Touch
(
	"doTouch",
	EV_CODEONLY,
	"e",
	"touchingEntity",
	"sent to entity when touched.",
	EV_NORMAL
);

Entity * CL_GetCEntity( int ent_num )
{
	// Iterate through simple client entities
	for( int i = 0; i < s_entities.NumObjects(); i++ )
	{
		Entity * ent = ( Entity * )s_entities[ i ];

		if( !checkInheritance( &Entity::ClassInfo, ent->classinfo() ) ) {
			continue;
		}

		int entnum = ent->GetNumber();

		if( entnum != -1 && entnum == ent_num ) {
			return ent;
		}
	}

	return NULL;
}

centity_t *CL_GetEntity( int ent_num )
{
	entityState_t *ce = NULL;
	int i;

	for( i = 0; i < cg->activeSnapshots[ 0 ].numEntities; i++ )
	{
		if( cg->activeSnapshots[ 0 ].entities[ i ].number == ent_num ) {
			ce = &cg->activeSnapshots[ 0 ].entities[ i ];
		}
	}

	return ( centity_t * )ce;
}

void CL_ProcessCEntities()
{
	// Iterate through client entities
	for( int i = s_entities.NumObjects(); i > 0; i-- )
	{
		Entity *ent = s_entities.ObjectAt( i );

		if( !checkInheritance( &Entity::ClassInfo, ent->classinfo() ) ) {
			continue;
		}

		if( ent->CanThink() ) {
			ent->Think();
		}
	}
}

Entity::Entity()
{
	cent = &cg_entities[ 0 ];
	centity = &cg_entities[ 0 ];

	model = "";
	modelhandle = 0;
	scale = 1.f;
	renderFx = 0;

	tiki = NULL;

	frame = new frameInfo_t[ 1 ];
	numframes = 1;

	entnum = -1;

	frame[ 0 ].index = 0;
	frame[ 0 ].time = 0.f;
	frame[ 0 ].weight = 1.0f;

	//turnThinkOn();

	absmin = absmax = mins = maxs = vec_zero;

	s_entities.AddObject( this );
}

Entity::~Entity()
{
	if( frame ) {
		delete[] frame;
	}

	numframes = 0;

	s_entities.RemoveObject( this );
}

bool Entity::IsTouching( Entity *ent )
{
	return ent->absmin[ 0 ] <= absmax[ 0 ]
		&& ent->absmin[ 1 ] <= absmax[ 1 ]
		&& ent->absmin[ 2 ] <= absmax[ 2 ]
		&& absmin[ 0 ] <= ent->absmax[ 0 ]
		&& absmin[ 1 ] <= ent->absmax[ 1 ]
		&& absmin[ 2 ] <= ent->absmax[ 2 ];
}

float Entity::getScale()
{
	return scale;
}

qboolean Entity::inRadius( Vector org, float rad )
{
	if( Vector::Distance( origin, org ) <= rad ) {
		return true;
	} else {
		return false;
	}
}

void Entity::setModel( const char *name )
{
	model = name;
	modelhandle = cgi.R_RegisterModel( name );

	tiki = cgi.R_Model_GetHandle( modelhandle );
}

void Entity::setOrigin( Vector origin )
{
	this->origin = origin;

	absmin = origin + mins;
	absmax = origin + maxs;
}

void Entity::setScale( float s )
{
	scale = s;
}

void Entity::Think()
{

}

void Entity::Archive( Archiver &arc )
{
	SimpleEntity::Archive( arc );

	arc.ArchiveBoolean( &fShader );

	if( fShader )
	{
		arc.ArchiveString( &shader );

		if( arc.Loading() ) {
			customshader = cgi.R_RegisterShader( shader );
		}
	}

	arc.ArchiveBoolean( &canThink );

	arc.ArchiveFloat( &scale );
	arc.ArchiveInteger( &entnum );
	arc.ArchiveInteger( &renderFx );

	arc.ArchiveString( &model );
}

frameInfo_t *Entity::GetFrameInfo( int index )
{
	if( index >= numframes ) {
		return NULL;
	} else {
		return &frame[ index ];
	}
}

int Entity::GetNumFrames()
{
	return numframes;
}

void Entity::GetTag( const char *tagname, orientation_t *orient )
{
	refEntity_t ref;

	if( tiki == NULL ) {
		return;
	}

	int num = cgi.Tag_NumForName( tiki, tagname );

	if( num == -1 ) {
		return;
	}

	memset( &ref, 0, sizeof( refEntity_t ) );

	VectorCopy( origin, ref.origin );
	MatrixToEulerAngles( ref.axis, angles );

	ref.reType = RT_MODEL;

	ref.scale = scale;
	ref.hModel = modelhandle;
	ref.hOldModel = modelhandle;
	ref.tiki = tiki;
	ref.entityNumber = 0;
	ref.actionWeight = 1.0f;

	for( int i = 0; i < GetNumFrames(); i++ )
	{
		frameInfo_t *frame = GetFrameInfo( i );

		ref.frameInfo[ i ].index = frame->index;
		ref.frameInfo[ i ].time = frame->time;
		ref.frameInfo[ i ].weight = frame->weight;
	}

	//cgi.TIKI_Orientation( orient, &ref, num );
	*orient = re.TIKI_Orientation( &ref, num );
}

void Entity::AssignNumber( int ent_num )
{
	if( entnum == -1 ) {
		entnum = ent_num;
	}
}

int Entity::GetNumber()
{
	return entnum;
}

qboolean Entity::CanThink()
{
	return canThink;
}

void Entity::turnThinkOn()
{
	canThink = true;
}

void Entity::turnThinkOff()
{
	canThink = false;
}

void Entity::SetShader( const char *name )
{
	if( name == NULL )
	{
		fShader = false;
		shader = "";

		return;
	}

	fShader = true;
	customshader = cgi.R_RegisterShader( name );
	shader = name;
}

//=== SCRIPT ===\\

void Entity::EventAttach( Event *ev )
{
	Entity *parent = ev->GetEntity( 1 );
	str tagname = ev->GetString( 2 );
	int use_angles;
	Vector offset;

	if( ev->NumArgs() > 2 ) {
		use_angles = ev->GetInteger( 3 );
	} else {
		use_angles = 1;
	}

	if( ev->NumArgs() > 3 ) {
		offset = ev->GetVector( 4 );
	} else {
		offset = vec_zero;
	}

	attached = parent;
	attached_tagname = tagname;
	attached_use_angles = use_angles;
	attached_offset = offset;
}

void Entity::EventDetach( Event *ev )
{
	attached = NULL;
}

void Entity::EventGetModel( Event *ev )
{
	ev->AddString( model );
}

void Entity::EventGetScale( Event *ev )
{
	ev->AddFloat( getScale() );
}

void Entity::EventSetScale( Event *ev )
{
	setScale( ev->GetFloat( 1 ) );
}

void Entity::EventSetModel( Event *ev )
{
	str name = ev->GetString( 1 );

	setModel( name );
}

void Entity::EventSetSize( Event *ev )
{
	mins = ev->GetVector( 1 );
	maxs = ev->GetVector( 2 );

	setOrigin( origin );
}

CLASS_DECLARATION( SimpleEntity, Entity, NULL )
{
	{ &EV_Entity_Attach,				&Entity::EventAttach },
	{ &EV_Entity_Detach,				&Entity::EventDetach },
	{ &EV_Entity_GetModel,				&Entity::EventGetModel },
	{ &EV_Entity_GetScale,				&Entity::EventGetScale },
	{ &EV_Entity_Getter_GetModel,		&Entity::EventGetModel },
	{ &EV_Entity_SetModel,				&Entity::EventSetModel },
	{ &EV_Entity_SetScale,				&Entity::EventSetScale },
	{ &EV_Entity_Setter_SetModel,		&Entity::EventSetModel },
	{ &EV_Entity_SetSize,				&Entity::EventSetSize },
	{ NULL, NULL }
};
