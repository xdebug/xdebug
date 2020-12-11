<?php
class Security
{
}

class Foo
{
	public function __construct(
		private int      $ft,
		private Security $security,
	) {}
}

$i = new Foo(42, new Security());
?>
