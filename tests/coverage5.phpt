--TEST--
Test with Code Coverage with path and branch checking
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.trace_options=0
xdebug.trace_output_dir=/tmp
xdebug.collect_params=1
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.dump_globals=0
xdebug.show_mem_delta=0
xdebug.trace_format=0
xdebug.extended_info=1
xdebug.overload_var_dump=0
--FILE--
<?php
    xdebug_start_code_coverage(XDEBUG_CC_UNUSED | XDEBUG_CC_DEAD_CODE | XDEBUG_CC_BRANCH_CHECK);

	include 'coverage5.inc';

    xdebug_stop_code_coverage(false);
    $c = xdebug_get_code_coverage();
	ksort($c);
	var_dump($c);
?>
--EXPECTF--
A NOT B
array(2) {
  ["%scoverage5.inc"]=>
  array(2) {
    ["lines"]=>
    array(8) {
      [2]=>
      int(1)
      [3]=>
      int(1)
      [5]=>
      int(1)
      [6]=>
      int(1)
      [7]=>
      int(1)
      [8]=>
      int(-1)
      [9]=>
      int(-1)
      [11]=>
      int(1)
    }
    ["functions"]=>
    array(1) {
      ["{main}"]=>
      array(2) {
        ["branches"]=>
        array(9) {
          [0]=>
          array(6) {
            ["op_start"]=>
            int(0)
            ["op_end"]=>
            int(5)
            ["line_start"]=>
            int(2)
            ["line_end"]=>
            int(5)
            ["out"]=>
            array(2) {
              [6]=>
              int(0)
              [8]=>
              int(0)
            }
            ["hit"]=>
            int(1)
          }
          [6]=>
          array(6) {
            ["op_start"]=>
            int(6)
            ["op_end"]=>
            int(7)
            ["line_start"]=>
            int(5)
            ["line_end"]=>
            int(5)
            ["out"]=>
            array(1) {
              [8]=>
              int(0)
            }
            ["hit"]=>
            int(1)
          }
          [8]=>
          array(6) {
            ["op_start"]=>
            int(8)
            ["op_end"]=>
            int(8)
            ["line_start"]=>
            int(5)
            ["line_end"]=>
            int(5)
            ["out"]=>
            array(2) {
              [9]=>
              int(0)
              [12]=>
              int(0)
            }
            ["hit"]=>
            int(1)
          }
          [9]=>
          array(6) {
            ["op_start"]=>
            int(9)
            ["op_end"]=>
            int(11)
            ["line_start"]=>
            int(6)
            ["line_end"]=>
            int(7)
            ["out"]=>
            array(1) {
              [19]=>
              int(0)
            }
            ["hit"]=>
            int(1)
          }
          [12]=>
          array(6) {
            ["op_start"]=>
            int(12)
            ["op_end"]=>
            int(13)
            ["line_start"]=>
            int(7)
            ["line_end"]=>
            int(7)
            ["out"]=>
            array(2) {
              [14]=>
              int(0)
              [15]=>
              int(0)
            }
            ["hit"]=>
            int(0)
          }
          [14]=>
          array(6) {
            ["op_start"]=>
            int(14)
            ["op_end"]=>
            int(14)
            ["line_start"]=>
            int(7)
            ["line_end"]=>
            int(7)
            ["out"]=>
            array(1) {
              [15]=>
              int(0)
            }
            ["hit"]=>
            int(0)
          }
          [15]=>
          array(6) {
            ["op_start"]=>
            int(15)
            ["op_end"]=>
            int(15)
            ["line_start"]=>
            int(7)
            ["line_end"]=>
            int(7)
            ["out"]=>
            array(2) {
              [16]=>
              int(0)
              [19]=>
              int(0)
            }
            ["hit"]=>
            int(0)
          }
          [16]=>
          array(6) {
            ["op_start"]=>
            int(16)
            ["op_end"]=>
            int(18)
            ["line_start"]=>
            int(8)
            ["line_end"]=>
            int(9)
            ["out"]=>
            array(1) {
              [19]=>
              int(0)
            }
            ["hit"]=>
            int(0)
          }
          [19]=>
          array(6) {
            ["op_start"]=>
            int(19)
            ["op_end"]=>
            int(20)
            ["line_start"]=>
            int(11)
            ["line_end"]=>
            int(11)
            ["out"]=>
            array(0) {
            }
            ["hit"]=>
            int(1)
          }
        }
        ["paths"]=>
        array(10) {
          [0]=>
          array(5) {
            [0]=>
            int(0)
            [1]=>
            int(6)
            [2]=>
            int(8)
            [3]=>
            int(9)
            [4]=>
            int(19)
          }
          [1]=>
          array(8) {
            [0]=>
            int(0)
            [1]=>
            int(6)
            [2]=>
            int(8)
            [3]=>
            int(12)
            [4]=>
            int(14)
            [5]=>
            int(15)
            [6]=>
            int(16)
            [7]=>
            int(19)
          }
          [2]=>
          array(7) {
            [0]=>
            int(0)
            [1]=>
            int(6)
            [2]=>
            int(8)
            [3]=>
            int(12)
            [4]=>
            int(14)
            [5]=>
            int(15)
            [6]=>
            int(19)
          }
          [3]=>
          array(7) {
            [0]=>
            int(0)
            [1]=>
            int(6)
            [2]=>
            int(8)
            [3]=>
            int(12)
            [4]=>
            int(15)
            [5]=>
            int(16)
            [6]=>
            int(19)
          }
          [4]=>
          array(6) {
            [0]=>
            int(0)
            [1]=>
            int(6)
            [2]=>
            int(8)
            [3]=>
            int(12)
            [4]=>
            int(15)
            [5]=>
            int(19)
          }
          [5]=>
          array(4) {
            [0]=>
            int(0)
            [1]=>
            int(8)
            [2]=>
            int(9)
            [3]=>
            int(19)
          }
          [6]=>
          array(7) {
            [0]=>
            int(0)
            [1]=>
            int(8)
            [2]=>
            int(12)
            [3]=>
            int(14)
            [4]=>
            int(15)
            [5]=>
            int(16)
            [6]=>
            int(19)
          }
          [7]=>
          array(6) {
            [0]=>
            int(0)
            [1]=>
            int(8)
            [2]=>
            int(12)
            [3]=>
            int(14)
            [4]=>
            int(15)
            [5]=>
            int(19)
          }
          [8]=>
          array(6) {
            [0]=>
            int(0)
            [1]=>
            int(8)
            [2]=>
            int(12)
            [3]=>
            int(15)
            [4]=>
            int(16)
            [5]=>
            int(19)
          }
          [9]=>
          array(5) {
            [0]=>
            int(0)
            [1]=>
            int(8)
            [2]=>
            int(12)
            [3]=>
            int(15)
            [4]=>
            int(19)
          }
        }
      }
    }
  }
  ["%scoverage5.php"]=>
  array(2) {
    [4]=>
    int(1)
    [6]=>
    int(1)
  }
}
