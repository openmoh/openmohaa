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

/**
 * The following modifications were made:
 *
 * 1) Non-blocking server list sockets. List requests are asynchronous and won't block the game. The only blocking request could be due to DNS resolution.
 * 2) Server list sockets will timeout after 10 seconds of no reply.
 * 3) Parallel server list requests.
 *  3.1) A pool of sockets is initially created
 *  3.2) The server list connects idling sockets to available masters, afterwards the server list state is set to `sl_listxfer`.
 *  3.3) The server list sends the list request to connected masters, and receives the data for each.
 *  3.4) When finished fetching 1 master (got `\final`), the server list state transitions to `sl_querying`.
 *  3.5) The server list queries all fetched servers. It can still continue fetching list from currently connected masters
 *       but it will not connect to other masters while servers are being queried. It won't be sending more queries if remaining masters replied in under 500 ms.
 *  3.6) When finished querying all fetched servers, the server list restarts from 3.2 and queries next masters.
 *
 * The idea here is to ensure the redundancy of the server list between multiple masters even if the results are combined.
 * Optionally for best results, the game should return a list of masters from the same community.
 */
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
#include "common/gsPlatformSocket.h"
#include "gutil.h"
#include "sv_gqueryreporting.h"
#ifndef UNDER_CE
#include <assert.h>
#else
#define assert(a)
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Custom function used to return the master host, based on game settings
 * 
 * @return const char* The master host
 */
extern unsigned int ServerListGetNumMasters();
extern const char *ServerListGetHost(int index);
extern int ServerListGetMsPort(int index);

//#define MSHOST ServerListGetHost(0)
//#define MSPORT ServerListGetMsPort(0)
#define SERVER_GROWBY 64
#define LAN_SEARCH_TIME 3000 //3 sec
#define LIST_NUMKEYBUCKETS 500
#define LIST_NUMKEYCHAINS 4
#define MAX_INFO_FIELDS 20

// Multi-master settings
#define SERVER_QUERY_MAX_PAUSE 500
#define INTERNET_SEARCH_TIME 10000 //10 sec

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

typedef enum { pi_fieldcount, pi_fields, pi_servers } GParseInfoState;
typedef enum { ls_none, ls_connecting, ls_connected } GListSocketState;

typedef struct GServerListSocketImplementation
{
    SOCKET s;
    GListSocketState socketstate;
    char data[2048];
    int oldlen;
    gsi_time lastreplytime;
    crypt_key cryptkey;
} *GServerListSocket;

struct GServerListImplementation
{
    GServerListState state;
    DArray servers;
    UpdateInfo *updatelist; //dynamic array of updateinfos
    char gamename[32];
    char seckey[32];
    char enginename[32];
    char filter[256];
    int maxupdates;
    int nextupdate;
    int abortupdate;
    ListCallBackFn CallBackFn;
    void *instance;
    char *sortkey;
    gbool sortascending;
    GServerListSocket slsockets;
    int numslsockets;
    unsigned int startslindex;
    gsi_time lastsltime;
    unsigned long lanstarttime;
    GQueryType querytype;
    HashTable keylist;

    // Added in 2.0
    int auxinsertcount;
    // Added in 2.0
    //Gcryptkey cryptkey;
    // Added in OPM
    int encryptdata;
    gbool async;

    GParseInfoState pistate;
};

GServerList g_sortserverlist; //global serverlist for sorting info!!

//PANTS - 10.2.00
char ServerListHostname[64] = "";

/* these correspond to the qt_ constants */
#define NUM_QUERYTYPES 6
const char *querystrings[NUM_QUERYTYPES] = {"\\basic\\","\\info\\","\\rules\\",
                                "\\players\\","\\info\\\\rules\\","\\status\\"};
const int querylengths[NUM_QUERYTYPES] = {7,6,7,9,13,8};

