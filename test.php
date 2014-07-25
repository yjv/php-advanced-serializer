<?php

class StdClassNormalizer implements SerializeNormalizerInterface
{
	public function normalize($object, array $properties = null)
	{
		return $properties;
		return ['key' => 'value'];
	}
}

class StdClassDenormalizer implements UnserializeDenormalizerInterface
{
	public function denormalize($object, array $properties = null)
	{

		return ['key2' => 'value'];
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
// var_dump(serialize('hello'));
$object = new \stdClass();
$object->hello = 'goodbye';
var_dump(serialize($object));
var_dump(unserialize(serialize($object)));
// var_dump(serialize(new \Serializeable()));
// ini_set('advanced_serializer.overload_serialize', '0');
// var_dump(serialize('hello'));
