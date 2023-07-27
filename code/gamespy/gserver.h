/******
gserver.h
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

******/

#include "goaceng.h"

#ifndef _GSERVER_H_
#define _GSERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__)
	#include "::hashtable.h"
#else
	#include "../hashtable.h"
#endif

struct GServerImplementation
{
	unsigned long ip;
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
GServer ServerNew(unsigned long ip, unsigned short port, GQueryType qtype, HashTable keylist);
void ServerParseKeyVals(GServer server, char *keyvals);
int GStringHash(const void *elem, int numbuckets);
int GCaseInsensitiveCompare(const void *entry1, const void *entry2);
void GStringFree(void *elem);
#ifdef __cplusplus
}
#endif

#endif