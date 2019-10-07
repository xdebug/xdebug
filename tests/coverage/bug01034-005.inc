<?php
function trycatch()
{
	try {
		throw new Exception("test");
		echo "Not thrown\n";
	} catch (Exception1 $e) {
		echo "caught\n";
	} catch (Exception2 $e) {
		echo "caught\n";
	} catch (Exception $e) {
		echo "caught\n";
	} catch (Exception4 $e) {
		echo "caught\n";
	}
}
trycatch();
?>
