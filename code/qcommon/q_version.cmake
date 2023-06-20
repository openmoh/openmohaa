add_library(qcommon_version INTERFACE)

# Current branch
execute_process(
	COMMAND git branch --show-current
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_BRANCH_NAME
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Revision
execute_process(
	COMMAND git show -s --format=%H HEAD
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_REVISION_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Commit date
execute_process(
	COMMAND git show -s --format=%ct
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	OUTPUT_VARIABLE GIT_REVISION_DATE
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(TIMESTAMP GIT_REVISION_DATE_TIME UTC)

set(GIT_REVISION_DATE ${GIT_REVISION_DATE_TIME})
string(TIMESTAMP GIT_REVISION_DATE "%b %d %Y UTC%z" UTC)

if(NOT GIT_REVISION_BUILD_NUMBER)
	set(GIT_REVISION_BUILD_NUMBER 0)
endif()

configure_file("q_version.generated.h.in" "generated/q_version.generated.h")
target_include_directories(qcommon_version INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/generated")
