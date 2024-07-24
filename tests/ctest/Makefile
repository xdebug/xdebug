C_TESTS=path_map_test.cpp
SOURCE_FILES=../../src/lib/maps/parser.c
INCLUDE_PATH=../../src/lib/maps
CPPFLAGS=-fsanitize=address -fsanitize=undefined -ggdb3
LDFLAGS=-fsanitize=undefined -l:libubsan.so -lCppUTest

all: ctest

ctest: ${C_TESTS} ${SOURCE_FILES} all_tests.cpp
	g++ ${CPPFLAGS} all_tests.cpp ${C_TESTS} ${SOURCE_FILES} -I ${INCLUDE_PATH} -o ctest ${LDFLAGS}
	
