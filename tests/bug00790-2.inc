<?php
function callback($buffer)
{
      // replace all the apples with oranges
      return "[$buffer]\n";
}
ob_start('callback');

function foo()
{
    // empty func
    echo "again\n";
}
xdebug_break();
trigger_error('test');
echo "test\n";
foo();
?>
