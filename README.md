php-advanced-serializer
=======================

a php extension that adds extra features to the serialize method.

Normalizers & Denormalizers
---------------------------

you can use normalizers and denormalizers to customize how objects get serialized and desrialized
you use the `advanced_serializer_set_normalizer` and `advanced_unserializer_set_denormalizer`
to assign normalizers and denormalizers for serialization and deserialization respectively.
both methods take 2 arguments.

1. the class name that this normalizer/denormalizer handles
2. the actual normalizer


###normalizers###
normalizers must follow the `SerializeNormalizerInterface` which defines the `normalize` method
it gets passed 2 arguments 

  1. the object to be normalized into an array.
  2. the objects properties as an array. this makes it so you dont have to use reflection to get the properties even though the normalizer is not in the objects scope.

the `normalize` method must return an array. whatever is in the array is what will be serialized.
any value in the array that is an object will be serialized with it's normalizer if there is one and
if not it will be serialized using the default serialization.

###denormalizers###
denormalizers must follow the `UnserializeDenormalizerInterface` which defines the `denormalize` method
it gets passed 2 arguments 

1. the object to be denormalized from the array passed in arg 2.
2. the objects properties deserialized as an array.

the `denormalize` method must return an array or false. if an array is returned it will be used to set the properties.
if false is returned then then the unserialization will suppose that the denormalizer set the peorperties itself.


###ini settings###
there is currently one ini setting that this extension adds

- `advanced_serializer.overload_serialization_functions` - this is 1 by default and tells the extension to overload the `serialize` and `unserialize` functions to use the normalizers when they are registered. if this is set to 0 then these functions will function as if the extension was not registered.
