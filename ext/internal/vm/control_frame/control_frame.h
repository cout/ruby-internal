#ifndef RUBY_INTERNAL__VM__CONTROL_FRAME__CONTROL_FRAME_H
#define RUBY_INTERNAL__VM__CONTROL_FRAME__CONTROL_FRAME_H

#include <ruby.h>

#ifdef RUBY_VM

#include "vm_core.h"

struct RubyInternalControlFrame
{
  rb_control_frame_t * control_frame;
  VALUE thread;
};

#endif

#endif
