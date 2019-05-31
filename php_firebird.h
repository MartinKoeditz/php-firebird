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
   |          Andrew Avdeev <andy@simgts.mv.ru>                           |
   |          Ard Biesheuvel <a.k.biesheuvel@its.tudelft.nl>              |
   +----------------------------------------------------------------------+
 */

#ifndef PHP_FIREBIRD_H
#define PHP_FIREBIRD_H

extern zend_module_entry fbird_module_entry;
#define phpext_firebird_ptr &fbird_module_entry

#include "php_version.h"
#define PHP_FIREBIRD_VERSION PHP_VERSION

PHP_MINIT_FUNCTION(fbird);
PHP_RINIT_FUNCTION(fbird);
PHP_MSHUTDOWN_FUNCTION(fbird);
PHP_RSHUTDOWN_FUNCTION(fbird);
PHP_MINFO_FUNCTION(fbird);

PHP_FUNCTION(fbird_connect);
PHP_FUNCTION(fbird_pconnect);
PHP_FUNCTION(fbird_close);
PHP_FUNCTION(fbird_drop_db);
PHP_FUNCTION(fbird_query);
PHP_FUNCTION(fbird_fetch_row);
PHP_FUNCTION(fbird_fetch_assoc);
PHP_FUNCTION(fbird_fetch_object);
PHP_FUNCTION(fbird_free_result);
PHP_FUNCTION(fbird_name_result);
PHP_FUNCTION(fbird_prepare);
PHP_FUNCTION(fbird_execute);
PHP_FUNCTION(fbird_free_query);

PHP_FUNCTION(fbird_timefmt);

PHP_FUNCTION(fbird_gen_id);
PHP_FUNCTION(fbird_num_fields);
PHP_FUNCTION(fbird_num_params);
#if abies_0
PHP_FUNCTION(fbird_num_rows);
#endif
PHP_FUNCTION(fbird_affected_rows);
PHP_FUNCTION(fbird_field_info);
PHP_FUNCTION(fbird_param_info);

PHP_FUNCTION(fbird_trans);
PHP_FUNCTION(fbird_commit);
PHP_FUNCTION(fbird_rollback);
PHP_FUNCTION(fbird_commit_ret);
PHP_FUNCTION(fbird_rollback_ret);

PHP_FUNCTION(fbird_blob_create);
PHP_FUNCTION(fbird_blob_add);
PHP_FUNCTION(fbird_blob_cancel);
PHP_FUNCTION(fbird_blob_open);
PHP_FUNCTION(fbird_blob_get);
PHP_FUNCTION(fbird_blob_close);
PHP_FUNCTION(fbird_blob_echo);
PHP_FUNCTION(fbird_blob_info);
PHP_FUNCTION(fbird_blob_import);

PHP_FUNCTION(fbird_add_user);
PHP_FUNCTION(fbird_modify_user);
PHP_FUNCTION(fbird_delete_user);

PHP_FUNCTION(fbird_service_attach);
PHP_FUNCTION(fbird_service_detach);
PHP_FUNCTION(fbird_backup);
PHP_FUNCTION(fbird_restore);
PHP_FUNCTION(fbird_maintain_db);
PHP_FUNCTION(fbird_db_info);
PHP_FUNCTION(fbird_server_info);

PHP_FUNCTION(fbird_errmsg);
PHP_FUNCTION(fbird_errcode);

PHP_FUNCTION(fbird_wait_event);
PHP_FUNCTION(fbird_set_event_handler);
PHP_FUNCTION(fbird_free_event_handler);

#else

#define phpext_firebird_ptr NULL

#endif /* PHP_FIREBIRD_H */
