<?php
class Foo
{
    public string $time_string {
        get {
            return $this->date->format('H:i:s');
        }
    }

    public function __construct(
        public DateTime $date = new DateTime('1979-01-01 00:00:00') {
            set (string|DateTime $value) {
                if (is_string($value)) {
                    $this->date = new DateTime($value);
                } else {
                    $this->date = $value;
                }
            }
        }
    )
    {
    }
}

$date = new DateTime('1980-01-01 00:00:00' );

$foo = new Foo();
var_dump($foo->date);
$foo->date = '1970-01-01 12:34:56';
var_dump($foo->date);
echo $foo->time_string;
echo '';
?>
