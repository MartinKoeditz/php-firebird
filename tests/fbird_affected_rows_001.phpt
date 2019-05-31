--TEST--
fbird_affected_rows(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

$x = fbird_connect($test_base);

fbird_query($x, 'INSERT INTO test1 VALUES (1, 100)');
fbird_query($x, 'INSERT INTO test1 VALUES (10000, 100)');

fbird_query($x, 'UPDATE test1 SET i = 10000');
var_dump(fbird_affected_rows($x));


fbird_query($x, 'UPDATE test1 SET i = 10000 WHERE i = 2.0');
var_dump(fbird_affected_rows($x));

fbird_query($x, 'UPDATE test1 SET i =');
var_dump(fbird_affected_rows($x));


?>
--EXPECTF--
int(3)
int(0)

Warning: fbird_query(): Dynamic SQL Error SQL error code = -104 %s on line %d
int(0)
