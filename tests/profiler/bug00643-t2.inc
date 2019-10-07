<?php








function errors_fatal($a) {
	$ia = 'is_array';
	array_count_values($a) && $ia($a);
	return FALSE;
}
?>
