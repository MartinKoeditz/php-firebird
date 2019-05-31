--TEST--
Bug #45373 (php crash on query with errors in params)
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

	require("firebird.inc");

	$db = fbird_connect($test_base);


	$sql = "select * from test1 where i = ? and c = ?";

	$q = fbird_prepare($db, $sql);
	$r = fbird_execute($q, 1, 'test table not created with isql');
	var_dump(fbird_fetch_assoc($r));
	fbird_free_result($r);

	$r = fbird_execute($q, 1, 'test table not created with isql', 1);
	var_dump(fbird_fetch_assoc($r));
	fbird_free_result($r);

	$r = fbird_execute($q, 1);
	var_dump(fbird_fetch_assoc($r));

?>
--EXPECTF--
array(2) {
  ["I"]=>
  int(1)
  ["C"]=>
  string(32) "test table not created with isql"
}

Notice: fbird_execute(): Statement expects 2 arguments, 3 given in %s on line %d
array(2) {
  ["I"]=>
  int(1)
  ["C"]=>
  string(32) "test table not created with isql"
}

Warning: fbird_execute(): Statement expects 2 arguments, 1 given in %s on line %d

Warning: fbird_fetch_assoc() expects parameter 1 to be resource, bool given in %s on line %d
NULL
