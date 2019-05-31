--TEST--
Firebird: connect, close and pconnect
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

	require("firebird.inc");

	fbird_connect($test_base);
	out_table("test1");
	fbird_close();

	$con = fbird_connect($test_base);
	$pcon1 = fbird_pconnect($test_base);
	$pcon2 = fbird_pconnect($test_base);
	fbird_close($con);
	unset($con);
	fbird_close($pcon1);
	unset($pcon1);

	out_table("test1");

	fbird_close($pcon2);
	unset($pcon2);
?>
--EXPECT--
--- test1 ---
1	test table not created with isql	
---
--- test1 ---
1	test table not created with isql	
---
