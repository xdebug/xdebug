<?php
declare(strict_types=1);
class MyObject
{
	public $pub;
	protected $prot;
	private $priv;

	public function __construct($pub, $prot, $priv)
	{
		$this->pub = $pub;
		$this->prot = $prot;
		$this->priv = $priv;
	}
}
/** @var MyObject[] $arr */
$arr = [
	new MyObject(1, 'first', true),
	new MyObject(2, 'second', false),
	new MyObject(3, '3rd', false),
];
echo $arr[0]->pub;
?>
