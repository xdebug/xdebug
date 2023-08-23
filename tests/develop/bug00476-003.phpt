--TEST--
Test for bug #476: Exception chanining doesn't work (HTML)
--INI--
xdebug.mode=develop
xdebug.dump.GET=
xdebug.dump.SERVER=
xdebug.show_local_vars=0
html_errors=1
xdebug.cli_color=0
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
?>
--FILE--
<?php
function throwSecondException( Exception $e )
{
   throw new Exception('Second exception', 0, $e);
}

class thrower
{
	function __construct( private Exception $e )
	{
	}

	function throwMe()
	{
		throw new Exception('Third exception', 42, $this->e );
	}
}

try {
   throw new Exception('First exception');
} catch(Exception $e) {
	try {
		throwSecondException( $e );
	} catch(Exception $f) {
		$t = new thrower($f);
		$t->throwMe();
	}
}
echo "DONE\n";
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-uncaught-exception' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Fatal error: Uncaught Exception: First exception in %sbug00476-003.php:20
Stack trace:
#0 {main}

Next Exception: Second exception in %sbug00476-003.php:4
Stack trace:
#0 %sbug00476-003.php(23): throwSecondException(Object(Exception))
#1 {main}

Next Exception: Third exception in %sbug00476-003.php on line <i>15</i></th></tr>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Exception: Third exception in %sbug00476-003.php on line <i>15</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug00476-003.php' bgcolor='#eeeeec'>%sbug00476-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>thrower->throwMe(  )</td><td title='%sbug00476-003.php' bgcolor='#eeeeec'>%sbug00476-003.php<b>:</b>26</td></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Nested Exceptions</th></tr><tr><td colspan='5'>
<table class='xdebug-error xe-nested' style='width: 80%; margin: 1em' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Exception: Second exception in %sbug00476-003.php on line <i>4</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}()</td><td title='%sbug00476-003.php' bgcolor='#eeeeec'>%sbug00476-003.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>throwSecondException()</td><td title='%sbug00476-003.php' bgcolor='#eeeeec'>%sbug00476-003.php<b>:</b>23</td></tr>
<table class='xdebug-error xe-nested' style='width: 80%; margin: 1em' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Exception: First exception in %sbug00476-003.php on line <i>20</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}()</td><td title='%sbug00476-003.php' bgcolor='#eeeeec'>%sbug00476-003.php<b>:</b>0</td></tr>
</table></tr>
</table></font>
