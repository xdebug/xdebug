--TEST--
Test for bug #1999: Show readonly properties (PHP >= 8.1, html)
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.1');
?>
--INI--
xdebug.mode=develop
html_errors=1
--FILE--
<?php
class WithReadOnlyProps
{
	static int $static_int = 1;

	function __construct(
		public string $static_string = "two",
		public readonly string $ro_string = "readonly-default",
	) {}
}

$obj = new WithReadOnlyProps(ro_string: "New Value");

var_dump($obj);
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug01999-html.php:14:</small>
<b>object</b>(<i>WithReadOnlyProps</i>)[<i>1</i>]
  <i>public</i> <i>string</i> 'static_string' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'two'</font> <i>(length=3)</i>
  <i>public</i> <i>readonly string</i> 'ro_string' <font color='#888a85'>=&gt;</font> <small>string</small> <font color='#cc0000'>'New Value'</font> <i>(length=9)</i>
</pre>
