--TEST--
xdebug_var_dump() with typed properties [html]
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 7.4');
?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.profiler_enable=0
html_errors=1
xdebug.var_display_max_children=11
xdebug.overload_var_dump=2
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
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sxdebug_var_dump_typed_properties-html.php:14:</small>
<b>object</b>(<i>foo</i>)[<i>1</i>]
  <i>public</i> 'v' <font color='#888a85'>=&gt;</font> <small>float</small> <font color='#f57900'>3.1415926535898</font>
  <i>public</i> 'w' <font color='#888a85'>=&gt;</font> <font color='#3465a4'>null</font>
  <i>private</i> <i>string</i> 'x' <font color='#888a85'>=&gt;</font> <font color='#3465a4'>*uninitialized*</font>
  <i>protected</i> <i>int</i> 'y' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>42</font>
  <i>public</i> <i>?Fibble</i> 'z' <font color='#888a85'>=&gt;</font> <font color='#3465a4'>*uninitialized*</font>
  <i>public</i> <i>DateTime</i> 'a' <font color='#888a85'>=&gt;</font> 
    <b>object</b>(<i>DateTime</i>)[<i>2</i>]
</pre><pre class='xdebug-var-dump' dir='ltr'>
<small>%sxdebug_var_dump_typed_properties-html.php:15:</small>
<b>object</b>(<i>class@anonymous</i>)[<i>3</i>]
  <i>public</i> <i>string</i> 'x' <font color='#888a85'>=&gt;</font> <font color='#3465a4'>*uninitialized*</font>
</pre>
