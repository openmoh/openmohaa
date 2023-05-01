typedef struct beam_s {
	Class_t baseClass;
	int entity;
	int hModel;
	int endtime;
	Vector start;
	Vector end;
	float scale;
	float alpha;
	int flags;
	int parent;
	float max_offset;
	float min_offset;
	int numSubdivisions;
	int overlap;
	int beamshader;
	unsigned char shaderRGBA[4];
	int update_time;
	int delay;
	float life;
	int numspherebeams;
	float sphereradius;
	int toggletime;
	int toggledelay;
	int active;
	float alphastep;
	int renderfx;
} beam_t;

typedef struct cg_common_data_s {
	Class_t baseClass;

	int life;
    int createTime;
    Vector origin;
    Vector oldorigin;
    Vector accel;
    Vector angles;
    Vector velocity;
    Vector avelocity;
    Vector parentOrigin;
    Vector parentMins;
    Vector parentMaxs;
    Vector minVel;
    Vector maxVel;
    float color[4];
    float alpha;
    float scaleRate;
    float scalemin;
    float scalemax;
    float bouncefactor;
    int bouncecount;
    int maxbouncecount;
    str bouncesound;
    int bouncesound_delay;
    int flags;
    int flags2;
    dtiki_t* tiki;
    int swarmfreq;
    float swarmmaxspeed;
    float swarmdelta;
    float lightIntensity;
    int lightType;
    int fadeintime;
    int fadedelay;
    int parent;
    int collisionmask;
    int min_twinkletimeoff;
    int max_twinkletimeoff;
    int min_twinkletimeon;
    int max_twinkletimeon;
    int lightstyle;
    int physicsRate;
    float scale;
    float scale2;
    str swipe_shader;
    str swipe_tag_start;
    str swipe_tag_end;
    str shadername;
    float swipe_life;
    float friction;

    float decal_orientation;
    float decal_radius;
} cg_common_data;

typedef struct enttracker_s {
	Class_t baseClass;
	qboolean usedNumbers[256];
} enttracker_t;

typedef struct emittertime_s {
	Class_t 		baseClass;
	int            	entity_number;
	int            	last_emit_time;
	Vector         	oldorigin;
	qboolean       	active;
	qboolean       	lerp_emitter;
} emittertime_t;

typedef struct Container_emittertime_s {
	emittertime_t	*objlist;
	int				numobjects;
	int				maxobjects;
} Container_emittertime_t;

typedef struct emitterthing_s {
	enttracker_t base_enttracker_t;
	Container_emittertime_t  m_emittertimes;
	qboolean startoff;
} emitterthing_t;

typedef struct commandtime_s {
	Class_t	baseClass;
	int		entity_number;
	int		command_number;
	int		last_command_time;
} commandtime_t;

typedef struct Container_commandtime_s {
	commandtime_t	*objlist;
	int				numobjects;
	int				maxobjects;
} Container_commandtime_t;

typedef struct commandthing_s {
	enttracker_t base_enttracker_t;
	Container_commandtime_t  m_commandtimes;
	qboolean startoff;
} commandthing_t;

typedef struct spawnthing_s {
	emitterthing_t base_emitterthing_t;
	Container_str m_modellist;
	Container_str m_taglist;
	cg_common_data cgd;
	int entnum;
	Vector origin_offset_base;
	Vector origin_offset_amplitude;
	Vector axis_offset_base;
	Vector axis_offset_amplitude;
	Vector randvel_base;
	Vector randvel_amplitude;
	Vector avelocity_base;
	Vector avelocity_amplitude;
	Vector angles_amplitude;
	vec3_t axis[3];
	vec3_t tag_axis[3];
	float life_random;
	float forwardVelocity;
	float sphereRadius;
	float coneHeight;
	float spawnRate;
	int lastTime;
	int count;
	int tagnum;
	str emittername;
	str animName;
	float dcolor[3];
	qboolean dlight;
	int numtempmodels;
	float linked_origin[3];
	float linked_axis[3][3];
	float fMinRangeSquared;
	float fMaxRangeSquared;
	str startTag;
	str endTag;
	float length;
	float min_offset;
	float max_offset;
	float overlap;
	float numSubdivisions;
	float delay;
	float toggledelay;
	int beamflags;
	int numspherebeams;
	float endalpha;
	float spreadx;
	float spready;
	qboolean use_last_trace_end;
} spawnthing_t;

typedef struct Container_spawnthing_s {
	emittertime_t	*objlist;
	int				numobjects;
	int				maxobjects;
} Container_spawnthing_t;

typedef struct ctempmodel_s {
	Class_t baseClass;
	struct ctempmodel_s *next;
	struct ctempmodel_s *prev;

	cg_common_data cgd;
	str            modelname;
   
	refEntity_t    lastEnt;
	refEntity_t    ent;
   
	int            number;
	int            lastAnimTime;
	int            lastPhysicsTime;
	int            killTime;
	int            next_bouncesound_time;
	int            seed;
	int            twinkleTime;
	int            aliveTime;
	qboolean       addedOnce;
	qboolean       lastEntValid;
	spawnthing_t*  m_spawnthing;
} ctempmodel_t;

typedef struct cvssource_s {
	Class_t baseClass;

    cvssource_s *next;
    cvssource_s *prev;
    cvssource_s *stnext;
    int stindex;
    Vector lastOrigin;
    float lastRadius;
    float lastDensity;
    float lastColor[3];
    float lastLighting[3];
    Vector newOrigin;
    float newRadius;
    float newDensity;
    float newColor[3];
    float newLighting[3];
    float ooRadius;
    Vector velocity;
    float startAlpha;
    int roll;
    Vector repulsion;
    int lifeTime;
    int collisionmask;
    int parent;
    int flags;
    int flags2;
    int smokeType;
    float typeInfo;
    float fadeMult;
    float scaleMult;
    int lastPhysicsTime;
    int lastLightingTime;
    qboolean lastValid;
} cvssource_t;

typedef struct cvssourcestate_s {
	Class_t baseClass;
    Vector origin;
    float color[3];
    float radius;
    float density;
} cvssourcestate_t;

typedef struct specialeffectcommand_s {
	Class_t baseClass;
	spawnthing_t* emitter;
	float fCommandTime;
	void (*endfcn)();
} specialeffectcommand_t;

typedef struct specialeffect_s {
	Class_t baseClass;
	int m_iCommandCount;
	specialeffectcommand_t *m_commands[32];
} specialeffect_t;

typedef struct ClientGameCommandManager_s {
	Listener2_t base_Listener;
	spawnthing_t m_localemitter;
	ctempmodel_t m_active_tempmodels;
	ctempmodel_t *m_free_tempmodels;
	ctempmodel_t m_tempmodels[1024];
	cvssource_t m_active_vsssources;
	cvssource_t *m_free_vsssources;
	cvssource_t *m_vsssources;
	int m_iAllocatedvsssources;
	class spawnthing_t *m_spawnthing;
	Container_spawnthing_t m_emitters;
	int m_seed;
	commandthing_t m_command_time_manager;
	specialeffect_t *m_pCurrentSfx;
	int m_iLastVSSRepulsionTime;
	float m_fEventWait;
	struct {
		short index;
		short delta;
		union {
			void (*__pfn) ();
			short int __delta2;
		} __pfn_or_delta2;
	} endblockfcn;
} ClientGameCommandManager;

typedef struct ClientSpecialEffectsManager_s {
	Listener2_t base_Listener;
	specialeffect_t m_effects[99];
	qboolean m_bEffectsLoaded;
	int m_iNumPendingEvents;
} ClientSpecialEffectsManager;
