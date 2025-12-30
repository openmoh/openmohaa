# Tracy profiler
option(TRACY_ENABLE "Enable the Tracy profiler" OFF)

# Make sure we build Tracy as objects with LTO that we can then link into a shared library.
set(TRACY_LTO ON)
add_subdirectory(${SOURCE_DIR}/thirdparty/tracy)

# In accordance with section 2.1.8, "Setup for multi-DLL projects" of the Tracy manual,
# we wrap TracyClient into a shared library that we then depend on.
if(TRACY_ENABLE)
	add_library(tracy SHARED ${SOURCE_DIR}/sys/sys_tracy.cpp)
	# Include Tracy as a private dependency so that we can control the public interface of the DLL.
	target_link_libraries(tracy PRIVATE Tracy::TracyClient)
	target_include_directories(tracy SYSTEM PUBLIC ${SOURCE_DIR}/thirdparty/tracy/public)
	# Copy over the public compile definitions from the TracyClient target - it's already set up for MSVC imports, for example.
	get_target_property(TRACY_DEFS Tracy::TracyClient INTERFACE_COMPILE_DEFINITIONS)
	target_compile_definitions(tracy PUBLIC ${TRACY_DEFS})
	INSTALL(TARGETS tracy DESTINATION ${INSTALL_LIBDIR_FULL})
else()
	# Even when disabled, we still want the includes available so that empty Tracy macros are declared.
	add_library(tracy INTERFACE)
	target_include_directories(tracy SYSTEM INTERFACE ${SOURCE_DIR}/thirdparty/tracy/public)
endif()

list(APPEND COMMON_LIBRARIES tracy)
