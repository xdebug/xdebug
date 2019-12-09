<?php
class bar
{
	public static function fc($a)
	{
		return self::nested(
			self::nested( //this line is marked as uncovered
				$a
			)
		);
	}

	private static function nested($a)
	{
		return $a;
	}

}

echo bar::fc("test"), "\n";
?>
