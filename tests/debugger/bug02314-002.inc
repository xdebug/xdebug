<?php
class Foo
{
    public string $time_string {
        get {
            return $this->date->format('H:i:s');
        }
    }

    public function __construct(
        private DateTime $date = new DateTime('1979-01-01 00:00:00') {
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

    function setNewDate( string|DateTime $date )
    {
        $this->date = $date;
    }
}

$date = new DateTime('1980-01-01 00:00:00' );

$foo = new Foo();
$foo->setNewDate( '1970-01-01 12:34:56' );
echo $foo->time_string;
echo '';
?>
