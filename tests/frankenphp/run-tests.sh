#!/bin/bash
# Automated smoke test for FrankenPHP worker mode support.
#
# Builds the image, then exercises the SAPI hooks with a scripted DBGp client
# (listener.php, started inside the container by the entrypoint). Each phase
# runs in a fresh container:
#
#   phase 1 — xdebug.start_with_request=trigger (default):
#     1. request with XDEBUG_TRIGGER in the query string -> one debug session
#     2. second triggered request, same worker           -> a NEW debug session
#     3. request with an XDEBUG_SESSION cookie           -> one debug session
#     4. request without trigger                         -> no session
#        (also proves triggers do not leak from one request to the next)
#
#   phase 2 — with xdebug.trigger_value=my_secret:
#     5. triggered request, wrong secret                 -> no session
#     6. triggered request, matching secret              -> one debug session
#
#   phase 3 — xdebug.start_with_request=yes:
#     7. worker boot itself                              -> one debug session
#     8. request without any trigger                     -> one debug session
#
# Requires: docker. Run from anywhere; paths are resolved from this script.
set -e
cd "$(dirname "$0")/../.."

PHP_VERSION=${PHP_VERSION:-8.4}
IMAGE=xdebug-frankenphp-test-$PHP_VERSION
NAME=xdebug-frankenphp-test-$PHP_VERSION
PORT=${PORT:-18080}

echo "=== building image (this compiles Xdebug against FrankenPHP's ZTS PHP $PHP_VERSION) ==="
docker build -q --build-arg PHP_VERSION="$PHP_VERSION" -f tests/frankenphp/Dockerfile -t $IMAGE . >/dev/null

cleanup() { docker rm -f $NAME >/dev/null 2>&1 || true; }
trap cleanup EXIT

start_container() { # $1 = extra ini lines (';'-separated), may be empty
	cleanup
	docker run -d --name $NAME -p "$PORT:8080" \
		-e XDEBUG_CLIENT_HOST=127.0.0.1 -e DBGP_LISTENER=1 \
		-e XDEBUG_EXTRA_INI="$1" $IMAGE >/dev/null
	# Wait for HTTP readiness; XDEBUG_IGNORE prevents the probe from starting
	# a debug session of its own (relevant with start_with_request=yes).
	for _ in $(seq 1 30); do
		curl -sf "http://localhost:$PORT/index.php?XDEBUG_IGNORE=1" >/dev/null 2>&1 && return 0
		sleep 0.5
	done
	echo "FAIL: server did not become ready"
	docker logs $NAME | tail -20
	exit 1
}

req() {
	curl -sf ${2:+-b "$2"} "http://localhost:$PORT/worker.php?$1" >/dev/null
}

assert_sessions() {
	local expected=$1 label=$2 got
	sleep 1
	got=$(docker exec $NAME sh -c 'grep -c "^CONN#.* init " /tmp/listener.log || true')
	echo "--- listener log ($label):"
	docker exec $NAME cat /tmp/listener.log
	if [ "$got" != "$expected" ]; then
		echo "FAIL ($label): expected $expected debug session(s), got $got"
		exit 1
	fi
	echo "PASS: $label"
}

echo "=== phase 1: start_with_request=trigger ==="
start_container ""
req "n=1&XDEBUG_TRIGGER=1"
req "n=2&XDEBUG_TRIGGER=1"
req "n=3" "XDEBUG_SESSION=1"
req "n=4"
assert_sessions 3 "one session per triggered request (query + cookie), none without trigger"

echo "=== phase 2: xdebug.trigger_value shared secret ==="
start_container "xdebug.trigger_value=my_secret"
req "n=5&XDEBUG_TRIGGER=wrong_secret"
req "n=6&XDEBUG_TRIGGER=my_secret"
assert_sessions 1 "trigger_value enforced: only the matching secret connects"

echo "=== phase 3: start_with_request=yes ==="
start_container "xdebug.start_with_request=yes"
req "n=7"
assert_sessions 2 "start_with_request=yes connects at worker boot and on each request"

echo "ALL TESTS PASSED"
