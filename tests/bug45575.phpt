--TEST--
Bug #45575 (Segfault with invalid non-string as event handler callback)
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

$db = fbird_connect($test_base);

function foobar($var) { var_dump($var); return true; }

fbird_set_event_handler($db, null, 'TEST1');
fbird_set_event_handler($db, 1, 'TEST1');
fbird_set_event_handler('foobar', 'TEST1');

?>
--EXPECTF--
Warning: fbird_set_event_handler(): Callback argument  is not a callable function in %s on line %d

Warning: fbird_set_event_handler(): Callback argument 1 is not a callable function in %s on line %d
