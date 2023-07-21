--TEST--
Test for bug #1133: PDO exception code value type is changed
--SKIPIF--
<?php
require __DIR__ . '/../utils.inc';
check_reqs('ext pdo_sqlite');
?>
--FILE--
<?php

try {
    $pdo = new PDO("sqlite::memory:");
    $pdo->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $pdo->query("INSERT INTO fake_table VALUES ('bogus')");
} catch (PDOException $e) {
    var_dump($e->getCode());
    var_dump($e->getMessage());
}
?>
--EXPECT--
string(5) "HY000"
string(59) "SQLSTATE[HY000]: General error: 1 no such table: fake_table"
