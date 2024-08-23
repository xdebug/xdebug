C_TESTS=path_map_test.cpp fuzz-cases.cpp
SOURCE_FILES=../../src/lib/maps/parser.c ../../src/lib/maps/maps.c ../../src/lib/maps/maps_private.c
HEADER_FILES=../../src/lib/maps/maps_private.h ../../src/lib/maps/maps.h
INCLUDE_PATH=../../src/lib/maps
CPPFLAGS=-DPHP_VERSION_ID=80300 -DXDEBUG_NO_PHP_FEATURES -ggdb3 -fsanitize=address -fsanitize=undefined
LDFLAGS=-lCppUTest -fsanitize=undefined -l:libubsan.so

all: ctest ftest afl-test

test: ctest
	./ctest

fuzz: afl-test
	AFL_SKIP_CPUFREQ=1 afl-fuzz -b 7  -i fuzz-seeds -o fuzz-output -- ./afl-test @@

ctest: xdebuglib.a ${C_TESTS} ${SOURCE_FILES} ${HEADER_FILES} all_tests.cpp
	g++ ${CPPFLAGS} all_tests.cpp ${C_TESTS} ${SOURCE_FILES} xdebuglib.a -I ${INCLUDE_PATH} -o ctest ${LDFLAGS}

afl-test: xdebuglib.a ${C_TESTS} ${SOURCE_FILES} ${HEADER_FILES} fuzzing-shim.c
	afl-g++ ${CPPFLAGS} fuzzing-shim.c ${SOURCE_FILES} xdebuglib.a -I ${INCLUDE_PATH} -o afl-test ${LDFLAGS}

ftest: xdebuglib.a ${C_TESTS} ${SOURCE_FILES} ${HEADER_FILES} fuzzing-shim.c
	g++ ${CPPFLAGS} fuzzing-shim.c ${SOURCE_FILES} xdebuglib.a -I ${INCLUDE_PATH} -o ftest ${LDFLAGS}

xdebuglib.a: hash.o llist.o str.o trim.o xdebug_strndup.o
	ar -rc xdebuglib.a hash.o llist.o str.o trim.o xdebug_strndup.o

llist.o: ../../src/lib/llist.c
	gcc -c ${CPPFLAGS} -o llist.o ../../src/lib/llist.c

hash.o: ../../src/lib/hash.c
	gcc -c ${CPPFLAGS} -o hash.o ../../src/lib/hash.c

str.o: ../../src/lib/str.c
	gcc -c ${CPPFLAGS} -o str.o ../../src/lib/str.c

trim.o: ../../src/lib/trim.c
	gcc -c ${CPPFLAGS} -o trim.o ../../src/lib/trim.c

xdebug_strndup.o: ../../src/lib/xdebug_strndup.c
	gcc -c ${CPPFLAGS} -o xdebug_strndup.o ../../src/lib/xdebug_strndup.c

clean:
	rm xdebuglib.a hash.o llist.o str.o trim.o xdebug_strndup.o ctest afl-test ftest
