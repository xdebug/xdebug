<?php
$closure_1 = Closure::fromCallable('substr');


function user_defined($a, $b)
{
	return substr($a, $b);
}
$closure_2 = Closure::fromCallable('user_defined');


$closure_3 = Closure::fromCallable(['DateTimeImmutable', 'createFromFormat']);


$dateTime = new DateTimeImmutable("2021-07-22");
$closure_4 = Closure::fromCallable([$dateTime, 'format']);
echo "Done!";
?>
