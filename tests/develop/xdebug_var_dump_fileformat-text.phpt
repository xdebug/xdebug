--TEST--
Test for file display with xdebug_var_dump()
--INI--
xdebug.mode=develop
html_errors=0
xdebug.var_display_max_children=11
xdebug.cli_color=0
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
xdebug_var_dump_fileformat-text.php:13:
class TimeStuff#%d (%d) {
  private $timestamp =>
  int(1092515106)
}

…%edevelop%exdebug_var_dump_fileformat-text.php:17:
class TimeStuff#%d (%d) {
  private $timestamp =>
  int(1092515106)
}

«tests%edevelop%exdebug_var_dump_fileformat-text.php»:21:
class TimeStuff#%d (%d) {
  private $timestamp =>
  int(1092515106)
}

{%s%etests%edevelop%exdebug_var_dump_fileformat-text.php}:25:
class TimeStuff#%d (%d) {
  private $timestamp =>
  int(1092515106)
}
