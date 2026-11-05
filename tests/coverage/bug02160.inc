<?php
class SampleGenerator
{
    public function __construct(private readonly array $values)
	{
    }

    public function generate(): \Generator
	{
        foreach ( $this->values as $value )
		{
            if ( $value % 2 )
			{
                yield 'ODD';
            }
			else
			{
                yield 'EVEN';
            }
        }
    }
}

$values = [ 1, 2, 3, 4 ];
$sut = new SampleGenerator( $values );

$actual = iterator_to_array( $sut->generate() );
var_dump( $actual );
