#include <ruby.h>
#include "tag.h"

void Init_tag()
{
  VALUE rb_mTag = rb_define_module("Tag");
  rb_define_const(rb_mTag, "RETURN", INT2NUM(TAG_RETURN));
  rb_define_const(rb_mTag, "BREAK",  INT2NUM(TAG_BREAK));
  rb_define_const(rb_mTag, "NEXT",   INT2NUM(TAG_NEXT));
  rb_define_const(rb_mTag, "RETRY",  INT2NUM(TAG_RETRY));
  rb_define_const(rb_mTag, "REDO",   INT2NUM(TAG_REDO));
  rb_define_const(rb_mTag, "RAISE",  INT2NUM(TAG_RAISE));
  rb_define_const(rb_mTag, "THROW",  INT2NUM(TAG_THROW));
  rb_define_const(rb_mTag, "FATAL",  INT2NUM(TAG_FATAL));
  rb_define_const(rb_mTag, "MASK",   INT2NUM(TAG_MASK));
}

