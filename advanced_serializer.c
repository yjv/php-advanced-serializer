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

ZEND_BEGIN_ARG_INFO_EX(arginfo_normalize, 0, 0, 2)
    ZEND_ARG_INFO(0, object)
    ZEND_ARG_ARRAY_INFO(0, properties, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_denormalize, 0, 0, 2)
    ZEND_ARG_INFO(0, object)
    ZEND_ARG_ARRAY_INFO(0, properties, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_set_serialize_normalizer, 0, 0, 2)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_OBJ_INFO(0, normalizer, SerializeNormalizerInterface, false)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_set_unserialize_denormalizer, 0, 0, 2)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_OBJ_INFO(0, denormalizer, UnserializeDenormalizerInterface, false)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_advanced_serialize, 0, 0, 1)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_advanced_unserialize, 0, 0, 1)
    ZEND_ARG_TYPE_INFO(0, value, IS_STRING, 0)
ZEND_END_ARG_INFO()


static zend_function_entry advanced_serializer_functions[] = {
    PHP_FE(advanced_serialize, arginfo_advanced_serialize)
    PHP_FE(advanced_unserialize, arginfo_advanced_unserialize)
    PHP_FE(advanced_serializer_set_normalizer, arginfo_set_serialize_normalizer)
    PHP_FE(advanced_serializer_set_denormalizer, arginfo_set_unserialize_denormalizer)
	PHP_FE(advanced_serializer_get_normalizers, NULL)
	PHP_FE(advanced_serializer_get_denormalizers, NULL)
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
	pefree(advanced_serializer_globals->registered_normalizers, 1);
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
	
	/* Reset serialize to the original function */
	zend_hash_find(EG(function_table), "unserialize", 12, (void **)&orig);
	orig->internal_function.handler = ASERIALIZER_G(orig_unserialize_func);
	return SUCCESS;
}