static void KeyValFree(void *elem);
static int KeyValCompareKeyA(const void *entry1, const void *entry2);
static int KeyValHashKeyA(const void *elem, int numbuckets);
static gbool ServerListHasFinishedFetchingList(GServerList serverlist);
static GError ServerListCheckSocketError(GServerList serverlist, GServerListSocket slsocket);
static GError ServerListStartQuery(GServerList serverlist, GServerListSocket slsocket, gbool async);
GError ServerListThinkSocket(GServerList serverlist, GServerListSocket slsocket);

/* ServerListNew
----------------
Creates and returns a new (empty) GServerList. */
GServerList	ServerListNew(const char *gamename, const char *enginename, const char *seckey, int maxconcupdates, void *CallBackFn, int CallBackFnType, void *instance)
{
    GServerList list;

    list = (GServerList) gsimalloc(sizeof(struct GServerListImplementation));
    assert(list != NULL);
    list->state = sl_idle;
    list->servers = ArrayNew(sizeof(GServer), SERVER_GROWBY, ServerFree);
    list->keylist = TableNew2(sizeof(char *),LIST_NUMKEYBUCKETS,LIST_NUMKEYCHAINS,GStringHash, GCaseInsensitiveCompare, GStringFree);
    list->maxupdates = maxconcupdates;
    list->updatelist = gsimalloc(maxconcupdates * sizeof(UpdateInfo));
    memset(list->updatelist, 0, maxconcupdates * sizeof(UpdateInfo));
    assert(list->updatelist != NULL);
    strcpy(list->gamename, gamename);
    strcpy(list->seckey, seckey);
    strcpy(list->enginename, enginename);
    list->CallBackFn = CallBackFn;
    assert(CallBackFn != NULL);
    list->instance = instance;
    list->sortkey = "";
    // Added in 2.0
    list->auxinsertcount = 0;
    // Added in OPM
    list->encryptdata = 1;
    list->async = 0;

    list->numslsockets = maxconcupdates / 4;
    if (list->numslsockets < 1) {
        list->numslsockets = 1;
    } else if (list->numslsockets > 4) {
        // Above 4 seems too much
        list->numslsockets = 4;
    }
    list->slsockets = (GServerListSocket)gsimalloc(sizeof(struct GServerListSocketImplementation) * list->numslsockets);
    memset(list->slsockets, 0, sizeof(struct GServerListSocketImplementation) * list->numslsockets);
    list->startslindex = 0;
    list->lastsltime = current_time();

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
    gsifree(serverlist->updatelist);
    gsifree(serverlist->slsockets);

    gsifree(serverlist);
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

//gsifree update sockets 
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
static GError CreateServerListSocket(GServerList serverlist, GServerListSocket slsocket, gbool async)
{
    struct   sockaddr_in saddr;
    struct hostent *hent;
    int lasterr;
    int port;
    const char *host;

    if (serverlist->startslindex >= ServerListGetNumMasters()) {
        return GE_NODNS;
    }

    if (slsocket->socketstate != ls_none) {
        closesocket(slsocket->s);
        slsocket->s = INVALID_SOCKET;
    }

    port = ServerListGetMsPort(serverlist->startslindex);
    host = ServerListGetHost(serverlist->startslindex);

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port);
    saddr.sin_addr.s_addr = inet_addr(host);
    if (saddr.sin_addr.s_addr == INADDR_NONE)
    {
        hent = gethostbyname(host);
        if (!hent)
            return GE_NODNS; 
        saddr.sin_addr.s_addr = *(u_long *)hent->h_addr_list[0];
    }
    slsocket->s = socket ( AF_INET, SOCK_STREAM, IPPROTO_TCP );
    if (slsocket->s == INVALID_SOCKET)
        return GE_NOSOCKET;

    if (async) {
        SetSockBlocking(slsocket->s, 0);
    }

    if (connect ( slsocket->s, (struct sockaddr *) &saddr, sizeof saddr ) != 0
        && (lasterr = GOAGetLastError(slsocket->slsocket), lasterr != WSAEWOULDBLOCK && lasterr != WSAEINPROGRESS))
    {
        closesocket(slsocket->s);
        slsocket->s = INVALID_SOCKET;
        return GE_NOCONNECT;
    }

    slsocket->socketstate = ls_connecting;
    slsocket->oldlen = 0;
    slsocket->cryptkey.index = -1; //mark as uninitialized
    slsocket->lastreplytime = current_time();
    serverlist->startslindex++;

    //else we are connected
    return 0;


}

