<?php
function extractFile()
{
	try {
		echo "try\n";
	} catch (Catch1 $e) {
		echo "catch1\n";
	} catch (Catch2 $e) {
		echo "catch2\n";
	} finally {
		echo "finally\n";
	}
	echo "end\n";
}

extractFile();
?>
