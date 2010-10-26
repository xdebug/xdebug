<?php
    function call($param1, $param2, $param3)
    {
        echo $param1;
        echo $param2;
        echo $param3;

		var_dump(xdebug_get_function_stack());
    }

    $test=getcwd();
	$value = 'candena\\a\nb';
	echo $value, "\n";
    call($test, $value, 'caneda \\\a \\\\b \\\\\c|');
