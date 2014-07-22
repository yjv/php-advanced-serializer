<?php

class StdClassNormalizer implements SerializeNormalizerInterface
{
	public function normalize($object, array $properties = null)
	{
		return ['key' => 'value'];
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


set_serialize_normalizer('stdClass', $normalizer = new StdClassNormalizer());
// var_dump(get_registered_normalizers());
// var_dump(serialize('hello'));
$object = new \stdClass();
$object->hello = 'goodbye';
var_dump(serialize($object));
var_dump(serialize(new \Serializeable()));
// ini_set('advanced_serializer.overload_serialize', '0');
var_dump(serialize('hello'));
