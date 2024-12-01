# Tracy profiler
option(TRACY_ENABLE "Enable the Tracy profiler" OFF)
option(TRACY_ON_DEMAND "Enable profiling only when there’s an established connection with the server" OFF)
add_subdirectory(${SOURCE_DIR}/thirdparty/tracy)

if(TRACY_ENABLE)
	list(APPEND COMMON_LIBRARIES Tracy::TracyClient)
endif(TRACY_ENABLE)
