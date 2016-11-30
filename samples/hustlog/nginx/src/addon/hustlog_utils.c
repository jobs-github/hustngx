#include "hustlog_utils.h"

void hustlog_write_fatal(zlog_category_t * category, const char * data)
{
	zlog_fatal(category, "%s", data);
}

void hustlog_write_error(zlog_category_t * category, const char * data)
{
	zlog_error(category, "%s", data);
}

void hustlog_write_warn(zlog_category_t * category, const char * data)
{
	zlog_warn(category, "%s", data);
}

void hustlog_write_notice(zlog_category_t * category, const char * data)
{
	zlog_notice(category, "%s", data);
}

void hustlog_write_info(zlog_category_t * category, const char * data)
{
	zlog_info(category, "%s", data);
}

void hustlog_write_debug(zlog_category_t * category, const char * data)
{
	zlog_debug(category, "%s", data);
}
