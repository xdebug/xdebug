<?php
$closure_1 = substr(...);


function user_defined($a, $b)
{
	return substr($a, $b);
}
$closure_2 = user_defined(...);


$closure_3 = DateTimeImmutable::createFromFormat(...);


$dateTime = new DateTimeImmutable("2021-07-22");
$closure_4 = $dateTime->format(...);
echo "Done!";
?>
