<?php
function exit_test()
{
	if (random_int(0, 1) > 2) {
		exit;

		echo "foo";
	}
}

exit_test();
?>
