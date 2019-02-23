#!/bin/bash
phpize
./configure --enable-xdebug-dev
make all && make install

