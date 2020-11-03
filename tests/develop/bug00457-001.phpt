--TEST--
Test for bug #457: var_dump() overloading from the command line
--INI--
html_errors=0
xdebug.mode=develop
xdebug.var_display_max_data=32
xdebug.var_display_max_children=4
xdebug.var_display_max_depth=2
xdebug.cli_color=1
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
%sbug00457-001.php:14:
array(5) {
  [0] =>
  string(58) "A very long string that should b"...
  [1] =>
  array(2) {
    [0] =>
    string(28) "a test for the depth setting"
    [1] =>
    array(1) {
      ...
    }
  }
  [2] =>
  string(13) "third element"
  [3] =>
  string(28) "fourth element (still shows)"

  (more elements)...
}
%sbug00457-001.php:27:
class stdClass#%d (5) {
  public $prop1 =>
  string(58) "A very long string that should b"...
  public $array =>
  array(2) {
    [0] =>
    string(28) "a test for the depth setting"
    [1] =>
    array(1) {
      ...
    }
  }
  public $prop3 =>
  string(13) "third element"
  public $prop4 =>
  string(28) "fourth element (still shows)"

  (more elements)...
}
