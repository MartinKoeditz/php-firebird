--TEST--
fbird_close(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

$x = fbird_connect($test_base);
var_dump(fbird_close($x));
var_dump(fbird_close($x));
var_dump(fbird_close());
var_dump(fbird_close('foo'));

?>
--EXPECTF--
bool(true)
bool(true)
bool(true)

Warning: fbird_close() expects parameter 1 to be resource, string given in %s on line %d
NULL
