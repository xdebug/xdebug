--TEST--
Test for overloaded var_dump() on the CLI
--INI--
xdebug.mode=develop
html_errors=0
xdebug.cli_color=0
xdebug.var_display_max_data=32
xdebug.var_display_max_depth=2
xdebug.var_display_max_children=8
--FILE--
<?php
$array = array(
	"Hello, this is a very long string with some other useful info",
	array(
		"depth2" => array(
			"depth3" => array(
				"depth4" => false,
			)
		)
	),
	range(0, 16)
);
var_dump($array);

ini_set('xdebug.cli_color', 2);
var_dump($array);
?>
--EXPECTF--
%svardump-overload-cli-001.php:13:
array(3) {
  [0] =>
  string(61) "Hello, this is a very long strin"...
  [1] =>
  array(1) {
    'depth2' =>
    array(1) {
      ...
    }
  }
  [2] =>
  array(17) {
    [0] =>
    int(0)
    [1] =>
    int(1)
    [2] =>
    int(2)
    [3] =>
    int(3)
    [4] =>
    int(4)
    [5] =>
    int(5)
    [6] =>
    int(6)
    [7] =>
    int(7)

    (more elements)...
  }
}
[1m%svardump-overload-cli-001.php[22m:[1m16[22m:
[1marray[22m([32m3[0m) {
  [0] [0m=>[0m
  [1mstring[22m([32m61[0m) "[31mHello, this is a very long strin[0m"...
  [1] [0m=>[0m
  [1marray[22m([32m1[0m) {
    'depth2' =>
    [1marray[22m([32m1[0m) {
      ...
    }
  }
  [2] [0m=>[0m
  [1marray[22m([32m17[0m) {
    [0] [0m=>[0m
    [1mint[22m([32m0[0m)
    [1] [0m=>[0m
    [1mint[22m([32m1[0m)
    [2] [0m=>[0m
    [1mint[22m([32m2[0m)
    [3] [0m=>[0m
    [1mint[22m([32m3[0m)
    [4] [0m=>[0m
    [1mint[22m([32m4[0m)
    [5] [0m=>[0m
    [1mint[22m([32m5[0m)
    [6] [0m=>[0m
    [1mint[22m([32m6[0m)
    [7] [0m=>[0m
    [1mint[22m([32m7[0m)

    (more elements)...
  }
}
