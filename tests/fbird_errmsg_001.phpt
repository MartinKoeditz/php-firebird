--TEST--
fbird_errmsg(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

$x = fbird_connect($test_base);

fbird_query('SELECT Foobar');
var_dump(fbird_errmsg());

fbird_close($x);
var_dump(fbird_errmsg());

?>
--EXPECTF--
Warning: fbird_query(): Dynamic SQL Error SQL error code = -104 %s on line %d
string(%d) "Dynamic SQL Error SQL error code = -104 %s"
bool(false)
