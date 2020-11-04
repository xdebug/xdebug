--TEST--
Test output from xdebug_var_dump() with DateTimeImmutable
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4');
?>
--INI--
xdebug.mode=develop
html_errors=0
date.timezone=Europe/Oslo
xdebug.var_display_max_children=11
xdebug.file_link_format=xdebug://%f@%l
xdebug.filename_format=
--FILE--
<?php
var_dump(new DateTimeImmutable('2019-12-20 13:00:00+0000'));
?>
--EXPECTF--
%sxdebug_var_dump_datetime-php74.php:%d:
class DateTimeImmutable#%d (3) {
  public $date =>
  string(26) "2019-12-20 13:00:00.000000"
  public $timezone_type =>
  int(1)
  public $timezone =>
  string(6) "+00:00"
}
