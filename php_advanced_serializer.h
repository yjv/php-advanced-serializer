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
	void (*orig_unserialize_func)(INTERNAL_FUNCTION_PARAMETERS);
	HashTable *registered_normalizers;
ZEND_END_MODULE_GLOBALS(advanced_serializer)

#ifdef ZTS
#define ASERIALIZER_G(v) TSRMG(advanced_serializer_globals_id, zend_advanced_serializer_globals *, v)
#else
#define ASERIALIZER_G(v) (advanced_serializer_globals.v)
#endif

typedef struct _advanced_serializer_normalization_data {
	zval *normalizer;
	zval *denormalizer;
	int (*original_serialize)(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC);
	int (*original_unserialize)(zval **object, zend_class_entry *ce, const unsigned char *buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC);
	zend_class_entry *object_class_entry;
} advanced_serializer_normalization_data;

#define AS_GET_NORMALIZATION_DATA(class_name, class_name_len, normalizer_data_ptr_ptr) \
	if(zend_hash_find(ASERIALIZER_G(registered_normalizers), class_name, class_name_len, (void **)&normalizer_data_ptr_ptr) == FAILURE) {    \
		advanced_serializer_normalization_data *normalizer_data_ptr = (advanced_serializer_normalization_data *)emalloc(sizeof(advanced_serializer_normalization_data));                \
		normalizer_data_ptr->normalizer = NULL; \
		normalizer_data_ptr->denormalizer = NULL; \
		normalizer_data_ptr->original_serialize = NULL; \
		normalizer_data_ptr->original_unserialize = NULL; \
		normalizer_data_ptr->object_class_entry = NULL; \
		zend_hash_update(ASERIALIZER_G(registered_normalizers), class_name, class_name_len, (void **)&normalizer_data_ptr, sizeof(advanced_serializer_normalization_data *), NULL);		\
		normalizer_data_ptr_ptr = &normalizer_data_ptr;        																					\
	}
	
#define AS_SET_NORMALIZATION_ZVAL(normalization_object, type) \
    advanced_serializer_normalization_data **normalization_data; \
    AS_GET_NORMALIZATION_DATA(class_name, (class_name_len + 1), normalization_data); \
    if ((*normalization_data)->type) {								\
		  zval_ptr_dtor(&((*normalization_data)->type));  			\
    }																\
    MAKE_STD_ZVAL((*normalization_data)->type);	\
	ZVAL_ZVAL((*normalization_data)->type, normalization_object, 1, 0); 	\
	
#define ADVANCED_SERIALIZER_NORMALIZATION_DATA_DTOR (void (*)(void *))advanced_serializer_normalization_data_dtor																																			\

PHP_MINIT_FUNCTION(advanced_serializer);
PHP_RINIT_FUNCTION(advanced_serializer);
PHP_RSHUTDOWN_FUNCTION(advanced_serializer);
ZEND_MODULE_POST_ZEND_DEACTIVATE_D(advanced_serializer);

PHP_FUNCTION(advanced_serialize);
PHP_FUNCTION(advanced_unserialize);
PHP_FUNCTION(advanced_serializer_set_normalizer);
PHP_FUNCTION(advanced_serializer_set_denormalizer);
PHP_FUNCTION(advanced_serializer_get_normalizers);
PHP_FUNCTION(advanced_serializer_get_denormalizers);
int advanced_serialize_proxy_to_normalizer(zval *object, unsigned char **buffer, zend_uint *buf_len, zend_serialize_data *data TSRMLS_DC);
int advanced_unserialize_proxy_to_denormalizer(zval **object, zend_class_entry *ce, const unsigned char *buf, zend_uint buf_len, zend_unserialize_data *data TSRMLS_DC);
void restore_serialize_handlers();
void replace_serialize_handlers();
void advanced_serializer_normalization_data_dtor(advanced_serializer_normalization_data **data);

extern zend_module_entry advanced_serializer_module_entry;
#define phpext_advanced_serializer_ptr &advanced_serializer_module_entry

#endif