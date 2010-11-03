--TEST--
Test for bug #562: Incorrect coverage information for closure function headers
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
<?php if(version_compare(phpversion(), "5.3.0", '<')) echo "skip PHP 5.3 needed\n"; ?>
--INI--
xdebug.coverage_enable=1
--FILE--
<?php

xdebug_start_code_coverage();
$mapped = array_map(
    // This line is flagged as executable, but not covered in PHPUnit code coverage reports
    function ( $value )
    {
        return $value;
    },
    array( 23, 42 )
);

var_dump( xdebug_get_code_coverage() );
--EXPECTF--
array(1) {
  ["%sbug00562.php"]=>
  array(8) {
    [0]=>
    int(1)
    [4]=>
    int(1)
    [6]=>
    int(1)
    [8]=>
    int(1)
    [9]=>
    int(1)
    [10]=>
    int(1)
    [11]=>
    int(1)
    [13]=>
    int(1)
  }
}

