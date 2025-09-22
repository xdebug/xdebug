<?php
namespace Xdebug;

require 'vendor/autoload.php';

$c = new Counter(42);
foreach ($c->range() as $number)
{
	echo $number, ' ';
}
