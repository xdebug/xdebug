--TEST--
Test for bug #1996: Show wrapped callable for first class callables (>= PHP 8.2, text)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2');
?>
--INI--
xdebug.mode=develop
html_errors=0
xdebug.cli_color=0
--FILE--
<?php
$closure = substr(...);
var_dump($closure);


function user_defined($a, $b)
{
	return substr($a, $b);
}
$closure = user_defined(...);
var_dump($closure);


$closure = DateTimeImmutable::createFromFormat(...);
var_dump($closure);


$dateTime = new DateTimeImmutable("2021-07-22");
$closure = $dateTime->format(...);
var_dump($closure);
?>
--EXPECTF--
%sbug01996-002-text-php82.php:3:
class Closure#1 (2) {
  public $function =>
  string(6) "substr"
  public $parameter =>
  array(3) {
    '$string' =>
    string(10) "<required>"
    '$offset' =>
    string(10) "<required>"
    '$length' =>
    string(10) "<optional>"
  }
}
%sbug01996-002-text-php82.php:11:
class Closure#2 (2) {
  public $function =>
  string(12) "user_defined"
  public $parameter =>
  array(2) {
    '$a' =>
    string(10) "<required>"
    '$b' =>
    string(10) "<required>"
  }
}
%sbug01996-002-text-php82.php:15:
class Closure#1 (2) {
  public $function =>
  string(35) "DateTimeImmutable::createFromFormat"
  public $parameter =>
  array(3) {
    '$format' =>
    string(10) "<required>"
    '$datetime' =>
    string(10) "<required>"
    '$timezone' =>
    string(10) "<optional>"
  }
}
%sbug01996-002-text-php82.php:20:
class Closure#3 (3) {
  public $function =>
  string(25) "DateTimeImmutable::format"
  public $this =>
  class DateTimeImmutable#2 (3) {
    public $date =>
    string(26) "2021-07-22 00:00:00.000000"
    public $timezone_type =>
    int(3)
    public $timezone =>
    string(3) "UTC"
  }
  public $parameter =>
  array(1) {
    '$format' =>
    string(10) "<required>"
  }
}
