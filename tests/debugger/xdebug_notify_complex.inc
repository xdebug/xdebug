<?php
$test = [
	[ 'two' => [ 'trois' => [ 'bar' ] ] ],
	[ 'long_string' => str_repeat( 'a', 1024 ) ],
	[ 'letters' => range( 'a', 'x' ) ],
];

$settings = [
	[], // use default passed on command line
	[ 'xdebug.var_display_max_depth' => 2, 'xdebug.var_display_max_children' => 2, 'xdebug.var_display_max_data' => 32 ],
	[ 'xdebug.var_display_max_depth' => -1, 'xdebug.var_display_max_children' => 2, 'xdebug.var_display_max_data' => 32 ],
	[ 'xdebug.var_display_max_depth' => -1, 'xdebug.var_display_max_children' => 16, 'xdebug.var_display_max_data' => 4 ],
];

foreach ($settings as $setting_group) {
	foreach ($setting_group as $setting => $value) {
		ini_set($setting, $value);
	}

	xdebug_notify($test);
}
