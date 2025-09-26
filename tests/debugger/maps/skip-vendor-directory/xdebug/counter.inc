<?php
namespace Xdebug;

class Counter
{
	public function __construct(private int $from) {}

	public function range()
	{
		for ($i = $this->from; $i > 0; $i--) {
			yield $i;
		}
	}
}
