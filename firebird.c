/*
   +----------------------------------------------------------------------+
   | PHP Version 7                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) The PHP Group                                          |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Jouni Ahto <jouni.ahto@exdec.fi>                            |
   |          Andrew Avdeev <andy@rsc.mv.ru>                              |
   |          Ard Biesheuvel <a.k.biesheuvel@ewi.tudelft.nl>              |
   +----------------------------------------------------------------------+
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define _GNU_SOURCE

#include "php.h"

#if HAVE_FBIRD

#include "php_ini.h"
#include "ext/standard/php_standard.h"
#include "ext/standard/md5.h"
#include "php_firebird.h"
#include "php_fbird_includes.h"
#include "SAPI.h"

#include <time.h>

#define ROLLBACK		0
#define COMMIT			1
#define RETAIN			2

#define CHECK_LINK(link) { if (link==NULL) { php_error_docref(NULL, E_WARNING, "A link to the server could not be established"); RETURN_FALSE; } }

ZEND_DECLARE_MODULE_GLOBALS(fbird)
static PHP_GINIT_FUNCTION(fbird);

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO(arginfo_fbird_errmsg, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_fbird_errcode, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_connect, 0, 0, 0)
	ZEND_ARG_INFO(0, database)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, charset)
	ZEND_ARG_INFO(0, buffers)
	ZEND_ARG_INFO(0, dialect)
	ZEND_ARG_INFO(0, role)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_pconnect, 0, 0, 0)
	ZEND_ARG_INFO(0, database)
	ZEND_ARG_INFO(0, username)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, charset)
	ZEND_ARG_INFO(0, buffers)
	ZEND_ARG_INFO(0, dialect)
	ZEND_ARG_INFO(0, role)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_close, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_drop_db, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_trans, 0, 0, 0)
	ZEND_ARG_INFO(0, trans_args)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, trans_args)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_commit, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_rollback, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_commit_ret, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_rollback_ret, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_gen_id, 0, 0, 1)
	ZEND_ARG_INFO(0, generator)
	ZEND_ARG_INFO(0, increment)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_create, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_open, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, blob_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_add, 0, 0, 2)
	ZEND_ARG_INFO(0, blob_handle)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_get, 0, 0, 2)
	ZEND_ARG_INFO(0, blob_handle)
	ZEND_ARG_INFO(0, len)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_close, 0, 0, 1)
	ZEND_ARG_INFO(0, blob_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_cancel, 0, 0, 1)
	ZEND_ARG_INFO(0, blob_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_info, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, blob_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_echo, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, blob_id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_blob_import, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, file)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_query, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, bind_arg)
	ZEND_ARG_INFO(0, bind_arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_affected_rows, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
ZEND_END_ARG_INFO()

#if abies_0
ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_num_rows, 0, 0, 1)
	ZEND_ARG_INFO(0, result_identifier)
ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_fetch_row, 0, 0, 1)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_INFO(0, fetch_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_fetch_assoc, 0, 0, 1)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_INFO(0, fetch_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_fetch_object, 0, 0, 1)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_INFO(0, fetch_flags)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_name_result, 0, 0, 2)
	ZEND_ARG_INFO(0, result)
	ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_free_result, 0, 0, 1)
	ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_prepare, 0, 0, 0)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, query)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_execute, 0, 0, 1)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, bind_arg)
	ZEND_ARG_INFO(0, bind_arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_free_query, 0, 0, 1)
	ZEND_ARG_INFO(0, query)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_num_fields, 0, 0, 1)
	ZEND_ARG_INFO(0, query_result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_field_info, 0, 0, 2)
	ZEND_ARG_INFO(0, query_result)
	ZEND_ARG_INFO(0, field_number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_num_params, 0, 0, 1)
	ZEND_ARG_INFO(0, query)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_param_info, 0, 0, 2)
	ZEND_ARG_INFO(0, query)
	ZEND_ARG_INFO(0, field_number)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_add_user, 0, 0, 3)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_INFO(0, user_name)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, first_name)
	ZEND_ARG_INFO(0, middle_name)
	ZEND_ARG_INFO(0, last_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_modify_user, 0, 0, 3)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_INFO(0, user_name)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, first_name)
	ZEND_ARG_INFO(0, middle_name)
	ZEND_ARG_INFO(0, last_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_delete_user, 0, 0, 3)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_INFO(0, user_name)
	ZEND_ARG_INFO(0, password)
	ZEND_ARG_INFO(0, first_name)
	ZEND_ARG_INFO(0, middle_name)
	ZEND_ARG_INFO(0, last_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_service_attach, 0, 0, 3)
	ZEND_ARG_INFO(0, host)
	ZEND_ARG_INFO(0, dba_username)
	ZEND_ARG_INFO(0, dba_password)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_service_detach, 0, 0, 1)
	ZEND_ARG_INFO(0, service_handle)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_backup, 0, 0, 3)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_INFO(0, source_db)
	ZEND_ARG_INFO(0, dest_file)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, verbose)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_restore, 0, 0, 3)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_INFO(0, source_file)
	ZEND_ARG_INFO(0, dest_db)
	ZEND_ARG_INFO(0, options)
	ZEND_ARG_INFO(0, verbose)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_maintain_db, 0, 0, 3)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_INFO(0, db)
	ZEND_ARG_INFO(0, action)
	ZEND_ARG_INFO(0, argument)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_db_info, 0, 0, 3)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_INFO(0, db)
	ZEND_ARG_INFO(0, action)
	ZEND_ARG_INFO(0, argument)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_server_info, 0, 0, 2)
	ZEND_ARG_INFO(0, service_handle)
	ZEND_ARG_INFO(0, action)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_wait_event, 0, 0, 1)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, event)
	ZEND_ARG_INFO(0, event2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_set_event_handler, 0, 0, 2)
	ZEND_ARG_INFO(0, link_identifier)
	ZEND_ARG_INFO(0, handler)
	ZEND_ARG_INFO(0, event)
	ZEND_ARG_INFO(0, event2)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_fbird_free_event_handler, 0, 0, 1)
	ZEND_ARG_INFO(0, event)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ extension definition structures */
