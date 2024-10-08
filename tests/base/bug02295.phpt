--TEST--
Test for bug #2295: var_dump(SensitiveParameterValue) segfaults
--INI--
xdebug.mode=develop
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('PHP >= 8.2');
?>
--FILE--
<?php

function sp(#[SensitiveParameter] string $str): void
{
	var_dump($str);
}

function spv(SensitiveParameterValue $spv): void
{
	var_dump($spv);
}

ini_set('html_errors', 0);
sp('a'); // ok
spv(new SensitiveParameterValue('p'));

ini_set('xdebug.cli_color', 2);
sp('a'); // ok
spv(new SensitiveParameterValue('p'));

ini_set('html_errors', 1);
sp('a'); // ok
spv(new SensitiveParameterValue('p'));
?>
--EXPECTF--
%sbug02295.php:5:
string(1) "a"
%sbug02295.php:10:
class SensitiveParameterValue#%d (0) {
}
[1m%sbug02295.php[22m:[1m5[22m:
[1mstring[22m([32m1[0m) "[31ma[0m"
[1m%sbug02295.php[22m:[1m10[22m:
[1mclass[22m [31mSensitiveParameterValue[0m#1 ([32m0[0m) {
}
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug02295.php:5:</small><small>string</small> <font color='#cc0000'>'a'</font> <i>(length=1)</i>
</pre><pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug02295.php:10:</small>
<b>object</b>(<i>SensitiveParameterValue</i>)[<i>1</i>]
</pre>
