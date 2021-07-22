<?php

function AB() {
}

function AA() {
}

function A() {
	AA();
	Fiber::suspend();
	AB();
}

function BA() {
}

function BB() {
}

function B() {
	BA();
	Fiber::suspend();
	BB();
}

$fiberA = new Fiber(function () {
	A();
});

$fiberB = new Fiber(function () {
	B();
});

$fiberA->start();
$fiberB->start();
$fiberA->resume();
$fiberB->resume();
