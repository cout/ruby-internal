#include "classpath.h"

VALUE
class2path(VALUE klass)
{
    VALUE path = rb_class_path(klass);
    const char *n;

    n = must_not_be_anonymous((TYPE(klass) == T_CLASS ? "class" : "module"), path);
    if (rb_path_to_class(path) != rb_class_real(klass)) {
	rb_raise(rb_eTypeError, "%s can't be referred to", n);
    }
    return path;
}

VALUE
path2class(VALUE path)
{
    VALUE v = rb_path_to_class(path);

    if (TYPE(v) != T_CLASS) {
	rb_raise(rb_eArgError, "%.*s does not refer to class",
		 (int)RSTRING_LEN(path), RSTRING_PTR(path));
    }
    return v;
}

VALUE
path2module(VALUE path)
{
    VALUE v = rb_path_to_class(path);

    if (TYPE(v) != T_MODULE) {
	rb_raise(rb_eArgError, "%.*s does not refer to module",
		 (int)RSTRING_LEN(path), RSTRING_PTR(path));
    }
    return v;
}


