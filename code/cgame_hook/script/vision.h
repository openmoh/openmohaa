#ifndef __VISION_H__
#define __VISION_H__

#include "class.h"

class ScriptTimer;

class VisionClass : public Class
{
private:
	str						name;

	str						defaultval;

protected:
	str						string;

	ScriptTimer				*m_timer;

	float					value;
	float					value_start;
	float					value_target;

public:
	qboolean				fProcessed;

protected:
	void					VisionInit( const char *name, const char *value, ScriptTimer *globalTimer );

public:
	CLASS_PROTOTYPE( VisionClass );

	virtual void Archive( Archiver &arc );

	static VisionClass *Get( str name );
	static void GlobalArchive( Archiver &arc );

	VisionClass();
	VisionClass( const char *name, const char *value, ScriptTimer *globalTimer );

	virtual void			Think();

	str						GetName();
	ScriptTimer				*GetTimer();
	virtual str				GetType();

	str						defaultValue();

	float					floatValue();
	str						stringValue();

	virtual qboolean		isBasic();
	virtual qboolean		isEnabled();
	virtual	qboolean		isNewEnabled();

	void					setFloatValue( float value );
	virtual void			setValue( str value );
};

class VisionActivator : public VisionClass
{
private:
	str						vistype;

public:
	CLASS_PROTOTYPE( VisionActivator );

	virtual void Archive( Archiver &arc );

	VisionActivator( const char *name, const char *value, const char *type = "vision", ScriptTimer *globalTimer = NULL );
	VisionActivator();

	virtual str				GetType();

	virtual qboolean		isBasic();
	virtual qboolean		isEnabled();
	virtual	qboolean		isNewEnabled();

	virtual void			setValue( str value );
};

class VisionVariable : public VisionClass
{
private:
	VisionActivator			*activator;

	Vector					vector;

	// target values
	Vector					vector_start;
	Vector					vector_target;

public:
	CLASS_PROTOTYPE( VisionVariable );

	virtual void Archive( Archiver &arc );

	VisionVariable( const char *name, const char *value, VisionActivator *activator );
	VisionVariable();

	virtual void			Think();

	VisionActivator			*GetActivator();
	ScriptTimer				*GetTimer();
	virtual str				GetType();

	virtual qboolean		isBasic();
	virtual qboolean		isEnabled();
	virtual	qboolean		isNewEnabled();

	Vector					vectorValue();

	virtual void			setValue( str value );
};

extern VisionActivator r_anaglyphTweakEnable;
extern VisionActivator r_distortionEnable;
extern VisionActivator r_filmEnable;
extern VisionActivator r_glow;

extern VisionVariable r_glowRadius0;
extern VisionVariable r_glowRadius1;
extern VisionVariable r_glowBloomCutoff;
extern VisionVariable r_glowBloomDesaturation;
extern VisionVariable r_glowBloomIntensity0;
extern VisionVariable r_glowBloomIntensity1;
extern VisionVariable r_glowBloomStreakX;
extern VisionVariable r_glowBloomStreakY;

extern VisionVariable r_filmContrast;
extern VisionVariable r_filmBleach;
extern VisionVariable r_filmBrightness;
extern VisionVariable r_filmHue;
extern VisionVariable r_filmMidStart;
extern VisionVariable r_filmMidEnd;
extern VisionVariable r_filmDarkTint;
extern VisionVariable r_filmMidTint;
extern VisionVariable r_filmLightTint;
extern VisionVariable r_filmSaturation;

extern VisionVariable r_anaglyphAngleMult;
extern VisionVariable r_anaglyphOffsetMult;

extern VisionVariable r_distortionRadius;
extern VisionVariable r_distortionScale;

#endif
