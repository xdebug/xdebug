--TEST--
Test for bug #457: var_dump() overloading from the command line
--INI--
html_errors=0
xdebug.mode=develop
xdebug.var_display_max_data=32
xdebug.var_display_max_children=4
xdebug.var_display_max_depth=2
xdebug.cli_color=2
xdebug.filename_format=
--FILE--
<?php
$array = array(
	"A very long string that should be cut off at 32 characters",
	array(
		"a test for the depth setting",
		array(
			"this should not show"
		)
	),
	"third element",
	"fourth element (still shows)",
	"fifth element (should not show)"
);
var_dump($array);

$object = new stdClass;
$object->prop1 = "A very long string that should be cut off at 32 characters";
$object->array = array(
		"a test for the depth setting",
		array(
			"this should not show"
		)
	);
$object->prop3 = "third element";
$object->prop4 = "fourth element (still shows)";
$object->prop5 = "fifth element (should not show)";
var_dump($object);
?>
--EXPECTF--
[1m%sbug00457-002.php[22m:[1m14[22m:
[1marray[22m([32m5[0m) {
  [0] [0m=>[0m
  [1mstring[22m([32m58[0m) "[31mA very long string that should b[0m"...
  [1] [0m=>[0m
  [1marray[22m([32m2[0m) {
    [0] [0m=>[0m
    [1mstring[22m([32m28[0m) "[31ma test for the depth setting[0m"
    [1] [0m=>[0m
    [1marray[22m([32m1[0m) {
      ...
    }
  }
  [2] [0m=>[0m
  [1mstring[22m([32m13[0m) "[31mthird element[0m"
  [3] [0m=>[0m
  [1mstring[22m([32m28[0m) "[31mfourth element (still shows)[0m"

  (more elements)...
}
[1m%sbug00457-002.php[22m:[1m27[22m:
[1mclass[22m [31mstdClass[0m#%d ([32m5[0m) {
  [32m[1mpublic[22m[0m $prop1 [0m=>[0m
  [1mstring[22m([32m58[0m) "[31mA very long string that should b[0m"...
  [32m[1mpublic[22m[0m $array [0m=>[0m
  [1marray[22m([32m2[0m) {
    [0] [0m=>[0m
    [1mstring[22m([32m28[0m) "[31ma test for the depth setting[0m"
    [1] [0m=>[0m
    [1marray[22m([32m1[0m) {
      ...
    }
  }
  [32m[1mpublic[22m[0m $prop3 [0m=>[0m
  [1mstring[22m([32m13[0m) "[31mthird element[0m"
  [32m[1mpublic[22m[0m $prop4 [0m=>[0m
  [1mstring[22m([32m28[0m) "[31mfourth element (still shows)[0m"

  (more elements)...
}
