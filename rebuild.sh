#!/bin/sh

phpize && ./configure --enable-xdebug-dev && make clean && make -j 5 all && make install
