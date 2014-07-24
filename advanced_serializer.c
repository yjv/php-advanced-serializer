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
zend_class_entry *unserialize_denormalizer_ce;

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
    PHP_FE(advanced_serializer_set_normalizer, arginfo_set_serialize_normalizer)
	PHP_FE(advanced_serializer_get_normalizers, NULL)
    {NULL, NULL, NULL}
};

const zend_function_entry serialize_normalizer_functions[] = {
    PHP_ABSTRACT_ME(SerializeNormalizerInterface, normalize, arginfo_normalize)
    PHP_FE_END
};

const zend_function_entry unserialize_denormalizer_functions[] = {
    PHP_ABSTRACT_ME(UnserializeDenormalizerInterface, denormalize, arginfo_denormalize)
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
	advanced_serializer_globals->registered_normalizers = (HashTable *) pemalloc(sizeof(HashTable), 1);	
	zend_hash_init(advanced_serializer_globals->registered_normalizers, 0, NULL, ADVANCED_SERIALIZER_NORMALIZATION_DATA_DTOR, 1);    
}

static void php_advanced_serializer_destroy_globals(zend_advanced_serializer_globals *advanced_serializer_globals)
{
	zend_hash_destroy(advanced_serializer_globals->registered_normalizers);	
	pefree((advanced_serializer_globals->registered_normalizers), 1);
}

PHP_MINIT_FUNCTION(advanced_serializer)
{
    ZEND_INIT_MODULE_GLOBALS(advanced_serializer, php_advanced_serializer_init_globals, php_advanced_serializer_destroy_globals);

    REGISTER_INI_ENTRIES();
	
    zend_class_entry tmp_ce;
    INIT_CLASS_ENTRY(tmp_ce, "SerializeNormalizerInterface", serialize_normalizer_functions);
    serialize_normalizer_ce = zend_register_internal_interface(&tmp_ce TSRMLS_CC);
	
    INIT_CLASS_ENTRY(tmp_ce, "UnserializeDenormalizerInterface", unserialize_denormalizer_functions);
    unserialize_denormalizer_ce = zend_register_internal_interface(&tmp_ce TSRMLS_CC);

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

PHP_RINIT_FUNCTION(advanced_serializer)
{
    zend_function *orig;
    
	/* Override serialize with our own function */
	zend_hash_find(EG(function_table), "serialize", 10, (void **)&orig);
	ASERIALIZER_G(orig_serialize_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_advanced_serialize;
	
	return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(advanced_serializer)
{
	zend_hash_clean(ASERIALIZER_G(registered_normalizers));
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
    
    replace_serialize_handlers();
        
	ASERIALIZER_G(orig_serialize_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
	restore_serialize_handlers();
	return;
}

PHP_FUNCTION(advanced_serializer_set_normalizer)
{
    zval *normalizer;
    char *class_name;
    int class_name_len;
    advanced_serializer_normalization_data **normalization_data;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &class_name, &class_name_len, &normalizer, serialize_normalizer_ce) == FAILURE) {
    
    	RETURN_FALSE;
    }
    
    AS_GET_NORMALIZATION_DATA(class_name, class_name_len, normalization_data);
    
	(*normalization_data)->normalizer = normalizer; 
    RETURN_TRUE;
}

PHP_FUNCTION(advanced_serializer_get_normalizers)
{
	HashTable * copy;
	char *key;
	uint key_len;
	advanced_serializer_normalization_data **normalization_data;
	HashPosition pos;
	
	ALLOC_HASHTABLE(copy);
	zend_hash_init(copy, 0, NULL, ZVAL_PTR_DTOR, 0);  
	
	for (zend_hash_internal_pointer_reset_ex(ASERIALIZER_G(registered_normalizers), &pos);
    	zend_hash_get_current_data_ex(ASERIALIZER_G(registered_normalizers), (void **)&normalization_data, &pos) == SUCCESS;
    	zend_hash_move_forward_ex(ASERIALIZER_G(registered_normalizers), &pos)
	) {
    	if (!(*normalization_data)->normalizer) {

            continue;
        }
        
        zend_hash_get_current_key_ex(ASERIALIZER_G(registered_normalizers), &key, &key_len, NULL, 0, &pos);
        zval_add_ref(&((*normalization_data)->normalizer));
        zend_hash_update(ASERIALIZER_G(registered_normalizers), key, key_len, (void **)&((*normalization_data)->normalizer), sizeof(zval *), NULL);
	}

	Z_TYPE_P(return_value) = IS_ARRAY;
	Z_ARRVAL_P(return_value) = copy;
}

PHP_FUNCTION(advanced_serializer_set_denormalizer)
{
    zval *denormalizer;
    char *class_name;
    int class_name_len;
    advanced_serializer_normalization_data **normalization_data;
    
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sO", &class_name, &class_name_len, &denormalizer, unserialize_denormalizer_ce) == FAILURE) {
    
    	RETURN_FALSE;
    }
        
    AS_GET_NORMALIZATION_DATA(class_name, class_name_len, normalization_data);
    
	(*normalization_data)->denormalizer = denormalizer; 
    RETURN_TRUE;

}

PHP_FUNCTION(advanced_serializer_get_denormalizers)
{
	HashTable * copy;
	ALLOC_HASHTABLE(copy);
	zend_hash_init(copy, 0, NULL, ZVAL_PTR_DTOR, 0);    
	zend_hash_copy(copy, ASERIALIZER_G(registered_normalizers), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));

	Z_TYPE_P(return_value) = IS_ARRAY;
	Z_ARRVAL_P(return_value) = copy;
}

