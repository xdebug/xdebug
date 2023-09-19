--TEST--
Tracing: Flamegraph with Fiber
--INI--
xdebug.mode=trace
xdebug.start_with_request=no
xdebug.trace_format=3
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--FILE--
<?php
require_once 'capture-trace.inc';

require dirname(__FILE__) . '/fiber-001.inc';

xdebug_stop_trace();

/*
 * You have to understand that in the following output, when Fiber::start()
 * is called, it executes until it reach an explicit Fiber::suspend().
 *
 * What happens next is that the Fiber::start() method is considered as being
 * completed, so the trace exit handler is called for it, when the Fiber is
 * being suspended, hence the A:AA() call with the Fiber::start() that follows
 * directly.
 *
 * Fiber::suspend() is not exited until the Fiber instance is resumed, so its
 * relative time can be extremelly long, even though it doesn't uses CPU at
 * all (since it's being suspended, it's just IDLE time).
 *
 * When Fiber::resume() is called, in the following trace, Fiber::suspend()
 * is exited at the same time, that's why resuming always start with suspend,
 * and in the very final step of Fiber::resume() operation, when the Fiber
 * stack is either resumed once again or deleted because finished, then and
 * only then, the exit hook is called for Fiber::resume().
 *
 * I hope this is clear enough for anyone reading this to understand why the
 * trace output ordering has nothing to do with the real function execution
 * order.
 *
 * In all cases, Fiber::suspend() timing will always be wrong, since it will
 * change the current stack being executed, its 'self' cost cannot be computed
 * because we cannot guess which other Fiber gets resumed, in other word with
 * the current algorithm, we cannot compute 'self' cost of IDLE Fiber.
 *
 * This is not a problem, because the Fiber being IDLE, it doesn't consume
 * any CPU. This is probably not the bottleneck you are looking for.
 */
?>
--EXPECTF--
dirname %d
require;Fiber->__construct %d
require;Fiber->__construct %d
{closure:%sfiber-001.inc:27-29};A;AA %d
require;Fiber->start %d
{closure:%sfiber-001.inc:31-33};B;BA %d
require;Fiber->start %d
{closure:%sfiber-001.inc:27-29};A;Fiber::suspend %d
{closure:%sfiber-001.inc:27-29};A;AB %d
{closure:%sfiber-001.inc:27-29};A %d
{closure:%sfiber-001.inc:27-29} %d
require;Fiber->resume %d
{closure:%sfiber-001.inc:31-33};B;Fiber::suspend %d
{closure:%sfiber-001.inc:31-33};B;BB %d
{closure:%sfiber-001.inc:31-33};B %d
{closure:%sfiber-001.inc:31-33} %d
require;Fiber->resume %d
require %d
