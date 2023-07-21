--TEST--
Test for bug #1534: Segfault when exception thrown in a closure bound to class scope
--INI--
xdebug.mode=develop
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
