--TEST--
xdebug_var_dump() with typed properties [ansi]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4');
?>
--INI--
date.timezone=UTC
xdebug.mode=develop
html_errors=0
xdebug.var_display_max_children=11
xdebug.cli_color=2
xdebug.file_link_format=
xdebug.filename_format=
--FILE--
<?php
class foo {
	public $v = M_PI;
	public $w;
	private string $x;
	protected int $y = 42;
	public ?Fibble $z;
	public \DateTime $a;
}

$f = new foo;
$f->a = new \DateTime;

var_dump($f);
var_dump(new class{public string $x;});
?>
--EXPECTF--
[1m%sxdebug_var_dump_typed_properties-ansi.php[22m:[1m14[22m:
[1mclass[22m [31mfoo[0m#%d ([32m6[0m) {
  [32m[1mpublic[22m[0m $v [0m=>[0m
  [1mdouble[22m([33m3.1415926535898[0m)
  [32m[1mpublic[22m[0m $w [0m=>[0m
  [1m[34mNULL[0m[22m
  [32m[1mprivate[22m string[0m $x [0m=>[0m
  [34m*uninitialized*[0m
  [32m[1mprotected[22m int[0m $y [0m=>[0m
  [1mint[22m([32m42[0m)
  [32m[1mpublic[22m ?Fibble[0m $z [0m=>[0m
  [34m*uninitialized*[0m
  [32m[1mpublic[22m DateTime[0m $a [0m=>[0m
  [1mclass[22m [31mDateTime[0m#%d ([32m3[0m) {
    [32m[1mpublic[22m[0m $date [0m=>[0m
    [1mstring[22m([32m26[0m) "[31m%s[0m"
    [32m[1mpublic[22m[0m $timezone_type [0m=>[0m
    [1mint[22m([32m3[0m)
    [32m[1mpublic[22m[0m $timezone [0m=>[0m
    [1mstring[22m([32m3[0m) "[31mUTC[0m"
  }
}
[1m%sxdebug_var_dump_typed_properties-ansi.php[22m:[1m15[22m:
[1mclass[22m [31mclass@anonymous[0m#%d ([32m1[0m) {
  [32m[1mpublic[22m string[0m $x [0m=>[0m
  [34m*uninitialized*[0m
}
