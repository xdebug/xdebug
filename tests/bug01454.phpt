--TEST--
Test for bug #1454: Seeing invalid memory read or segfaults from a __call() method
--INI--
xdebug.default_enable=1
xdebug.auto_trace=0
xdebug.collect_params=3
xdebug.collect_return=0
xdebug.collect_assignments=0
xdebug.auto_profile=0
xdebug.profiler_enable=0
xdebug.show_mem_delta=1
xdebug.trace_format=2
--ENV--
USE_ZEND_ALLOC=0
--FILE--
<?php

/**
 * Constraint that accepts any input value.
 */
class IsAnything
{
    protected $exporter;

    public function __construct() {
        $this->exporter = new Exporter;
    }
}

class Exporter { }

class __phockito_mockedclass_Mock {};

class Phockito {
    /* ** INTERNAL INTERFACES END ** */

    static function mock(string $class) {
        return new __phockito_mockedclass_Mock();
    }

    static function verify($mock, $times = 1) {
        return new Phockito_VerifyBuilder();
    }
}

class Phockito_VerifyBuilder {

    function __call($called, $args) {
        printf("Verifying Phockito_Verify_Builder->%s\n", var_export($args, true));
        flush();

        throw new \Exception('x');
    }
}

function anything()
{
    return new IsAnything();
}

////////////////////////////////////////////////////////////////////////////////
// This is the main part of the test case, below
////////////////////////////////////////////////////////////////////////////////

class Test {
    public function outer() {
        $mock = new __phockito_mockedclass_Mock;
        Phockito::verify($mock, 1)->add(anything(), anything(), anything(), anything(), 1500, 3000, anything(), 'P', 3, anything(), anything(), 'U', anything(), anything(), 0);
    }
}

function foo() {
    $t = new Test();
    $t->outer();
}
foo();
?>
==DONE==
--EXPECTF--
Verifying Phockito_Verify_Builder->array (
  0 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  1 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  2 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  3 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  4 => 1500,
  5 => 3000,
  6 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  7 => 'P',
  8 => 3,
  9 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  10 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  11 => 'U',
  12 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  13 => 
  IsAnything::__set_state(array(
     'exporter' => 
    Exporter::__set_state(array(
    )),
  )),
  14 => 0,
)

Fatal error: Uncaught Exception: x in %sbug01454.php on line 37

Exception: x in %sbug01454.php on line 37

Call Stack:
%w%f %w%d   1. {main}() %sbug01454.php:0
%w%f %w%d   2. foo() %sbug01454.php:61
%w%f %w%d   3. Test->outer() %sbug01454.php:59
%w%f %w%d   4. Phockito_VerifyBuilder->add(class IsAnything { protected $exporter = class Exporter {  } }, class IsAnything { protected $exporter = class Exporter {  } }, class IsAnything { protected $exporter = class Exporter {  } }, class IsAnything { protected $exporter = class Exporter {  } }, 1500, 3000, class IsAnything { protected $exporter = class Exporter {  } }, 'P', 3, class IsAnything { protected $exporter = class Exporter {  } }, class IsAnything { protected $exporter = class Exporter {  } }, 'U', class IsAnything { protected $exporter = class Exporter {  } }, class IsAnything { protected $exporter = class Exporter {  } }, 0) %sbug01454.php:53
%w%f %w%d   5. Phockito_VerifyBuilder->__call('add', array (0 => class IsAnything { protected $exporter = class Exporter { ... } }, 1 => class IsAnything { protected $exporter = class Exporter { ... } }, 2 => class IsAnything { protected $exporter = class Exporter { ... } }, 3 => class IsAnything { protected $exporter = class Exporter { ... } }, 4 => 1500, 5 => 3000, 6 => class IsAnything { protected $exporter = class Exporter { ... } }, 7 => 'P', 8 => 3, 9 => class IsAnything { protected $exporter = class Exporter { ... } }, 10 => class IsAnything { protected $exporter = class Exporter { ... } }, 11 => 'U', 12 => class IsAnything { protected $exporter = class Exporter { ... } }, 13 => class IsAnything { protected $exporter = class Exporter { ... } }, 14 => 0)) %sbug01454.php:53
