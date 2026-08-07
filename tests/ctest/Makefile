C_TESTS=path_map_test.cpp fuzz-cases.cpp
SOURCE_FILES=../../src/lib/maps/parser.c ../../src/lib/maps/maps_private.c ../../src/lib/cmd_parser.c
HEADER_FILES=../../src/lib/maps/maps_private.h ../../src/lib/maps/maps.h
INCLUDE_PATHS=-I ../../src/lib/maps -I ../../src/lib
CPPFLAGS=-DPHP_VERSION_ID=80300 -DXDEBUG_NO_PHP_FEATURES -ggdb3 -fsanitize=address -fsanitize=undefined -O0
LDFLAGS=-lCppUTest -fsanitize=undefined -l:libubsan.so

all: ctest afl-test-parser afl-test-mapper afl-test-cmd-parser

test: ctest
	./ctest

fuzz: afl-test-parser afl-test-mapper afl-test-cmd-parser
	AFL_SKIP_CPUFREQ=1 afl-fuzz -V 100 -b 7  -i fuzz-seeds-cmd-parser -o fuzz-output -- ./afl-test-cmd-parse @@
	AFL_SKIP_CPUFREQ=1 afl-fuzz -V 100 -b 7  -i fuzz-seeds-parser -o fuzz-output -- ./afl-test-parser @@
	AFL_SKIP_CPUFREQ=1 afl-fuzz -V 100 -b 7  -i fuzz-seeds-mapper -o fuzz-output -- ./afl-test-mapper @@

ctest: xdebuglib.a ${C_TESTS} ${SOURCE_FILES} ${HEADER_FILES} all_tests.cpp
	g++ ${CPPFLAGS} all_tests.cpp ${C_TESTS} ${SOURCE_FILES} xdebuglib.a ${INCLUDE_PATHS} -o ctest ${LDFLAGS}

afl-test-parser: xdebuglib.a ${C_TESTS} ${SOURCE_FILES} ${HEADER_FILES} fuzzing-shim-parser.c
	afl-clang-fast ${CPPFLAGS} fuzzing-shim-parser.c ${SOURCE_FILES} xdebuglib.a ${INCLUDE_PATHS} -o afl-test-parser ${LDFLAGS}

afl-test-mapper: xdebuglib.a ${C_TESTS} ${SOURCE_FILES} ${HEADER_FILES} fuzzing-shim-mapper.c
	afl-clang-fast ${CPPFLAGS} fuzzing-shim-mapper.c ${SOURCE_FILES} xdebuglib.a ${INCLUDE_PATHS} -o afl-test-mapper ${LDFLAGS}

afl-test-cmd-parser: xdebuglib.a ${C_TESTS} ../../src/lib/cmd_parser.c ${SOURCE_FILES} ${HEADER_FILES} fuzzing-shim-cmd-parse.c
	afl-clang-fast ${CPPFLAGS} fuzzing-shim-cmd-parse.c ${SOURCE_FILES} xdebuglib.a ${INCLUDE_PATHS} -o afl-test-cmd-parse ${LDFLAGS}

xdebuglib.a: hash.o llist.o normalize_path.o str.o stripcslashes.o trim.o xdebug_strndup.o
	ar -rc xdebuglib.a hash.o llist.o normalize_path.o str.o stripcslashes.o trim.o xdebug_strndup.o

hash.o: ../../src/lib/hash.c
	gcc -c ${CPPFLAGS} -o hash.o ../../src/lib/hash.c

llist.o: ../../src/lib/llist.c
	gcc -c ${CPPFLAGS} -o llist.o ../../src/lib/llist.c

normalize_path.o: ../../src/lib/normalize_path.c
	gcc -c ${CPPFLAGS} -o normalize_path.o ../../src/lib/normalize_path.c

str.o: ../../src/lib/str.c
	gcc -c ${CPPFLAGS} -o str.o ../../src/lib/str.c

stripcslashes.o: ../../src/lib/stripcslashes.c
	gcc -c ${CPPFLAGS} -o stripcslashes.o ../../src/lib/stripcslashes.c

trim.o: ../../src/lib/trim.c
	gcc -c ${CPPFLAGS} -o trim.o ../../src/lib/trim.c

xdebug_strndup.o: ../../src/lib/xdebug_strndup.c
	gcc -c ${CPPFLAGS} -o xdebug_strndup.o ../../src/lib/xdebug_strndup.c

clean:
	rm xdebuglib.a hash.o llist.o normalize_path.o str.o trim.o xdebug_strndup.o ctest afl-test-parser afl-test-mapper afl-test-cmd-parse
