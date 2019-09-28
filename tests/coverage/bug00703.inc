<?php

$a = 'foo';

$b = <<<EOF
the line after this is marked as not executed ${a}

EOF;

$c = <<<EOF
the line after ${a} is marked as executed

EOF;
?>
