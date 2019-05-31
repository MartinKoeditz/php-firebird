--TEST--
fbird_trans(): Basic operations
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

$x = fbird_connect($test_base);

$trans = fbird_trans(FBIRD_DEFAULT, $x);
$sth = fbird_prepare($trans, 'INSERT INTO test1 VALUES (?, ?)');

$res = fbird_execute($sth, 100, 100);
var_dump($res);

fbird_commit($trans);

$rs = fbird_query($x, 'SELECT * FROM test1 WHERE i = 100');
var_dump(fbird_fetch_assoc($rs));

fbird_free_query($sth);
unset($res);

?>
--EXPECT--
int(1)
array(2) {
  ["I"]=>
  int(100)
  ["C"]=>
  string(3) "100"
}
