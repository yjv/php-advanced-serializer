#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_advanced_serializer.h"

static function_entry advanced_serializer_functions[] = {
    PHP_FE(advanced_serialize, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry advanced_serializer_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_ADVANCED_SERIALIZER_EXTNAME,
    advanced_serializer_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_ADVANCED_SERIALIZER_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_ADVANCED_SERIALIZER
ZEND_GET_MODULE(advanced_serializer)
#endif

PHP_FUNCTION(advanced_serialize)
{
    RETURN_STRING("Hello World", 1);
}