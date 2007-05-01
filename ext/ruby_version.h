#ifndef RUBY_VERSION_H
#define RUBY_VERSION_H

#include "version.h"

/* TODO: Not the best way to check this */
#if RUBY_RELEASE_CODE >= 20070427
#define RUBY_HAS_YARV
#endif

#endif
