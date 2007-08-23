--TEST--
Test for correct display with non overloaded var_dump() (ZE2)
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(zend_version(), "2.0.0-dev", '<')) echo "skip Zend Engine 2 needed\n"; ?>
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
		private $user_defined;
		private $self;
		protected $tm;
		public $date;

		function TimeStuff($ts = null)
		{
			$this->self = &$this;
			$this->timestamp = $ts === null ? time() : $ts;
			$this->user_defined = ($ts !== null);
			$this->date = date("Y-m-d H:i:s T", $this->timestamp);
			$this->tm = getdate($this->timestamp);
		}
	}

	$ts1 = new TimeStuff(1092515106);

	var_dump($ts1);
	ini_set('xdebug.overload_var_dump', 1);
	var_dump($ts1);
?>
--EXPECT--
object(TimeStuff)#1 (5) {
  ["timestamp:private"]=>
  int(1092515106)
  ["user_defined:private"]=>
  bool(true)
  ["self:private"]=>
  object(TimeStuff)#1 (5) {
    ["timestamp:private"]=>
    int(1092515106)
    ["user_defined:private"]=>
    bool(true)
    ["self:private"]=>
    *RECURSION*
    ["tm:protected"]=>
    array(11) {
      ["seconds"]=>
      int(6)
      ["minutes"]=>
      int(25)
      ["hours"]=>
      int(22)
      ["mday"]=>
      int(14)
      ["wday"]=>
      int(6)
      ["mon"]=>
      int(8)
      ["year"]=>
      int(2004)
      ["yday"]=>
      int(226)
      ["weekday"]=>
      string(8) "Saturday"
      ["month"]=>
      string(6) "August"
      [0]=>
      int(1092515106)
    }
    ["date"]=>
    string(24) "2004-08-14 22:25:06 CEST"
  }
  ["tm:protected"]=>
  array(11) {
    ["seconds"]=>
    int(6)
    ["minutes"]=>
    int(25)
    ["hours"]=>
    int(22)
    ["mday"]=>
    int(14)
    ["wday"]=>
    int(6)
    ["mon"]=>
    int(8)
    ["year"]=>
    int(2004)
    ["yday"]=>
    int(226)
    ["weekday"]=>
    string(8) "Saturday"
    ["month"]=>
    string(6) "August"
    [0]=>
    int(1092515106)
  }
  ["date"]=>
  string(24) "2004-08-14 22:25:06 CEST"
}
object(TimeStuff)#1 (5) {
  ["timestamp:private"]=>
  int(1092515106)
  ["user_defined:private"]=>
  bool(true)
  ["self:private"]=>
  object(TimeStuff)#1 (5) {
    ["timestamp:private"]=>
    int(1092515106)
    ["user_defined:private"]=>
    bool(true)
    ["self:private"]=>
    *RECURSION*
    ["tm:protected"]=>
    array(11) {
      ["seconds"]=>
      int(6)
      ["minutes"]=>
      int(25)
      ["hours"]=>
      int(22)
      ["mday"]=>
      int(14)
      ["wday"]=>
      int(6)
      ["mon"]=>
      int(8)
      ["year"]=>
      int(2004)
      ["yday"]=>
      int(226)
      ["weekday"]=>
      string(8) "Saturday"
      ["month"]=>
      string(6) "August"
      [0]=>
      int(1092515106)
    }
    ["date"]=>
    string(24) "2004-08-14 22:25:06 CEST"
  }
  ["tm:protected"]=>
  array(11) {
    ["seconds"]=>
    int(6)
    ["minutes"]=>
    int(25)
    ["hours"]=>
    int(22)
    ["mday"]=>
    int(14)
    ["wday"]=>
    int(6)
    ["mon"]=>
    int(8)
    ["year"]=>
    int(2004)
    ["yday"]=>
    int(226)
    ["weekday"]=>
    string(8) "Saturday"
    ["month"]=>
    string(6) "August"
    [0]=>
    int(1092515106)
  }
  ["date"]=>
  string(24) "2004-08-14 22:25:06 CEST"
}
