--TEST--
Test with Code Coverage with unused lines
--INI--
xdebug.mode=coverage
xdebug.trace_options=0
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.dump_globals=0
xdebug.trace_format=0
--FILE--
<?php
    function a($b)
    {
        if ($b == 0)
        {
            return true;
        }
        else
        if ($b == 1)
        {
            return false;
        }
    };

    xdebug_start_code_coverage(true);

    a(1);

    xdebug_stop_code_coverage(false);
    var_dump(xdebug_get_code_coverage());
?>
--EXPECTF--
array(1) {
  ["%scoverage3.php"]=>
  array(7) {
    [4]=>
    int(1)
    [6]=>
    int(-1)
    [9]=>
    int(1)
    [11]=>
    int(1)
    [13]=>
    int(-1)
    [17]=>
    int(1)
    [19]=>
    int(1)
  }
}
