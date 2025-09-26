<?php
namespace Xdebug;

require 'vendor/autoload.inc';

$c = new Counter(42);
foreach ($c->range() as $number)
{
	echo $number, ' ';
}
