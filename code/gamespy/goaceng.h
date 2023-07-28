/************
GameSpy Open Architecture 
 Portable C Engine
*************/
/******
goaceng.h
GameSpy C Engine SDK
  
Copyright 1999 GameSpy Industries, Inc

Suite E-204
2900 Bristol Street
Costa Mesa, CA 92626
(714)549-7689
Fax(714)549-0757

******

 Please see the GameSpy C Engine SDK documentation for more 
 information

 Updated 3/16/00 - DDW
  Added ServerListRemoveServer (to remove a server from the serverlist)
  Updated ServerListAuxUpdate to update the existing server if its already in the list
   instead of adding a duplicate
  Note that these functions change the indexes of servers in the list. You should never
  rely on the indexes staying constant (they change if you use the sort functions too).

 Updated 3/18/00 - DDW
  Added Dreamcast CE support (needs to recreate sockets for each query)

 Updated 4/17/00 - DDW
  Added compressed server lists and advanced filtering

 Updated 5/16/00 - DDW
  Do a select() in ServerListReadList to keep from blocking if the list is slow

 Updated 5/25/00 - DDW
  Extensive changes to minimize memory usage and increse flexibility
	-Server IPs are now stored internally as unsigned longs
	-Server key names are stored in a single shared list to minimize memory usage
	-Hashtable and DArray allocations optimized
	-You can now specify the query type using ServerListUpdate2 and ServerListAuxUpdate
	-ServerListAuxUpdate is now the official way to do multi-pass updates and manual updates
 Updated 6/13/99 - DDW
  More memory optimizations (keys/values share same list)
  Fixed alignment compiler problem for MWERKS on Dreamcast
  Remove server list select on ACCESS stack (doesn't seem to work)
******/
//todo: max results and ordering

#ifndef _GOACENG_H
#define _GOACENG_H

