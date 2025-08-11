<?php

/**
 * This file has been copied from the php-src project. It is just a php process that runs some fairly standard
 * algorithms and which serves as an artificial benchmark. We removed a couple of functions which were taking
 * long to run and which made these benchmarks too slow
 */

if (function_exists("date_default_timezone_set")) {
    date_default_timezone_set("UTC");
}

function simple() {
  $a = 0;
  for ($i = 0; $i < 1000000; $i++)
    $a++;

  $thisisanotherlongname = 0;
  for ($thisisalongname = 0; $thisisalongname < 1000000; $thisisalongname++)
    $thisisanotherlongname++;
}

/****/

function simplecall() {
  for ($i = 0; $i < 1000000; $i++)
    strlen("hallo");
}

/****/

function hallo($a) {
}

function simpleucall() {
  for ($i = 0; $i < 1000000; $i++)
    hallo("hallo");
}

/****/

function simpleudcall() {
  for ($i = 0; $i < 1000000; $i++)
    hallo2("hallo");
}

function hallo2($a) {
}

/****/

function Ack($m, $n){
  if($m == 0) return $n+1;
  if($n == 0) return Ack($m-1, 1);
  return Ack($m - 1, Ack($m, ($n - 1)));
}

function ackermann($n) {
  $r = Ack(3,$n);
  print "Ack(3,$n): $r\n";
}

/****/

function ary($n) {
  for ($i=0; $i<$n; $i++) {
    $X[$i] = $i;
  }
  for ($i=$n-1; $i>=0; $i--) {
    $Y[$i] = $X[$i];
  }
  $last = $n-1;
  print "$Y[$last]\n";
}

/****/

function ary2($n) {
  for ($i=0; $i<$n;) {
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;

    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
    $X[$i] = $i; ++$i;
  }
  for ($i=$n-1; $i>=0;) {
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;

    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
    $Y[$i] = $X[$i]; --$i;
  }
  $last = $n-1;
  print "$Y[$last]\n";
}

/****/

function ary3($n) {
  for ($i=0; $i<$n; $i++) {
    $X[$i] = $i + 1;
    $Y[$i] = 0;
  }
  for ($k=0; $k<1000; $k++) {
    for ($i=$n-1; $i>=0; $i--) {
      $Y[$i] += $X[$i];
    }
  }
  $last = $n-1;
  print "$Y[0] $Y[$last]\n";
}

/****/

/****/

function hash1($n) {
  for ($i = 1; $i <= $n; $i++) {
    $X[dechex($i)] = $i;
  }
  $c = 0;
  for ($i = $n; $i > 0; $i--) {
    if ($X[dechex($i)]) { $c++; }
  }
  print "$c\n";
}

/****/

function hash2($n) {
  for ($i = 0; $i < $n; $i++) {
    $hash1["foo_$i"] = $i;
    $hash2["foo_$i"] = 0;
  }
  for ($i = $n; $i > 0; $i--) {
    foreach($hash1 as $key => $value) $hash2[$key] += $value;
  }
  $first = "foo_0";
  $last  = "foo_".($n-1);
  print "$hash1[$first] $hash1[$last] $hash2[$first] $hash2[$last]\n";
}

/****/

function gen_random ($n) {
    global $LAST;
    return( ($n * ($LAST = ($LAST * IA + IC) % IM)) / IM );
}

function heapsort_r($n, &$ra) {
    $l = ($n >> 1) + 1;
    $ir = $n;

    while (1) {
    if ($l > 1) {
        $rra = $ra[--$l];
    } else {
        $rra = $ra[$ir];
        $ra[$ir] = $ra[1];
        if (--$ir == 1) {
        $ra[1] = $rra;
        return;
        }
    }
    $i = $l;
    $j = $l << 1;
    while ($j <= $ir) {
        if (($j < $ir) && ($ra[$j] < $ra[$j+1])) {
        $j++;
        }
        if ($rra < $ra[$j]) {
        $ra[$i] = $ra[$j];
        $j += ($i = $j);
        } else {
        $j = $ir + 1;
        }
    }
    $ra[$i] = $rra;
    }
}

