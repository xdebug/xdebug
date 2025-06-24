<?php

class TEEST {
	public string $time_string {
		get => date_create()->format('H:i:s');
	}

	public String $AAA = '' {
		get {
			if (str_contains($this->AAA, ':')) {
				$Sub_Script_Depth = explode(':', $this->AAA)[1];
				$Sub_Script_Depth = (int)$Sub_Script_Depth + 1;
				$New_Script_Depth = explode(':', $this->AAA)[0] . ':' . $Sub_Script_Depth;
				$this->AAA = $New_Script_Depth;
				return $this->AAA;
			} else {
				$this->AAA .= ':0';
			}
			return $this->AAA;
		}
		set (String $value) {
			$this->AAA = $value;
		}
	}

	public function __construct()
	{
		$this->main();
	}

	public function main()
	{
		$this->AAA = 'Hello World.';
		$fff = $this->AAA;
		echo $fff;
	}
}

$aaa = new TEEST();
