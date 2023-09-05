cmake_minimum_required(VERSION 3.5)
project(libmad)

set(SRCS
	"./bit.c"
	"./decoder.c"
	"./fixed.c"
	"./frame.c"
	"./huffman.c"
	"./layer3.c"
	"./layer12.c"
	"./stream.c"
	"./synth.c"
	"./timer.c"
	"./version.c"
)

add_library(libmad STATIC ${SRCS})
target_compile_definitions(libmad PUBLIC FPM_64BIT)
target_compile_definitions(libmad PUBLIC NDEBUG) # otherwise assert() will call abort() and it fails to compile on clang-16 (call to undeclared library function 'abort')
