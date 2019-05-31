--TEST--
Bug #46543 (fbird_trans() memory leaks when using wrong parameters)
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

@fbird_close();

fbird_trans(1);
fbird_trans();
fbird_trans('foo');
fbird_trans(fopen(__FILE__, 'r'));

$x = fbird_connect($test_base);
fbird_trans(1, 2, $x, $x, 3);

?>
--EXPECTF--
Warning: fbird_trans(): supplied resource is not a valid Firebird/Firebird link resource in %sbug46543.php on line %d

Warning: fbird_trans(): supplied resource is not a valid Firebird/Firebird link resource in %sbug46543.php on line %d

Warning: fbird_trans(): supplied resource is not a valid Firebird/Firebird link resource in %sbug46543.php on line %d

Warning: fbird_trans(): supplied resource is not a valid Firebird/Firebird link resource in %sbug46543.php on line %d
