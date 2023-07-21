<?php
class test
{
    function testFunc()
    {
        $coveredLines = [0 => []];
        $currentFile = 0;
        $reader = new reader();

        $coveredLines
            [$currentFile]
            [$reader->getAttribute('num')]
            = (int) $reader->getAttribute("count");
    }
}
class reader
{
    public function getAttribute()
    {
        return 0;
    }
}