PHP_RINIT_FUNCTION(advanced_serializer)
{
    zend_function *orig;
    
	/* Override serialize with our own function */
	zend_hash_find(EG(function_table), "serialize", 10, (void **)&orig);
	ASERIALIZER_G(orig_serialize_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_advanced_serialize;
	
	/* Override unserialize with our own function */
	zend_hash_find(EG(function_table), "unserialize", 12, (void **)&orig);
	ASERIALIZER_G(orig_unserialize_func) = orig->internal_function.handler;
	orig->internal_function.handler = zif_advanced_unserialize;
	
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

PHP_FUNCTION(advanced_unserialize)
{
    if (!ASERIALIZER_G(overload_serialize)) {
		ASERIALIZER_G(orig_serialize_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
		return;
	}
    
    replace_serialize_handlers();
        
	ASERIALIZER_G(orig_unserialize_func)(INTERNAL_FUNCTION_PARAM_PASSTHRU);
	
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
    
    AS_GET_NORMALIZATION_DATA(class_name, (class_name_len + 1), normalization_data);
    
    if ((*normalization_data)->normalizer) {
		
		  zval_ptr_dtor(&((*normalization_data)->normalizer));  
    }
    
    MAKE_STD_ZVAL((*normalization_data)->normalizer);
	ZVAL_ZVAL((*normalization_data)->normalizer, normalizer, 1, 0); 
    RETURN_TRUE;
}

PHP_FUNCTION(advanced_serializer_get_normalizers)
{
	HashTable * copy;
	char *key;
	uint key_len;
	zval *normalizer;
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
        
        MAKE_STD_ZVAL(normalizer);
		ZVAL_ZVAL(normalizer, (*normalization_data)->normalizer, 1, 0); 
        
        zend_hash_get_current_key_ex(ASERIALIZER_G(registered_normalizers), &key, &key_len, NULL, 0, &pos);
        zend_hash_update(copy, key, key_len, (void **)&normalizer, sizeof(zval *), NULL);
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
        
    AS_GET_NORMALIZATION_DATA(class_name, (class_name_len + 1), normalization_data);
    
    if ((*normalization_data)->denormalizer) {
		
		  zval_ptr_dtor(&((*normalization_data)->denormalizer));  
    }

    MAKE_STD_ZVAL((*normalization_data)->denormalizer);
	ZVAL_ZVAL((*normalization_data)->denormalizer, denormalizer, 1, 0); 
    RETURN_TRUE;
}

PHP_FUNCTION(advanced_serializer_get_denormalizers)
{
	HashTable * copy;
	char *key;
	uint key_len;
	zval *denormalizer;
	advanced_serializer_normalization_data **normalization_data;
	HashPosition pos;
	
	ALLOC_HASHTABLE(copy);
	zend_hash_init(copy, 0, NULL, ZVAL_PTR_DTOR, 0); 
	
	for (zend_hash_internal_pointer_reset_ex(ASERIALIZER_G(registered_normalizers), &pos);
    	zend_hash_get_current_data_ex(ASERIALIZER_G(registered_normalizers), (void **)&normalization_data, &pos) == SUCCESS;
    	zend_hash_move_forward_ex(ASERIALIZER_G(registered_normalizers), &pos)
	) {
    	if (!(*normalization_data)->denormalizer) {

            continue;
        }
        
        MAKE_STD_ZVAL(denormalizer);
		ZVAL_ZVAL(denormalizer, (*normalization_data)->denormalizer, 1, 0); 
        
        zend_hash_get_current_key_ex(ASERIALIZER_G(registered_normalizers), &key, &key_len, NULL, 0, &pos);
        zend_hash_update(copy, key, key_len, (void **)&denormalizer, sizeof(zval *), NULL);
	}

	Z_TYPE_P(return_value) = IS_ARRAY;
	Z_ARRVAL_P(return_value) = copy;
}

int advanced_serialize_proxy_to_normalizer(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC)
{
	zval *normalizer;
	advanced_serializer_normalization_data **normalization_data;
    
    if (zend_hash_find(ASERIALIZER_G(registered_normalizers), Z_OBJCE_P(object)->name, Z_OBJCE_P(object)->name_length + 1, (void **)&normalization_data) != SUCCESS)
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

    int result;
	smart_str buf = {0};
	php_serialize_data_t var_hash;

    if (!retval || EG(exception) || Z_TYPE_P(retval) != IS_ARRAY) {
        result = FAILURE;
    } else {
		PHP_VAR_SERIALIZE_INIT(var_hash);
    	php_var_serialize(&buf, &retval, &var_hash TSRMLS_CC);
    	PHP_VAR_SERIALIZE_DESTROY(var_hash);            
    	*buffer = (unsigned char*)buf.c;
        *buf_len = buf.len;
        result = SUCCESS;
        zval_ptr_dtor(&retval);
    }
	
	Z_ARRVAL_P(properties) = NULL;
	zval_ptr_dtor(&properties);
    
    if (result == FAILURE && !EG(exception)) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "%s::normalize() must return an array", ce->name);
    }
    return result;
}

int advanced_unserialize_proxy_to_denormalizer(zval **object, zend_class_entry *ce, const unsigned char *buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC)
{
	zval *denormalizer;
    zval zv, *properties = &zv;
    zval *retval;    
	php_unserialize_data_t var_hash;
	advanced_serializer_normalization_data **normalization_data;
	const unsigned char *max;
    
    if (zend_hash_find(ASERIALIZER_G(registered_normalizers), ce->name, ce->name_length + 1, (void **)&normalization_data) != SUCCESS)
    {
    	return FAILURE;
    }

	denormalizer = (*normalization_data)->denormalizer;

    zend_class_entry * denormalizer_ce = Z_OBJCE_P(denormalizer);
	
    object_init_ex(*object, ce);
    
    PHP_VAR_UNSERIALIZE_INIT(var_hash);

	max = (unsigned char *)buf + buf_len;	

	int result = FAILURE;

    INIT_ZVAL(zv);
    if (!php_var_unserialize(&properties, &buf, max, &var_hash TSRMLS_CC) || Z_TYPE_P(properties) != IS_ARRAY) {
        zend_throw_exception(NULL, "Could not unserialize buffer", 0 TSRMLS_CC);
        goto exit;
    }

    zend_call_method_with_2_params(&denormalizer, denormalizer_ce, NULL, "denormalize", &retval, *object, properties);

	if (!retval || EG(exception)) {
        goto exit;
    }
    switch(Z_TYPE_P(retval)) {
    case IS_NULL:
        /* we could also make this '*buf_len = 0' but this allows to skip variables */
        zval_ptr_dtor(&retval);
        goto exit;
    case IS_ARRAY:	
    	zend_hash_copy(
        	zend_std_get_properties(*object TSRMLS_CC), Z_ARRVAL_P(retval),
        	(copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *)
    	);
    	result = SUCCESS;
        break;
    default: /* failure */
        result = FAILURE;
        break;
    }
    zval_ptr_dtor(&retval);


exit: 
    if (result == FAILURE && !EG(exception)) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "%s::denormalize() must return an array", denormalizer_ce->name);
    }
    zval_dtor(properties);
    PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
	return result;
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

    		if (zend_lookup_class_ex(key, key_len - 1, NULL, 0, &ce TSRMLS_CC) == FAILURE) {
            	php_printf("continuing: %s\n", key);
            	continue;
        	}
        	
        	(*normalization_data)->object_class_entry = *ce;
        }
        
    	(*normalization_data)->original_serialize = (*normalization_data)->object_class_entry->serialize;
    	(*normalization_data)->original_unserialize = (*normalization_data)->object_class_entry->unserialize;
	
		(*normalization_data)->object_class_entry->serialize = advanced_serialize_proxy_to_normalizer;
		(*normalization_data)->object_class_entry->unserialize = advanced_unserialize_proxy_to_denormalizer;
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
		(*normalization_data)->original_serialize = NULL;
		(*normalization_data)->object_class_entry->unserialize = (*normalization_data)->original_unserialize;
		(*normalization_data)->original_unserialize = NULL;
	}
}

void advanced_serializer_normalization_data_dtor(advanced_serializer_normalization_data **data)
{
	TSRMLS_FETCH();
	
	if ((*data)->normalizer) {
		zval_ptr_dtor(&((*data)->normalizer));
	}
	
	if ((*data)->denormalizer) {
		zval_ptr_dtor(&((*data)->denormalizer));
	}
			
	efree((void *)(*data));
}