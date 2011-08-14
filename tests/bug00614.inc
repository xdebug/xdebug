<?php
class Base
{
    private $a;
    private $b;
    private $c;
    private $d;
    private $e;
    private $f;
    private $g;
    private $h;
    private $i;
    private $j;
    private $k;
    private $l;
    private $m;
    private $n;
    private $o;
    private $p;
    private $q;
    private $r;
    private $s;
    private $t;
    private $u;
    private $v;
    private $w;
    private $x;
    private $y;
    private $z;
    private $z1; // comment these last few variable out and $z then disappears from member list!
    private $z2;
    private $z3;
    private $z4;
    
    public function __construct()
    {
        $this->i = 99;
        $this->j = 88;
        $this->k = 11;
    }
}

class Extension extends Base
{
    private $k;
    private $m;
    private $j;
    
    public function __construct()
    {
        $this->k = 77;
        $this->m = 66;
        $this->j = 55;

        parent::__construct();
    }
}

$b = new Base();
$e = new Extension();

echo "Hello World\n";
?>