//create and connect a server list sockets
static GError CreateServerListForAvailableSockets(GServerList serverlist, gbool async)
{
    int i;

    for(i = 0; i < serverlist->numslsockets; i++) {
        if (serverlist->slsockets[i].socketstate == ls_none) {
            CreateServerListSocket(serverlist, &serverlist->slsockets[i], async);
        }
    }

    return 0;
}


//create and connect a server list socket
static GError CreateServerListLANSocket(GServerList serverlist)
{
    int optval = 1;
    GServerListSocket slsocket = &serverlist->slsockets[0];

    slsocket->s = socket ( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (slsocket->s == INVALID_SOCKET)
        return GE_NOSOCKET;
    if (setsockopt(slsocket->s, SOL_SOCKET, SO_BROADCAST, (char *)&optval, sizeof(optval)) != 0)
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
static GError SendListRequest(GServerList serverlist, GServerListSocket slsocket, char *filter)
{
    char data[256], *ptr, result[64];
    int len;
    int i;
    char *modifier;
    
    len = recv(slsocket->s, data, sizeof(data) - 1, 0);
    if (gsiSocketIsError(len)) {
        return GE_NOCONNECT;
    }
    data[len] = '\0'; //null terminate it
    
    ptr = strstr ( data, SECURE );
    if (!ptr)
        return GE_DATAERROR;
    ptr = ptr + strlen(SECURE);
    cengine_gs_encrypt   ( (uchar *) serverlist->seckey, 6, (uchar *)ptr, 6 );
    if (serverlist->encryptdata) {
        // Added in 2.0
        for(i = 0; i < 6; i++) {
            ptr[i] ^= serverlist->seckey[i];
        }
    }
    cengine_gs_encode ( (uchar *)ptr, 6, (uchar *) result );

    if (serverlist->encryptdata) {
        // Added in 2.0
        //  Encrypt data
        //validate to the master
        sprintf(data, "\\gamename\\%s\\gamever\\%s\\location\\0\\validate\\%s\\enctype\\2\\final\\\\queryid\\1.1\\",
                serverlist->enginename, "2", result); //validate us		
    } else {
        //validate to the master
        sprintf(data, "\\gamename\\%s\\gamever\\%s\\location\\0\\validate\\%s\\final\\\\queryid\\1.1\\",
                serverlist->enginename, ENGINE_VERSION, result); //validate us	
    }
    
    len = send ( slsocket->s, data, strlen(data), 0 );
    if (gsiSocketIsError(len) || len == 0)
        return GE_NOCONNECT;

    if (serverlist->querytype == qt_grouprooms)
    {
        modifier = "groups";
        serverlist->pistate = pi_fieldcount;
    } 
    else if (serverlist->querytype == qt_masterinfo)
    {
        modifier = "info2";
        serverlist->pistate = pi_fieldcount;
    }
    else
        modifier = "cmp";
    //send the list request
    if (filter)
        sprintf(data, "\\list\\%s\\gamename\\%s\\where\\%s\\final\\", modifier, serverlist->gamename, filter);
    else
        sprintf(data, "\\list\\%s\\gamename\\%s\\final\\", modifier, serverlist->gamename);
    len = send ( slsocket->s, data, strlen(data), 0 );
    if (gsiSocketIsError(len) || len == 0)
        return GE_NOCONNECT;

    if (serverlist->state == sl_idle) {
        ServerListModeChange(serverlist, sl_listxfer);
    }
    return 0;
}


static GError SendBroadcastRequest(GServerList serverlist, int startport, int endport, int delta)
{
    struct   sockaddr_in saddr;
    unsigned short i;

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = 0xFFFFFFFF; //broadcast
    for (i = startport ; i <= endport ; i += delta)
    {
        saddr.sin_port = htons(i);
        sendto(serverlist->slsockets[0].s, "\\status\\",8,0,(struct sockaddr *)&saddr,sizeof(saddr));
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
    int i;

    if (serverlist->state != sl_idle)
        return GE_BUSY;

    serverlist->querytype = querytype;
    error = CreateServerListForAvailableSockets(serverlist, async);
    //if (error) return error;

    serverlist->nextupdate = 0;
    serverlist->abortupdate = 0;
    // Added in 2.0
    serverlist->auxinsertcount = ServerListCount(serverlist);
    // Added in 2.0
    //serverlist->cryptkey.index = -1;
    if (filter) {
        strncpy(serverlist->filter, filter, sizeof(serverlist->filter));
    } else {
        serverlist->filter[0] = 0;
    }

    serverlist->async = async;

    if (async) {
        // As it's asynchronous, set the state to transfering
        ServerListModeChange(serverlist, sl_listxfer);
        return 0;
    }

    for(i = 0; i < serverlist->numslsockets; i++) {
        ServerListStartQuery(serverlist, &serverlist->slsockets[i], async);
    }

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


static GServer ServerListAddServerData(GServerList serverlist, char **fieldlist, int fieldcount, char *serverdata, GQueryType qtype)
{
    GServer server;
    server = ServerNewData(fieldlist, fieldcount, serverdata, qtype, serverlist->keylist);
    ArrayAppend(serverlist->servers,&server);
    return server;
}

//add the server to the list with the given ip, port
//removed "static" token so that outsiders can add servers to a list - did not alter .h files (28mar01/bgw)
GServer ServerListAddServer(GServerList serverlist, goa_uint32 ip, unsigned short port, GQueryType qtype)
{
    GServer server;
    
    // 2001.Apr.10.JED - can't add anything to a bogus server list
    assert(serverlist);
    if(!serverlist)
        return NULL;

    server =  ServerNew(ip, port, qtype, serverlist->keylist);
    ArrayAppend(serverlist->servers,&server);
    return server;
}

//add the server to the list with the given ip, port
static GServer ServerListInsertServer(GServerList serverlist, goa_uint32 ip, unsigned short port, int pos, GQueryType qtype)
{
    GServer server;
    server =  ServerNew(ip, port, qtype, serverlist->keylist);
    ArrayInsertAt(serverlist->servers,&server,pos);
    return server;
}


//find the server in the list (up to max), returns -1 if it does not exist
static int ServerListFindServerMax(GServerList serverlist, unsigned int ip, int port, int maxcheck)
{
    int i;
    GServer server;

    for (i = 0; i < maxcheck ; i++)
    {
        server = *(GServer *)ArrayNth(serverlist->servers,i);
        if (port == ServerGetQueryPort(server) && ServerGetInetAddress(server)==ip)
        {
            return i;
        }
    }
    return -1;
}

//find the server in the list, returns -1 if it does not exist
static int ServerListFindServer(GServerList serverlist, unsigned int ip, int port)
{
    return ServerListFindServerMax(serverlist, ip, port, ArrayLength(serverlist->servers));
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
            holdserver->querytype = querytype;
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
        {
            ServerListInsertServer(serverlist, real_ip, (unsigned short)port, serverlist->nextupdate, querytype);
            if (serverlist->state == sl_listxfer || serverlist->state == sl_lanlist) //list is still being xfer'd - make sure it won't add this again!
                serverlist->auxinsertcount++;
        }
        else 
        { //it exists, find out whats happening to it
            GServer holdserver = *(GServer *)ArrayNth(serverlist->servers,currentindex);
            if (currentindex >= serverlist->nextupdate) //hasn't been queried yet!
                return 0; //it will be queried soon anyway
            holdserver->querytype = querytype;
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

static GError ServerListLANList(GServerList serverlist, GServerListSocket slsocket)
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
        FD_SET( slsocket->s, &set);
        error = select(FD_SETSIZE, &set, NULL, NULL, &timeout);
        if (gsiSocketIsError(error) || 0 == error) //no data
            break;
        error = recvfrom(slsocket->s, indata, sizeof(indata) - 1, 0, (struct sockaddr *)&saddr, &saddrlen );
        if (gsiSocketIsError(error))
            continue; 
        indata[sizeof(indata) - 1] = 0;
        //we got data, add the server to the list to update
        if (strstr(indata,"\\final\\") != NULL)
        {
            if (ServerListFindServer(serverlist,saddr.sin_addr.s_addr,ntohs(saddr.sin_port)) == -1)
                ServerListAddServer(serverlist,saddr.sin_addr.s_addr, ntohs(saddr.sin_port), qt_status);
        }
    }
    if (current_time() - serverlist->lanstarttime > LAN_SEARCH_TIME) //done waiting for replies
    {
        closesocket(slsocket->s);
        slsocket->s = INVALID_SOCKET;
        ServerListModeChange(serverlist, sl_querying);
    }
    return 0;

}

//parses a \ delimited string and counts the number of characters to the Nth \ character
// e.g. \test\3\test2\2335 N = 3, returns 8
static int CountSlashOffset(char *data, int len, int slashcount)
{
    char *p = data;
    while (slashcount && p != data + len)
    {
        if (*p == '\\')
            slashcount--;
        p++;		
    }
    if (slashcount == 0)
        return p - data;
    else
        return -1;
}

#define FIELDCOUNT_LENGTH 12 //length of string \fieldcount\ --
//parses a server list with info in it, the format is:
// \fieldcount\N\field1\field2\...\fieldN\server1Field1\server1Field2\...
// \server1FieldN\server2Field1\server2Field2\...\serverXFieldN\final\ ---
static int ServerListParseInfoList(GServerList serverlist, char *data, int len)
{
    static char *fieldlist[MAX_INFO_FIELDS];
    char tempfield[64], *tempptr;
    static int fieldcount;
    int offset;
    int i;
    GServer server;
    
    switch (serverlist->pistate)
    {
    case pi_fieldcount:
        offset = CountSlashOffset(data, len, 3); //looking for \fieldcount\N\ --
        if (offset == -1)
            return 0;
        if (offset < FIELDCOUNT_LENGTH)
            return -1; //the data is incorrect!
        strncpy(tempfield, data + FIELDCOUNT_LENGTH, offset - FIELDCOUNT_LENGTH);
        tempfield[offset - FIELDCOUNT_LENGTH - 1] = 0;
        fieldcount = atoi(tempfield);
        if (fieldcount > MAX_INFO_FIELDS)
            return -1; //this won't work!
        serverlist->pistate = pi_fields;
        for (i = 0 ; i < fieldcount ; i++)
            fieldlist[i] = NULL; //make sure it gets null'd
        return offset - 1;//don't count the slash character..
        break;
    case pi_fields:
        offset = CountSlashOffset(data, len, 2); //looking for \fieldname\ --
        if (offset == -1)
            return 0;
        strncpy(tempfield, data + 1, offset - 2); //copy just "fieldname"
        tempfield[offset-2] = 0;
        tempptr = strdup(tempfield);
        TableEnter(serverlist->keylist,&tempptr);
        //find the place to insert it..
        for (i = 0; i < fieldcount ; i++)
            if (fieldlist[i] == NULL)
            {
                fieldlist[i] = tempptr;
                if (i == fieldcount - 1) //that's the last field, advance us!
                {
                    serverlist->pistate = pi_servers;
                }
                return offset - 1;
            }		
        break;
    case pi_servers:
        offset = CountSlashOffset(data, len, fieldcount+1); //get the next set of fields
        if (offset == -1)
            return 0;
        //now add a server with the data as the string
        server = ServerListAddServerData(serverlist, fieldlist, fieldcount, data, serverlist->querytype);
        data[offset - 1] = '\\'; //make sure the \ char doesn't get nuked insde AddServerData
        serverlist->CallBackFn(serverlist, 
                                LIST_PROGRESS, 
                                serverlist->instance,
                                server,
                                0); //percent done
                                
        return offset - 1;
        break;
    }
    return -1; //bad state!
}

//reads the server list from the socket and parses it
static GError ServerListReadList(GServerList serverlist, GServerListSocket slsocket)
{
    //static char data[2048]; //static input buffer
    //static int oldlen = 0;
    fd_set set;
    struct timeval timeout = {0,0};
    int len, i;
    char *p;
    unsigned long ip;
    unsigned short port;

    FD_ZERO(&set);
    FD_SET(slsocket->s, &set);
#ifndef KGTRN_ACCESS
    i = select( FD_SETSIZE, &set, NULL, NULL, &timeout );
    if (i <= 0)
        return GE_NOERROR;
#endif

//append to data
    len = recv(slsocket->s, slsocket->data + slsocket->oldlen, sizeof(slsocket->data) - slsocket->oldlen - 1, 0);
    if (gsiSocketIsError(len) || len == 0)
    {
        closesocket(slsocket->s);
        slsocket->s = INVALID_SOCKET;
        slsocket->socketstate = ls_none;
        slsocket->oldlen = 0; //clear data so it can be used again
        //ServerListHalt(serverlist);
        ServerListModeChange(serverlist, sl_querying);
        return GE_NOCONNECT;

    }

    slsocket->lastreplytime = current_time();
    serverlist->lastsltime = slsocket->lastreplytime;

    if (serverlist->encryptdata && slsocket->cryptkey.index != -1)
    {
        // Added in 2.0
        crypt_docrypt(&slsocket->cryptkey, slsocket->data + slsocket->oldlen, len);
    }

    slsocket->oldlen += len;

    p = slsocket->data;

    if (!serverlist->encryptdata) {
        slsocket->cryptkey.index = 0;
    } else if (slsocket->cryptkey.index == -1) {
        // Added in 2.0
        if (slsocket->oldlen > (p[0] ^ 0xEC)) //the key length
        {
            //xor our key into the buffer
            p[0] ^= 0xEC;
            len = strlen(serverlist->seckey);
            for (i = 0 ; i < len ; i++)
            {
                p[1 + i] ^= serverlist->seckey[i];
            }
            //init the key
            init_crypt_key((unsigned char *)(p + 1), p[0], &slsocket->cryptkey);
            p += (p[0] + 1); //advance the data pointer
            //decrypt any remaining data
            crypt_docrypt(&slsocket->cryptkey, p, slsocket->oldlen - (p - slsocket->data));
        }
    }

    if (slsocket->cryptkey.index != -1) //try to read some data
    {
        while (p - slsocket->data <= slsocket->oldlen - 6)
        {
            if (strncmp(p,"\\final\\",7) == 0 || serverlist->abortupdate)
            {
                closesocket(slsocket->s);
                slsocket->s = INVALID_SOCKET;
                slsocket->socketstate = ls_none;
                slsocket->oldlen = 0; //clear data so it can be used again
                if (serverlist->querytype == qt_grouprooms || serverlist->querytype == qt_masterinfo)
                    ServerListModeChange(serverlist, sl_idle); //go idle if we are just getting a GR list
                else
                    ServerListModeChange(serverlist, sl_querying);
                return 0; //get out!!
            }
            if (slsocket->oldlen < 6) //no way it could be a full IP, quit
                break;
            if (serverlist->querytype == qt_grouprooms || serverlist->querytype == qt_masterinfo) {
                i = ServerListParseInfoList(serverlist, p,slsocket->oldlen - (p - slsocket->data));
                if (i < 0) //the data was in a bad format, abort!
                    serverlist->abortupdate = 1;
                else if (i == 0) //not enough info yet, read more!
                    break;
                else
                    p += i;
            } else
            {
                int currentindex;
                memcpy(&ip, p, 4);
                p += 4;
                memcpy(&port, p, 2);
                p += 2;
                // Added in 2.0
                //  Skip adding the server if already exists
                //currentindex = ServerListFindServerMax(serverlist,ip,ntohs(port), serverlist->auxinsertcount);
                // Fixed in OPM
                //  Use the entire array instead
                currentindex = ServerListFindServer(serverlist,ip,ntohs(port));
                if (currentindex == -1)
                    ServerListAddServer(serverlist,ip,  ntohs(port), serverlist->querytype );	
            }
        }
    }
    slsocket->oldlen = slsocket->oldlen - (p - slsocket->data);
    memmove(slsocket->data,p,slsocket->oldlen); //shift it over
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
        if (!gsiSocketIsError(error) && 0 != error)
            for (i = 0 ; i < serverlist->maxupdates ; i++)
                if (serverlist->updatelist[i].currentserver != NULL && FD_ISSET(serverlist->updatelist[i].s, &set) ) //there is a server waiting
                { //we can read data!!
                    saddrlen = sizeof(saddr);
                    error = recvfrom(serverlist->updatelist[i].s, indata, sizeof(indata) - 1, 0,(struct sockaddr *)&saddr, &saddrlen);

                    if (saddr.sin_addr.s_addr != serverlist->updatelist[i].saddr.sin_addr.s_addr ||
                            saddr.sin_port != serverlist->updatelist[i].saddr.sin_port)
                        continue; //it wasn't from this server

                    if (!gsiSocketIsError(error)) //we got data
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
        if (serverlist->abortupdate) {
            ServerListModeChange(serverlist, sl_idle);
            return 0;
        }

        if (serverlist->startslindex < ServerListGetNumMasters() || !ServerListHasFinishedFetchingList(serverlist)) {
            ServerListModeChange(serverlist, sl_listxfer);
        } else {
            // No more masters
            ServerListModeChange(serverlist, sl_idle);
        }
        return 0;
    }

    if (!ServerListHasFinishedFetchingList(serverlist) && current_time() < serverlist->lastsltime + SERVER_QUERY_MAX_PAUSE) {
        // Make sure to not send out other queries if currently fetching from other lists
        // to avoid overloading the network
        return GE_BUSY;
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
    int i;

    for(i = 0; i < serverlist->numslsockets; i++) {
        ServerListThinkSocket(serverlist, &serverlist->slsockets[i]);
    }

    switch(serverlist->state)
    {
        case sl_idle:
        case sl_listxfer:
            if (!serverlist->abortupdate && serverlist->startslindex < ServerListGetNumMasters()) {
                CreateServerListForAvailableSockets(serverlist, serverlist->async);
            } else if (serverlist->state == sl_listxfer && ServerListHasFinishedFetchingList(serverlist)) {
                ServerListModeChange(serverlist, sl_idle);
            }
            break;
        case sl_querying: 
            //do some queries
            return ServerListQueryLoop(serverlist);
        break;
        default: break;
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
Clear and gsifree all of the servers from the server list.
List must be in the sl_idle state */
GError ServerListClear(GServerList serverlist)
{
    
    if (serverlist->state != sl_idle)
        return GE_BUSY;
    //fastest way to clear is kill and recreate
    ArrayFree(serverlist->servers);
    serverlist->servers = ArrayNew(sizeof(GServer), SERVER_GROWBY, ServerFree);
    TableFree(serverlist->keylist);
    serverlist->keylist = TableNew2(sizeof(char *),LIST_NUMKEYBUCKETS,LIST_NUMKEYCHAINS,GStringHash, GCaseInsensitiveCompare, GStringFree);
    serverlist->auxinsertcount = 0;
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
    default: 
        comparator = StrNoCaseKeyCompare;
    }
    serverlist->sortkey = sortkey;
    serverlist->sortascending = ascending;
    g_sortserverlist = serverlist;
    ArraySort(serverlist->servers,comparator);

}




/* ServerListCheckSocketError
-------------------------
Verify if the server list socket has an error or has timed out, and abort. */
static GError ServerListCheckSocketError(GServerList serverlist, GServerListSocket slsocket)
{
    int so_error;
    int len = sizeof(so_error);
    getsockopt(slsocket->s, SOL_SOCKET, SO_ERROR, (void*)&so_error, &len);

    // Abort if the socket has an error or if it connect/recv has timed out
    if (so_error || current_time() >= slsocket->lastreplytime + INTERNET_SEARCH_TIME) {
        closesocket(slsocket->s);
        slsocket->s = INVALID_SOCKET;
        slsocket->socketstate = ls_none;
        return GE_NOCONNECT;
    }

    return 0;
}

/* ServerListStartQuery
-------------------------
Start querying a GServerList. */
static GError ServerListStartQuery(GServerList serverlist, GServerListSocket slsocket, gbool async)
{
    GError error;
    int i;

    if (slsocket->socketstate != ls_connecting) {
        return GE_BUSY;
    }

    if (serverlist->abortupdate) {
        for(i = 0; i < serverlist->numslsockets; i++) {
            closesocket(serverlist->slsockets[i].s);
            serverlist->slsockets[i].s = INVALID_SOCKET;
        }
        ServerListModeChange(serverlist, sl_idle);
        return GE_NOCONNECT;
    }

    if (async) {
        // Check for pending data
        char buf;
        if (recv(slsocket->s, &buf, 1, MSG_PEEK) == -1) {
            return GE_BUSY;
        }
    }

    slsocket->socketstate = ls_connected;

    error = SendListRequest(serverlist, slsocket, serverlist->filter);
    if (error) return error;
    if (serverlist->querytype != qt_grouprooms && serverlist->querytype != qt_masterinfo) {
        error = InitUpdateList(serverlist);
        if (error) return error;
    }

    if (!async)
        DoSyncLoop(serverlist);

    return 0;
}

/* ServerListHasFinishedFetchingList
------------------
Whether or not the list has finished fetching */
static gbool ServerListHasFinishedFetchingList(GServerList serverlist)
{
    int i;

    for(i = 0; i < serverlist->numslsockets; i++) {
        if (serverlist->slsockets[i].s != INVALID_SOCKET) {
            return 0;
        }
    }

    return 1;
}

/* ServerListThinkSocket
------------------
For use with Async Updates. This needs to be called every ~10ms for list processing and
updating to occur during async server list updates */
GError ServerListThinkSocket(GServerList serverlist, GServerListSocket slsocket)
{
    GError socketerror;

    if (slsocket->s == INVALID_SOCKET) {
        return GE_NOSOCKET;
    }

    socketerror = ServerListCheckSocketError(serverlist, slsocket);
    if (socketerror) {
        return socketerror;
    }

    switch(slsocket->socketstate)
    {
        case ls_connecting:
                ServerListStartQuery(serverlist, slsocket, 1);
                break;
        case ls_connected:
                //read the data
                return ServerListReadList(serverlist, slsocket);
                break;
        default:
                break;
    }

    switch (serverlist->state)
    {
        case sl_idle: return 0;
        case sl_listxfer:
                 //read the data
                //return ServerListReadList(serverlist, slsocket);
                break;
        case sl_lanlist:
                return ServerListLANList(serverlist, slsocket);
        default:
                break;
    }

    return 0;
}

#ifdef __cplusplus
}
#endif

