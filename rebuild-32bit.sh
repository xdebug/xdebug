#!/bin/sh

export CFLAGS="-m32 -Wall -Werror -Wextra -Wmaybe-uninitialized -Wdeclaration-after-statement -Wmissing-field-initializers -Wshadow -Wno-unused-parameter -ggdb3"
phpize && ./configure && make clean && make -j 5 all && make install