function heapsort($N) {
  global $LAST;

  define("IM", 139968);
  define("IA", 3877);
  define("IC", 29573);

  $LAST = 42;
  for ($i=1; $i<=$N; $i++) {
    $ary[$i] = gen_random(1);
  }
  heapsort_r($N, $ary);
  printf("%.10f\n", $ary[$N]);
}

/****/

function mkmatrix ($rows, $cols) {
    $count = 1;
    $mx = array();
    for ($i=0; $i<$rows; $i++) {
    for ($j=0; $j<$cols; $j++) {
        $mx[$i][$j] = $count++;
    }
    }
    return($mx);
}

function mmult ($rows, $cols, $m1, $m2) {
    $m3 = array();
    for ($i=0; $i<$rows; $i++) {
    for ($j=0; $j<$cols; $j++) {
        $x = 0;
        for ($k=0; $k<$cols; $k++) {
        $x += $m1[$i][$k] * $m2[$k][$j];
        }
        $m3[$i][$j] = $x;
    }
    }
    return($m3);
}

function matrix($n) {
  $SIZE = 30;
  $m1 = mkmatrix($SIZE, $SIZE);
  $m2 = mkmatrix($SIZE, $SIZE);
  while ($n--) {
    $mm = mmult($SIZE, $SIZE, $m1, $m2);
  }
  print "{$mm[0][0]} {$mm[2][3]} {$mm[3][2]} {$mm[4][4]}\n";
}

/****/

function nestedloop($n) {
  $x = 0;
  for ($a=0; $a<$n; $a++)
    for ($b=0; $b<$n; $b++)
      for ($c=0; $c<$n; $c++)
        for ($d=0; $d<$n; $d++)
          for ($e=0; $e<$n; $e++)
            for ($f=0; $f<$n; $f++)
             $x++;
  print "$x\n";
}

/****/

function sieve($n) {
  $count = 0;
  while ($n-- > 0) {
    $count = 0;
    $flags = range (0,8192);
    for ($i=2; $i<8193; $i++) {
      if ($flags[$i] > 0) {
        for ($k=$i+$i; $k <= 8192; $k+=$i) {
          $flags[$k] = 0;
        }
        $count++;
      }
    }
  }
  print "Count: $count\n";
}

/****/

function strcat($n) {
  $str = "";
  while ($n-- > 0) {
    $str .= "hello\n";
  }
  $len = strlen($str);
  print "$len\n";
}

/*****/

function gethrtime()
{
  $hrtime = hrtime();
  return (($hrtime[0]*1000000000 + $hrtime[1]) / 1000000000);
}

function start_test()
{
    ob_start();
  return gethrtime();
}

function end_test($start, $name)
{
  global $total;
  $end = gethrtime();
  ob_end_clean();
  $total += $end-$start;
  $num = number_format($end-$start,3);
  $pad = str_repeat(" ", 24-strlen($name)-strlen($num));

  echo $name.$pad.$num."\n";
    ob_start();
  return gethrtime();
}

function total()
{
  global $total;
  $pad = str_repeat("-", 24);
  echo $pad."\n";
  $num = number_format($total,3);
  $pad = str_repeat(" ", 24-strlen("Total")-strlen($num));
  echo "Total".$pad.$num."\n";
}

$t0 = $t = start_test();
simple();
$t = end_test($t, "simple");
simplecall();
$t = end_test($t, "simplecall");
simpleucall();
$t = end_test($t, "simpleucall");
simpleudcall();
$t = end_test($t, "simpleudcall");
ackermann(7);
$t = end_test($t, "ackermann(7)");
ary(50000);
$t = end_test($t, "ary(50000)");
ary2(50000);
$t = end_test($t, "ary2(50000)");
ary3(2000);
$t = end_test($t, "ary3(2000)");
hash1(50000);
$t = end_test($t, "hash1(50000)");
hash2(500);
$t = end_test($t, "hash2(500)");
heapsort(20000);
$t = end_test($t, "heapsort(20000)");
matrix(20);
$t = end_test($t, "matrix(20)");
nestedloop(12);
$t = end_test($t, "nestedloop(12)");
sieve(30);
$t = end_test($t, "sieve(30)");
strcat(200000);
$t = end_test($t, "strcat(200000)");
total();
?>
