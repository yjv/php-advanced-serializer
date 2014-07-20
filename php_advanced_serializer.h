#ifndef PHP_ADVANCED_SERIALIZER_H
#define PHP_ADVANCED_SERIALIZER_H 1

#define PHP_ADVANCED_SERIALIZER_VERSION "1.0"
#define PHP_ADVANCED_SERIALIZER_EXTNAME "advanced_serializer"

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(advanced_serializer)
	bool overload_serialize;
	void (*orig_serialize_func)(INTERNAL_FUNCTION_PARAMETERS);
ZEND_END_MODULE_GLOBALS(advanced_serializer)

#ifdef ZTS
#define ASERIALIZER_G(v) TSRMG(advanced_serializer_globals_id, zend_advanced_serializer_globals *, v)
#else
#define ASERIALIZER_G(v) (advanced_serializer_globals.v)
#endif

PHP_MINIT_FUNCTION(advanced_serializer);
PHP_RINIT_FUNCTION(advanced_serializer);

PHP_FUNCTION(advanced_serialize);
PHP_FUNCTION(set_serialize_normalizer);

extern zend_module_entry advanced_serializer_module_entry;
#define phpext_advanced_serializer_ptr &advanced_serializer_module_entry

#endif