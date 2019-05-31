--TEST--
Firebird: transactions
--SKIPIF--
<?php include("skipif.inc"); ?>
--FILE--
<?php

    require("firebird.inc");

    fbird_connect($test_base);

    @fbird_query("create table test5 (i integer)");
    @fbird_query("delete from test5");
    fbird_close();


    echo "default transaction:\n";

/*
Difference between default and other transactions:
default committed when you call  fbird_close().
Other transaction doing rollback.

If you not open default transaction with
fbird_trans, default transaction open
when you call fbird_query(), fbird_prepare(),
fbird_blob_create(), fbird_blob_import()  first time.
*/

/*
simple default transaction test without fbird_trans()
*/

    fbird_connect($test_base);

    echo "empty table\n";

	/*  out_table call fbird_query()
      and fbird_query() start default transaction */
    out_table("test5");

    /* in default transaction context */
    fbird_query("insert into test5 (i) values (1)");

    echo "one row\n";
    out_table("test5");

    fbird_rollback(); /* default rolled */

    echo "after rollback table empty again\n";
    out_table("test5");  /* started new default transaction */

    fbird_query("insert into test5 (i) values (2)");

    fbird_close(); /* commit here! */

    fbird_connect($test_base);

    echo "one row\n";
    out_table("test5");
    fbird_close();

/*
default transaction on default link
First open transaction on link will be default.
$tr_def_l1 may be omitted. All queryes without link and trans
parameters run in this context
*/

    $link_def = fbird_connect($test_base);

    $tr_def_l1 = fbird_trans(FBIRD_READ); /* here transaction start */

    /* all default */
	$res = fbird_query("select * from test5");

    echo "one row\n";
    out_result($res,"test5");

    fbird_free_result($res);

    /* specify transaction context...  */
	$res = fbird_query($tr_def_l1, "select * from test5");

    echo "one row... again.\n";
    out_result($res,"test5");

    fbird_free_result($res);

    /* specify default transaction on link  */
	$res = fbird_query($link_def, "select * from test5");

    echo "one row.\n";
    out_result($res,"test5");

    fbird_free_result($res);

    fbird_rollback($link_def); /* just for example */

    fbird_close();

/*
three transaction on default link
*/
    fbird_connect($test_base);

	$res = fbird_query("select * from test5");

    echo "one row\n";
    out_result($res,"test5");

    fbird_free_result($res);

	$tr_1 = fbird_query("SET TRANSACTION");
	$tr_2 = fbird_query("SET TRANSACTION READ ONLY");
	$tr_3 = fbird_trans(FBIRD_READ+FBIRD_COMMITTED+FBIRD_REC_VERSION+FBIRD_WAIT);
	$tr_4 = fbird_trans(FBIRD_READ+FBIRD_COMMITTED+FBIRD_REC_NO_VERSION+FBIRD_NOWAIT);

    /* insert in first transaction context...  */
    /* as default */
    fbird_query("insert into test5 (i) values (3)");
    /* specify context */
    fbird_query($tr_1, "insert into test5 (i) values (4)");

	$res = fbird_query("select * from test5");

    echo "two rows\n";
    out_result($res,"test5");

    fbird_free_result($res);

	$res = fbird_query($tr_1, "select * from test5");

    echo "two rows again\n";
    out_result($res,"test5");

    fbird_free_result($res);

	fbird_commit();
    fbird_commit($tr_1);

	$tr_1 = fbird_trans();
  	 fbird_query($tr_1, "insert into test5 (i) values (5)");

	/* tr_2 is FBIRD_READ + FBIRD_CONCURRENCY + FBIRD_WAIT */
	$res = fbird_query($tr_2, "select * from test5");

    echo "one row in second transaction\n";
    out_result($res,"test5");

    fbird_free_result($res);

	/* tr_3 is FBIRD_COMMITTED + FBIRD_REC_VERSION + FBIRD_WAIT */
	$res = fbird_query($tr_3, "select * from test5");

    echo "three rows in third transaction\n";
    out_result($res,"test5");

    fbird_free_result($res);

 	/* tr_4 FBIRD_COMMITTED + FBIRD_REC_NO_VERSION + FBIRD_NOWAIT */
 	$res = fbird_query($tr_4, "select * from test5");

 	 echo "three rows in fourth transaction with deadlock\n";
    out_result_trap_error($res,"test5");

    fbird_free_result($res);

	 fbird_rollback($tr_1);
    fbird_close();
/*
transactions on second link
*/
    $link_1 = fbird_pconnect($test_base);
    $link_2 = fbird_pconnect($test_base);

	$tr_1 = fbird_trans(FBIRD_DEFAULT, $link_2);  /* this default transaction also */
	$tr_2 = fbird_trans(FBIRD_COMMITTED, $link_2);

	$res = fbird_query($tr_1, "select * from test5");

    echo "three rows\n";
    out_result($res,"test5");

    fbird_free_result($res);

    fbird_query($tr_1, "insert into test5 (i) values (5)");

	$res = fbird_query($tr_1, "select * from test5");

    echo "four rows\n";
    out_result($res,"test5");

    fbird_free_result($res);

    fbird_commit($tr_1);

	$res = fbird_query($tr_2, "select * from test5");

    echo "four rows again\n";
    out_result($res,"test5");

    fbird_free_result($res);

    fbird_close($link_1);
    fbird_close($link_2);

    echo "end of test\n";
?>
--EXPECTF--
default transaction:
empty table
--- test5 ---
---
one row
--- test5 ---
1	
---
after rollback table empty again
--- test5 ---
---
one row
--- test5 ---
2	
---
one row
--- test5 ---
2	
---
one row... again.
--- test5 ---
2	
---
one row.
--- test5 ---
2	
---
one row
--- test5 ---
2	
---
two rows
--- test5 ---
2	
3	
---
two rows again
--- test5 ---
2	
4	
---
one row in second transaction
--- test5 ---
2	
---
three rows in third transaction
--- test5 ---
2	
3	
4	
---
three rows in fourth transaction with deadlock
--- test5 ---
2	
3	
4	
errmsg [lock conflict on no wait transaction deadlock %a]
---
three rows
--- test5 ---
2	
3	
4	
---
four rows
--- test5 ---
2	
3	
4	
5	
---
four rows again
--- test5 ---
2	
3	
4	
5	
---
end of test
