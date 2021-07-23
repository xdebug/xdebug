--TEST--
Test for bug #1996: Show wrapped callable for first class callables (PHP >= 8.1, ansi)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=develop
html_errors=0
xdebug.cli_color=2
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
[1m%sbug01996-002-ansi.php[22m:[1m3[22m:
[1mclass[22m [31mClosure[0m#1 ([32m1[0m) {
  [32m[1mvirtual[0m $closure =>
  "[31msubstr[0m"
  [32m[1mpublic[22m[0m $parameter [0m=>[0m
  [1marray[22m([32m3[0m) {
    '$string' =>
    [1mstring[22m([32m10[0m) "[31m<required>[0m"
    '$offset' =>
    [1mstring[22m([32m10[0m) "[31m<required>[0m"
    '$length' =>
    [1mstring[22m([32m10[0m) "[31m<optional>[0m"
  }
}
[1m%sbug01996-002-ansi.php[22m:[1m11[22m:
[1mclass[22m [31mClosure[0m#2 ([32m1[0m) {
  [32m[1mvirtual[0m $closure =>
  "[31muser_defined[0m"
  [32m[1mpublic[22m[0m $parameter [0m=>[0m
  [1marray[22m([32m2[0m) {
    '$a' =>
    [1mstring[22m([32m10[0m) "[31m<required>[0m"
    '$b' =>
    [1mstring[22m([32m10[0m) "[31m<required>[0m"
  }
}
[1m%sbug01996-002-ansi.php[22m:[1m15[22m:
[1mclass[22m [31mClosure[0m#1 ([32m1[0m) {
  [32m[1mvirtual[0m $closure =>
  "[31mDateTimeImmutable[0m::[31mcreateFromFormat[0m"
  [32m[1mpublic[22m[0m $parameter [0m=>[0m
  [1marray[22m([32m3[0m) {
    '$format' =>
    [1mstring[22m([32m10[0m) "[31m<required>[0m"
    '$datetime' =>
    [1mstring[22m([32m10[0m) "[31m<required>[0m"
    '$timezone' =>
    [1mstring[22m([32m10[0m) "[31m<optional>[0m"
  }
}
[1m%sbug01996-002-ansi.php[22m:[1m20[22m:
[1mclass[22m [31mClosure[0m#3 ([32m2[0m) {
  [32m[1mvirtual[0m $closure =>
  "[31m$this[0m->[31mformat[0m"
  [32m[1mpublic[22m[0m $this [0m=>[0m
  [1mclass[22m [31mDateTimeImmutable[0m#2 ([32m3[0m) {
    [32m[1mpublic[22m[0m $date [0m=>[0m
    [1mstring[22m([32m26[0m) "[31m2021-07-22 00:00:00.000000[0m"
    [32m[1mpublic[22m[0m $timezone_type [0m=>[0m
    [1mint[22m([32m3[0m)
    [32m[1mpublic[22m[0m $timezone [0m=>[0m
    [1mstring[22m([32m3[0m) "[31mUTC[0m"
  }
  [32m[1mpublic[22m[0m $parameter [0m=>[0m
  [1marray[22m([32m1[0m) {
    '$format' =>
    [1mstring[22m([32m10[0m) "[31m<required>[0m"
  }
}
