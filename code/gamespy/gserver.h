/******
gserver.h
GameSpy C Engine SDK
  
Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

******

 Please see the GameSpy C Engine SDK documentation for more 
 information

******/

#include "goaceng.h"

#ifndef _GSERVER_H_
#define _GSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__) && !defined(__mips64)
    #include "::hashtable.h"
    #include "::nonport.h"
#else
    #include "../hashtable.h"
    #include "../nonport.h"
#endif

struct GServerImplementation
{
    goa_uint32 ip;
    unsigned short port;
    short ping;
    GQueryType querytype;
    HashTable keyvals;
    HashTable keylist;

};

typedef struct
{
    char *key;
    char *value;
} GKeyValuePair;
/*
typedef struct
{
    char *key, *value;
} GKeyValuePair;
*/
typedef struct 
{
    KeyEnumFn EnumFn;
    void *instance;
    HashTable keylist;
} GEnumData;

void ServerFree(void *elem);
GServer ServerNew(goa_uint32 ip, unsigned short port, GQueryType qtype, HashTable keylist);
GServer ServerNewData(char **fieldlist, int fieldcount, char *serverdata, GQueryType qtype, HashTable keylist);
void ServerParseKeyVals(GServer server, char *keyvals);
int GStringHash(const void *elem, int numbuckets);
int GCaseInsensitiveCompare(const void *entry1, const void *entry2);
void GStringFree(void *elem);
#ifdef __cplusplus
}
#endif

#endif