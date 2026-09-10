<?php
declare(strict_types=1);

namespace Xdebug;

class SampleClass
{
    private $field0 = '0';
    private $field1 = '1';
    private $field2 = '2';
    private $field3 = '3';

    public function getField0NonStatic(): string
    {
        return $this->field0;
    }

    public function getField1NonStatic(): string
    {
        return $this->field1;
    }

    public function getField2NonStatic(): string
    {
        return $this->field2;
    }

    public function getField3NonStatic(): string
    {
        return $this->field3;
    }
}
