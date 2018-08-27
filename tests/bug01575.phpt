--TEST--
Test star/stop debug session at runtime
--INI--
xdebug.default_enable=1
xdebug.connect_back=1
--FILE--
<?php
    xdebug_session_start();
    echo 'ok1'.PHP_EOL;

    xdebug_session_stop();
    echo 'ok2'.PHP_EOL;

    xdebug_session_start("NEW_IDE_KEY");
    echo 'ok3'.PHP_EOL;

    xdebug_session_stop();
    echo 'ok4'.PHP_EOL;
?>
--EXPECTF--
ok1
ok2
ok3
ok4