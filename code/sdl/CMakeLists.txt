cmake_minimum_required(VERSION 3.12)

project(omohsdl)

set(SOURCES_SDL_CLIENT
"./sdl_input.c"
)

set(SOURCES_SDL_GL
"./sdl_gamma.c"
"./sdl_glimp.c"
)

if (NO_MODERN_DMA)
	list(APPEND SOURCES_SDL "./sdl_snd.c")
endif()

add_library(omohsdl_client STATIC ${SOURCES_SDL_CLIENT})
target_compile_features(omohsdl_client PUBLIC c_variadic_macros)
target_link_libraries(omohsdl_client PRIVATE qcommon qcommon_standalone)

if (NO_MODERN_DMA)
	target_compile_definitions(omohsdl_client PRIVATE NO_MODERN_DMA=1)
endif()

add_library(omohsdl_gl STATIC ${SOURCES_SDL_GL})
target_link_libraries(omohsdl_gl PRIVATE qcommon)

if(${CMAKE_VERSION} VERSION_GREATER "3.11")
	cmake_policy(SET CMP0074 NEW)
endif()

add_library(sdllib INTERFACE)

if(WIN32)
	find_package(SDL2)

	if (SDL2_FOUND)
		string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
		target_include_directories(sdllib INTERFACE ${SDL2_INCLUDE_DIRS})
		target_link_libraries(sdllib INTERFACE ${SDL2_LIBRARIES})
	else()
		message(WARNING "SDL2 not found, falling back to using SDL2 from the source tree")

		target_include_directories(sdllib INTERFACE "../SDL2/include-2.0.22")

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

			target_link_libraries(sdllib INTERFACE sdl2 sdl2main)
		endif()
	endif()

elseif(UNIX)
	find_package(SDL2 REQUIRED)
	
	if (SDL2_FOUND)
		string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)
		target_include_directories(sdllib INTERFACE ${SDL2_INCLUDE_DIRS})
		target_link_libraries(sdllib INTERFACE ${SDL2_LIBRARIES})
	endif()
endif()

target_link_libraries(omohsdl_client PUBLIC sdllib)
target_link_libraries(omohsdl_gl PUBLIC sdllib)
