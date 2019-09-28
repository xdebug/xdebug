<?php
$text = <<<EOD
function foo(\$a1, \$b1) {
  echo("In foo\\nline two of echo\\n");
  return \$a1 = \$b1;
}
EOD;
$text = str_replace("\r\n", "\n", $text);
 eval( $text );
$res = foo(3, 4);
echo $res, "\n";
?>
