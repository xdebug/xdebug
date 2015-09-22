<?php








function errors_fatal($a) {
	$ia = 'is_array';
	count($a) && $ia($a);
	return FALSE;
}
?>
