<?php
function ret1(){
	return 1;
}
function ret2(){
	return 2;
}
function dummy1(){
	echo 'dummy1';
}
function dummy2(){
	echo 'dummy2';
}
if(false)
	dummy1();
else
	dummy2();
if(ret1()&&ret2())
	dummy1();
else
	dummy2();
?>
