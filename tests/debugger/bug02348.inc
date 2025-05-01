<?php
$example = new class {
	public function __construct() {}

	public DateTimeImmutable $datetime {
		get {
			return new DateTimeImmutable();
		}
		set (mixed $void) {}
	}
};

echo $example->datetime->format("c");
