<?php
$var = '';
for ($i = 0; $i < 1000; $i++) {
	$var .= sprintf("%05d ", $i );
}
echo $var; //breakpoint here