static const zend_function_entry fbird_functions[] = {
	PHP_FE(fbird_connect, 		arginfo_fbird_connect)
	PHP_FE(fbird_pconnect, 		arginfo_fbird_pconnect)
	PHP_FE(fbird_close, 		arginfo_fbird_close)
	PHP_FE(fbird_drop_db, 		arginfo_fbird_drop_db)
	PHP_FE(fbird_query, 		arginfo_fbird_query)
	PHP_FE(fbird_fetch_row, 	arginfo_fbird_fetch_row)
	PHP_FE(fbird_fetch_assoc, 	arginfo_fbird_fetch_assoc)
	PHP_FE(fbird_fetch_object, 	arginfo_fbird_fetch_object)
	PHP_FE(fbird_free_result, 	arginfo_fbird_free_result)
	PHP_FE(fbird_name_result, 	arginfo_fbird_name_result)
	PHP_FE(fbird_prepare, 		arginfo_fbird_prepare)
	PHP_FE(fbird_execute, 		arginfo_fbird_execute)
	PHP_FE(fbird_free_query, 	arginfo_fbird_free_query)
	PHP_FE(fbird_gen_id, 		arginfo_fbird_gen_id)
	PHP_FE(fbird_num_fields, 	arginfo_fbird_num_fields)
	PHP_FE(fbird_num_params, 	arginfo_fbird_num_params)
#if abies_0
	PHP_FE(fbird_num_rows, 		arginfo_fbird_num_rows)
#endif
	PHP_FE(fbird_affected_rows, arginfo_fbird_affected_rows)
	PHP_FE(fbird_field_info, 	arginfo_fbird_field_info)
	PHP_FE(fbird_param_info, 	arginfo_fbird_param_info)

	PHP_FE(fbird_trans, 		arginfo_fbird_trans)
	PHP_FE(fbird_commit, 		arginfo_fbird_commit)
	PHP_FE(fbird_rollback, 		arginfo_fbird_rollback)
	PHP_FE(fbird_commit_ret, 	arginfo_fbird_commit_ret)
	PHP_FE(fbird_rollback_ret, 	arginfo_fbird_rollback_ret)

	PHP_FE(fbird_blob_info, 	arginfo_fbird_blob_info)
	PHP_FE(fbird_blob_create, 	arginfo_fbird_blob_create)
	PHP_FE(fbird_blob_add, 		arginfo_fbird_blob_add)
	PHP_FE(fbird_blob_cancel, 	arginfo_fbird_blob_cancel)
	PHP_FE(fbird_blob_close, 	arginfo_fbird_blob_close)
	PHP_FE(fbird_blob_open, 	arginfo_fbird_blob_open)
	PHP_FE(fbird_blob_get, 		arginfo_fbird_blob_get)
	PHP_FE(fbird_blob_echo, 	arginfo_fbird_blob_echo)
	PHP_FE(fbird_blob_import, 	arginfo_fbird_blob_import)
	PHP_FE(fbird_errmsg, 		arginfo_fbird_errmsg)
	PHP_FE(fbird_errcode, 		arginfo_fbird_errcode)

	PHP_FE(fbird_add_user, 		arginfo_fbird_add_user)
	PHP_FE(fbird_modify_user, 	arginfo_fbird_modify_user)
	PHP_FE(fbird_delete_user, 	arginfo_fbird_delete_user)

	PHP_FE(fbird_service_attach, arginfo_fbird_service_attach)
	PHP_FE(fbird_service_detach, arginfo_fbird_service_detach)
	PHP_FE(fbird_backup, 		arginfo_fbird_backup)
	PHP_FE(fbird_restore, 		arginfo_fbird_restore)
	PHP_FE(fbird_maintain_db, 	arginfo_fbird_maintain_db)
	PHP_FE(fbird_db_info, 		arginfo_fbird_db_info)
	PHP_FE(fbird_server_info, 	arginfo_fbird_server_info)

	PHP_FE(fbird_wait_event, 			arginfo_fbird_wait_event)
	PHP_FE(fbird_set_event_handler, 	arginfo_fbird_set_event_handler)
	PHP_FE(fbird_free_event_handler, 	arginfo_fbird_free_event_handler)

	PHP_FE_END
};

zend_module_entry fbird_module_entry = {
	STANDARD_MODULE_HEADER,
	"firebird",
	fbird_functions,
	PHP_MINIT(fbird),
	PHP_MSHUTDOWN(fbird),
	NULL,
	PHP_RSHUTDOWN(fbird),
	PHP_MINFO(fbird),
	PHP_FIREBIRD_VERSION,
	PHP_MODULE_GLOBALS(fbird),
	PHP_GINIT(fbird),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_FIREBIRD
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(fbird)
#endif

/* True globals, no need for thread safety */
int le_link, le_plink, le_trans;

/* }}} */

/* error handling ---------------------------- */

/* {{{ proto string fbird_errmsg(void)
   Return error message */
PHP_FUNCTION(fbird_errmsg)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (IBG(sql_code) != 0) {
		RETURN_STRING(IBG(errmsg));
	}

	RETURN_FALSE;
}
/* }}} */

/* {{{ proto int fbird_errcode(void)
   Return error code */
PHP_FUNCTION(fbird_errcode)
{
	if (zend_parse_parameters_none() == FAILURE) {
		return;
	}

	if (IBG(sql_code) != 0) {
		RETURN_LONG(IBG(sql_code));
	}
	RETURN_FALSE;
}
/* }}} */

