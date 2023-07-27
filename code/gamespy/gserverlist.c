/******
gserverlist.c
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

  The goaceng.h file has extensive comments on each of the public functions
  for this SDK. It also has a change history for the SDK.

******/
#include "goaceng.h"
#include "gserver.h"
#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__)
	#include "::nonport.h"
#else
	#include "../nonport.h"
#endif
#ifdef _MACOS
	#include "::darray.h"
	#include "::hashtable.h"
#else
	#include "../darray.h"
	#include "../hashtable.h"
#endif
#include "gutil.h"
#ifndef UNDER_CE
#include <assert.h>
#else
#define assert(a)
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MSHOST "master.gamespy.com"
#define MSPORT	28900
#define SERVER_GROWBY 64
#define LAN_SEARCH_TIME 3000 //3 sec
#define LIST_NUMKEYBUCKETS 500
#define LIST_NUMKEYCHAINS 4

#ifdef __cplusplus
extern "C" {
#endif


//todo: check state changes on error
typedef struct 
{
	SOCKET s;
	GServer currentserver;
	unsigned long starttime;
	struct sockaddr_in saddr;
} UpdateInfo;



struct GServerListImplementation
{
	GServerListState state;
	DArray servers;
	UpdateInfo *updatelist; //dynamic array of updateinfos
	char gamename[32];
	char seckey[32];
	char enginename[32];
	int maxupdates;
	int nextupdate;
	int abortupdate;
	ListCallBackFn CallBackFn;
	void *instance;
	char *sortkey;
	gbool sortascending;
	SOCKET slsocket;
	unsigned long lanstarttime;
	GQueryType querytype;
	HashTable keylist;
};

GServerList g_sortserverlist; //global serverlist for sorting info!!

/* these correspond to the qt_ constants */
#define NUM_QUERYTYPES 6
const char *querystrings[NUM_QUERYTYPES] = {"\\basic\\","\\info\\","\\rules\\",
								"\\players\\","\\info\\\\rules\\","\\status\\"};
const int querylengths[NUM_QUERYTYPES] = {7,6,7,9,13,8};

static void KeyValFree(void *elem);
static int KeyValCompareKeyA(const void *entry1, const void *entry2);
static int KeyValHashKeyA(const void *elem, int numbuckets);


/* ServerListNew
----------------
Creates and returns a new (empty) GServerList. */
GServerList	ServerListNew(char *gamename, char *enginename, char *seckey, int maxconcupdates, void *CallBackFn, int CallBackFnType, void *instance)
{
	GServerList list;

	list = (GServerList) malloc(sizeof(struct GServerListImplementation));
	assert(list != NULL);
	list->state = sl_idle;
	list->servers = ArrayNew(sizeof(GServer), SERVER_GROWBY, ServerFree);
	list->keylist = TableNew2(sizeof(char *),LIST_NUMKEYBUCKETS,LIST_NUMKEYCHAINS,GStringHash, GCaseInsensitiveCompare, GStringFree);
	list->maxupdates = maxconcupdates;
	list->updatelist = malloc(maxconcupdates * sizeof(UpdateInfo));
	memset(list->updatelist, 0, maxconcupdates * sizeof(UpdateInfo));
	assert(list->updatelist != NULL);
	strcpy(list->gamename, gamename);
	strcpy(list->seckey, seckey);
	strcpy(list->enginename, enginename);
	list->CallBackFn = CallBackFn;
	assert(CallBackFn != NULL);
	list->instance = instance;
	list->sortkey = "";
	SocketStartUp();
	return list;	
}

/* ServerListFree
-----------------
Free a GServerList and all internal sturctures and servers */
void ServerListFree(GServerList serverlist)
{
	ArrayFree(serverlist->servers);
	TableFree(serverlist->keylist);
	free(serverlist->updatelist);

	free(serverlist);
	SocketShutDown();
}

 //create update sockets and init structures
static GError InitUpdateList(GServerList serverlist)
{
	int i;

	for (i = 0 ; i < serverlist->maxupdates ; i++)
	{
		serverlist->updatelist[i].s = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP);
		if (serverlist->updatelist[i].s == INVALID_SOCKET)
		{ //ran out of sockets, just cap maxupdates here, unless we don't have any
			if (i == 0)
				return GE_NOSOCKET;
			serverlist->maxupdates = i;
			return 0;
		}
		serverlist->updatelist[i].currentserver = NULL;
		serverlist->updatelist[i].starttime = 0;
	}
	return 0;


}

