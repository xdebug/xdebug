--TEST--
Test star/stop debug session at runtime
--SKIPIF--
<?php if (getenv("SKIP_DBGP_TESTS")) { exit("skip Excluding DBGp tests"); } ?>
--INI--
xdebug.default_enable=1
xdebug.connect_back=1
--FILE--
<?php

	function echo_session_state_helper($check_id) {
		if(xdebug_is_debugger_active()) {
			echo $check_id . ': debugger active' . PHP_EOL;
		} else {
			echo $check_id . ': debugger inactive' . PHP_EOL;
		}
	}

	//Check (1) session stop before start
	xdebug_session_stop();
	echo_session_state_helper(1); //inactive

	//Check (2) simple session start/stop
	xdebug_session_start();
	echo_session_state_helper(2); //active
	xdebug_session_stop();
	echo_session_state_helper(2); //inactive

	//Check (3) session start/stop with IDE key
	xdebug_session_start("NEW_IDE_KEY3");
	echo_session_state_helper(3); //active
	xdebug_session_stop();
	echo_session_state_helper(3); //inactive

	//Check (4) simple session start twice and then stop once
	xdebug_session_start();
	echo_session_state_helper(4); //active
	xdebug_session_start();
	echo_session_state_helper(4); //active
	xdebug_session_stop();
	echo_session_state_helper(4); //inactive

	//Check (5) session with key start twice and then stop once
	xdebug_session_start("NEW_IDE_KEY5");
	echo_session_state_helper(5); //active
	xdebug_session_start("NEW_IDE_KEY5");
	echo_session_state_helper(5); //active
	xdebug_session_stop();
	echo_session_state_helper(5); //inactive

	//Check (6) simple session start once and then stop twice
	xdebug_session_start();
	echo_session_state_helper(6); //active
	xdebug_session_stop();
	echo_session_state_helper(6); //inactive
	xdebug_session_stop();
	echo_session_state_helper(6); //inactive

	//Check (7) session with key start once and then stop twice
	xdebug_session_start("NEW_IDE_KEY7");
	echo_session_state_helper(7); //active
	xdebug_session_stop();
	echo_session_state_helper(7); //inactive
	xdebug_session_stop();
	echo_session_state_helper(7); //inactive

	//Check (8) simple session start in loop
	for($i=0; $i<10; $i++) {
		xdebug_session_start();
		echo_session_state_helper(8); //active
	}

	//Check (9) session stop in loop
	for($i=0; $i<10; $i++) {
		xdebug_session_stop();
		echo_session_state_helper(9); //inactive
	}

	//Check (10) session with key start in loop
	for($i=0; $i<10; $i++) {
		xdebug_session_start("NEW_IDE_KEY9");
		echo_session_state_helper(10); //active
	}

	//Check (11) session with different key start twice and then stop once
	xdebug_session_start("NEW_IDE_KEY10_0");
	echo_session_state_helper(11); //active
	xdebug_session_start("NEW_IDE_KEY10_1");
	echo_session_state_helper(11); //active
	xdebug_session_stop();
	echo_session_state_helper(11); //inactive

?>
--EXPECTF--
1: debugger inactive
2: debugger active
2: debugger inactive
3: debugger active
3: debugger inactive
4: debugger active
4: debugger active
4: debugger inactive
5: debugger active
5: debugger active
5: debugger inactive
6: debugger active
6: debugger inactive
6: debugger inactive
7: debugger active
7: debugger inactive
7: debugger inactive
8: debugger active
8: debugger active
8: debugger active
8: debugger active
8: debugger active
8: debugger active
8: debugger active
8: debugger active
8: debugger active
8: debugger active
9: debugger inactive
9: debugger inactive
9: debugger inactive
9: debugger inactive
9: debugger inactive
9: debugger inactive
9: debugger inactive
9: debugger inactive
9: debugger inactive
9: debugger inactive
10: debugger active
10: debugger active
10: debugger active
10: debugger active
10: debugger active
10: debugger active
10: debugger active
10: debugger active
10: debugger active
10: debugger active
11: debugger active
11: debugger active
11: debugger inactive
