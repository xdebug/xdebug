# FrankenPHP worker mode tests

Exercises the SAPI activate/deactivate hooks added in
`src/debugger/frankenphp.c`. In worker mode, `RINIT`/`RSHUTDOWN` only run once
per worker script, so the per-request debugger lifecycle is driven through
`sapi_module.activate` / `sapi_module.deactivate` instead.

## Automated smoke test

Requires Docker. From the repository root (or anywhere):

```bash
tests/frankenphp/run-tests.sh
```

This builds Xdebug against FrankenPHP's bundled ZTS PHP, starts a worker, and
uses a scripted DBGp client to assert that:

- each request carrying `XDEBUG_TRIGGER` gets its **own** debug session
  (`init` → `run` → `stopping` → `stop`), on the same worker process;
- requests without a trigger do not open a session;
- `xdebug.trigger_value` (shared secret) is enforced: a non-matching trigger
  value does not start a session, a matching one does.

## Manual test with a real IDE

Build and run, with the IDE listening on port 9003 on the host:

```bash
docker build -f tests/frankenphp/Dockerfile -t xdebug-frankenphp .

# macOS / Windows (host.docker.internal is the image default):
docker run --rm -p 8080:8080 xdebug-frankenphp

# Linux:
docker run --rm -p 8080:8080 \
  --add-host=host.docker.internal:host-gateway \
  xdebug-frankenphp
```

Configure a path mapping in the IDE (`tests/frankenphp/app` → `/app/public`),
put a breakpoint inside the `$handler` closure of `worker.php`, then:

```bash
# No trigger => no pause:
curl http://localhost:8080/worker.php

# Trigger => pauses on the breakpoint, on every request, same worker pid:
curl -b 'XDEBUG_SESSION=1' http://localhost:8080/worker.php
curl 'http://localhost:8080/worker.php?XDEBUG_TRIGGER=1'
```

The `pid=` in the response stays constant across requests (proving worker
reuse) while the IDE gets a fresh debug session for each triggered request.
Breakpoints added or moved between two requests are honored on the next
request, because the IDE re-sends them on each new session.

`tail -f /tmp/xdebug.log` inside the container shows the connection attempts.