/* print firebird error and save it for fbird_errmsg() */
void _php_fbird_error(void) /* {{{ */
{
	char *s = IBG(errmsg);
	const ISC_STATUS *statusp = IB_STATUS;

	IBG(sql_code) = isc_sqlcode(IB_STATUS);

	while ((s - IBG(errmsg)) < MAX_ERRMSG && fb_interpret(s, MAX_ERRMSG - strlen(IBG(errmsg)) - 1, &statusp)) {
		strcat(IBG(errmsg), " ");
		s = IBG(errmsg) + strlen(IBG(errmsg));
	}

	php_error_docref(NULL, E_WARNING, "%s", IBG(errmsg));
}
/* }}} */

/* print php firebird module error and save it for fbird_errmsg() */
void _php_fbird_module_error(char *msg, ...) /* {{{ */
{
	va_list ap;

	va_start(ap, msg);

	/* vsnprintf NUL terminates the buf and writes at most n-1 chars+NUL */
	vsnprintf(IBG(errmsg), MAX_ERRMSG, msg, ap);
	va_end(ap);

	IBG(sql_code) = -999; /* no SQL error */

	php_error_docref(NULL, E_WARNING, "%s", IBG(errmsg));
}
/* }}} */

/* {{{ internal macros, functions and structures */
typedef struct {
	isc_db_handle *db_ptr;
	zend_long tpb_len;
	char *tpb_ptr;
} ISC_TEB;

/* }}} */

/* Fill ib_link and trans with the correct database link and transaction. */
void _php_fbird_get_link_trans(INTERNAL_FUNCTION_PARAMETERS, /* {{{ */
	zval *link_id, fbird_db_link **ib_link, fbird_trans **trans)
{
	IBDEBUG("Transaction or database link?");
	if (Z_RES_P(link_id)->type == le_trans) {
		/* Transaction resource: make sure it refers to one link only, then
		   fetch it; database link is stored in ib_trans->db_link[]. */
		IBDEBUG("Type is le_trans");
		*trans = (fbird_trans *)zend_fetch_resource_ex(link_id, LE_TRANS, le_trans);
		if ((*trans)->link_cnt > 1) {
			_php_fbird_module_error("Link id is ambiguous: transaction spans multiple connections."
				);
			return;
		}
		*ib_link = (*trans)->db_link[0];
		return;
	}
	IBDEBUG("Type is le_[p]link or id not found");
	/* Database link resource, use default transaction. */
	*trans = NULL;
	*ib_link = (fbird_db_link *)zend_fetch_resource2_ex(link_id, LE_LINK, le_link, le_plink);
}
/* }}} */

/* destructors ---------------------- */

static void _php_fbird_commit_link(fbird_db_link *link) /* {{{ */
{
	unsigned short i = 0, j;
	fbird_tr_list *l;
	fbird_event *e;
	IBDEBUG("Checking transactions to close...");

	for (l = link->tr_list; l != NULL; ++i) {
		fbird_tr_list *p = l;
		if (p->trans != 0) {
			if (i == 0) {
				if (p->trans->handle != 0) {
					IBDEBUG("Committing default transaction...");
					if (isc_commit_transaction(IB_STATUS, &p->trans->handle)) {
						_php_fbird_error();
					}
				}
				efree(p->trans); /* default transaction is not a registered resource: clean up */
			} else {
				if (p->trans->handle != 0) {
					/* non-default trans might have been rolled back by other call of this dtor */
					IBDEBUG("Rolling back other transactions...");
					if (isc_rollback_transaction(IB_STATUS, &p->trans->handle)) {
						_php_fbird_error();
					}
				}
				/* set this link pointer to NULL in the transaction */
				for (j = 0; j < p->trans->link_cnt; ++j) {
					if (p->trans->db_link[j] == link) {
						p->trans->db_link[j] = NULL;
						break;
					}
				}
			}
		}
		l = l->next;
		efree(p);
	}
	link->tr_list = NULL;

	for (e = link->event_head; e; e = e->event_next) {
		_php_fbird_free_event(e);
		e->link = NULL;
	}
}

/* }}} */

static void php_fbird_commit_link_rsrc(zend_resource *rsrc) /* {{{ */
{
	fbird_db_link *link = (fbird_db_link *) rsrc->ptr;

	_php_fbird_commit_link(link);
}
/* }}} */

static void _php_fbird_close_link(zend_resource *rsrc) /* {{{ */
{
	fbird_db_link *link = (fbird_db_link *) rsrc->ptr;

	_php_fbird_commit_link(link);
	if (link->handle != 0) {
		IBDEBUG("Closing normal link...");
		isc_detach_database(IB_STATUS, &link->handle);
	}
	IBG(num_links)--;
	efree(link);
}
/* }}} */

static void _php_fbird_close_plink(zend_resource *rsrc) /* {{{ */
{
	fbird_db_link *link = (fbird_db_link *) rsrc->ptr;

	_php_fbird_commit_link(link);
	IBDEBUG("Closing permanent link...");
	if (link->handle != 0) {
		isc_detach_database(IB_STATUS, &link->handle);
	}
	IBG(num_persistent)--;
	IBG(num_links)--;
	free(link);
}
/* }}} */

static void _php_fbird_free_trans(zend_resource *rsrc) /* {{{ */
{
	fbird_trans *trans = (fbird_trans *)rsrc->ptr;
	unsigned short i;

	IBDEBUG("Cleaning up transaction resource...");
	if (trans->handle != 0) {
		IBDEBUG("Rolling back unhandled transaction...");
		if (isc_rollback_transaction(IB_STATUS, &trans->handle)) {
			_php_fbird_error();
		}
	}

	/* now remove this transaction from all the connection-transaction lists */
	for (i = 0; i < trans->link_cnt; ++i) {
		if (trans->db_link[i] != NULL) {
			fbird_tr_list **l;
			for (l = &trans->db_link[i]->tr_list; *l != NULL; l = &(*l)->next) {
				if ( (*l)->trans == trans) {
					fbird_tr_list *p = *l;
					*l = p->next;
					efree(p);
					break;
				}
			}
		}
	}
	efree(trans);
}
/* }}} */

