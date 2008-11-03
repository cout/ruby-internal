#include "classpath.h"

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


