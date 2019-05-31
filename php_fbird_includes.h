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
   |          Ard Biesheuvel <a.k.biesheuvel@ewi.tudelft.nl>              |
   +----------------------------------------------------------------------+
 */

#ifndef PHP_FBIRD_INCLUDES_H
#define PHP_FBIRD_INCLUDES_H

#include <ibase.h>

#ifndef SQLDA_CURRENT_VERSION
#define SQLDA_CURRENT_VERSION SQLDA_VERSION1
#endif

#ifndef METADATALENGTH
#define METADATALENGTH 68
#endif

#define RESET_ERRMSG do { IBG(errmsg)[0] = '\0'; IBG(sql_code) = 0; } while (0)

#define IB_STATUS (IBG(status))

#ifdef FBIRD_DEBUG
#define IBDEBUG(a) php_printf("::: %s (%d)\n", a, __LINE__);
#endif

#ifndef IBDEBUG
#define IBDEBUG(a)
#endif

extern int le_link, le_plink, le_trans;

#define LE_LINK "Firebird/Firebird link"
#define LE_PLINK "Firebird/Firebird persistent link"
#define LE_TRANS "Firebird/Firebird transaction"

#define FBIRD_MSGSIZE 512
#define MAX_ERRMSG (FBIRD_MSGSIZE*2)

#define IB_DEF_DATE_FMT "%Y-%m-%d"
#define IB_DEF_TIME_FMT "%H:%M:%S"

/* this value should never be > USHRT_MAX */
#define FBIRD_BLOB_SEG 4096

ZEND_BEGIN_MODULE_GLOBALS(fbird)
	ISC_STATUS status[20];
	zend_resource *default_link;
	zend_long num_links, num_persistent;
	char errmsg[MAX_ERRMSG];
	zend_long sql_code;
ZEND_END_MODULE_GLOBALS(fbird)

ZEND_EXTERN_MODULE_GLOBALS(fbird)

typedef struct {
	isc_db_handle handle;
	struct tr_list *tr_list;
	unsigned short dialect;
	struct event *event_head;
} fbird_db_link;

typedef struct {
	isc_tr_handle handle;
	unsigned short link_cnt;
	unsigned long affected_rows;
	fbird_db_link *db_link[1]; /* last member */
} fbird_trans;

typedef struct tr_list {
	fbird_trans *trans;
	struct tr_list *next;
} fbird_tr_list;

typedef struct {
	isc_blob_handle bl_handle;
	unsigned short type;
	ISC_QUAD bl_qd;
} fbird_blob;

typedef struct event {
	fbird_db_link *link;
	zend_resource* link_res;
	ISC_LONG event_id;
	unsigned short event_count;
	char **events;
	char *event_buffer, *result_buffer;
	zval callback;
	void *thread_ctx;
	struct event *event_next;
	enum event_state { NEW, ACTIVE, DEAD } state;
} fbird_event;

enum php_firebird_option {
	PHP_FBIRD_DEFAULT 			= 0,
	PHP_FBIRD_CREATE            = 0,
	/* fetch flags */
	PHP_FBIRD_FETCH_BLOBS		= 1,
	PHP_FBIRD_FETCH_ARRAYS      = 2,
	PHP_FBIRD_UNIXTIME 			= 4,
	/* transaction access mode */
	PHP_FBIRD_WRITE 			= 1,
	PHP_FBIRD_READ 				= 2,
	/* transaction isolation level */
	PHP_FBIRD_CONCURRENCY 		= 4,
	PHP_FBIRD_COMMITTED 		= 8,
	  PHP_FBIRD_REC_NO_VERSION 	= 32,
	  PHP_FBIRD_REC_VERSION 	= 64,
	PHP_FBIRD_CONSISTENCY 		= 16,
	/* transaction lock resolution */
	PHP_FBIRD_WAIT 				= 128,
	PHP_FBIRD_NOWAIT 			= 256
};

#define IBG(v) ZEND_MODULE_GLOBALS_ACCESSOR(fbird, v)

#if defined(ZTS) && defined(COMPILE_DL_FIREBIRD)
ZEND_TSRMLS_CACHE_EXTERN()
#endif

#define BLOB_ID_LEN		18
#define BLOB_ID_MASK	"0x%" LL_MASK "x"

#define BLOB_INPUT		1
#define BLOB_OUTPUT		2

#ifdef PHP_WIN32
#define LL_MASK "I64"
#define LL_LIT(lit) lit ## I64
typedef void (__stdcall *info_func_t)(char*);
#else
#define LL_MASK "ll"
#define LL_LIT(lit) lit ## ll
typedef void (*info_func_t)(char*);
#endif

void _php_fbird_error(void);
void _php_fbird_module_error(char *, ...)
	PHP_ATTRIBUTE_FORMAT(printf,1,2);

/* determine if a resource is a link or transaction handle */
#define PHP_FBIRD_LINK_TRANS(zv, lh, th)													\
		do {                                                                                \
			if (!zv) {                                                                      \
				lh = (fbird_db_link *)zend_fetch_resource2(                                 \
						IBG(default_link), "Firebird link", le_link, le_plink);            \
			} else {                                                                        \
				_php_fbird_get_link_trans(INTERNAL_FUNCTION_PARAM_PASSTHRU, zv, &lh, &th);  \
			}                                                                               \
			if (SUCCESS != _php_fbird_def_trans(lh, &th)) { RETURN_FALSE; }                 \
		} while (0)

int _php_fbird_def_trans(fbird_db_link *ib_link, fbird_trans **trans);
void _php_fbird_get_link_trans(INTERNAL_FUNCTION_PARAMETERS, zval *link_id,
	fbird_db_link **ib_link, fbird_trans **trans);

/* provided by fbird_query.c */
void php_fbird_query_minit(INIT_FUNC_ARGS);

/* provided by fbird_blobs.c */
void php_fbird_blobs_minit(INIT_FUNC_ARGS);
int _php_fbird_string_to_quad(char const *id, ISC_QUAD *qd);
zend_string *_php_fbird_quad_to_string(ISC_QUAD const qd);
int _php_fbird_blob_get(zval *return_value, fbird_blob *ib_blob, zend_ulong max_len);
int _php_fbird_blob_add(zval *string_arg, fbird_blob *ib_blob);

/* provided by fbird_events.c */
void php_fbird_events_minit(INIT_FUNC_ARGS);
void _php_fbird_free_event(fbird_event *event);

/* provided by fbird_service.c */
void php_fbird_service_minit(INIT_FUNC_ARGS);

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif

#endif /* PHP_FBIRD_INCLUDES_H */
