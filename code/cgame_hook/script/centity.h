#ifndef __CENTITY_H__
#define __CENTITY_H__

#include "simpleentity.h"

#define MT_PHYSICS			(1<<0)				// This entity has physics

extern Event EV_Touch;

class Entity : public SimpleEntity {
public:
	int				entnum;
	centity_t		*centity;
	qboolean		fShader;
	qhandle_t		customshader;
	str				shader;

	str				model;
	qhandle_t		modelhandle;
	dtiki_t			*tiki;
	int				renderFx;
	float			scale;
	int				moveType;

	SafePtr< Entity >	attached;
	str					attached_tagname;
	int					attached_use_angles;
	Vector				attached_offset;

	Vector				absmin, absmax;
	Vector				mins, maxs;

private:
	qboolean			canThink;

protected:
	frameInfo_t		*frame;
	int				numframes;

public:
	CLASS_PROTOTYPE( Entity );

	virtual void Archive( Archiver &arc );

	Entity();
	virtual ~Entity();

	frameInfo_t				*GetFrameInfo( int index );
	int						GetNumFrames();
	void					GetTag( const char *tagname, orientation_t *orient );

	virtual void			Think();

	bool					IsTouching( Entity *ent );

	float					getScale();
	qboolean				inRadius( Vector org, float rad );
	void					setModel( const char *name );
	virtual void			setOrigin( Vector origin );
	void					setScale( float scale );

	void					AssignNumber( int ent_num );
	qboolean				CanThink( void );
	int						GetNumber( void );
	void					SetShader( const char *name );
	void					turnThinkOn( void );
	void					turnThinkOff( void );

	void					EventAttach( Event *ev );
	void					EventDetach( Event *ev );
	void					EventGetModel( Event *ev );
	void					EventGetScale( Event *ev );
	void					EventSetModel( Event *ev );
	void					EventSetScale( Event *ev );
	void					EventSetSize( Event *ev );
};

Entity * CL_GetCEntity( int ent_num );
centity_t * CL_GetEntity( int ent_num );

void CL_ProcessCEntities();

extern Container< SafePtr< Entity > > s_entities;

#endif /* __CENTITY_H__ */
