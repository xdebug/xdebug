--TEST--
Test with Code Coverage with unused lines
--SKIPIF--
<?php if (!extension_loaded("xdebug")) print "skip"; ?>
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.extended_info=1
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
  array(12) {
    [2]=>
    int(-1)
    [4]=>
    int(1)
    [5]=>
    int(1)
    [6]=>
    int(-1)
    [7]=>
    int(-1)
    [9]=>
    int(1)
    [10]=>
    int(1)
    [11]=>
    int(1)
    [12]=>
    int(-1)
    [13]=>
    int(-1)
    [17]=>
    int(1)
    [19]=>
    int(1)
  }
}
