#ifndef __ASM_H__
#define __ASM_H__

#if defined ( CGAME_DLL )
	#define _CGAME_CODE() if(1)
#else
	#define _CGAME_CODE() if(0)
#endif

#if defined ( WIN32 )
	#define _LINUX_CODE() if(0)
	#define WIN32_CODE() if(1)
#else
	#define _CGAME_CODE() if(0)
	#define _LINUX_CODE() if(1)
	#define WIN32_CODE() if(0)
#endif

#ifdef WIN32
#define NAKED __declspec(naked)
#else
#define NAKED
#endif

#ifdef __GNUC__
#define JMPTO( addr ) asm("leave;mov %0, %%edx;jmp *%%edx;" : : "g"((DWORD)addr) )
#define JMPTO_DIRECT( addr ) asm("movl %0, %%edx;jmp *%%edx;" : : "g"((DWORD)addr) )
#define CALL( addr ) asm("movl %0, %%edx;call *%%edx;" : : "g"((DWORD)addr) ) //{ DWORD address_CALL = (DWORD) addr; __asm__ __volatile__("movl %0, %%edx;call *%%edx;" : : "g"(address_CALL) ); }
#define PUSH( addr ) asm("pushl %0;" : : "g"(*(DWORD *)&addr) ) //{ DWORD address_PUSH = (DWORD) addr; __asm__ __volatile__("push %0;" :: "g"(address_PUSH)); }
#define POP( addr ) asm("popl %0;" : : "g"(*(DWORD *)&addr) ) //{ DWORD address_PUSH = (DWORD) addr; __asm__ __volatile__("push %0;" :: "g"(address_PUSH)); }
#ifndef WIN32
#define SET_CLASS( addr ) asm("movl %0, %%eax; movl %%eax, (%%esp);" : : "g"((DWORD)addr) ) // asm("movl %0, %%ecx;" : : "g"(addr) ) //{ DWORD address_SET_CLASS = (DWORD) addr; __asm__ __volatile__("movl %0, %%ecx;" : : "g"(address_SET_CLASS) ); }
#define GET_CLASS( var ) asm("movl %%esp, %0;" : "=r"(var) )
#else
#define SET_CLASS( addr ) asm("movl %0, %%ecx;" : : "g"((DWORD)addr) ) // asm("movl %0, %%ecx;" : : "g"(addr) ) //{ DWORD address_SET_CLASS = (DWORD) addr; __asm__ __volatile__("movl %0, %%ecx;" : : "g"(address_SET_CLASS) ); }
#define GET_CLASS( var ) asm("movl %%ecx, %0;" : "=r"(var) )
#endif

#define MAKE_JUMPER( declaration, gamex86_address, fgameded_symbol, cgamex86_address ) declaration {} //asm( ".globl " #fgameded_symbol "\n\t" #fgameded_symbol ":\n\tmovl _" #fgameded_symbol "_, %edx;jmp *%edx;" )
#else
#define JMPTO( addr ) { _asm leave _asm mov edx, addr _asm jmp edx }
#define JMPTO_DIRECT( addr ) { _asm mov edx, addr _asm jmp edx }
#define CALL( addr ) { _asm mov edx, addr _asm call edx }
#define PUSH( addr ) { _asm push addr }
#define POP( addr ) { _asm pop addr }
#define SET_CLASS( addr ) { _asm mov ecx, addr }
#define GET_CLASS( var )  { _asm mov var, ecx }

#define MAKE_JUMPER( declaration, gamex86_address, fgameded_symbol, cgamex86_address ) \
	__declspec(naked) declaration { JMPTO_DIRECT_WIN32( gamex86_address) JMPTO_DIRECT_CGAME( cgamex86_address ) }
#endif

#if defined ( _UBER_SDK )
	#define MP_NUMBER( gamex86, fgameded, cgamex86 ) 0
	#define MP_STRING( gamex86, fgameded, cgamex86 ) 0
#endif

#if defined ( CGAME_DLL )
	#define MP_NUMBER( gamex86, fgameded, cgamex86 ) cgamex86
	#define MP_STRING( gamex86, fgameded, cgamex86 ) cgamex86
	#define JMPTO_CGAME(addr) JMPTO(addr)
	#define JMPTO_DIRECT_CGAME(addr) JMPTO_DIRECT(addr)

#if defined( SPEARHEAD ) || defined( SH )
	#define JMPTO_SH_CGAME(addr) JMPTO(addr)
	#define JMPTO_SH_DIRECT_GAME(addr) JMPTO_DIRECT(addr)
#else
	#define JMPTO_SH_CGAME(addr)
	#define JMPTO_SH_DIRECT_GAME(addr)
#endif

#if defined( BREAKTHROUGH ) || defined( BT )
	#define JMPTO_BT_CGAME(addr) JMPTO(addr)
	#define JMPTO_BT_DIRECT_GAME(addr) JMPTO_DIRECT(addr)
#else
	#define JMPTO_BT_CGAME(addr)
	#define JMPTO_BT_DIRECT_GAME(addr)
