--TEST--
Test for bug #328: Private properties are incorrectly enumerated in case of extended classes
--INI--
html_errors=1
xdebug.default_enable=1
xdebug.overload_var_dump=2
xdebug.file_link_format=
--FILE--
<?php
class Daddy
{
	private $bar = 42;
	protected $pro = 242;
	public    $pub = 342;
}
class Inherit extends Daddy
{
	private $bar = 43;
	protected $pro = 243;
	public    $pub = 343;
}
$a = new Inherit;
var_dump( $a );
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug00328.php:15:</small>
<b>object</b>(<i>Inherit</i>)[<i>1</i>]
  <i>private</i> 'bar' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>43</font>
  <i>protected</i> 'pro' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>243</font>
  <i>public</i> 'pub' <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>343</font>
  <i>private</i> 'bar' <small>(Daddy)</small> <font color='#888a85'>=&gt;</font> <small>int</small> <font color='#4e9a06'>42</font>
</pre>
