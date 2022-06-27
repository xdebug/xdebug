<?php
class Fluent
{
	function __construct(private string $start)
	{
	}

	function __toString() : string
	{
		return $this->start;
	}

	private function format(string $name, string $value) : string
	{
		return " ({$name}={$value})";
	}

	function __call(string $name, array $values) : Fluent
	{
		foreach ( $values as $value )
		{
			$this->start .= $this->format( $name, $value );
		}

		return $this;
	}
}

$f = new Fluent("Return value inspection");
$g = $f->php('8.1', 'great')->xdebug('awesome', 'amazing');

echo $g, "\n";
?>
