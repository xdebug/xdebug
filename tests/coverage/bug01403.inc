<?php
class Foo {
	function reduce( $callable )
	{
		$callable();
	}
}
$someObject = new Foo; $outerItem = $anotherOuterItem = 42;
// Because it is "line based" and not statement base coverage
// the variables for "use" are shown as not covered.
$someObject->reduce(function () use (
    $outerItem,
    $anotherOuterItem
) {
   return false;
});
?>
