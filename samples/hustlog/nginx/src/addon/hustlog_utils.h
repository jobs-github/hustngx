#ifndef __hustlog_utils_20150716175436_h__
#define __hustlog_utils_20150716175436_h__

#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>
#include "zlog.h"

#undef MAX_HUSTLOG_CATEGORY_LEN
#define MAX_HUSTLOG_CATEGORY_LEN 64

#undef MAX_HUSTLOG_WORKER_SIZE
#define MAX_HUSTLOG_WORKER_SIZE  32

typedef unsigned char bool;
typedef void (*hustlog_write_t) (zlog_category_t * category, const char * data);

void hustlog_write_fatal(zlog_category_t * category, const char * data);
void hustlog_write_error(zlog_category_t * category, const char * data);
void hustlog_write_warn(zlog_category_t * category, const char * data);
void hustlog_write_notice(zlog_category_t * category, const char * data);
void hustlog_write_info(zlog_category_t * category, const char * data);
void hustlog_write_debug(zlog_category_t * category, const char * data);

#endif // __hustlog_utils_20150716175436_h__
