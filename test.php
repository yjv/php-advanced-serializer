<?php

class StdClassNormalizer implements SerializeNormalizerInterface
{
	public function normalize($object, array $properties = null)
	{
	}
}


set_serialize_normalizer('stdClass', $normalizer = new StdClassNormalizer());
var_dump(get_registered_normalizers());
var_dump(serialize('hello'));
var_dump(serialize(new \stdClass()));
ini_set('advanced_serializer.overload_serialize', '0');
var_dump(serialize('hello'));
