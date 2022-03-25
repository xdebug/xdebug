<?php
class Issue2075
{
	static private $groupNamespace = "GROUP SPACE";
	static public $currentUse = false;
	static public $recordPart = false;

    private static function recordTokenClass()
    {
        self::$currentUse = array(       // this line is not marked as covered... the other 4 lines are
            'class' => self::$groupNamespace,
            'alias' => '',
        );
        self::$recordPart = 'class';
    }

	public static function showMe()
	{
		self::recordTokenClass();
	}
}
?>
