--TEST--
fbird_trans(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

$x = fbird_connect($test_base);
var_dump(fbird_trans($x));
var_dump(fbird_trans(1));
var_dump(fbird_close());
var_dump(fbird_close($x));

?>
--EXPECTF--
resource(%d) of type (Firebird/Firebird transaction)
resource(%d) of type (Firebird/Firebird transaction)
bool(true)
bool(true)