/* TODO this function should be part of either Zend or PHP API */
static PHP_INI_DISP(php_fbird_password_displayer_cb)
{

	if ((type == PHP_INI_DISPLAY_ORIG && ini_entry->orig_value)
			|| (type == PHP_INI_DISPLAY_ACTIVE && ini_entry->value)) {
		PUTS("********");
	} else if (!sapi_module.phpinfo_as_text) {
		PUTS("<i>no value</i>");
	} else {
		PUTS("no value");
	}
}

/* {{{ startup, shutdown and info functions */
PHP_INI_BEGIN()
	PHP_INI_ENTRY_EX("fbird.allow_persistent", "1", PHP_INI_SYSTEM, NULL, zend_ini_boolean_displayer_cb)
	PHP_INI_ENTRY_EX("fbird.max_persistent", "-1", PHP_INI_SYSTEM, NULL, display_link_numbers)
	PHP_INI_ENTRY_EX("fbird.max_links", "-1", PHP_INI_SYSTEM, NULL, display_link_numbers)
	PHP_INI_ENTRY("fbird.default_db", NULL, PHP_INI_SYSTEM, NULL)
	PHP_INI_ENTRY("fbird.default_user", NULL, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY_EX("fbird.default_password", NULL, PHP_INI_ALL, NULL, php_fbird_password_displayer_cb)
	PHP_INI_ENTRY("fbird.default_charset", NULL, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("fbird.timestampformat", IB_DEF_DATE_FMT " " IB_DEF_TIME_FMT, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("fbird.dateformat", IB_DEF_DATE_FMT, PHP_INI_ALL, NULL)
	PHP_INI_ENTRY("fbird.timeformat", IB_DEF_TIME_FMT, PHP_INI_ALL, NULL)
PHP_INI_END()

static PHP_GINIT_FUNCTION(fbird)
{
#if defined(COMPILE_DL_FIREBIRD) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	fbird_globals->num_persistent = fbird_globals->num_links = 0;
	fbird_globals->sql_code = *fbird_globals->errmsg = 0;
	fbird_globals->default_link = NULL;
}

PHP_MINIT_FUNCTION(fbird)
{
	REGISTER_INI_ENTRIES();

	le_link = zend_register_list_destructors_ex(_php_fbird_close_link, NULL, LE_LINK, module_number);
	le_plink = zend_register_list_destructors_ex(php_fbird_commit_link_rsrc, _php_fbird_close_plink, LE_PLINK, module_number);
	le_trans = zend_register_list_destructors_ex(_php_fbird_free_trans, NULL, LE_TRANS, module_number);

	REGISTER_LONG_CONSTANT("FBIRD_DEFAULT", PHP_FBIRD_DEFAULT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_CREATE", PHP_FBIRD_CREATE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_TEXT", PHP_FBIRD_FETCH_BLOBS, CONST_PERSISTENT); /* deprecated, for BC only */
	REGISTER_LONG_CONSTANT("FBIRD_FETCH_BLOBS", PHP_FBIRD_FETCH_BLOBS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_FETCH_ARRAYS", PHP_FBIRD_FETCH_ARRAYS, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_UNIXTIME", PHP_FBIRD_UNIXTIME, CONST_PERSISTENT);

	/* transactions */
	REGISTER_LONG_CONSTANT("FBIRD_WRITE", PHP_FBIRD_WRITE, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_READ", PHP_FBIRD_READ, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_COMMITTED", PHP_FBIRD_COMMITTED, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_CONSISTENCY", PHP_FBIRD_CONSISTENCY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_CONCURRENCY", PHP_FBIRD_CONCURRENCY, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_REC_VERSION", PHP_FBIRD_REC_VERSION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_REC_NO_VERSION", PHP_FBIRD_REC_NO_VERSION, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_NOWAIT", PHP_FBIRD_NOWAIT, CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("FBIRD_WAIT", PHP_FBIRD_WAIT, CONST_PERSISTENT);

	php_fbird_query_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_fbird_blobs_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_fbird_events_minit(INIT_FUNC_ARGS_PASSTHRU);
	php_fbird_service_minit(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(fbird)
{
#ifndef PHP_WIN32
	/**
	 * When the Firebird client API library libgds.so is first loaded, it registers a call to
	 * gds__cleanup() with atexit(), in order to clean up after itself when the process exits.
	 * This means that the library is called at process shutdown, and cannot be unloaded beforehand.
	 * PHP tries to unload modules after every request [dl()'ed modules], and right before the
	 * process shuts down [modules loaded from php.ini]. This results in a segfault for this module.
	 * By NULLing the dlopen() handle in the module entry, Zend omits the call to dlclose(),
	 * ensuring that the module will remain present until the process exits. However, the functions
	 * and classes exported by the module will not be available until the module is 'reloaded'.
	 * When reloaded, dlopen() will return the handle of the already loaded module. The module will
	 * be unloaded automatically when the process exits.
	 */
	zend_module_entry *fbird_entry;
	if ((fbird_entry = zend_hash_str_find_ptr(&module_registry, fbird_module_entry.name,
			strlen(fbird_module_entry.name))) != NULL) {
		fbird_entry->handle = 0;
	}
#endif
	UNREGISTER_INI_ENTRIES();
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(fbird)
{
	IBG(num_links) = IBG(num_persistent);
	IBG(default_link)= NULL;

	RESET_ERRMSG;

	return SUCCESS;
}

PHP_MINFO_FUNCTION(fbird)
{
	char tmp[64], *s;

	php_info_print_table_start();
	php_info_print_table_row(2, "Firebird/Firebird Support",
#ifdef COMPILE_DL_FIREBIRD
		"dynamic");
#else
		"static");
#endif

#ifdef FB_API_VER
	snprintf( (s = tmp), sizeof(tmp), "Firebird API version %d", FB_API_VER);
#elif (SQLDA_CURRENT_VERSION > 1)
	s =  "Firebird 7.0 and up";
#endif
	php_info_print_table_row(2, "Compile-time Client Library Version", s);

#if defined(__GNUC__) || defined(PHP_WIN32)
	do {
		info_func_t info_func = NULL;
#ifdef __GNUC__
		info_func = (info_func_t)dlsym(RTLD_DEFAULT, "isc_get_client_version");
#else
		HMODULE l = GetModuleHandle("fbclient");

		if (!l && !(l = GetModuleHandle("gds32"))) {
			break;
		}
		info_func = (info_func_t)GetProcAddress(l, "isc_get_client_version");
#endif
		if (info_func) {
			info_func(s = tmp);
		}
		php_info_print_table_row(2, "Run-time Client Library Version", s);
	} while (0);
#endif
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();

}
/* }}} */

enum connect_args { DB = 0, USER = 1, PASS = 2, CSET = 3, ROLE = 4, BUF = 0, DLECT = 1, SYNC = 2 };

static char const dpb_args[] = {
	0, isc_dpb_user_name, isc_dpb_password, isc_dpb_lc_ctype, isc_dpb_sql_role_name, 0
};

int _php_fbird_attach_db(char **args, size_t *len, zend_long *largs, isc_db_handle *db) /* {{{ */
{
	short i, dpb_len, buf_len = 257-2;  /* version byte at the front, and a null at the end */
	char dpb_buffer[257] = { isc_dpb_version1, 0 }, *dpb;

	dpb = dpb_buffer + 1;

	for (i = 0; i < sizeof(dpb_args); ++i) {
		if (dpb_args[i] && args[i] && len[i] && buf_len > 0) {
			dpb_len = slprintf(dpb, buf_len, "%c%c%s", dpb_args[i],(unsigned char)len[i],args[i]);
			dpb += dpb_len;
			buf_len -= dpb_len;
		}
	}
	if (largs[BUF] && buf_len > 0) {
		dpb_len = slprintf(dpb, buf_len, "%c\2%c%c", isc_dpb_num_buffers,
			(char)(largs[BUF] >> 8), (char)(largs[BUF] & 0xff));
		dpb += dpb_len;
		buf_len -= dpb_len;
	}
	if (largs[SYNC] && buf_len > 0) {
		dpb_len = slprintf(dpb, buf_len, "%c\1%c", isc_dpb_force_write, largs[SYNC] == isc_spb_prp_wm_sync);
		dpb += dpb_len;
		buf_len -= dpb_len;
	}
	if (isc_attach_database(IB_STATUS, (short)len[DB], args[DB], db, (short)(dpb-dpb_buffer), dpb_buffer)) {
		_php_fbird_error();
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static void _php_fbird_connect(INTERNAL_FUNCTION_PARAMETERS, int persistent) /* {{{ */
{
	char *c, hash[16], *args[] = { NULL, NULL, NULL, NULL, NULL };
	int i;
	size_t len[] = { 0, 0, 0, 0, 0 };
	zend_long largs[] = { 0, 0, 0 };
	PHP_MD5_CTX hash_context;
	zend_resource new_index_ptr, *le;
	isc_db_handle db_handle = 0;
	fbird_db_link *ib_link;

	RESET_ERRMSG;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "|ssssllsl",
			&args[DB], &len[DB], &args[USER], &len[USER], &args[PASS], &len[PASS],
			&args[CSET], &len[CSET], &largs[BUF], &largs[DLECT], &args[ROLE], &len[ROLE],
			&largs[SYNC])) {
		RETURN_FALSE;
	}

	/* restrict to the server/db in the .ini if in safe mode */
	if (!len[DB] && (c = INI_STR("fbird.default_db"))) {
		args[DB] = c;
		len[DB] = strlen(c);
	}
	if (!len[USER] && (c = INI_STR("fbird.default_user"))) {
		args[USER] = c;
		len[USER] = strlen(c);
	}
	if (!len[PASS] && (c = INI_STR("fbird.default_password"))) {
		args[PASS] = c;
		len[PASS] = strlen(c);
	}
	if (!len[CSET] && (c = INI_STR("fbird.default_charset"))) {
		args[CSET] = c;
		len[CSET] = strlen(c);
	}

	/* don't want usernames and passwords floating around */
	PHP_MD5Init(&hash_context);
	for (i = 0; i < sizeof(args)/sizeof(char*); ++i) {
		PHP_MD5Update(&hash_context,args[i],len[i]);
	}
	for (i = 0; i < sizeof(largs)/sizeof(zend_long); ++i) {
		PHP_MD5Update(&hash_context,(char*)&largs[i],sizeof(zend_long));
	}
	PHP_MD5Final((unsigned char*)hash, &hash_context);

	/* try to reuse a connection */
	if ((le = zend_hash_str_find_ptr(&EG(regular_list), hash, sizeof(hash)-1)) != NULL) {
		zend_resource *xlink;

		if (le->type != le_index_ptr) {
			RETURN_FALSE;
		}

		xlink = (zend_resource*) le->ptr;
		if ((!persistent && xlink->type == le_link) || xlink->type == le_plink) {
			if (IBG(default_link) != xlink) {
				GC_ADDREF(xlink);
				if (IBG(default_link)) {
					zend_list_delete(IBG(default_link));
				}
				IBG(default_link) = xlink;
			}
			GC_ADDREF(xlink);
			RETURN_RES(xlink);
		} else {
			zend_hash_str_del(&EG(regular_list), hash, sizeof(hash)-1);
		}
	}

	/* ... or a persistent one */
	do {
		zend_long l;
		static char info[] = { isc_info_base_level, isc_info_end };
		char result[8];
		ISC_STATUS status[20];

		if ((le = zend_hash_str_find_ptr(&EG(persistent_list), hash, sizeof(hash)-1)) != NULL) {
			if (le->type != le_plink) {
				RETURN_FALSE;
			}
			/* check if connection has timed out */
			ib_link = (fbird_db_link *) le->ptr;
			if (!isc_database_info(status, &ib_link->handle, sizeof(info), info, sizeof(result), result)) {
				RETVAL_RES(zend_register_resource(ib_link, le_plink));
				break;
			}
			zend_hash_str_del(&EG(persistent_list), hash, sizeof(hash)-1);
		}

		/* no link found, so we have to open one */

		if ((l = INI_INT("fbird.max_links")) != -1 && IBG(num_links) >= l) {
			_php_fbird_module_error("Too many open links (%ld)", IBG(num_links));
			RETURN_FALSE;
		}

		/* create the ib_link */
		if (FAILURE == _php_fbird_attach_db(args, len, largs, &db_handle)) {
			RETURN_FALSE;
		}

		/* use non-persistent if allowed number of persistent links is exceeded */
		if (!persistent || ((l = INI_INT("fbird.max_persistent") != -1) && IBG(num_persistent) >= l)) {
			ib_link = (fbird_db_link *) emalloc(sizeof(fbird_db_link));
			RETVAL_RES(zend_register_resource(ib_link, le_link));
		} else {
			ib_link = (fbird_db_link *) malloc(sizeof(fbird_db_link));
			if (!ib_link) {
				RETURN_FALSE;
			}

			/* hash it up */
			if (zend_register_persistent_resource(hash, sizeof(hash)-1, ib_link, le_plink) == NULL) {
				free(ib_link);
				RETURN_FALSE;
			}
			RETVAL_RES(zend_register_resource(ib_link, le_plink));
			++IBG(num_persistent);
		}
		ib_link->handle = db_handle;
		ib_link->dialect = largs[DLECT] ? (unsigned short)largs[DLECT] : SQL_DIALECT_CURRENT;
		ib_link->tr_list = NULL;
		ib_link->event_head = NULL;

		++IBG(num_links);
	} while (0);

	/* add it to the hash */
	new_index_ptr.ptr = (void *) Z_RES_P(return_value);
	new_index_ptr.type = le_index_ptr;
	zend_hash_str_update_mem(&EG(regular_list), hash, sizeof(hash)-1,
			(void *) &new_index_ptr, sizeof(zend_resource));
	if (IBG(default_link)) {
		zend_list_delete(IBG(default_link));
	}
	IBG(default_link) = Z_RES_P(return_value);
	Z_TRY_ADDREF_P(return_value);
	Z_TRY_ADDREF_P(return_value);
}
/* }}} */

/* {{{ proto resource fbird_connect([string database [, string username [, string password [, string charset [, int buffers [, int dialect [, string role]]]]]]])
   Open a connection to an Firebird database */
PHP_FUNCTION(fbird_connect)
{
	_php_fbird_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto resource fbird_pconnect([string database [, string username [, string password [, string charset [, int buffers [, int dialect [, string role]]]]]]])
   Open a persistent connection to an Firebird database */
PHP_FUNCTION(fbird_pconnect)
{
	_php_fbird_connect(INTERNAL_FUNCTION_PARAM_PASSTHRU, INI_INT("fbird.allow_persistent"));
}
/* }}} */

/* {{{ proto bool fbird_close([resource link_identifier])
   Close an Firebird connection */
PHP_FUNCTION(fbird_close)
{
	zval *link_arg = NULL;
	zend_resource *link_res;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r", &link_arg) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		link_res = IBG(default_link);
		CHECK_LINK(link_res);
		IBG(default_link) = NULL;
	} else {
		link_res = Z_RES_P(link_arg);
	}

	/* we have at least 3 additional references to this resource ??? */
	if (GC_REFCOUNT(link_res) < 4) {
		zend_list_close(link_res);
	} else {
		zend_list_delete(link_res);
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool fbird_drop_db([resource link_identifier])
   Drop an Firebird database */
PHP_FUNCTION(fbird_drop_db)
{
	zval *link_arg = NULL;
	fbird_db_link *ib_link;
	fbird_tr_list *l;
	zend_resource *link_res;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r", &link_arg) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		link_res = IBG(default_link);
		CHECK_LINK(link_res);
		IBG(default_link) = NULL;
	} else {
		link_res = Z_RES_P(link_arg);
	}

	ib_link = (fbird_db_link *)zend_fetch_resource2(link_res, LE_LINK, le_link, le_plink);

	if (!ib_link) {
		RETURN_FALSE;
	}

	if (isc_drop_database(IB_STATUS, &ib_link->handle)) {
		_php_fbird_error();
		RETURN_FALSE;
	}

	/* isc_drop_database() doesn't invalidate the transaction handles */
	for (l = ib_link->tr_list; l != NULL; l = l->next) {
		if (l->trans != NULL) l->trans->handle = 0;
	}

	zend_list_delete(link_res);

	RETURN_TRUE;
}
/* }}} */

/* {{{ proto resource fbird_trans([int trans_args [, resource link_identifier [, ... ], int trans_args [, resource link_identifier [, ... ]] [, ...]]])
   Start a transaction over one or several databases */

#define TPB_MAX_SIZE (8*sizeof(char))

PHP_FUNCTION(fbird_trans)
{
	unsigned short i, link_cnt = 0, tpb_len = 0;
	int argn = ZEND_NUM_ARGS();
	char last_tpb[TPB_MAX_SIZE];
	fbird_db_link **ib_link = NULL;
	fbird_trans *ib_trans;
	isc_tr_handle tr_handle = 0;
	ISC_STATUS result;

	RESET_ERRMSG;

	/* (1+argn) is an upper bound for the number of links this trans connects to */
	ib_link = (fbird_db_link **) safe_emalloc(sizeof(fbird_db_link *),1+argn,0);

	if (argn > 0) {
		zend_long trans_argl = 0;
		char *tpb;
		ISC_TEB *teb;
		zval *args = NULL;

		if (zend_parse_parameters(argn, "+", &args, &argn) == FAILURE) {
			efree(ib_link);
			RETURN_FALSE;
		}

		teb = (ISC_TEB *) safe_emalloc(sizeof(ISC_TEB),argn,0);
		tpb = (char *) safe_emalloc(TPB_MAX_SIZE,argn,0);

		/* enumerate all the arguments: assume every non-resource argument
		   specifies modifiers for the link ids that follow it */
		for (i = 0; i < argn; ++i) {

			if (Z_TYPE(args[i]) == IS_RESOURCE) {

				if ((ib_link[link_cnt] = (fbird_db_link *)zend_fetch_resource2_ex(&args[i], LE_LINK, le_link, le_plink)) == NULL) {
					efree(teb);
					efree(tpb);
					efree(ib_link);
					RETURN_FALSE;
				}

				/* copy the most recent modifier string into tbp[] */
				memcpy(&tpb[TPB_MAX_SIZE * link_cnt], last_tpb, TPB_MAX_SIZE);

				/* add a database handle to the TEB with the most recently specified set of modifiers */
				teb[link_cnt].db_ptr = &ib_link[link_cnt]->handle;
				teb[link_cnt].tpb_len = tpb_len;
				teb[link_cnt].tpb_ptr = &tpb[TPB_MAX_SIZE * link_cnt];

				++link_cnt;

			} else {

				tpb_len = 0;

				convert_to_long_ex(&args[i]);
				trans_argl = Z_LVAL(args[i]);

				if (trans_argl != PHP_FBIRD_DEFAULT) {
					last_tpb[tpb_len++] = isc_tpb_version3;

					/* access mode */
					if (PHP_FBIRD_READ == (trans_argl & PHP_FBIRD_READ)) {
						last_tpb[tpb_len++] = isc_tpb_read;
					} else if (PHP_FBIRD_WRITE == (trans_argl & PHP_FBIRD_WRITE)) {
						last_tpb[tpb_len++] = isc_tpb_write;
					}

					/* isolation level */
					if (PHP_FBIRD_COMMITTED == (trans_argl & PHP_FBIRD_COMMITTED)) {
						last_tpb[tpb_len++] = isc_tpb_read_committed;
						if (PHP_FBIRD_REC_VERSION == (trans_argl & PHP_FBIRD_REC_VERSION)) {
							last_tpb[tpb_len++] = isc_tpb_rec_version;
						} else if (PHP_FBIRD_REC_NO_VERSION == (trans_argl & PHP_FBIRD_REC_NO_VERSION)) {
							last_tpb[tpb_len++] = isc_tpb_no_rec_version;
						}
					} else if (PHP_FBIRD_CONSISTENCY == (trans_argl & PHP_FBIRD_CONSISTENCY)) {
						last_tpb[tpb_len++] = isc_tpb_consistency;
					} else if (PHP_FBIRD_CONCURRENCY == (trans_argl & PHP_FBIRD_CONCURRENCY)) {
						last_tpb[tpb_len++] = isc_tpb_concurrency;
					}

					/* lock resolution */
					if (PHP_FBIRD_NOWAIT == (trans_argl & PHP_FBIRD_NOWAIT)) {
						last_tpb[tpb_len++] = isc_tpb_nowait;
					} else if (PHP_FBIRD_WAIT == (trans_argl & PHP_FBIRD_WAIT)) {
						last_tpb[tpb_len++] = isc_tpb_wait;
					}
				}
			}
		}

		if (link_cnt > 0) {
			result = isc_start_multiple(IB_STATUS, &tr_handle, link_cnt, teb);
		}

		efree(tpb);
		efree(teb);
	}

	if (link_cnt == 0) {
		link_cnt = 1;
		if ((ib_link[0] = (fbird_db_link *)zend_fetch_resource2(IBG(default_link), LE_LINK, le_link, le_plink)) == NULL) {
			efree(ib_link);
			RETURN_FALSE;
		}
		result = isc_start_transaction(IB_STATUS, &tr_handle, 1, &ib_link[0]->handle, tpb_len, last_tpb);
	}

	/* start the transaction */
	if (result) {
		_php_fbird_error();
		efree(ib_link);
		RETURN_FALSE;
	}

	/* register the transaction in our own data structures */
	ib_trans = (fbird_trans *) safe_emalloc(link_cnt-1, sizeof(fbird_db_link *), sizeof(fbird_trans));
	ib_trans->handle = tr_handle;
	ib_trans->link_cnt = link_cnt;
	ib_trans->affected_rows = 0;
	for (i = 0; i < link_cnt; ++i) {
		fbird_tr_list **l;
		ib_trans->db_link[i] = ib_link[i];

		/* the first item in the connection-transaction list is reserved for the default transaction */
		if (ib_link[i]->tr_list == NULL) {
			ib_link[i]->tr_list = (fbird_tr_list *) emalloc(sizeof(fbird_tr_list));
			ib_link[i]->tr_list->trans = NULL;
			ib_link[i]->tr_list->next = NULL;
		}

		/* link the transaction into the connection-transaction list */
		for (l = &ib_link[i]->tr_list; *l != NULL; l = &(*l)->next);
		*l = (fbird_tr_list *) emalloc(sizeof(fbird_tr_list));
		(*l)->trans = ib_trans;
		(*l)->next = NULL;
	}
	efree(ib_link);
	RETVAL_RES(zend_register_resource(ib_trans, le_trans));
	Z_TRY_ADDREF_P(return_value);
}
/* }}} */

int _php_fbird_def_trans(fbird_db_link *ib_link, fbird_trans **trans) /* {{{ */
{
	if (ib_link == NULL) {
		php_error_docref(NULL, E_WARNING, "Invalid database link");
		return FAILURE;
	}

	/* the first item in the connection-transaction list is reserved for the default transaction */
	if (ib_link->tr_list == NULL) {
		ib_link->tr_list = (fbird_tr_list *) emalloc(sizeof(fbird_tr_list));
		ib_link->tr_list->trans = NULL;
		ib_link->tr_list->next = NULL;
	}

	if (*trans == NULL) {
		fbird_trans *tr = ib_link->tr_list->trans;

		if (tr == NULL) {
			tr = (fbird_trans *) emalloc(sizeof(fbird_trans));
			tr->handle = 0;
			tr->link_cnt = 1;
			tr->affected_rows = 0;
			tr->db_link[0] = ib_link;
			ib_link->tr_list->trans = tr;
		}
		if (tr->handle == 0) {
			if (isc_start_transaction(IB_STATUS, &tr->handle, 1, &ib_link->handle, 0, NULL)) {
				_php_fbird_error();
				return FAILURE;
			}
		}
		*trans = tr;
	}
	return SUCCESS;
}
/* }}} */

static void _php_fbird_trans_end(INTERNAL_FUNCTION_PARAMETERS, int commit) /* {{{ */
{
	fbird_trans *trans = NULL;
	int res_id = 0;
	ISC_STATUS result;
	fbird_db_link *ib_link;
	zval *arg = NULL;

	RESET_ERRMSG;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|r", &arg) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		ib_link = (fbird_db_link *)zend_fetch_resource2(IBG(default_link), LE_LINK, le_link, le_plink);
		if (ib_link->tr_list == NULL || ib_link->tr_list->trans == NULL) {
			/* this link doesn't have a default transaction */
			_php_fbird_module_error("Default link has no default transaction");
			RETURN_FALSE;
		}
		trans = ib_link->tr_list->trans;
	} else {
		/* one id was passed, could be db or trans id */
		if (Z_RES_P(arg)->type == le_trans) {
			trans = (fbird_trans *)zend_fetch_resource_ex(arg, LE_TRANS, le_trans);
			res_id = Z_RES_P(arg)->handle;
		} else {
			ib_link = (fbird_db_link *)zend_fetch_resource2_ex(arg, LE_LINK, le_link, le_plink);

			if (ib_link->tr_list == NULL || ib_link->tr_list->trans == NULL) {
				/* this link doesn't have a default transaction */
				_php_fbird_module_error("Link has no default transaction");
				RETURN_FALSE;
			}
			trans = ib_link->tr_list->trans;
		}
	}

	switch (commit) {
		default: /* == case ROLLBACK: */
			result = isc_rollback_transaction(IB_STATUS, &trans->handle);
			break;
		case COMMIT:
			result = isc_commit_transaction(IB_STATUS, &trans->handle);
			break;
		case (ROLLBACK | RETAIN):
			result = isc_rollback_retaining(IB_STATUS, &trans->handle);
			break;
		case (COMMIT | RETAIN):
			result = isc_commit_retaining(IB_STATUS, &trans->handle);
			break;
	}

	if (result) {
		_php_fbird_error();
		RETURN_FALSE;
	}

	/* Don't try to destroy implicitly opened transaction from list... */
	if ((commit & RETAIN) == 0 && res_id != 0) {
		zend_list_delete(Z_RES_P(arg));
	}
	RETURN_TRUE;
}
/* }}} */

/* {{{ proto bool fbird_commit( resource link_identifier )
   Commit transaction */
PHP_FUNCTION(fbird_commit)
{
	_php_fbird_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, COMMIT);
}
/* }}} */

/* {{{ proto bool fbird_rollback( resource link_identifier )
   Rollback transaction */
PHP_FUNCTION(fbird_rollback)
{
	_php_fbird_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, ROLLBACK);
}
/* }}} */

/* {{{ proto bool fbird_commit_ret( resource link_identifier )
   Commit transaction and retain the transaction context */
PHP_FUNCTION(fbird_commit_ret)
{
	_php_fbird_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, COMMIT | RETAIN);
}
/* }}} */

/* {{{ proto bool fbird_rollback_ret( resource link_identifier )
   Rollback transaction and retain the transaction context */
PHP_FUNCTION(fbird_rollback_ret)
{
	_php_fbird_trans_end(INTERNAL_FUNCTION_PARAM_PASSTHRU, ROLLBACK | RETAIN);
}
/* }}} */

/* {{{ proto int fbird_gen_id(string generator [, int increment [, resource link_identifier ]])
   Increments the named generator and returns its new value */
PHP_FUNCTION(fbird_gen_id)
{
	zval *link = NULL;
	char query[128], *generator;
	size_t gen_len;
	zend_long inc = 1;
	fbird_db_link *ib_link;
	fbird_trans *trans = NULL;
	XSQLDA out_sqlda;
	ISC_INT64 result;

	RESET_ERRMSG;

	if (FAILURE == zend_parse_parameters(ZEND_NUM_ARGS(), "s|lr", &generator, &gen_len,
			&inc, &link)) {
		RETURN_FALSE;
	}

	if (gen_len > 31) {
		php_error_docref(NULL, E_WARNING, "Invalid generator name");
		RETURN_FALSE;
	}

	PHP_FBIRD_LINK_TRANS(link, ib_link, trans);

	snprintf(query, sizeof(query), "SELECT GEN_ID(%s,%ld) FROM rdb$database", generator, inc);

	/* allocate a minimal descriptor area */
	out_sqlda.sqln = out_sqlda.sqld = 1;
	out_sqlda.version = SQLDA_CURRENT_VERSION;

	/* allocate the field for the result */
	out_sqlda.sqlvar[0].sqltype = SQL_INT64;
	out_sqlda.sqlvar[0].sqlscale = 0;
	out_sqlda.sqlvar[0].sqllen = sizeof(result);
	out_sqlda.sqlvar[0].sqldata = (void*) &result;

	/* execute the query */
	if (isc_dsql_exec_immed2(IB_STATUS, &ib_link->handle, &trans->handle, 0, query,
			SQL_DIALECT_CURRENT, NULL, &out_sqlda)) {
		_php_fbird_error();
		RETURN_FALSE;
	}

	/* don't return the generator value as a string unless it doesn't fit in a long */
#if SIZEOF_ZEND_LONG < 8
	if (result < ZEND_LONG_MIN || result > ZEND_LONG_MAX) {
		char *res;
		int l;

		l = spprintf(&res, 0, "%" LL_MASK "d", result);
		RETURN_STRINGL(res, l);
	}
#endif
	RETURN_LONG((zend_long)result);
}

/* }}} */

#endif /* HAVE_FBIRD */
