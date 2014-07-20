PHP_ARG_ENABLE(advanced-serializer, whether to enable Advanced Serializer support,
[ --enable-advanced-serializer   Enable Advanced Seralizer support])

if test "$PHP_ADVANCE-SERIALIZER" = "yes"; then
  AC_DEFINE(HAVE_ADVANCED_SERIALIZER, 1, [Whether you have Advanced Serializer])
  PHP_NEW_EXTENSION(advanced_serializer, advanced_serializer.c, $ext_shared)
fi