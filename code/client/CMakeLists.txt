cmake_minimum_required(VERSION 3.5)
project(omohclient)

add_subdirectory("../cgame" "./cgame")

file(GLOB_RECURSE SOURCES_CLIENT "./*.c*")
file(GLOB_RECURSE SOURCES_UILIB "../uilib/*.c*")

# Made as an interface and not static, as static only links used methods
add_library(omohclient INTERFACE)
target_compile_definitions(omohclient INTERFACE APP_MODULE)
target_compile_definitions(omohclient INTERFACE USE_OPENAL_DLOPEN=1)
target_compile_features(omohclient INTERFACE cxx_nullptr)
target_compile_features(omohclient INTERFACE c_variadic_macros)
target_link_libraries(omohclient INTERFACE omohsdl)
target_link_libraries(omohclient INTERFACE gcd)

# Sound stuff
target_compile_definitions(omohclient INTERFACE USE_CODEC_MP3)

if (USE_SOUND_NEW)
	# Try to use OpenAL
	find_package(OpenAL REQUIRED)
	if (OPENAL_FOUND)
		target_compile_definitions(omohclient INTERFACE USE_OPENAL=1 USE_SOUND_NEW=1)
		if (MSVC)
			target_include_directories(omohclient INTERFACE ${OPENAL_INCLUDE_DIR}/AL)
		else()
			target_include_directories(omohclient INTERFACE ${OPENAL_INCLUDE_DIR})
		endif()
		target_link_libraries(omohclient INTERFACE ${OPENAL_LIBRARY})
	endif()

	list(FILTER SOURCES_CLIENT EXCLUDE REGEX "./snd_([a-zA-Z0-9_]+)\.c$")
	file(GLOB_RECURSE SOURCES_CLIENT_SND "./snd_*_new.c*" "./snd_codec*.c*")
	list(APPEND SOURCES_CLIENT ${SOURCES_CLIENT_SND})

	set(OLD_VALUE ${BUILD_SHARED_LIBS})
	set(BUILD_SHARED_LIBS OFF)
	add_subdirectory("../libmad" "./libmad")
	set(BUILD_SHARED_LIBS ${OLD_VALUE})
	target_link_libraries(omohclient INTERFACE mad)
else()
	add_subdirectory("../libmad-0.15.1b" "./libmad")
	target_link_libraries(omohclient INTERFACE libmad)
endif()

if (USE_OPENAL)
	# Try to use OpenAL
	find_package(OpenAL)
	if (OPENAL_FOUND)
		target_compile_definitions(omohclient INTERFACE USE_OPENAL=1)
		target_include_directories(omohclient INTERFACE ${OPENAL_INCLUDE_DIR}/AL)
		target_link_libraries(omohclient INTERFACE ${OPENAL_LIBRARY})
	endif()
endif()

target_sources(omohclient INTERFACE ${SOURCES_CLIENT} ${SOURCES_UILIB})
