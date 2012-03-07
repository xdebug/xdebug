--TEST--
Test for bug #787: Segmentation Fault with PHP header_remove()
--SKIPIF--
<?php if (!version_compare(phpversion(), "5.3", '>=')) echo "skip >= PHP 5.3 needed\n"; ?>
--FILE--
<?php
class Utils
{
    public static function redirect( $url)
    {
        header_remove();
        exit( 'After header_remove()');
    }
}
Utils::redirect('');
--EXPECT--
After header_remove()