#ifdef __cplusplus
extern "C" {
#endif


#define ENGINE_VERSION "1.5"
#define SERVER_TIMEOUT 3000
/* GServerList and GServer are the abstracted serverlist and server data types.
All access to their internal data structures is done via the functions below */
typedef struct GServerListImplementation *GServerList;
typedef struct GServerImplementation *GServer;

/* A simple error type that we can use for return values to certain functions */
typedef int GError; //we will define some error return values

typedef int gbool;  //a simple boolean type

/* Various Server List States 
idle - no processing or querying underway, ready to being updates
listreq - a server list request has been or is being sent
listxfer - we are transfering the server list
lanlist - we are waiting for replies from servers on the LAN
querying - the servers on the list are being queried */
typedef enum {sl_idle, sl_listxfer, sl_lanlist, sl_querying} GServerListState;

/* Comparision types for the ServerListSort function
int - assume the values are int and do an integer compare
float - assume the values are float and do a flot compare
strcase - assume the values are strings and do a case sensitive compare
stricase - assume the values are strings and do a case insensitive compare */
typedef enum {cm_int, cm_float, cm_strcase, cm_stricase} GCompareMode;


typedef enum {qt_basic, qt_info, qt_rules, qt_players, qt_info_rules, qt_status} GQueryType;

/* Messages that are passed to the ListCallBackFn */
#define	LIST_STATECHANGED		1 // ServerListState changed, no parameters
#define LIST_PROGRESS			2 // New Server updated, param1 = GServer (server updated), param2 = percent done

//Single callback function into the client app for status  / progress messages
typedef void (*ListCallBackFn)(GServerList serverlist, int msg, void *instance, void *param1, void *param2);

// Callback function used for enumerating the keys/values for a server
typedef void (*KeyEnumFn)(char *key, char *value, void *instance);


/* Callback parameter types (e.g. window handle, thread window, event, function etc) */
#define GCALLBACK_FUNCTION 1	//only currently supported callback type

#define GE_NOERROR		0
#define GE_NOSOCKET		1
#define GE_NODNS		2
#define GE_NOCONNECT	3
#define GE_BUSY			4
#define GE_DATAERROR	5

/*********
Server List Functions
**********/

/* ServerListNew
----------------
Creates and returns a new (empty) GServerList.
gamename - game to ask the master for servers
enginename - the engine name that matches your seckey
seckey - secret key used for talking to the master
maxconcupdates - max number of concurent updates (10-15 for modem users, 20-30 for high-bandwidth)
CallBackFn - The function or handle used for progress updates
CallBackFnType - The type of the CallBackFn parameter (from the #define list above)
instance - user-defined instance data (e.g. structure or object pointer) */
GServerList	ServerListNew(const char *gamename, const char *enginename, const char *seckey, int maxconcupdates, void *CallBackFn, int CallBackFnType, void *instance);

/* ServerListFree
-----------------
Free a GServerList and all internal sturctures and servers */
void ServerListFree(GServerList serverlist);

/* ServerListUpdate
-------------------
Start updating a GServerList from the master server.
Can only be called when the list is in the sl_idle state, will return an error otherwise.
The server list will be retrieved from the master, then each server in the list will be
added and updated. 
If async = false, the function will not return until the entire list has been processed, or
ServerListHalt has been called (from another thread or from the progress callback)
If async = true, the function will return immediately, but you must call ServerListThink 
every ~10ms for list processing and querying to occur. */ 
GError ServerListUpdate(GServerList serverlist, gbool async);


/* ServerListUpdate2
-------------------------
Identical to ServerListUpdate, except that the server list is filtered
on the server side by the filter, and you can specify the type of query to perform.
If you want to specify the type of query, but no filter, simply pass NULL for filter.
Filter is a SQL-style boolean statement such as:
"gametype='ctf'"
"numplayers > 1 and numplayers < 8"
"gamestate='openplaying'"
The filter can be arbitrarily complex and supports all standard SQL
groupings and boolean operations. The following fields are available
for filtering:
hostport
gamever
location
hostname
mapname
gametype
gamemode
numplayers
maxplayers*/ 
GError ServerListUpdate2(GServerList serverlist, gbool async, char *filter, GQueryType querytype);



/* ServerListLANUpdate
-------------------
Search for servers on the local LAN and start updating them. This will search over a
range of ports for any servers within broadcast range.
Can only be called when the list is in the sl_idle state, will return an error otherwise.
A query will be sent out on each port between start and end (inclusive) on intervals of delta
(e.g. 10,20,2 would search 10,12,14,16,18,20)
If async = false, the function will not return until the entire list has been processed, or
ServerListHalt has been called (from another thread or from the progress callback)
If async = true, the function will return immediately, but you must call ServerListThink 
every ~10ms for list processing and querying to occur. */ //TODO: add filter
GError ServerListLANUpdate(GServerList serverlist, gbool async, int startsearchport, int endsearchport, int searchdelta);

/* ServerListAuxUpdate
-------------------
Adds an "auxilliary" server -- this is a way to either update a server manually or
get additional/updated information from a server already in the list.
If there is an update currently in progress (not sl_idle), the aux server will
be processed along with the current batch. If the state is idle, this function behaves
like ServerListUpdate() and starts the engine.
If async = false, the function will not return until the server has been processed, or
ServerListHalt has been called (from another thread or from the progress callback)
If async = true, the function will return immediately, but you must call ServerListThink 
every ~10ms for list processing and querying to occur. Note that async has no effect
if an update is in progress. */
GError ServerListAuxUpdate(GServerList serverlist, const char *ip, int port, gbool async, GQueryType querytype);

/* ServerListRemoveServer
-------------------------
Removes a single server from the list. Frees the memory associated with the GServer.
Do not reference the GServer object after calling this function. Note that this changes
the indexes of the servers in the list. You should never store the indexes of the servers
in the list, just their GServer objects */
void ServerListRemoveServer(GServerList serverlist, char *ip, int port);

/* ServerListThink
------------------
For use with Async Updates. This needs to be called every ~10ms for list processing and
updating to occur during async server list updates */
GError ServerListThink(GServerList serverlist); 

/* ServerListHalt
-----------------
Halts the current updates batch  */
GError ServerListHalt(GServerList serverlist);

/* ServerListClear
------------------
Clear and free all of the servers from the server list.
List must be in the sl_idle state */
GError ServerListClear(GServerList serverlist);

/* ServerListState
------------------
Returns the current state of the server list */
GServerListState ServerListState(GServerList serverlist);

/* ServerListErrorDesc
----------------------
Returns a static string description of the specified error */
char *ServerListErrorDesc(GServerList serverlist, GError error);

/* ServerListGetServer
----------------------
Returns the server at the specified index, or NULL if the index is out of bounds */
GServer ServerListGetServer(GServerList serverlist, int index);

/* ServerListCount
------------------
Returns the number of servers on the specified list. Indexing is 0 based, so
the actual server indexes are 0 <= valid index < Count */
int ServerListCount(GServerList serverlist);

/* ServerListSort
-----------------
Sort the server list in either ascending or descending order using the 
specified comparemode.
sortkey can be a normal server key, or "ping" or "hostaddr" */
void ServerListSort(GServerList serverlist, gbool ascending, char *sortkey, GCompareMode comparemode);


/**************
ServerFunctions
***************/

/* ServerGetPing
----------------
Returns the ping for the specified server. 
A ping of 9999 indicates that the server has not been queried or did not respond */
int ServerGetPing(GServer server);

/* ServerGetAddress
-------------------
Returns the string, dotted IP address for the specified server */
char *ServerGetAddress(GServer server);

/* ServerGetInetAddress
-------------------
Returns the network-ordered IP address for the specified server */
unsigned int ServerGetInetAddress(GServer server);


/* ServerGetPort
----------------
Returns the "query" port for the specified server. If the game uses a seperate
"game" port, it can be retrieved via: ServerGetIntValue(server,"hostport",0) */
int ServerGetQueryPort(GServer server);

/* ServerGet[]Value
------------------
Returns the value for the specified key. If the key does not exist for the
given server, the default value is returned */
char *ServerGetStringValue(GServer server, char *key, char *sdefault);
int ServerGetIntValue(GServer server, char *key, int idefault);
double ServerGetFloatValue(GServer server, char *key, double fdefault);


/* ServerGetPlayer[]Value
------------------
Returns the value for the specified key on the specified player. If the key does not exist for the
given server, the default value is returned */
char *ServerGetPlayerStringValue(GServer server, int playernum, char *key, char *sdefault);
int ServerGetPlayerIntValue(GServer server, int playernum, char *key, int idefault);
double ServerGetPlayerFloatValue(GServer server, int playernum, char *key, double fdefault);

/* ServerEnumKeys 
-----------------
Enumerates the keys/values for a given server by calling KeyEnumFn with each
key/value. The user-defined instance data will be passed to the KeyFn callback */
void ServerEnumKeys(GServer server, KeyEnumFn KeyFn, void *instance);

#ifdef __cplusplus
}
#endif

#endif //_GOACENG_H