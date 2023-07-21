--TEST--
xdebug_var_dump() with typed properties [text]
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
xdebug.cli_color=0
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
%sxdebug_var_dump_typed_properties-text.php:14:
class foo#%d (6) {
  public $v =>
  double(3.1415926535898)
  public $w =>
  NULL
  private string $x =>
  *uninitialized*
  protected int $y =>
  int(42)
  public ?Fibble $z =>
  *uninitialized*
  public DateTime $a =>
  class DateTime#%d (%d) {
    public $date =>
    string(26) "%s"
    public $timezone_type =>
    int(3)
    public $timezone =>
    string(3) "UTC"
  }
}
%sxdebug_var_dump_typed_properties-text.php:15:
class class@anonymous#%d (1) {
  public string $x =>
  *uninitialized*
}