#endif

	#define CALL_CGAME(addr) CALL(addr)
#else
	#define JMPTO_CGAME(addr)
	#define JMPTO_DIRECT_CGAME(addr)
	#define CALL_CGAME(addr) CALL(addr)
	#define JMPTO_SH_CGAME(addr)
	#define JMPTO_SH_DIRECT_GAME(addr)
	#define JMPTO_BT_CGAME(addr)
	#define JMPTO_BT_DIRECT_GAME(addr)
#endif

#if !defined( SPEARHEAD ) && !defined( SH )
	#define JMPTO_SH_WIN32(addr)
	#define JMPTO_SH_DIRECT_WIN32(addr)
	#define JMPTO_SH_LINUX(addr)
	#define JMPTO_SH_DIRECT_LINUX(addr)
#endif

#if !defined( BREAKTHROUGH ) && !defined( BT )
	#define JMPTO_BT_WIN32(addr)
	#define JMPTO_BT_DIRECT_WIN32(addr)
	#define JMPTO_BT_LINUX(addr)
	#define JMPTO_BT_DIRECT_LINUX(addr)
#endif

#if defined ( WIN32 )

	#if defined( GAME_DLL )
		#define MP_NUMBER( gamex86, fgameded, cgamex86 ) gamex86
		#define MP_STRING( gamex86, fgameded, cgamex86 ) gamex86
		#define CALL_WIN32(addr) CALL(addr)
		#define JMPTO_WIN32(addr) JMPTO(addr)
		#define JMPTO_DIRECT_WIN32(addr) JMPTO_DIRECT(addr)

	#if defined( SPEARHEAD ) || defined( SH )
		#define JMPTO_SH_WIN32(addr) JMPTO(addr)
		#define JMPTO_SH_DIRECT_WIN32(addr) JMPTO_DIRECT(addr)
	#endif

	#if defined( BREAKTHROUGH ) || defined( BT )
		#define JMPTO_BT_WIN32(addr) JMPTO(addr)
		#define JMPTO_BT_DIRECT_WIN32(addr) JMPTO_DIRECT(addr)
	#endif
	#else
		#define CALL_WIN32(addr)
		#define JMPTO_WIN32(addr)
		#define JMPTO_DIRECT_WIN32(addr)
		#define JMPTO_SH_WIN32(addr)
		#define JMPTO_SH_DIRECT_WIN32(addr)
		#define JMPTO_BT_WIN32(addr)
		#define JMPTO_BT_DIRECT_WIN32(addr)
	#endif

	#define JMPTO_LINUX(addr)
	#define JMPTO_DIRECT_LINUX(addr)
	#define JMPTO_SH_LINUX(addr)
	#define JMPTO_SH_DIRECT_LINUX(addr)
	#define JMPTO_BT_LINUX(addr)
	#define JMPTO_BT_DIRECT_LINUX(addr)
	#define CALL_LINUX(addr)
	#define SET_CLASS_LINUX( addr )
	#define SET_CLASS_WIN32( addr ) SET_CLASS( addr )
#else
	#define MP_NUMBER( gamex86, fgameded, cgamex86 ) fgameded
	#define MP_STRING( gamex86, fgameded, cgamex86 ) fgameded
	#define JMPTO_CLIENT(addr)
	#define JMPTO_LINUX(addr) JMPTO(addr)
	#define JMPTO_WIN32(addr)
	#define JMPTO_DIRECT_CLIENT(addr)
	#define JMPTO_DIRECT_LINUX(addr) JMPTO_DIRECT(addr)
	#define JMPTO_DIRECT_WIN32(addr)

	#if defined( SPEARHEAD ) || defined( SH )
		#define JMPTO_SH_LINUX(addr) JMPTO(addr)
		#define JMPTO_SH_DIRECT_LINUX(addr) JMPTO_DIRECT(addr)
	#endif

	#if defined( BREAKTHROUGH ) || defined( BT )
		#define JMPTO_BT_LINUX(addr) JMPTO(addr)
		#define JMPTO_BT_DIRECT_LINUX(addr) JMPTO_DIRECT(addr)
	#endif

	#define CALL_LINUX(addr) CALL(addr)
	#define CALL_WIN32(addr)
	#define SET_CLASS_LINUX( addr ) SET_CLASS( addr )
	#define SET_CLASS_WIN32( addr )
#endif

#define MP_CODE( gamex86, fgameded, cgamex86 ) MP_NUMBER( gamex86, fgameded, cgamex86 )

#ifndef WIN32

extern void* dl_fgame_sym( const char *symbol );

#define SYMBOL_DECLARE( symbol ) DWORD _##symbol##_ = (DWORD)dl_fgame_sym( #symbol ) //void* symbol = dlsym( hmod, #symbol )
#define SYMBOL_GET( symbol ) _##symbol##_
#else
#define SYMBOL_DECLARE( symbol )
#define SYMBOL_GET( symbol ) 0
#endif

#define SYM( symbol ) SYMBOL_GET( symbol )

#endif /* __ASM_H__ */
