<?php
class Unzip
{
    public function extractFile()
    {
        try {
            echo "before true\n";
            return true;
            echo "after true\n";
        } finally {
            echo "Do somthing\n";
        }
        echo "More stuff\n";
    }
}
?>
