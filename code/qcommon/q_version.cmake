add_library(qcommon_version INTERFACE)

# Current branch
execute_process(
	COMMAND git branch --show-current
	OUTPUT_VARIABLE GIT_BRANCH_NAME
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Revision
execute_process(
	COMMAND git show -s --format=%H HEAD
	OUTPUT_VARIABLE GIT_REVISION_HASH
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Commit date
execute_process(
	COMMAND git show -s --format=%ct
	OUTPUT_VARIABLE GIT_REVISION_DATE
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

string(TIMESTAMP GIT_REVISION_DATE UTC)

set(GIT_REVISION_DATE_HUMAN ${GIT_REVISION_DATE})
string(TIMESTAMP GIT_REVISION_DATE_HUMAN "%b %d %Y" UTC)

configure_file("q_version.generated.h.in" "generated/q_version.generated.h")
target_include_directories(qcommon_version INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/generated")
