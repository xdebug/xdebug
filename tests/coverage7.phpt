--TEST--
Test with Code Coverage with path and branch checking
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.3", '>=')) echo "skip >= PHP 5.3 needed\n"; ?>
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

	include 'coverage7.inc';

    xdebug_stop_code_coverage(false);
    $c = xdebug_get_code_coverage();
	ksort($c);
	var_dump($c);
?>
--EXPECTF--
A NOT B
2
1
array(2) {
  ["%scoverage7.inc"]=>
  array(2) {
    ["lines"]=>
    array(20) {
      [2]=>
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
      [10]=>
      int(1)
      [15]=>
      int(1)
      [16]=>
      int(1)
      [17]=>
      int(1)
      [21]=>
      int(1)
      [22]=>
      int(1)
      [23]=>
      int(1)
      [25]=>
      int(1)
      [26]=>
      int(1)
      [29]=>
      int(1)
      [31]=>
      int(1)
      [32]=>
      int(1)
      [33]=>
      int(1)
      [35]=>
      int(1)
    }
    ["functions"]=>
    array(5) {
      ["foo->loop_test"]=>
      array(2) {
        ["branches"]=>
        array(3) {
          [0]=>
          array(6) {
            ["op_start"]=>
            int(0)
            ["op_end"]=>
            int(2)
            ["line_start"]=>
            int(12)
            ["line_end"]=>
            int(15)
            ["out"]=>
            array(1) {
              [3]=>
              int(0)
            }
            ["hit"]=>
            int(1)
          }
          [3]=>
          array(6) {
            ["op_start"]=>
            int(3)
            ["op_end"]=>
            int(7)
            ["line_start"]=>
            int(15)
            ["line_end"]=>
            int(16)
            ["out"]=>
            array(2) {
              [8]=>
              int(0)
              [3]=>
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
            int(9)
            ["line_start"]=>
            int(17)
            ["line_end"]=>
            int(17)
            ["out"]=>
            array(0) {
            }
            ["hit"]=>
            int(1)
          }
        }
        ["paths"]=>
        array(2) {
          [0]=>
          array(3) {
            [0]=>
            int(0)
            [1]=>
            int(3)
            [2]=>
            int(8)
          }
          [1]=>
          array(4) {
            [0]=>
            int(0)
            [1]=>
            int(3)
            [2]=>
            int(3)
            [3]=>
            int(8)
          }
        }
      }
      ["{closure:%scoverage7.inc:21-23}"]=>
      array(2) {
        ["branches"]=>
        array(1) {
          [0]=>
          array(6) {
            ["op_start"]=>
            int(0)
            ["op_end"]=>
            int(7)
            ["line_start"]=>
            int(21)
            ["line_end"]=>
            int(23)
            ["out"]=>
            array(0) {
            }
            ["hit"]=>
            int(0)
          }
        }
        ["paths"]=>
        array(1) {
          [0]=>
          array(1) {
            [0]=>
            int(0)
          }
        }
      }
      ["{main}"]=>
      array(2) {
        ["branches"]=>
        array(1) {
          [0]=>
          array(6) {
            ["op_start"]=>
            int(0)
            ["op_end"]=>
            int(28)
            ["line_start"]=>
            int(2)
            ["line_end"]=>
            int(35)
            ["out"]=>
            array(0) {
            }
            ["hit"]=>
            int(1)
          }
        }
        ["paths"]=>
        array(1) {
          [0]=>
          array(1) {
            [0]=>
            int(0)
          }
        }
      }
      ["foo->test_closure"]=>
      array(2) {
        ["branches"]=>
        array(1) {
          [0]=>
          array(6) {
            ["op_start"]=>
            int(0)
            ["op_end"]=>
            int(13)
            ["line_start"]=>
            int(19)
            ["line_end"]=>
            int(26)
            ["out"]=>
            array(0) {
            }
            ["hit"]=>
            int(1)
          }
        }
        ["paths"]=>
        array(1) {
          [0]=>
          array(1) {
            [0]=>
            int(0)
          }
        }
      }
      ["foo->ok"]=>
      array(2) {
        ["branches"]=>
        array(9) {
          [0]=>
          array(6) {
            ["op_start"]=>
            int(0)
            ["op_end"]=>
            int(4)
            ["line_start"]=>
            int(3)
            ["line_end"]=>
            int(5)
            ["out"]=>
            array(2) {
              [5]=>
              int(0)
              [7]=>
              int(0)
            }
            ["hit"]=>
            int(1)
          }
          [5]=>
          array(6) {
            ["op_start"]=>
            int(5)
            ["op_end"]=>
            int(6)
            ["line_start"]=>
            int(5)
            ["line_end"]=>
            int(5)
            ["out"]=>
            array(1) {
              [7]=>
              int(0)
            }
            ["hit"]=>
            int(1)
          }
          [7]=>
          array(6) {
            ["op_start"]=>
            int(7)
            ["op_end"]=>
            int(7)
            ["line_start"]=>
            int(5)
            ["line_end"]=>
            int(5)
            ["out"]=>
            array(2) {
              [8]=>
              int(0)
              [11]=>
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
            int(10)
            ["line_start"]=>
            int(6)
            ["line_end"]=>
            int(7)
            ["out"]=>
            array(1) {
              [18]=>
              int(0)
            }
            ["hit"]=>
            int(1)
          }
          [11]=>
          array(6) {
            ["op_start"]=>
            int(11)
            ["op_end"]=>
            int(12)
            ["line_start"]=>
            int(7)
            ["line_end"]=>
            int(7)
            ["out"]=>
            array(2) {
              [13]=>
              int(0)
              [14]=>
              int(0)
            }
            ["hit"]=>
            int(0)
          }
          [13]=>
          array(6) {
            ["op_start"]=>
            int(13)
            ["op_end"]=>
            int(13)
            ["line_start"]=>
            int(7)
            ["line_end"]=>
            int(7)
            ["out"]=>
            array(1) {
              [14]=>
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
            array(2) {
              [15]=>
              int(0)
              [18]=>
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
            int(17)
            ["line_start"]=>
            int(8)
            ["line_end"]=>
            int(9)
            ["out"]=>
            array(1) {
              [18]=>
              int(0)
            }
            ["hit"]=>
            int(0)
          }
          [18]=>
          array(6) {
            ["op_start"]=>
            int(18)
            ["op_end"]=>
            int(19)
            ["line_start"]=>
            int(10)
            ["line_end"]=>
            int(10)
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
            int(5)
            [2]=>
            int(7)
            [3]=>
            int(8)
            [4]=>
            int(18)
          }
          [1]=>
          array(8) {
            [0]=>
            int(0)
            [1]=>
            int(5)
            [2]=>
            int(7)
            [3]=>
            int(11)
            [4]=>
            int(13)
            [5]=>
            int(14)
            [6]=>
            int(15)
            [7]=>
            int(18)
          }
          [2]=>
          array(7) {
            [0]=>
            int(0)
            [1]=>
            int(5)
            [2]=>
            int(7)
            [3]=>
            int(11)
            [4]=>
            int(13)
            [5]=>
            int(14)
            [6]=>
            int(18)
          }
          [3]=>
          array(7) {
            [0]=>
            int(0)
            [1]=>
            int(5)
            [2]=>
            int(7)
            [3]=>
            int(11)
            [4]=>
            int(14)
            [5]=>
            int(15)
            [6]=>
            int(18)
          }
          [4]=>
          array(6) {
            [0]=>
            int(0)
            [1]=>
            int(5)
            [2]=>
            int(7)
            [3]=>
            int(11)
            [4]=>
            int(14)
            [5]=>
            int(18)
          }
          [5]=>
          array(4) {
            [0]=>
            int(0)
            [1]=>
            int(7)
            [2]=>
            int(8)
            [3]=>
            int(18)
          }
          [6]=>
          array(7) {
            [0]=>
            int(0)
            [1]=>
            int(7)
            [2]=>
            int(11)
            [3]=>
            int(13)
            [4]=>
            int(14)
            [5]=>
            int(15)
            [6]=>
            int(18)
          }
          [7]=>
          array(6) {
            [0]=>
            int(0)
            [1]=>
            int(7)
            [2]=>
            int(11)
            [3]=>
            int(13)
            [4]=>
            int(14)
            [5]=>
            int(18)
          }
          [8]=>
          array(6) {
            [0]=>
            int(0)
            [1]=>
            int(7)
            [2]=>
            int(11)
            [3]=>
            int(14)
            [4]=>
            int(15)
            [5]=>
            int(18)
          }
          [9]=>
          array(5) {
            [0]=>
            int(0)
            [1]=>
            int(7)
            [2]=>
            int(11)
            [3]=>
            int(14)
            [4]=>
            int(18)
          }
        }
      }
    }
  }
  ["%scoverage7.php"]=>
  array(2) {
    [4]=>
    int(1)
    [6]=>
    int(1)
  }
}
