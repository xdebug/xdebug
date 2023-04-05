<?php
$append = new AppendIterator();
$append->append( new ArrayIterator( [ 0, 1, 3, 5, 6, 8, 9, 10, 12, 14, 16 ] ) );
$append->append( new ArrayIterator( [ 2, 13, 15, 17 ] ) );


$array = new ArrayIterator( [ 2, 13, 15, 17 ] );


$caching = new CachingIterator( new ArrayIterator( [ 0, 1, 3, 5, 6, 8, 9, 10, 12, 14, 16 ] ) );


$callbackFilter = new CallbackFilterIterator(
	new ArrayIterator( [ 0, 1, 3, 5, 6, 8, 9, 10, 12, 14, 16 ] ),
	function($c, $k, $i) { return true; },
);


$directory = new DirectoryIterator( __DIR__ );


$empty_ = new EmptyIterator();


$fileSystem = new FileSystemIterator( __DIR__ );


class MyFilterIterator extends FilterIterator
{
	function accept() : bool
	{
		return true;
	}
}
$filter = new MyFilterIterator( new ArrayIterator( [ 2, 13, 15, 17 ] ) );


$glob = new GlobIterator( __DIR__ . '/*.inc' );


$infinite = new InfiniteIterator( new ArrayIterator( [ 2, 13, 15, 17 ] ) );


$iterator = new IteratorIterator( new ArrayIterator( [ 2, 13, 15, 17 ] ) );


$limit = new LimitIterator(
	new ArrayIterator( [ 0, 1, 3, 5, 6, 8, 9, 10, 12, 14, 16 ] ),
	3, 6,
);

$multiple = new MultipleIterator( MultipleIterator::MIT_NEED_ANY );
$multiple->attachIterator( new ArrayIterator( [ 0, 1, 3, 5, 6, 8, 9, 10, 12, 14, 16 ] ) );
$multiple->attachIterator( new ArrayIterator( [ 2, 13, 15, 17 ] ) );


$norewind = new NoRewindIterator( new ArrayIterator( [ 0, 1, 3, 5, 6, 8, 9, 10, 12, 14, 16 ] ) );


$parent = new ParentIterator(
    new RecursiveArrayIterator( [
        [ 0, 1, 3, 5, 6, 8, 9, 10, 12, 14, 16 ],
        [ 2, 13, 15, 17 ]
    ] )
);


$recursiveArray = new RecursiveArrayIterator( [
	0 => 'a',
	1 => [
		'subA',
		'subB',
		[
			0 => 'subsubA',
			1 => 'subsubB',
			2 => [
				0 => 'deepA',
				1 => 'deepB'
			]
		]
	],
	2 => 'b',
	3 => [ 'subA', 'subB', 'subC' ],
	4 => 'c'
] );


$recursiveIterator = new RecursiveIteratorIterator(
	new RecursiveArrayIterator( [
		'test' => 'value',
		'level_one' => [
			'level_two' => [
				'level_three' => [
					'replace_this_array' => [
						'special_key' => 'replacement_value',
						'key_one' => 'testing',
						'key_two' => 'value',
						'four' => 'another value'
					]
				],
				'ordinary_key' => 'value'
			]
		]
	] ),
	RecursiveIteratorIterator::SELF_FIRST
);


$regex = new RegexIterator(
	new ArrayIterator( [ 'test1', 'test2', 'test3' ] ),
	'/^(test)(\d+)/',
	RegexIterator::REPLACE
);
$regex->replacement = '$2:$1';

echo "Finished\n";
?>
