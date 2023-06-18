set(archdetect_c_code "
#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
#error cmake_ARCH x86
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
#error cmake_ARCH x86_64
#elif defined(__ppc64__) || defined(__powerpc64__)
#error cmake_ARCH ppc64
#elif defined(__ppc__) || defined(__ppc) || defined(__powerpc__)
#error cmake_ARCH ppc
#elif defined __s390__
#error cmake_ARCH s390
#elif defined __s390x__
#error cmake_ARCH s390x
#elif defined __ia64__
#error cmake_ARCH ia64
#elif defined __alpha__
#error cmake_ARCH alpha
#elif defined __sparc__
#error cmake_ARCH sparc
#elif defined __arm__ || defined (_M_ARM)
#error cmake_ARCH arm
#elif defined(__aarch64__) || defined(__ARM64__) || defined(_M_ARM64)
#error cmake_ARCH aarch64
#elif defined __cris__
#error cmake_ARCH cris
#elif defined __hppa__
#error cmake_ARCH hppa
#elif defined __mips__
#error cmake_ARCH mips
#elif defined __sh__
#error cmake_ARCH sh
#else
#error cmake_ARCH unknown
#endif

")

# Set ppc_support to TRUE before including this file or ppc and ppc64
# will be treated as invalid architectures since they are no longer supported by Apple

function(target_architecture output_var)
    if((APPLE AND DEFINED CMAKE_OSX_ARCHITECTURES) AND NOT "${CMAKE_OSX_ARCHITECTURES}" STREQUAL "")
        # On OS X we use CMAKE_OSX_ARCHITECTURES *if* it was set
        # First let's normalize the order of the values

        # Note that it's not possible to compile PowerPC applications if you are using
        # the OS X SDK version 10.6 or later - you'll need 10.4/10.5 for that, so we
        # disable it by default
        # See this page for more information:
        # http://stackoverflow.com/questions/5333490/how-can-we-restore-ppc-ppc64-as-well-as-full-10-4-10-5-sdk-support-to-xcode-4

        # Architecture defaults to i386 or ppc on OS X 10.5 and earlier, depending on the CPU type detected at runtime.
        # On OS X 10.6+ the default is x86_64 if the CPU supports it, i386 otherwise.

        foreach(osx_arch ${CMAKE_OSX_ARCHITECTURES})
            if("${osx_arch}" STREQUAL "ppc" AND ppc_support)
                set(osx_arch_ppc TRUE)
            elseif("${osx_arch}" STREQUAL "i386")
                set(osx_arch_i386 TRUE)
            elseif("${osx_arch}" STREQUAL "x86_64")
                set(osx_arch_x86_64 TRUE)
            elseif("${osx_arch}" STREQUAL "ppc64" AND ppc_support)
                set(osx_arch_ppc64 TRUE)
            else()
                message(FATAL_ERROR "Invalid OS X arch name: ${osx_arch}")
            endif()
        endforeach()

        # Now add all the architectures in our normalized order
        if(osx_arch_ppc)
            list(APPEND ARCH ppc)
        endif()

        if(osx_arch_i386)
            list(APPEND ARCH i386)
        endif()

        if(osx_arch_x86_64)
            list(APPEND ARCH x86_64)
        endif()

        if(osx_arch_ppc64)
            list(APPEND ARCH ppc64)
        endif()
    else()
    	# figure out extension for the source file
		if(CMAKE_C_COMPILER_LOADED)
			set(TA_EXTENSION "c")
		elseif(CMAKE_CXX_COMPILER_LOADED)
			set(TA_EXTENSION "cpp")
		elseif(CMAKE_FORTRAN_COMPILER_LOADED)
			set(TA_EXTENSION "F90")
		else()
			message(FATAL_ERROR "You must enable a C, CXX, or Fortran compiler to use TargetArch.cmake")
		endif()
		
        file(WRITE "${CMAKE_BINARY_DIR}/arch.${TA_EXTENSION}" "${archdetect_c_code}")

        # Detect the architecture in a rather creative way...
        # This compiles a small C program which is a series of ifdefs that selects a
        # particular #error preprocessor directive whose message string contains the
        # target architecture. The program will always fail to compile (both because
        # file is not a valid C program, and obviously because of the presence of the
        # #error preprocessor directives... but by exploiting the preprocessor in this
        # way, we can detect the correct target architecture even when cross-compiling,
        # since the program itself never needs to be run (only the compiler/preprocessor)
        
        if(DEFINED CMAKE_OSX_ARCHITECTURES)
        	set(TA_CMAKE_FLAGS CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES})
        else()
        	set(TA_CMAKE_FLAGS "")
    	endif()
        try_run(
            run_result_unused
            compile_result_unused
            "${CMAKE_BINARY_DIR}"
            "${CMAKE_BINARY_DIR}/arch.${TA_EXTENSION}"
            COMPILE_OUTPUT_VARIABLE ARCH
            CMAKE_FLAGS ${TA_CMAKE_FLAGS}
        )

        # Parse the architecture name from the compiler output
        string(REGEX MATCH "cmake_ARCH ([a-zA-Z0-9_]+)" ARCH "${ARCH}")

        # Get rid of the value marker leaving just the architecture name
        string(REPLACE "cmake_ARCH " "" ARCH "${ARCH}")

        # If we are compiling with an unknown architecture this variable should
        # already be set to "unknown" but in the case that it's empty (i.e. due
        # to a typo in the code), then set it to unknown
        if (NOT ARCH)
            set(ARCH unknown)
        endif()
    endif()

    set(${output_var} "${ARCH}" PARENT_SCOPE)
endfunction()

