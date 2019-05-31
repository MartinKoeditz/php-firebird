--TEST--
fbird_free_query(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

$x = fbird_connect($test_base);

$q =fbird_prepare($x, 'SELECT 1 FROM test1 WHERE i = ?');
$q =fbird_prepare($x, 'SELECT 1 FROM test1 WHERE i = ?');
$q = fbird_prepare($x, 'SELECT 1 FROM test1 WHERE i = ?');

var_dump(fbird_free_query($q));
var_dump(fbird_free_query($q));
var_dump(fbird_free_query($x));

?>
--EXPECTF--
bool(true)

Warning: fbird_free_query(): supplied resource is not a valid Firebird/Firebird query resource in %s on line %d
bool(false)

Warning: fbird_free_query(): supplied resource is not a valid Firebird/Firebird query resource in %s on line %d
bool(false)
