#ifndef __CLIENTGAMECOMMAND_H__
#define __CLIENTGAMECOMMAND_H__

#include "listener.h"
#include "scripttimer.h"

class LocalSound : public Listener {
private:
	void			*stream;

	str				name;
	str				path;

	qboolean		loop;

	qboolean		stopping;
	int				position;
	int				baserate;

	float			rate, start_rate, target_rate;
	float			volume, start_volume, target_volume;

	ScriptTimer		timer;
	ScriptTimer		timer_rate;

public:
	CLASS_PROTOTYPE( LocalSound );

	virtual void Archive( Archiver &arc );

	LocalSound( str name, qboolean loop );
	LocalSound();
	~LocalSound();

	str		GetName();

	void	Play( float time, float volume );
	void	Stop( float time );

	void	SetRate( float rate, float time );

	void	Think( Event *ev );
};

#define STREAM_PLAYING			4
#define STREAM_PAUSED			16

class ClientGameCommandManager : public Listener
{
public:
	static void		ArchiveFunction( Archiver &arc );
	static void		CleanUp( void );
	static void		Initialize( void );

	void			PlaySound( str sound_name, float *origin, int channel, float volume, float min_distance, float pitch, int argstype );

	void			PlayLocalSound( str sound_name, qboolean loop, float volume, float time = 0.0f );
	void			SetLocalSoundRate( str sound_name, float rate, float time = 0.0f );
	void			StopLocalSound( str sound_name, float time = 0.0f );
	LocalSound		*GetLocalSound( str sound_name );
};

class ClientSpecialEffectsManager : public Listener
{
public:
	void MakeEffect_Normal( int iEffect, Vector position, Vector normal );
};

extern ClientGameCommandManager *commandManager;
extern ClientSpecialEffectsManager *sfxManager;

#endif
