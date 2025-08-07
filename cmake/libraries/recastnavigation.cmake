# Recast navigation
add_subdirectory(${SOURCE_DIR}/thirdparty/recastnavigation/Detour recastnav_detour)
add_subdirectory(${SOURCE_DIR}/thirdparty/recastnavigation/DetourCrowd recastnav_crowd)
add_subdirectory(${SOURCE_DIR}/thirdparty/recastnavigation/Recast recacstnav_recast)
# Enable position independant code on recast navigation libraries.
# Otherwise linking will fail relocating some functions on Linux
set_property(TARGET Detour DetourCrowd Recast PROPERTY POSITION_INDEPENDENT_CODE ON)

set_target_properties(Detour DetourCrowd Recast PROPERTIES LINKER_LANGUAGE CXX)
