<?php

class StdClassNormalizer implements SerializeNormalizerInterface
{
	public function normalize($object, array $properties)
	{
		return ['key' => $properties['hello']];
	}
}

class StdClassDenormalizer implements UnserializeDenormalizerInterface
{
	public function denormalize($object, array $properties)
	{
//return true;
		return ['key2' => $properties['key']];
	}
}

class Serializeable implements Serializable
{
public function serialize()
{
	return serialize(['key' => 'value']);
}

public function unserialize($serialized)
{
}
}


advanced_serializer_set_normalizer('stdClass', new StdClassNormalizer());
advanced_serializer_set_denormalizer('stdClass', new StdClassDenormalizer());
var_dump(advanced_serializer_get_normalizers());
var_dump(advanced_serializer_get_denormalizers());
var_dump(serialize('hello'));
$object = new \stdClass();
$object->hello = new \stdClass();
$object->hello->hello = 'goodbye';
var_dump(serialize($object));
var_dump(unserialize(serialize($object)));
var_dump(serialize(new \Serializeable()));
ini_set('advanced_serializer.overload_serialization_functions', '0');
var_dump(serialize('hello'));
var_dump(serialize($object));
var_dump(unserialize(serialize($object)));

