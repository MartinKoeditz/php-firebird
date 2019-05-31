--TEST--
fbird_drop_db(): Basic test
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

require("firebird.inc");

unlink($file = tempnam('/tmp',"php_fbird_test"));


$db = fbird_query(FBIRD_CREATE,
		sprintf("CREATE SCHEMA '%s' USER '%s' PASSWORD '%s' DEFAULT CHARACTER SET %s",$file,
		$user, $password, ($charset = ini_get('fbird.default_charset')) ? $charset : 'NONE'));

var_dump($db);
var_dump(fbird_drop_db($db));
var_dump(fbird_drop_db(1));
var_dump(fbird_drop_db(NULL));

?>
--EXPECTF--
resource(%d) of type (Firebird/Firebird link)
bool(true)

Warning: fbird_drop_db() expects parameter 1 to be resource, int given in %s on line %d
NULL

Warning: fbird_drop_db() expects parameter 1 to be resource, null given in %s on line %d
NULL