//free update sockets 
static GError FreeUpdateList(GServerList serverlist) 
{
	int i;
	for (i = 0 ; i < serverlist->maxupdates ; i++)
	{
		closesocket(serverlist->updatelist[i].s);
	}
	return 0;


}

//create and connect a server list socket
static GError CreateServerListSocket(GServerList serverlist)
{
	struct   sockaddr_in saddr;
	struct hostent *hent;

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(MSPORT);
	saddr.sin_addr.s_addr = inet_addr(MSHOST);
	if (saddr.sin_addr.s_addr == INADDR_NONE)
	{
		hent = gethostbyname(MSHOST);
		if (!hent)
			return GE_NODNS; 
		saddr.sin_addr.s_addr = *(u_long *)hent->h_addr_list[0];
	}
	serverlist->slsocket = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
	if (serverlist->slsocket == INVALID_SOCKET)
		return GE_NOSOCKET;
	if (connect ( serverlist->slsocket, (struct sockaddr *) &saddr, sizeof saddr ) != 0)
	{
		closesocket(serverlist->slsocket);
		return GE_NOCONNECT; 
	}

	//else we are connected
	return 0;


}


//create and connect a server list socket
static GError CreateServerListLANSocket(GServerList serverlist)
{
	int optval = 1;

	serverlist->slsocket = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
	if (serverlist->slsocket == INVALID_SOCKET)
		return GE_NOSOCKET;
	if (setsockopt(serverlist->slsocket, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
		return GE_NOSOCKET;

	//else we are ready to broadcast
	return 0;


}

//trigger the callback and set the new mode
static void ServerListModeChange(GServerList serverlist, GServerListState newstate)
{
	serverlist->state = newstate;
	serverlist->CallBackFn(serverlist, LIST_STATECHANGED, serverlist->instance, NULL, NULL);

}


// validate us to the master and send a list request
#define SECURE "\\secure\\"
static GError SendListRequest(GServerList serverlist, char *filter)
{
	char data[256], *ptr, result[64];
	int len;

	
	len = recv(serverlist->slsocket, data, sizeof(data) - 1, 0);
	if (len == SOCKET_ERROR)
		return GE_NOCONNECT;
	data[len] = '\0'; //null terminate it
	
	ptr = strstr ( data, SECURE );
	if (!ptr)
		return GE_DATAERROR;
	ptr = ptr + strlen(SECURE);
	gs_encrypt   ( (uchar *) serverlist->seckey, 6, (uchar *)ptr, 6 );
	gs_encode ( (uchar *)ptr, 6, (uchar *) result );

	//validate to the master
	sprintf(data, "\\gamename\\%s\\gamever\\%s\\location\\0\\validate\\%s\\final\\\\queryid\\1.1\\",
			serverlist->enginename, ENGINE_VERSION, result); //validate us		
	
	len = send ( serverlist->slsocket, data, strlen(data), 0 );
	if (len == SOCKET_ERROR || len == 0)
		return GE_NOCONNECT;

	//send the list request
	if (filter)
		sprintf(data, "\\list\\cmp\\gamename\\%s\\where\\%s\\final\\", serverlist->gamename, filter);
	else
		sprintf(data, "\\list\\cmp\\gamename\\%s\\final\\", serverlist->gamename);
	len = send ( serverlist->slsocket, data, strlen(data), 0 );
	if (len == SOCKET_ERROR || len == 0)
		return GE_NOCONNECT;

	ServerListModeChange(serverlist, sl_listxfer);
	return 0;
}


static GError SendBroadcastRequest(GServerList serverlist, int startport, int endport, int delta)
{
	struct   sockaddr_in saddr;
	short i;

	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = 0xFFFFFFFF; //broadcast
	for (i = startport ; i <= endport ; i += delta)
	{
		saddr.sin_port = htons(i);
		sendto(serverlist->slsocket, "\\status\\",8,0,(struct sockaddr *)&saddr,sizeof(saddr));
	}
	ServerListModeChange(serverlist, sl_lanlist);
	serverlist->lanstarttime = current_time();
	return 0;



}

//just wait for the server list to become idle
static void DoSyncLoop(GServerList serverlist)
{
	while (serverlist->state != sl_idle)
	{
		ServerListThink(serverlist);
		msleep(10);
		
	}
	
}

/* ServerListUpdate
-------------------
Start updating a GServerList without filters */
GError ServerListUpdate(GServerList serverlist, gbool async)
{
	return ServerListUpdate2(serverlist, async, NULL, qt_status);
}

/* ServerListUpdate2
-------------------------
Start updating a GServerList. */
GError ServerListUpdate2(GServerList serverlist, gbool async, char *filter, GQueryType querytype)
{
	GError error;

	if (serverlist->state != sl_idle)
		return GE_BUSY;

	serverlist->querytype = querytype;
	error = CreateServerListSocket(serverlist);
	if (error) return error;
	error = SendListRequest(serverlist, filter);
	if (error) return error;
	error = InitUpdateList(serverlist);
	if (error) return error;
	serverlist->nextupdate = 0;
	serverlist->abortupdate = 0;
	if (!async)
		DoSyncLoop(serverlist);

	return 0;
}

/* ServerListLANUpdate
-------------------
Start updating a GServerList from servers on the LAN. */
GError ServerListLANUpdate(GServerList serverlist, gbool async, int startsearchport, int endsearchport, int searchdelta)
{
	GError error;

	assert(searchdelta > 0);

	if (serverlist->state != sl_idle)
		return GE_BUSY;

	error = InitUpdateList(serverlist);
	if (error) return error;
	error = CreateServerListLANSocket(serverlist);
	if (error) return error;
	error = SendBroadcastRequest(serverlist, startsearchport, endsearchport, searchdelta);
	if (error) return error;
	serverlist->nextupdate = 0;
	serverlist->abortupdate = 0;
	if (!async)
		DoSyncLoop(serverlist);

	return 0;
}


//add the server to the list with the given ip, port
static void ServerListAddServer(GServerList serverlist, unsigned long ip, unsigned short port, GQueryType qtype)
{
	GServer server;
	server =  ServerNew(ip, port, qtype, serverlist->keylist);
	ArrayAppend(serverlist->servers,&server);
 //printf("%d %s:%d\n",++count, ip,port);
}

//add the server to the list with the given ip, port
static void ServerListInsertServer(GServerList serverlist, unsigned long ip, unsigned short port, int pos, GQueryType qtype)
{
	GServer server;
	server =  ServerNew(ip, port, qtype, serverlist->keylist);
	ArrayInsertAt(serverlist->servers,&server,pos);
 //printf("%d %s:%d\n",++count, ip,port);
}

//find the server in the list, returns -1 if it does not exist
static int ServerListFindServer(GServerList serverlist, unsigned int ip, int port)
{
	int i;
	GServer server;

	for (i = 0; i < ArrayLength(serverlist->servers) ; i++)
	{
		server = *(GServer *)ArrayNth(serverlist->servers,i);
		if (port == ServerGetQueryPort(server) && ServerGetInetAddress(server)==ip)
		{
			return i;
		}
	}
	return -1;
}

//finds the server in the list of servers currently being queried
// returns -1 if it does not exist
static int ServerListFindServerInUpdateList(GServerList serverlist, GServer server)
{
	int i;

	for (i = 0 ; i < serverlist->maxupdates ; i++)
	{
		if (serverlist->updatelist[i].currentserver == server)
			return i;
	}
	return -1;
}

/* ServerListRemoveServer
-------------------------
Removes a single server from the list. Frees the memory associated with the GServer */
void ServerListRemoveServer(GServerList serverlist, char *ip, int port)
{
	int currentindex = ServerListFindServer(serverlist, inet_addr(ip), port);
	int updateindex;

	if (currentindex == -1)
		return; //can't do anything, it doesn't exist

	//check to see whether we need to change the updatelist or move the nextupdate
	if (serverlist->state != sl_idle && serverlist->nextupdate > currentindex) 
	{
		GServer holdserver = *(GServer *)ArrayNth(serverlist->servers,currentindex);
		updateindex = ServerListFindServerInUpdateList(serverlist, holdserver);
		if (updateindex != -1) //is currently being queried, stop it
			serverlist->updatelist[updateindex].currentserver = NULL;
		serverlist->nextupdate--; //decrement the next update, since we are removing a server
	}
	ArrayDeleteAt(serverlist->servers, currentindex); 
}

/* ServerListUpdate
-------------------
Adds an auxilliary (non-fetched) server to the update list.
If the engine is idle, the server is added and the engine started. */
GError ServerListAuxUpdate(GServerList serverlist, const char *ip, int port, gbool async, GQueryType querytype)
{
	GError error;
	int currentindex;
	int updateindex;
	unsigned int real_ip = inet_addr(ip);
	//first, see if the server already exists
	currentindex = ServerListFindServer(serverlist,real_ip,port);


	//if we're idle, start things up
	if (serverlist->state == sl_idle)
	{
		//prepare as if we're going to do a normal list fetch,
		//but skip the call to SendListRequest().

		
		error = InitUpdateList(serverlist);
		if (error) return error;
		if (currentindex != -1) //we need to "move" this server to the end of the list
		{ //move the server to the end of the array
			GServer holdserver = *(GServer *)ArrayNth(serverlist->servers,currentindex);
			holdserver->ping = 9999;//clear the ping so it gets recalculated
			ArrayRemoveAt(serverlist->servers,currentindex);
			ArrayAppend(serverlist->servers,&holdserver);
		} else
		{	//add the aux server
			ServerListAddServer(serverlist, real_ip, (unsigned short)port, querytype);
		}
		
		serverlist->nextupdate = ArrayLength(serverlist->servers) - 1;
		serverlist->abortupdate = 0;

		//chane the mode straight to querying
		ServerListModeChange(serverlist, sl_querying);
		//is it's a sync call, do it until done
		if (!async)
			DoSyncLoop(serverlist);

	}
	else
	{
		//if we're in the middle of an update, we should
		//be able to just slip the aux server in for querying
		//ServerListAddServer(serverlist, ip, port);
		//crt -- make it the next server to be queried
		//note: this should NEVER be called in a different thread from think!!
		if (currentindex == -1) //it doesn't exist yet
			ServerListInsertServer(serverlist, real_ip, (unsigned short)port, serverlist->nextupdate, querytype);
		else 
		{ //it exists, find out whats happening to it
			GServer holdserver = *(GServer *)ArrayNth(serverlist->servers,currentindex);
			if (currentindex >= serverlist->nextupdate) //hasn't been queried yet!
				return 0; //it will be queried soon anyway
			holdserver->ping = 9999;//clear the ping so it gets recalculated
			updateindex = ServerListFindServerInUpdateList(serverlist, holdserver);
			if (updateindex != -1) //is currently being queried, stop it
				serverlist->updatelist[updateindex].currentserver = NULL;
			ArrayInsertAt(serverlist->servers,&holdserver, serverlist->nextupdate); //insert at new place
			ArrayRemoveAt(serverlist->servers,currentindex); //remove the old one
			serverlist->nextupdate--; //decrement the next update, since we are removing a server
		}
	}
	return 0;
}

static GError ServerListLANList(GServerList serverlist)
{
	struct timeval timeout = {0,0};
	fd_set set;
	char indata[1500];
	struct   sockaddr_in saddr;
	int saddrlen = sizeof(saddr);
	int error;

	while (1) //we break if the select fails
	{
		FD_ZERO(&set);
		FD_SET( serverlist->slsocket, &set);
		error = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		if (SOCKET_ERROR == error || 0 == error) //no data
			break;
		error = recvfrom(serverlist->slsocket, indata, sizeof(indata) - 1, 0, (struct sockaddr *)&saddr, &saddrlen );
		if (SOCKET_ERROR == error)
			continue; 
		//we got data, add the server to the list to update
		if (strstr(indata,"\\final\\") != NULL)
			ServerListAddServer(serverlist,saddr.sin_addr.s_addr, ntohs(saddr.sin_port), qt_status);
	}
	if (current_time() - serverlist->lanstarttime > LAN_SEARCH_TIME) //done waiting for replies
	{
		closesocket(serverlist->slsocket);
		serverlist->slsocket = INVALID_SOCKET;
		ServerListModeChange(serverlist, sl_querying);
	}
	return 0;

}
							 
//reads the server list from the socket and parses it
static GError ServerListReadList(GServerList serverlist)
{
	static char data[2048]; //static input buffer
	static int oldlen = 0;
	fd_set set;
	struct timeval timeout = {0,0};
	int len, i;
	char *p;
	unsigned long ip;
	unsigned short port;

	FD_ZERO(&set);
	FD_SET(serverlist->slsocket, &set);
#ifndef KGTRN_ACCESS
	i = select( FD_SETSIZE, &set, NULL, NULL, &timeout );
	if (i <= 0)
		return GE_NOERROR;
#endif

//append to data
	len = recv(serverlist->slsocket, data + oldlen, sizeof(data) - oldlen - 1, 0);
	if (len == SOCKET_ERROR || len == 0)
	{
		closesocket(serverlist->slsocket);
		serverlist->slsocket = INVALID_SOCKET;
		oldlen = 0; //clear data so it can be used again
		ServerListHalt(serverlist);
		ServerListModeChange(serverlist, sl_querying);
		return GE_NOCONNECT;

	}
	oldlen += len;

	p = data;
	while (p - data <= oldlen - 6)
	{
		if (strncmp(p,"\\final\\",7) == 0 || serverlist->abortupdate)
		{
			closesocket(serverlist->slsocket);
			serverlist->slsocket = INVALID_SOCKET;
			oldlen = 0; //clear data so it can be used again
			ServerListModeChange(serverlist, sl_querying);
			return 0; //get out!!
		}
		if (oldlen < 6) //no way it could be a full IP, quit
			break;
		memcpy(&ip,p,4);
		p += 4;
		memcpy(&port,p,2);
		p += 2;
		ServerListAddServer(serverlist,ip,  ntohs(port), serverlist->querytype );
	}
	oldlen = oldlen - (p - data);
	memmove(data,p,oldlen); //shift it over
	return 0;

}

//loop through pending queries and send out new ones

static GError ServerListQueryLoop(GServerList serverlist)
{
	int i, scount = 0, error, final;
	fd_set set;
	struct timeval timeout = {0,0};
	char indata[1500];
	struct sockaddr_in saddr;
	int saddrlen = sizeof(saddr);
	GServer server;

//first, check for available data
	FD_ZERO(&set);
	for (i = 0 ; i < serverlist->maxupdates ; i++)
		if (serverlist->updatelist[i].currentserver != NULL) //there is a server waiting
		{
			scount++;
			FD_SET( serverlist->updatelist[i].s, &set);

		}
	if (scount > 0) //there are sockets to check for data
	{

		error = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
		if (SOCKET_ERROR != error && 0 != error)
			for (i = 0 ; i < serverlist->maxupdates ; i++)
				if (serverlist->updatelist[i].currentserver != NULL && FD_ISSET(serverlist->updatelist[i].s, &set) ) //there is a server waiting
				{ //we can read data!!
					saddrlen = sizeof(saddr);
					error = recvfrom(serverlist->updatelist[i].s, indata, sizeof(indata) - 1, 0,(struct sockaddr *)&saddr, &saddrlen);

					if (saddr.sin_addr.s_addr != serverlist->updatelist[i].saddr.sin_addr.s_addr ||
							saddr.sin_port != serverlist->updatelist[i].saddr.sin_port)
						continue; //it wasn't from this server

					if (SOCKET_ERROR != error) //we got data
					{
						indata[error] = 0; //truncate and parse it
						final = (strstr(indata,"\\final\\") != NULL);
						server = serverlist->updatelist[i].currentserver;
						if (server->ping == 9999) //set the ping
							server->ping = (short)(current_time() - serverlist->updatelist[i].starttime);
						ServerParseKeyVals(server, indata); 
						if (final) //it's all done
						{
							serverlist->CallBackFn(serverlist, 
													LIST_PROGRESS, 
													serverlist->instance,
													server,
													(void *)((serverlist->nextupdate * 100) / ArrayLength(serverlist->servers))); //percent done
							serverlist->updatelist[i].currentserver = NULL; //reuse the updatelist
						} 
					} else
						serverlist->updatelist[i].currentserver = NULL; //reuse the updatelist
					
				}
	}
	//kill expired ones
	for (i = 0 ; i < serverlist->maxupdates ; i++)
		if (serverlist->updatelist[i].currentserver != NULL && current_time() - serverlist->updatelist[i].starttime > SERVER_TIMEOUT ) 
		{
			/* serverlist->CallBackFn(serverlist,  //do we want to notify of dead servers? if so, uncomment!
				LIST_PROGRESS, 
				serverlist->instance,
				*(GServer *)serverlist->updatelist[i].currentserver,
				(void *)((serverlist->nextupdate * 100) / ArrayLength(serverlist->servers))); //percent done
				*/
			serverlist->updatelist[i].currentserver = NULL; //reuse the updatelist
		}
		
	if (serverlist->abortupdate || (serverlist->nextupdate >= ArrayLength(serverlist->servers) && scount == 0)) 
	{ //we are done!!
		FreeUpdateList(serverlist);
		ServerListModeChange(serverlist, sl_idle);
		return 0;
	}
	
//now, send out queries on available sockets
	for (i = 0 ; i < serverlist->maxupdates && serverlist->nextupdate < ArrayLength(serverlist->servers) ; i++)
		if (serverlist->updatelist[i].currentserver == NULL) //it's availalbe
		{
			server = *(GServer *)ArrayNth(serverlist->servers,serverlist->nextupdate);
			serverlist->nextupdate++;
			serverlist->updatelist[i].currentserver = server;
			serverlist->updatelist[i].saddr.sin_family = AF_INET;
			serverlist->updatelist[i].saddr.sin_addr.s_addr = inet_addr(ServerGetAddress(server));
			serverlist->updatelist[i].saddr.sin_port = htons((short)ServerGetQueryPort(server));
			sendto(serverlist->updatelist[i].s,querystrings[server->querytype] ,querylengths[server->querytype],0,(struct sockaddr *)&serverlist->updatelist[i].saddr,sizeof(struct sockaddr_in));
			serverlist->updatelist[i].starttime = current_time();
		}


	return 0;
}

/* ServerListThink
------------------
For use with Async Updates. This needs to be called every ~10ms for list processing and
updating to occur during async server list updates */
GError ServerListThink(GServerList serverlist)
{

	switch (serverlist->state)
	{
		case sl_idle: return 0;
		case sl_listxfer:
				 //read the data
				return ServerListReadList(serverlist);
				break;
		case sl_lanlist:
				return ServerListLANList(serverlist);
		case sl_querying: 
				//do some queries
				return ServerListQueryLoop(serverlist);
				break;
	}


	return 0;
}

/* ServerListHalt
-----------------
Halts the current update batch */
GError ServerListHalt(GServerList serverlist)
{
	if (serverlist->state != sl_idle)
		serverlist->abortupdate = 1;

	return 0;
}

/* ServerListClear
------------------
Clear and free all of the servers from the server list.
List must be in the sl_idle state */
GError ServerListClear(GServerList serverlist)
{
	
	if (serverlist->state != sl_idle)
		return GE_BUSY;
	//fastest way to clear is kill and recreate
	ArrayFree(serverlist->servers);
	serverlist->servers = ArrayNew(sizeof(GServer), SERVER_GROWBY, ServerFree);
	return 0;
}

/* ServerListState
------------------
Returns the current state of the server list */
GServerListState ServerListState(GServerList serverlist)
{
	return serverlist->state;
}

/* ServerListErrorDesc
----------------------
Returns a static string description of the specified error */
char *ServerListErrorDesc(GServerList serverlist, GError error)
{
	switch (error)
	{
	case GE_NOERROR: return "";
	case GE_NOSOCKET: return "Unable to create socket";
	case GE_NODNS: return "Unable to resolve master";
	case GE_NOCONNECT: return "Connection to master reset";
	case GE_BUSY: return "Server List is busy";
	case GE_DATAERROR: return "Unexpected data in server list";
	}
	return "UNKNOWN ERROR CODE";

}

/* ServerListGetServer
----------------------
Returns the server at the specified index, or NULL if the index is out of bounds */
GServer ServerListGetServer(GServerList serverlist, int index)
{
	if (index < 0 || index >= ArrayLength(serverlist->servers))
		return NULL;
	return *(GServer *)ArrayNth(serverlist->servers,index);
}

/* ServerListCount
------------------
Returns the number of servers on the specified list. Indexing is 0 based, so
the actual server indexes are 0 <= valid index < Count */
int ServerListCount(GServerList serverlist)
{
	return ArrayLength(serverlist->servers);
}

/****
Comparision Functions
***/
static int IntKeyCompare(const void *entry1, const void *entry2)
{
	GServer server1 = *(GServer *)entry1, server2 = *(GServer *)entry2;
	int diff;
	diff = ServerGetIntValue(server1, g_sortserverlist->sortkey, 0) - 
			ServerGetIntValue(server2, g_sortserverlist->sortkey, 0);
	if (!g_sortserverlist->sortascending) 
		diff = -diff;
	return diff;
	
}

static int FloatKeyCompare(const void *entry1, const void *entry2)
{
    GServer server1 = *(GServer *)entry1, server2 = *(GServer *)entry2;
	double f = ServerGetFloatValue(server1, g_sortserverlist->sortkey, 0) - 
			ServerGetFloatValue(server2, g_sortserverlist->sortkey, 0);
	if (!g_sortserverlist->sortascending) 
		f = -f;
	if ((float)f > (float)0.0) 
		return 1;
	else if ((float)f < (float)0.0) 
		return -1; 
	else 
		return 0;
}

static int StrCaseKeyCompare(const void *entry1, const void *entry2)
{
	
    GServer server1 = *(GServer *)entry1, server2 = *(GServer *)entry2;
    int diff = strcmp(ServerGetStringValue(server1, g_sortserverlist->sortkey, ""),
				ServerGetStringValue(server2, g_sortserverlist->sortkey, ""));
	if (!g_sortserverlist->sortascending) 
		diff = -diff;
	return diff;
}

static int StrNoCaseKeyCompare(const void *entry1, const void *entry2)
{
	GServer server1 = *(GServer *)entry1, server2 = *(GServer *)entry2;
	int diff = strcasecmp(ServerGetStringValue(server1, g_sortserverlist->sortkey, ""),
				ServerGetStringValue(server2, g_sortserverlist->sortkey, ""));
	if (!g_sortserverlist->sortascending) 
		diff = -diff;
	return diff;
}

/* ServerListSort
-----------------
Sort the server list in either ascending or descending order using the 
specified comparemode.
sortkey can be a normal server key, or "ping" or "hostaddr" */
void ServerListSort(GServerList serverlist, gbool ascending, char *sortkey, GCompareMode comparemode)
{
	ArrayCompareFn comparator;
	switch (comparemode)
	{
	case cm_int: comparator = IntKeyCompare;
		break;
	case cm_float: comparator = FloatKeyCompare;
		break;
	case cm_strcase: comparator = StrCaseKeyCompare;
		break;
	case cm_stricase: comparator = StrNoCaseKeyCompare;
		break;
	}
	serverlist->sortkey = sortkey;
	serverlist->sortascending = ascending;
	g_sortserverlist = serverlist;
	ArraySort(serverlist->servers,comparator);

}



#ifdef __cplusplus
}
#endif

