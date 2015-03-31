--TEST--
Test for bug #1133: PDO exception code value type is changed
--SKIPIF--
<?php if (!extension_loaded('pdo_sqlite')) echo "skip The PDO/SQLite extension needs to be installed\n"; ?>
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
