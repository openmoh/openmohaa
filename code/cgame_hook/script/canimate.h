#ifndef __CANIMATE_H__
#define __CANIMATE_H__

#include "centity.h"

class Animate : public Entity
{
public:
	CLASS_PROTOTYPE( Animate );

	virtual void Archive( Archiver &arc );

	Animate();
	~Animate();

	virtual void	Think();

	void			PlayAnim( const char *name, int index );
	void			PlayAnim( int num, int index );
	void			SetTime( int index, float time );

	void			ForwardExec( Event *ev );
};

#endif /* __CANIMATE_H__ */
