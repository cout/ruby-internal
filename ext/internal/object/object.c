#include <ruby.h>

/*
 * call-seq:
 *   class.real_class => Class
 *
 * Return the object's first immediate ancestor; this may be the
 * object's class, its singleton class, or a module singleton.
 */
static VALUE real_class(VALUE self)
{
  if(IMMEDIATE_P(self))
  {
    return CLASS_OF(self);
  }
  else
  {
    return RBASIC(self)->klass;
  }
}

/*
 * call-seq:
 *   class.singleton? => true or false
 *
 * Return true if this object is a singleton (that is, it has the
 * FL_SINGLETON flag set).
 */
static VALUE is_singleton(VALUE self)
{
  return FL_TEST(self, FL_SINGLETON) ? Qtrue : Qfalse;
}

/*
 * call-seq:
 *   class.has_singleton? => true or false
 *
 * Return true if this object has a singleton class.
 */
static VALUE has_singleton(VALUE self)
{
  if(FIXNUM_P(self) || SYMBOL_P(self))
  {
    return Qfalse;
  }
  else if(rb_special_const_p(self))
  {
    return Qtrue;
  }
  else
  {
    return FL_TEST(RBASIC(self)->klass, FL_SINGLETON) ? Qtrue : Qfalse;
  }
}

/*
 * call-seq:
 *   class.singleton_class => Class
 *
 * Return the object's singleton class.  Creats a new singleton class
 * for the object if it does not have one.  See RCR#231.
 */
static VALUE singleton_class(VALUE self)
{
  return rb_singleton_class(self);
}

void Init_object(void)
{
  rb_define_method(rb_mKernel, "real_class", real_class, 0);
  rb_define_method(rb_mKernel, "singleton?", is_singleton, 0);
  rb_define_method(rb_mKernel, "has_singleton?", has_singleton, 0);
  rb_define_method(rb_mKernel, "singleton_class", singleton_class, 0);
}

