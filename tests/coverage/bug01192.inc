<?php
function gen(&$output, $branch = false)
{
	yield;

	if($branch) {
		$output = 'branched';
		return;
	} // This line is never covered.
	$output = 'did not branch';

}

function testGen()
{
	$output = '';
	$gen = gen($output, true);

	while($gen->valid()) {
		$gen->next();
	}
}
