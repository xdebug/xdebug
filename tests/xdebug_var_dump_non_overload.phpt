--TEST--
Test for correct display with non overloaded var_dump()
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.profiler_enable=0
html_errors=1
date.timezone=Europe/Oslo
xdebug.var_display_max_children=11
xdebug.overload_var_dump=0
--FILE--
<?php
	class TimeStuff {
		private $timestamp;

		function TimeStuff($ts = null)
		{
			$this->timestamp = $ts === null ? time() : $ts;
		}
	}

	$ts1 = new TimeStuff(1092515106);

	var_dump($ts1);
	ini_set('xdebug.overload_var_dump', 1); // has no effect, because it's INI_SYSTEM/INI_PERDIR
	var_dump($ts1);
?>
--EXPECTF--
object(TimeStuff)#1 (1) {
  ["timestamp%sprivat%s]=>
  int(1092515106)
}
object(TimeStuff)#1 (1) {
  ["timestamp%sprivat%s]=>
  int(1092515106)
}
