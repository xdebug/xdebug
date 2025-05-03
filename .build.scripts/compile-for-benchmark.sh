#!/bin/bash
phpize
./configure --enable-xdebug
make all
sudo make install
