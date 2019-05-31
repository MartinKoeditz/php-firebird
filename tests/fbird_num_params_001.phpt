--TEST--
fbird_num_params(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

$x = fbird_connect($test_base);

$rs = fbird_prepare('SELECT * FROM test1 WHERE 1 = ? AND 2 = ?');
var_dump(fbird_num_params($rs));

$rs = fbird_prepare('SELECT * FROM test1 WHERE 1 = ? AND 2 = ?');
var_dump(fbird_num_params());

$rs = fbird_prepare('SELECT * FROM test1 WHERE 1 = ? AND 2 = ? AND 3 = :x');
var_dump(fbird_num_params($rs));


?>
--EXPECTF--
int(2)

Warning: fbird_num_params() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: fbird_prepare(): Dynamic SQL Error SQL error code = -206 %s in %s on line %d

Warning: fbird_num_params() expects parameter 1 to be resource, bool given in %s on line %d
NULL
