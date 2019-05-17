--TEST--
Test for bug #1534: Segfault when exception thrown in a closure bound to class scope (fixed by #1665, bugs have same root cause)
--INI--
xdebug.default_enable=1
xdebug.collect_params=4
--FILE--
<?php
try {
	class bug {
		public function on(\Closure $closure) {
			$closure = $closure->bindTo($this);
			$closure();
		}
	}

	$bug = new bug();
	$bug->on(function() {
		throw new \Exception("Exception thrown successfully");
	});
} catch (\Exception $ex) {
	echo "Works!";
}
?>
--EXPECTF--
Works!
