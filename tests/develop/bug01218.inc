<?php

class StringCodeException extends Exception
{
    public function __construct()
    {
        $this->code = 'SomeString';
		$this->message = 'test';
    }
}

$e = new StringCodeException;

try {
    throw $e;
} catch (StringCodeException $e) {
    var_dump($e->getCode());
}
?>
