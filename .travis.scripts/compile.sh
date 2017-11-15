#!/bin/bash
export CFLAGS="-Wall -Werror -Wextra -Wdeclaration-after-statement -Wmissing-field-initializers -Wshadow -Wno-unused-parameter -ggdb3"
phpize
./configure
make all

