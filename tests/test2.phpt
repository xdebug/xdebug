--TEST--
Test with xdebug_get_function_trace
--INI--
xdebug.auto_trace=0
--FILE--
<?php
	xdebug_start_trace();
	include 'test2a.php';

	function foo4 ($a, $b, $c)
	{
		echo "In foo4: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		foo3 ($b, $c, $a);
	}

	function foo5 ($a, $b, $c)
	{
		echo "In foo5: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		foo4 ($b, $c, $a);
	}

	function foo6 ($a, $b, $c)
	{
		echo "In foo6: ".xdebug_call_function();
		echo "-".xdebug_call_line();
		echo "-".xdebug_call_file(). "\n";
		foo5 ($b, $c, $a);
	}


	echo foo6 (1,2,3);
	var_dump (xdebug_get_function_trace());
?>
--EXPECTF--
In foo6: {main}-30-/%s/test2.php
In foo5: foo6-26-/%s/test2.php
In foo4: foo5-18-/%s/test2.php
In foo3: foo4-10-/%s/test2.php
In foo2: foo3-23-/%s/test2a.php
In foo1: foo2-15-/%s/test2a.php
array(7) {
  [0]=>
  array(4) {
    ["function"]=>
    string(6) "{main}"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(0)
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(4) {
    ["function"]=>
    string(4) "foo6"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(30)
    ["params"]=>
    array(3) {
      [0]=>
      string(1) "1"
      [1]=>
      string(1) "2"
      [2]=>
      string(1) "3"
    }
  }
  [2]=>
  array(4) {
    ["function"]=>
    string(4) "foo5"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(26)
    ["params"]=>
    array(3) {
      [0]=>
      string(1) "2"
      [1]=>
      string(1) "3"
      [2]=>
      string(1) "1"
    }
  }
  [3]=>
  array(4) {
    ["function"]=>
    string(4) "foo4"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(18)
    ["params"]=>
    array(3) {
      [0]=>
      string(1) "3"
      [1]=>
      string(1) "1"
      [2]=>
      string(1) "2"
    }
  }
  [4]=>
  array(4) {
    ["function"]=>
    string(4) "foo3"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(10)
    ["params"]=>
    array(3) {
      [0]=>
      string(1) "1"
      [1]=>
      string(1) "2"
      [2]=>
      string(1) "3"
    }
  }
  [5]=>
  array(4) {
    ["function"]=>
    string(4) "foo2"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(23)
    ["params"]=>
    array(3) {
      [0]=>
      string(1) "2"
      [1]=>
      string(1) "3"
      [2]=>
      string(1) "1"
    }
  }
  [6]=>
  array(4) {
    ["function"]=>
    string(4) "foo1"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(15)
    ["params"]=>
    array(3) {
      [0]=>
      string(1) "3"
      [1]=>
      string(1) "1"
      [2]=>
      string(1) "2"
    }
  }
}
array(27) {
  [0]=>
  array(4) {
    ["function"]=>
    string(6) "{main}"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(0)
    ["params"]=>
    array(0) {
    }
  }
  [1]=>
  array(4) {
    ["function"]=>
    string(4) "foo6"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(30)
    ["params"]=>
    array(1) {
      [1]=>
      string(1) "3"
    }
  }
  [2]=>
  array(4) {
    ["function"]=>
    string(20) "xdebug_call_function"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(23)
    ["params"]=>
    array(0) {
    }
  }
  [3]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_line"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(24)
    ["params"]=>
    array(0) {
    }
  }
  [4]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_file"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(25)
    ["params"]=>
    array(0) {
    }
  }
  [5]=>
  array(4) {
    ["function"]=>
    string(4) "foo5"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(26)
    ["params"]=>
    array(1) {
      [1]=>
      string(1) "1"
    }
  }
  [6]=>
  array(4) {
    ["function"]=>
    string(20) "xdebug_call_function"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(15)
    ["params"]=>
    array(0) {
    }
  }
  [7]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_line"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(16)
    ["params"]=>
    array(0) {
    }
  }
  [8]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_file"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(17)
    ["params"]=>
    array(0) {
    }
  }
  [9]=>
  array(4) {
    ["function"]=>
    string(4) "foo4"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(18)
    ["params"]=>
    array(1) {
      [1]=>
      string(1) "2"
    }
  }
  [10]=>
  array(4) {
    ["function"]=>
    string(20) "xdebug_call_function"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(7)
    ["params"]=>
    array(0) {
    }
  }
  [11]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_line"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(8)
    ["params"]=>
    array(0) {
    }
  }
  [12]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_file"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(9)
    ["params"]=>
    array(0) {
    }
  }
  [13]=>
  array(4) {
    ["function"]=>
    string(4) "foo3"
    ["file"]=>
    string(%d) "/%s/test2.php"
    ["line"]=>
    int(10)
    ["params"]=>
    array(1) {
      [1]=>
      string(1) "3"
    }
  }
  [14]=>
  array(4) {
    ["function"]=>
    string(20) "xdebug_call_function"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(20)
    ["params"]=>
    array(0) {
    }
  }
  [15]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_line"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(21)
    ["params"]=>
    array(0) {
    }
  }
  [16]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_file"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(22)
    ["params"]=>
    array(0) {
    }
  }
  [17]=>
  array(4) {
    ["function"]=>
    string(4) "foo2"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(23)
    ["params"]=>
    array(1) {
      [1]=>
      string(1) "1"
    }
  }
  [18]=>
  array(4) {
    ["function"]=>
    string(20) "xdebug_call_function"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(12)
    ["params"]=>
    array(0) {
    }
  }
  [19]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_line"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(13)
    ["params"]=>
    array(0) {
    }
  }
  [20]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_file"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(14)
    ["params"]=>
    array(0) {
    }
  }
  [21]=>
  array(4) {
    ["function"]=>
    string(4) "foo1"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(15)
    ["params"]=>
    array(1) {
      [1]=>
      string(1) "2"
    }
  }
  [22]=>
  array(4) {
    ["function"]=>
    string(20) "xdebug_call_function"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(4)
    ["params"]=>
    array(0) {
    }
  }
  [23]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_line"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(5)
    ["params"]=>
    array(0) {
    }
  }
  [24]=>
  array(4) {
    ["function"]=>
    string(16) "xdebug_call_file"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(6)
    ["params"]=>
    array(0) {
    }
  }
  [25]=>
  array(4) {
    ["function"]=>
    string(25) "xdebug_get_function_stack"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(7)
    ["params"]=>
    array(0) {
    }
  }
  [26]=>
  array(4) {
    ["function"]=>
    string(8) "var_dump"
    ["file"]=>
    string(%d) "/%s/test2a.php"
    ["line"]=>
    int(7)
    ["params"]=>
    array(1) {
      [1]=>
      string(%d) "array (0 => array ('function' => '{main}', 'file' => '/%s/test2.php', 'line' => 0, 'params' => array)), 1 => array ('function' => 'foo6', 'file' => '/%s/test2.php', 'line' => 30, 'params' => array (0 => '1', 1 => '2', 2 => '3')), 2 => array ('function' => 'foo5', 'file' => '/%s/test2.php', 'line' => 26, 'params' => array (0 => '2', 1 => '3', 2 => '1')), 3 => array ('function' => 'foo4', 'file' => '/%s/test2.php', 'line' => 18, 'params' => array (0 => '3', 1 => '1', 2 => '2')), 4 => array ('function' => 'foo3', 'file' => '/%s/test2.php', 'line' => 10, 'params' => array (0 => '1', 1 => '2', 2 => '3')), 5 => array ('function' => 'foo2', 'file' => '/%s/test2a.php', 'line' => 23, 'params' => array (0 => '2', 1 => '3', 2 => '1')), 6 => array ('function' => 'foo1', 'file' => '/%s/test2a.php', 'line' => 15, 'params' => array (0 => '3', 1 => '1', 2 => '2')))"
    }
  }
}
