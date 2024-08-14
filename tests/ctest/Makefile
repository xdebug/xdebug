C_TESTS=path_map_test.cpp
SOURCE_FILES=../../src/lib/maps/parser.c ../../src/lib/maps/maps.c ../../src/lib/maps/maps_private.c
HEADER_FILES=../../src/lib/maps/maps_private.h ../../src/lib/maps/maps.h
INCLUDE_PATH=../../src/lib/maps
CPPFLAGS=-DPHP_VERSION_ID=80300 -DXDEBUG_NO_PHP_FEATURES -ggdb3 -fsanitize=address -fsanitize=undefined
LDFLAGS=-lCppUTest -fsanitize=undefined -l:libubsan.so -lCppUTest

all: ctest

test: ctest
	./ctest

ctest: xdebuglib.a ${C_TESTS} ${SOURCE_FILES} ${HEADER_FILES} all_tests.cpp
	g++ ${CPPFLAGS} all_tests.cpp ${C_TESTS} ${SOURCE_FILES} xdebuglib.a -I ${INCLUDE_PATH} -o ctest ${LDFLAGS}

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
	rm xdebuglib.a hash.o llist.o str.o trim.o xdebug_strndup.o ctest
