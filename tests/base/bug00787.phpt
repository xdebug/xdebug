--TEST--
Test for bug #787: Segmentation Fault with PHP header_remove()
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
