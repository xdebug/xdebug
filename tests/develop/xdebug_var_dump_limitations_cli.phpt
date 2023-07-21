--TEST--
Test for display limitations with xdebug_var_dump() on CLI.
--INI--
xdebug.mode=develop
xdebug.var_display_max_children=-1
xdebug.var_display_max_data=-1
xdebug.var_display_max_depth=-1
xdebug.cli_color=0
html_errors=0
--FILE--
<?php
$array = array( 1, true, "string" );
xdebug_var_dump( $array ); echo "\n\n";

ini_set('xdebug.var_display_max_depth', 0);
xdebug_var_dump( $array ); echo "\n\n";

ini_set('xdebug.var_display_max_depth', -1);
ini_set('xdebug.var_display_max_data', 0);
xdebug_var_dump( $array ); echo "\n\n";

ini_set('xdebug.var_display_max_children', 0);
ini_set('xdebug.var_display_max_data', -1);
xdebug_var_dump( $array ); echo "\n\n";
?>
--EXPECTF--
%sxdebug_var_dump_limitations_cli.php:3:
array(3) {
  [0] =>
  int(1)
  [1] =>
  bool(true)
  [2] =>
  string(6) "string"
}


%sxdebug_var_dump_limitations_cli.php:6:
array(3) {
  ...
}


%sxdebug_var_dump_limitations_cli.php:10:
array(3) {
  [0] =>
  int(1)
  [1] =>
  bool(true)
  [2] =>
  string(6) ""...
}


%sxdebug_var_dump_limitations_cli.php:14:
array(3) {

  (more elements)...
}
