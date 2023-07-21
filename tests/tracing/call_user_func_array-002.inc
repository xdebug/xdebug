<?php
function debug($var, $val) {
    $ia = 'is_array'; $io = 'is_object'; $ir = 'is_resource';
    if ($ia($val) || $io($val) || $ir($val)) {
        /* Do nothing */
    } else {
        /* Do nothing */
	}
}
?>
