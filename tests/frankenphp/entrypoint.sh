#!/bin/sh
# Test-image entrypoint.
#
# XDEBUG_CLIENT_HOST: overrides xdebug.client_host (default xdebug://gateway).
# XDEBUG_EXTRA_INI:   extra ini lines (';'-separated) appended to the xdebug ini.
# DBGP_LISTENER=1:    starts the scripted DBGp client (listener.php) before the
#                     server, so that connections made during worker boot
#                     (xdebug.start_with_request=yes) are captured too.
set -e

if [ -n "$XDEBUG_CLIENT_HOST" ]; then
	sed -i "s/^xdebug.client_host=.*/xdebug.client_host=$XDEBUG_CLIENT_HOST/" \
		/usr/local/etc/php/conf.d/zz-xdebug.ini
fi

if [ -n "$XDEBUG_EXTRA_INI" ]; then
	echo "$XDEBUG_EXTRA_INI" | tr ';' '\n' >> /usr/local/etc/php/conf.d/zz-xdebug.ini
fi

if [ "$DBGP_LISTENER" = "1" ]; then
	php -d xdebug.mode=off /usr/local/bin/listener.php > /tmp/listener.log 2>&1 &
	sleep 1
fi

exec "$@"
