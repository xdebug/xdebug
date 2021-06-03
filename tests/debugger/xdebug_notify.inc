<?php
xdebug_notify( range( 'a', 'x' ) );
xdebug_notify( [ "test\0bar" => 42 ] );
xdebug_notify( [ "test" => "foo\0bar" ] );
