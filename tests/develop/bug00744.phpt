--TEST--
Test for bug #744: new lines in a PHP file from Windows are displayed with an extra white line with var_dump().
--INI--
html_errors=1
xdebug.mode=develop
xdebug.file_link_format=
xdebug.filename_format=
--FILE--
<?php
$my_test_string = 'hello
                    world';
                    
var_dump($my_test_string);
?>
--EXPECTF--
<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug00744.php:5:</small><small>string</small> <font color='#cc0000'>'hello&#13;&#10;                    world'</font> <i>(length=32)</i>
</pre>
