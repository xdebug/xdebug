--TEST--
Test for bug #562: Incorrect coverage information for closure function headers
--INI--
xdebug.mode=coverage
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
  array(6) {
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
    [13]=>
    int(1)
  }
}

