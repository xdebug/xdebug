<?php
class Assert
{
	static function lazy()
	{
		return new self;
	}

	function __call($name, $args)
	{
		echo $name, ': ', join( ', ', $args ), "\n";
		return $this;
	}
}

class Sample
{
    public function __construct(string $latitude, string $longitude)
    {
        Assert::lazy()
            ->that($latitude, 'latitude')->range(-90, 90)
            ->that($longitude, 'longitude')->range(-140, 140)
            ->verifyNow();
    }
}
?>
