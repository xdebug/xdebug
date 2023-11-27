<?php

$data = "data://text/plain;base64,".base64_encode("<"."?php\nreturn function() {\nxdebug_break();\n};");

$f = include($data);
$f();
