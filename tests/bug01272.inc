<?php

$xml = "<a att1='att-a'><b attb='attb-1'/><b attb='attb-2'/></a>";

$e = new SimpleXmlElement($xml);

foreach($e->b as $b) { // breakpoint here
    echo $b['attb'].PHP_EOL;
}
?>
