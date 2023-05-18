cmake_minimum_required(VERSION 3.5)

set(CURRENT_DIR "code/gamespy")

include_directories("${CURRENT_DIR}/common")
include_directories("${CURRENT_DIR}/gcdkey")

file(GLOB SRCS_common
	"${CURRENT_DIR}/common/*.c"
    "${CURRENT_DIR}/darray.c"
	"${CURRENT_DIR}/hashtable.c"
	"${CURRENT_DIR}/md5c.c"
	"${CURRENT_DIR}/gutil.c"
)

file(GLOB SRCS_gcdkey
"${CURRENT_DIR}/gcdkey/*.c"
)

file(GLOB_RECURSE SRCS_ghttp
"${CURRENT_DIR}/gcdkey/*.c"
)

file(GLOB SRCS_GP
"${CURRENT_DIR}/GP/*.c"
)

file(GLOB_RECURSE SRCS_gstats
"${CURRENT_DIR}/gstats/*.c"
)

file(GLOB SRCS_natneg
"${CURRENT_DIR}/natneg/*.c"
)

file(GLOB SRCS_pinger
"${CURRENT_DIR}/pinger/*.c"
)

file(GLOB SRCS_pt
"${CURRENT_DIR}/pt/*.c"
)

file(GLOB SRCS_qr2
"${CURRENT_DIR}/qr2/*.c"
)

file(GLOB SRCS_sake
"${CURRENT_DIR}/sake/*.c"
)

file(GLOB SRCS_sc
"${CURRENT_DIR}/sc/*.c"
)

file(GLOB SRCS_serverbrowsing
"${CURRENT_DIR}/serverbrowsing/*.c"
)

file(GLOB_RECURSE SRCS_webservices
"${CURRENT_DIR}/webservices/*.c"
)

add_library(gcd_common STATIC ${SRCS_common})
add_library(gcd_key STATIC ${SRCS_gcdkey})
add_library(gcd_gp STATIC ${SRCS_GP})
add_library(gcd_gstats STATIC ${SRCS_gstats})
add_library(gcd_natneg STATIC ${SRCS_natneg})
add_library(gcd_pinger STATIC ${SRCS_pinger})
add_library(gcd_pt STATIC ${SRCS_pt})
add_library(gcd_qr2 STATIC ${SRCS_qr2})
add_library(gcd_sake STATIC ${SRCS_sake})
add_library(gcd_sc STATIC ${SRCS_sc})
add_library(gcd_serverbrowsing STATIC ${SRCS_serverbrowsing})
add_library(gcd_webservices STATIC ${SRCS_webservices})
add_library(gcd INTERFACE)

set_property(TARGET gcd_common gcd_key PROPERTY POSITION_INDEPENDENT_CODE ON)

if(UNIX)
	add_definitions(-D_LINUX=1)
endif(UNIX)

set(DEPENDENT_LIBS
	gcd_key
	gcd_gp
	gcd_gstats
	gcd_natneg
	gcd_pinger
	gcd_pt
	gcd_qr2
	gcd_sake
	gcd_sc
	gcd_serverbrowsing
	gcd_webservices
)

target_link_libraries(gcd INTERFACE ${DEPENDENT_LIBS})

foreach(LIB ${DEPENDENT_LIBS})
	target_link_libraries(${LIB} PUBLIC gcd_common)
endforeach()

target_link_libraries(gcd_qr2 PRIVATE gcd_natneg)
