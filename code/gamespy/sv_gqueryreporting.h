typedef void (*qr_querycallback_t) (char* outbuf, int maxlen, void* userdata);
typedef void (*qr_custom_handler_t) (const char* query, struct sockaddr* sender);

typedef enum query_e {
	qtunknown,
	qtbasic,
	qtinfo,
	qtrules,
	qtplayers,
	qtstatus,
	qtpackets,
	qtecho,
	qtsecure
} query_t;

typedef struct qr_implementation_s {
	void* querysock;
	void* hbsock;
	char gamename[64];
	char secret_key[128];
	qr_querycallback_t qr_basic_callback;
	qr_querycallback_t qr_info_callback;
	qr_querycallback_t qr_rules_callback;
	qr_querycallback_t qr_players_callback;
	long unsigned int lastheartbeat;
	int queryid;
	int packetnumber;
	int qport;
	char no_query;
	int unk1;
	int unk2;
	int unk3;
	int unk4;
	qr_custom_handler_t qr_custom_handler;
	void* udata;
} qr_implementation_t, *qr_t;
