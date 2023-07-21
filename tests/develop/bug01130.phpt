--TEST--
Test for bug #1130: PHP documentation links to local manual reference are broken at title description
--INI--
docref_root=http://www.php.net/
docref_ext=.php
html_errors=1
xdebug.mode=develop
xdebug.file_link_format=
xdebug.filename_format=
--FILE--
<?php
html_entity_decode("&amp;", 0, "l<r");
?>
--EXPECTF--
<br />
<font size='1'><table class='xdebug-error xe-warning' dir='ltr' border='1' cellspacing='0' cellpadding='1'>
<tr><th align='left' bgcolor='#f57900' colspan="5"><span style='background-color: #cc0000; color: #fce94f; font-size: x-large;'>( ! )</span> Warning: html_entity_decode() [<a href='http://www.php.net/function.html-entity-decode.php'>function.html-entity-decode.php</a>: %sharset %sl&amp;lt;r%s not supported, assuming %s8 in %sbug01130.php on line <i>2</i></th></tr>
<tr><th align='left' bgcolor='#e9b96e' colspan='5'>Call Stack</th></tr>
<tr><th align='center' bgcolor='#eeeeec'>#</th><th align='left' bgcolor='#eeeeec'>Time</th><th align='left' bgcolor='#eeeeec'>Memory</th><th align='left' bgcolor='#eeeeec'>Function</th><th align='left' bgcolor='#eeeeec'>Location</th></tr>
<tr><td bgcolor='#eeeeec' align='center'>1</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'>{main}(  )</td><td title='%sbug01130.php' bgcolor='#eeeeec'>...%sbug01130.php<b>:</b>0</td></tr>
<tr><td bgcolor='#eeeeec' align='center'>2</td><td bgcolor='#eeeeec' align='center'>%f</td><td bgcolor='#eeeeec' align='right'>%d</td><td bgcolor='#eeeeec'><a href='http://www.php.net/function.html-entity-decode.php' target='_new'>html_entity_decode</a>( <span>$string = </span><span>&#39;&amp;amp;&#39;</span>, <span>$%s = </span><span>0</span>, <span>$encoding = </span><span>&#39;l&lt;r&#39;</span> )</td><td title='%sbug01130.php' bgcolor='#eeeeec'>...%sbug01130.php<b>:</b>2</td></tr>
</table></font>
