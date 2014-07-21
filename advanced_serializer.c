#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "zend_interfaces.h"
#include "zend_exceptions.h"
#include "php_advanced_serializer.h"
#include "ext/standard/php_var.h"
#include "ext/standard/php_smart_str.h"

zend_class_entry *serialize_normalizer_ce;
zend_class_entry *serialize_denormalizer_ce;

ZEND_DECLARE_MODULE_GLOBALS(advanced_serializer);

ZEND_BEGIN_ARG_INFO_EX(arginfo_normalize, 0, 0, 1)
    ZEND_ARG_INFO(0, object)
    ZEND_ARG_ARRAY_INFO(0, properties, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_denormalize, 0, 0, 1)
    ZEND_ARG_INFO(0, object)
    ZEND_ARG_INFO(0, properties)
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
	PHP_FE(get_registered_normalizers, NULL)
    {NULL, NULL, NULL}
};

const zend_function_entry serialize_normalizer_functions[] = {
    PHP_ABSTRACT_ME(SerializeNormalizerInterface, normalize, arginfo_normalize)
    PHP_FE_END
};

const zend_function_entry serialize_denormalizer_functions[] = {
    PHP_ABSTRACT_ME(SerializeDenormalizerInterface, denormalize, arginfo_denormalize)
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
    PHP_RSHUTDOWN(advanced_serializer),
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_ADVANCED_SERIALIZER_VERSION,
    PHP_MODULE_GLOBALS(advanced_serializer),
#endif
	NULL,
    NULL,	
    ZEND_MODULE_POST_ZEND_DEACTIVATE_N(advanced_serializer),
    STANDARD_MODULE_PROPERTIES_EX
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
    
	/* Override serialize with our own function */
	zend_hash_find(EG(function_table), "serialize", 10, (void **)&orig);
	ASERIALIZER_G(orig_serialize_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_advanced_serialize;
	
	ALLOC_HASHTABLE(ASERIALIZER_G(registered_normalizers));	
	zend_hash_init(ASERIALIZER_G(registered_normalizers), 0, NULL, ZVAL_PTR_DTOR, 0);    
	return SUCCESS;
}

PHP_MINIT_FUNCTION(advanced_serializer)
{
    ZEND_INIT_MODULE_GLOBALS(advanced_serializer, php_advanced_serializer_init_globals, NULL);

    REGISTER_INI_ENTRIES();
	
    zend_class_entry tmp_ce;
    INIT_CLASS_ENTRY(tmp_ce, "SerializeNormalizerInterface", serialize_normalizer_functions);
    serialize_normalizer_ce = zend_register_internal_interface(&tmp_ce TSRMLS_CC);
	
    INIT_CLASS_ENTRY(tmp_ce, "SerializeDenormalizerInterface", serialize_normalizer_functions);
    serialize_denormalizer_ce = zend_register_internal_interface(&tmp_ce TSRMLS_CC);

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(advanced_serializer)
{
	zend_hash_destroy(ASERIALIZER_G(registered_normalizers));	
	FREE_HASHTABLE(ASERIALIZER_G(registered_normalizers));
    return SUCCESS;
}

ZEND_MODULE_POST_ZEND_DEACTIVATE_D(advanced_serializer)
{
	zend_function *orig;
	TSRMLS_FETCH();
	
	/* Reset serialize to the original function */
	zend_hash_find(EG(function_table), "serialize", 10, (void **)&orig);
	orig->internal_function.handler = ASERIALIZER_G(orig_serialize_func);
	return SUCCESS;
}

PHP_FUNCTION(advanced_serialize)
{
    if (!ASERIALIZER_G(overload_serialize)) {
		ASERIALIZER_G(orig_serialize_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}
    
    zval *data;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &data) == FAILURE) {
    
    	RETURN_FALSE;
    }
    
    if (Z_TYPE_P(data) != IS_OBJECT) {
		
		ASERIALIZER_G(orig_serialize_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
    }
    
    zval *normalizer;
	zend_class_entry * ce = Z_OBJCE_P(data);
    
    if (zend_hash_find(ASERIALIZER_G(registered_normalizers), ce->name, ce->name_length, (void **) &normalizer) != SUCCESS) {
    	
		ASERIALIZER_G(orig_serialize_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}
	
	ce->serialize = advanced_serialize_proxy_to_normalizer;
    
	ASERIALIZER_G(orig_serialize_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	return;
}



PHP_FUNCTION(set_serialize_normalizer)
{
    zval *normalizer;
    char *class_name;
    int class_name_len;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &class_name, &class_name_len, &normalizer, serialize_normalizer_ce) == FAILURE) {
    
    	RETURN_FALSE;
    }
    
    php_printf("class: %d\n", Z_REFCOUNT_P(normalizer));
    zend_hash_update(ASERIALIZER_G(registered_normalizers), class_name, class_name_len, &normalizer, sizeof(zval *), NULL);
    
    RETURN_TRUE;
}

PHP_FUNCTION(get_registered_normalizers)
{
	HashTable * copy;
	ALLOC_HASHTABLE(copy);
	zend_hash_copy(copy, ASERIALIZER_G(registered_normalizers), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));

	Z_TYPE_P(return_value) = IS_ARRAY;
	Z_ARRVAL_P(return_value) = copy;
	
	
}

int advanced_serialize_proxy_to_normalizer(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	zval *normalizer;
    
    if (zend_hash_find(ASERIALIZER_G(registered_normalizers), Z_OBJCE_P(object)->name, Z_OBJCE_P(object)->name_length, (void **) &normalizer) != SUCCESS)
    {
    }
    
    php_printf("class: %d\n", Z_TYPE_P(normalizer));

    zend_class_entry * ce = Z_OBJCE_P(normalizer);
    zval *retval;
    zval *properties;
    int result;
	smart_str buf = {0};
	php_serialize_data_t var_hash;
	MAKE_STD_ZVAL(properties);
	Z_TYPE_P(properties) = IS_ARRAY;
	Z_ARRVAL_P(properties) = HASH_OF(object);
	
    zend_call_method_with_2_params(&normalizer, ce, NULL, "normalize", &retval, object, properties);
    
    if (!retval || EG(exception)) {
        result = FAILURE;
    } else {
        switch(Z_TYPE_P(retval)) {
        case IS_NULL:
            /* we could also make this '*buf_len = 0' but this allows to skip variables */
            zval_ptr_dtor(&retval);
            return FAILURE;
        case IS_ARRAY:
			PHP_VAR_SERIALIZE_INIT(var_hash);
    		php_var_serialize(&buf, &retval, &var_hash TSRMLS_CC);
    		PHP_VAR_SERIALIZE_DESTROY(var_hash);            
    		*buffer = (unsigned char*)estrndup(buf.c, buf.len);
            *buf_len = buf.len;
            result = SUCCESS;
            break;
        default: /* failure */
            result = FAILURE;
            break;
        }
        zval_ptr_dtor(&retval);
    }

    if (result == FAILURE && !EG(exception)) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "%s::normalize() must return an array or NULL", ce->name);
    }
    return result;
}

int advanced_unserialize_proxy_to_denormalizer(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
//     zval * zdata;
// 
//     object_init_ex(*object, ce);
// 
//     MAKE_STD_ZVAL(zdata);
//     ZVAL_STRINGL(zdata, (char*)buf, buf_len, 1);
// 
//     zend_call_method_with_1_params(object, ce, &ce->unserialize_func, "unserialize", NULL, zdata);
// 
//     zval_ptr_dtor(&zdata);

    if (EG(exception)) {
        return FAILURE;
    } else {
        return SUCCESS;
    }
}