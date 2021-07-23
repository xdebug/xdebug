--TEST--
Test for bug #1996: Show wrapped callable for closures (text)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--INI--
xdebug.mode=develop
html_errors=0
--FILE--
<?php
$closure = Closure::fromCallable('substr');
var_dump($closure);


function user_defined($a, $b)
{
	return substr($a, $b);
}
$closure = Closure::fromCallable('user_defined');
var_dump($closure);


$closure = Closure::fromCallable(['DateTimeImmutable', 'createFromFormat']);
var_dump($closure);


$dateTime = new DateTimeImmutable("2021-07-22");
$closure = Closure::fromCallable([$dateTime, 'format']);
var_dump($closure);
?>
--EXPECTF--
%sbug01996-001-text.php:3:
class Closure#1 (1) {
  virtual $closure =>
  "substr"
  public $parameter =>
  array(3) {
    '$%s' =>
    string(10) "<required>"
    '$%s' =>
    string(10) "<required>"
    '$length' =>
    string(10) "<optional>"
  }
}
%sbug01996-001-text.php:11:
class Closure#2 (1) {
  virtual $closure =>
  "user_defined"
  public $parameter =>
  array(2) {
    '$a' =>
    string(10) "<required>"
    '$b' =>
    string(10) "<required>"
  }
}
%sbug01996-001-text.php:15:
class Closure#1 (1) {
  virtual $closure =>
  "DateTimeImmutable::createFromFormat"
  public $parameter =>
  array(3) {
    '$format' =>
    string(10) "<required>"
    '$%s' =>
    string(10) "<required>"
    '$%s' =>
    string(10) "<optional>"
  }
}
%sbug01996-001-text.php:20:
class Closure#3 (2) {
  virtual $closure =>
  "$this->format"
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
