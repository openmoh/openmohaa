/*
** This file contains glue linked into the "shared" object
*/

#include "dll.h"

unsigned long __stack = 4 * 1024 * 1024; // yep !

extern void *GetCGameAPI(void *);

dll_tExportSymbol DLL_ExportSymbols[] =
{
    {(void *)GetCGameAPI, "GetCGameAPI"},
    {0,0}
};

dll_tImportSymbol DLL_ImportSymbols[] =
{
    {0,0,0,0}
};

int DLL_Init(void)
{
    return 1;
}

void DLL_DeInit(void)
{
}
