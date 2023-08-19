cmake_minimum_required(VERSION 3.5)
project(omohsdl)

file(GLOB_RECURSE SOURCES_SDL "./*.c")

add_library(omohsdl STATIC ${SOURCES_SDL})
target_compile_features(omohsdl PUBLIC c_variadic_macros)
target_link_libraries(omohsdl PRIVATE qcommon qcommon_standalone)


if(${CMAKE_VERSION} VERSION_GREATER "3.11")
	cmake_policy(SET CMP0074 NEW)
endif()

if(WIN32)
	find_package(SDL2)

	if (SDL2_FOUND)
		string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
		target_include_directories(omohsdl PUBLIC ${SDL2_INCLUDE_DIRS})
		target_link_libraries(omohsdl PRIVATE ${SDL2_LIBRARIES})
	else()
		message(WARNING "SDL2 not found, falling back to using SDL2 from the source tree")

		target_include_directories(omohsdl PUBLIC "../SDL2/include-2.0.22")

		if (MSVC)
			if(CMAKE_SIZEOF_VOID_P EQUAL 8)
				add_library(sdl2 SHARED IMPORTED)
				set_target_properties(sdl2 PROPERTIES
				  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/code/libs/win64/SDL264.lib"
				)

				add_library(sdl2main SHARED IMPORTED)
				set_target_properties(sdl2main PROPERTIES
				  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/code/libs/win64/SDL264main.lib"
				)
			else()
				add_library(sdl2 SHARED IMPORTED)
				set_target_properties(sdl2 PROPERTIES
				  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/code/libs/win32/SDL2.lib"
				)

				add_library(sdl2main SHARED IMPORTED)
				set_target_properties(sdl2main PROPERTIES
				  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/code/libs/win32/SDL2main.lib"
				)
			endif()

			target_link_libraries(omohsdl PRIVATE sdl2 sdl2main)
		endif()
	endif()

elseif(UNIX)
	find_package(SDL2 REQUIRED)
	
	if (SDL2_FOUND)
		string(STRIP ${SDL2_LIBRARIES} SDL2_LIBRARIES)
		target_include_directories(omohsdl PUBLIC ${SDL2_INCLUDE_DIRS})
		target_link_libraries(omohsdl PRIVATE ${SDL2_LIBRARIES})
	endif()
endif()
