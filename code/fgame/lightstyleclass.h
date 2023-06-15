#include "listener.h"

class LightStyleClass : public Class
{
private:
	CLASS_PROTOTYPE(LightStyleClass);

	str            styles[MAX_LIGHTSTYLES];

public:

	void              SetLightStyle(int index, str style);
	void              Archive(Archiver& arc) override;
};

extern LightStyleClass lightStyles;
