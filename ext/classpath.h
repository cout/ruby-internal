#ifndef classpath__h_
#define classpath__h_

#include <ruby.h>

VALUE class2path(VALUE klass);

VALUE path2class(char * path);

VALUE path2module(char * path);

#endif

