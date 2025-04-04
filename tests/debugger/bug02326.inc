<?php
$reader = null;
function ReloadXmlFile($fname)
{
    global $reader;
    $reader = new XMLReader();

    $err = $reader->open($fname);
    return $err;
}

$err = ReloadXmlFile(__DIR__ . "/bug02326.xml");
if (!$err)
{
    echo ("error");
}
echo ("Next line1");
echo ("Next line2");
?>
