#ifndef CLASSPATH_H
#define CLASSPATH_H

#include "ruby.h"

VALUE path2class(
    char *path
);
VALUE path2module(
    char *path
);

#endif

