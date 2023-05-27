file(GLOB_RECURSE SHADER_GLSL_SRCS "code/renderergl2/glsl/*.glsl")

add_executable(stringify "code/tools/stringify.cpp")

foreach (shader ${SHADER_GLSL_SRCS})
	get_filename_component(shaderfile ${shader} NAME_WE)
	list(APPEND SHADER_SRCS ${CMAKE_CURRENT_BINARY_DIR}/${shaderfile}.c)
	set_source_files_properties(${shaderfile}.c PROPERTIES GENERATED TRUE)
	add_custom_command(OUTPUT ${shaderfile}.c
		DEPENDS stringify ${shader}
		COMMAND stringify ${shader} ${CMAKE_CURRENT_BINARY_DIR}/${shaderfile}.c
		COMMENT "Generate c source for ${shader}: ${shaderfile}.c"
	)
endforeach()
