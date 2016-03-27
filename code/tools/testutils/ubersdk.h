#ifndef __UBERSDK_H__
#define __UBERSDK_H__

#ifdef WIN32
#include <Windows.h>
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#endif

#undef STRING_NONE

#include <console.h>

extern ConsoleInput *g_console;

#endif
