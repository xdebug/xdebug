--TEST--
Test for bug #1790: Segfault in var_dump() or while debugging with protobuf extension
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext protobuf');
?>
--INI--
xdebug.mode=develop
--FILE--
<?php
ini_set('html_errors', 0);
ini_set('xdebug.cli_color', 0);
var_dump(new \Google\Protobuf\Timestamp());

echo "\n--\n\n";

ini_set('html_errors', 1);
ini_set('xdebug.cli_color', 0);
var_dump(new \Google\Protobuf\Timestamp());

echo "\n\n--\n\n";

ini_set('html_errors', 0);
ini_set('xdebug.cli_color', 1);
var_dump(new \Google\Protobuf\Timestamp());
?>
--EXPECTF--
%sbug01790.php:%d:
class Google\Protobuf\Timestamp#%d (0) {
  ...
}

--

<pre class='xdebug-var-dump' dir='ltr'>
<small>%sbug01790.php:%d:</small>
<b>object</b>(<i>Google\Protobuf\Timestamp</i>)[<i>%d</i>]
  ...
</pre>

--

%sbug01790.php:%d:
class Google\Protobuf\Timestamp#%d (0) {
  ...
}%A
