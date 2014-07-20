#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "zend_interfaces.h"
#include "php_advanced_serializer.h"

zend_class_entry *serialize_normalizer_ce;

ZEND_DECLARE_MODULE_GLOBALS(advanced_serializer);

ZEND_BEGIN_ARG_INFO_EX(arginfo_normalize, 0, 0, 1)
    ZEND_ARG_INFO(0, object)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_set_serialize_normalizer, 0, 0, 2)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_OBJ_INFO(0, normalizer, SerializeNormalizerInterface, false)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_advanced_serialize, 0, 0, 1)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()


static zend_function_entry advanced_serializer_functions[] = {
    PHP_FE(advanced_serialize, arginfo_advanced_serialize)
    PHP_FE(set_serialize_normalizer, arginfo_set_serialize_normalizer)
    {NULL, NULL, NULL}
};

const zend_function_entry serialize_normalizer_functions[] = {
    PHP_ABSTRACT_ME(SerializeNormalizerInterface, normalize, arginfo_normalize)
    PHP_FE_END
};

zend_module_entry advanced_serializer_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_ADVANCED_SERIALIZER_EXTNAME,
    advanced_serializer_functions,
    PHP_MINIT(advanced_serializer),
    NULL,
    PHP_RINIT(advanced_serializer),
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

PHP_INI_BEGIN()
	STD_PHP_INI_BOOLEAN("advanced_serializer.overload_serialize", "1", PHP_INI_ALL, OnUpdateBool, overload_serialize, zend_advanced_serializer_globals, advanced_serializer_globals)
PHP_INI_END()

static void php_advanced_serializer_init_globals(zend_advanced_serializer_globals *advanced_serializer_globals)
{
}

PHP_RINIT_FUNCTION(advanced_serializer)
{
    zend_function *orig;
    
	zend_hash_find(EG(function_table), "serialize", 10, (void **)&orig);
	ASERIALIZER_G(orig_serialize_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_advanced_serialize;
    return SUCCESS;
}

PHP_MINIT_FUNCTION(advanced_serializer)
{
    ZEND_INIT_MODULE_GLOBALS(advanced_serializer, php_advanced_serializer_init_globals, NULL);

    REGISTER_INI_ENTRIES();
	
    zend_class_entry tmp_ce;
    INIT_CLASS_ENTRY(tmp_ce, "SerializeNormalizerInterface", serialize_normalizer_functions);
    serialize_normalizer_ce = zend_register_internal_interface(&tmp_ce TSRMLS_CC);

    return SUCCESS;
}

PHP_FUNCTION(advanced_serialize)
{
    if (!ASERIALIZER_G(overload_serialize)) {
		ASERIALIZER_G(orig_serialize_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}
    RETURN_STRING("Serializer!", 1);
}

PHP_FUNCTION(set_serialize_normalizer)
{
    zval *obj;
    char *class_name;
    int *class_name_len;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &class_name, &class_name_len, &obj, serialize_normalizer_ce) == FAILURE) {
    
    	RETURN_FALSE;
    }
    
    RETURN_TRUE;
}