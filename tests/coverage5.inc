<?php
$a = true;
$b = false;

if ($a && !$b) {
	echo "A NOT B\n";
} else if (!$a && $b) {
	echo "B NOT A\n";
}
?>
