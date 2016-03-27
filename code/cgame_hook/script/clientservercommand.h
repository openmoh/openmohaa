#ifndef __CLIENTSERVERCOMMAND_H__
#define __CLIENTSERVERCOMMAND_H__

#include "listener.h"

class ClientServerCommandManager : public Listener {
public:
	CLASS_PROTOTYPE( ClientServerCommandManager );

	virtual void Archive( Archiver &arc );

	void			ExecuteCommand( char *arguments[], int num_arguments );

	// Events
	void			EventEarthquake( Event *ev );
	void			EventFadeSound( Event *ev );
	void			EventRestoreSound( Event *ev );
	void			EventSetClientFlag( Event *ev );
	void			EventSetShader( Event *ev );
	void			EventVisionSetBlur( Event *ev );
	void			EventVisionSetNaked( Event *ev );
};

extern ClientServerCommandManager svcommandManager;

#endif // __CLIENTSERVERCOMMAND_H__
