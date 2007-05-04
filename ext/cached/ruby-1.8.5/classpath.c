#include "classpath.h"

VALUE
class2path(klass)
    VALUE klass;
{
    VALUE path = rb_class_path(klass);
    char *n = RSTRING(path)->ptr;

    if (n[0] == '#') {
	rb_raise(rb_eTypeError, "can't dump anonymous %s %s",
		 (TYPE(klass) == T_CLASS ? "class" : "module"),
		 n);
    }
    if (rb_path2class(n) != rb_class_real(klass)) {
	rb_raise(rb_eTypeError, "%s can't be referred", n);
    }
    return path;
}

VALUE
path2class(path)
    char *path;
{
    VALUE v = rb_path2class(path);

    if (TYPE(v) != T_CLASS) {
	rb_raise(rb_eArgError, "%s does not refer class", path);
    }
    return v;
}

VALUE
path2module(path)
    char *path;
{
    VALUE v = rb_path2class(path);

    if (TYPE(v) != T_MODULE) {
	rb_raise(rb_eArgError, "%s does not refer module", path);
    }
    return v;
}


