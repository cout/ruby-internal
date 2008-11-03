#ifndef CLASSPATH_H
#define CLASSPATH_H

#include "ruby.h"

VALUE class2path(
    VALUE klass
);
VALUE path2class(
    const char *path
);
VALUE path2module(
    const char *path
);

#endif

