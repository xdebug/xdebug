--TEST--
Test for file display with xdebug_var_dump()
--INI--
xdebug.mode=develop
html_errors=0
xdebug.var_display_max_children=11
xdebug.cli_color=2
xdebug.file_link_format=
xdebug.filename_format=
--FILE--
<?php
class TimeStuff {
	private $timestamp;

	function __construct($ts = null)
	{
		$this->timestamp = $ts === null ? time() : $ts;
	}
}
$ts1 = new TimeStuff(1092515106);

ini_set('xdebug.filename_format', "%n");
var_dump($ts1);
echo "\n";

ini_set('xdebug.filename_format', "…%s%p");
var_dump($ts1);
echo "\n";

ini_set('xdebug.filename_format', "«%a»");
var_dump($ts1);
echo "\n";

ini_set('xdebug.filename_format', "{%f}");
var_dump($ts1);
echo "\n";
?>
--EXPECTF--
[1mxdebug_var_dump_fileformat-ansi.php[22m:[1m13[22m:
[1mclass[22m [31mTimeStuff[0m#%d ([32m1[0m) {
  [32m[1mprivate[22m[0m $timestamp [0m=>[0m
  [1mint[22m([32m1092515106[0m)
}

[1m…%edevelop%exdebug_var_dump_fileformat-ansi.php[22m:[1m17[22m:
[1mclass[22m [31mTimeStuff[0m#%d ([32m1[0m) {
  [32m[1mprivate[22m[0m $timestamp [0m=>[0m
  [1mint[22m([32m1092515106[0m)
}

[1m«tests%edevelop%exdebug_var_dump_fileformat-ansi.php»[22m:[1m21[22m:
[1mclass[22m [31mTimeStuff[0m#%d ([32m1[0m) {
  [32m[1mprivate[22m[0m $timestamp [0m=>[0m
  [1mint[22m([32m1092515106[0m)
}

[1m{%s%etests%edevelop%exdebug_var_dump_fileformat-ansi.php}[22m:[1m25[22m:
[1mclass[22m [31mTimeStuff[0m#%d ([32m1[0m) {
  [32m[1mprivate[22m[0m $timestamp [0m=>[0m
  [1mint[22m([32m1092515106[0m)
}
