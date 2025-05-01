<?php
$example = new class {
	private string $name;

	public function __construct() { }

	public DateTimeImmutable $datetime {
		get {
			return new DateTimeImmutable();
		}
		set (mixed $void) {}
	}
};

echo $example->datetime->format("c");
echo "DONE\n";
