#ifndef PHP_ADVANCED_SERIALIZER_H
#define PHP_ADVANCED_SERIALIZER_H 1

#define PHP_ADVANCED_SERIALIZER_VERSION "1.0"
#define PHP_ADVANCED_SERIALIZER_EXTNAME "advanced_serializer"

PHP_MINIT_FUNCTION(advanced_serializer);
PHP_FUNCTION(advanced_serialize);
PHP_FUNCTION(set_serialize_normalizer);

extern zend_module_entry advanced_serializer_module_entry;
#define phpext_hello_ptr &advanced_serializer_module_entry

#endif