int advanced_serialize_proxy_to_normalizer(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	zval *normalizer;
	advanced_serializer_normalization_data **normalization_data;
    
    if (zend_hash_find(ASERIALIZER_G(registered_normalizers), Z_OBJCE_P(object)->name, Z_OBJCE_P(object)->name_length, (void **)&normalization_data) != SUCCESS)
    {
    	return FAILURE;
    }

	normalizer = (*normalization_data)->normalizer;

    zend_class_entry * ce = Z_OBJCE_P(normalizer);
    zval *retval;    
    zval *properties;
	MAKE_STD_ZVAL(properties);
	Z_TYPE_P(properties) = IS_ARRAY;
	Z_ARRVAL_P(properties) = Z_OBJ_HT_P(object)->get_properties((object) TSRMLS_CC);
	
    zend_call_method_with_2_params(&normalizer, ce, NULL, "normalize", &retval, object, properties);
    
	Z_ARRVAL_P(properties) = NULL;
	zval_ptr_dtor(&properties);

    int result;
	smart_str buf = {0};
	php_serialize_data_t var_hash;

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
    		*buffer = (unsigned char*)buf.c;
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

void replace_serialize_handlers()
{
	HashPosition pos;
	char * key;
	uint key_len;
	advanced_serializer_normalization_data **normalization_data;
	zend_class_entry **ce;
		
	TSRMLS_FETCH();	
	
	for (zend_hash_internal_pointer_reset_ex(ASERIALIZER_G(registered_normalizers), &pos);
    	zend_hash_get_current_data_ex(ASERIALIZER_G(registered_normalizers), (void **)&normalization_data, &pos) == SUCCESS;
    	zend_hash_move_forward_ex(ASERIALIZER_G(registered_normalizers), &pos)
	) {
    	if (!(*normalization_data)->object_class_entry) {
    	
    		zend_hash_get_current_key_ex(ASERIALIZER_G(registered_normalizers), &key, &key_len, NULL, 0, &pos);

    		if (zend_lookup_class_ex(key, key_len, NULL, 0, &ce TSRMLS_CC) == FAILURE) {
            	continue;
        	}
        	
        	(*normalization_data)->object_class_entry = *ce;
        }
        
    	if ((*normalization_data)->object_class_entry->serialize) {
    	
    		(*normalization_data)->original_serialize = (*normalization_data)->object_class_entry->serialize;
    	}
	
		(*normalization_data)->object_class_entry->serialize = advanced_serialize_proxy_to_normalizer;
	}
}

void restore_serialize_handlers()
{
	advanced_serializer_normalization_data **normalization_data;
	HashPosition pos;
		
	TSRMLS_FETCH();	
	
	for (zend_hash_internal_pointer_reset_ex(ASERIALIZER_G(registered_normalizers), &pos);
    	zend_hash_get_current_data_ex(ASERIALIZER_G(registered_normalizers), (void **)&normalization_data, &pos) == SUCCESS;
    	zend_hash_move_forward_ex(ASERIALIZER_G(registered_normalizers), &pos)
	) {
    	
    	if (!(*normalization_data)->object_class_entry) {
    	
            continue;
        }

	
		(*normalization_data)->object_class_entry->serialize = (*normalization_data)->original_serialize;
	}
}

void replace_deserialize_handlers()
{
}

void restore_deserialize_handlers()
{
}

void advanced_serializer_normalization_data_dtor(advanced_serializer_normalization_data *data)
{
	if (data->normalizer) {
		zval_ptr_dtor(&(data->normalizer));
	}
	
	if (data->denormalizer) {
		zval_ptr_dtor(&(data->denormalizer));
	}
			
// 	pefree(data, 1);
}