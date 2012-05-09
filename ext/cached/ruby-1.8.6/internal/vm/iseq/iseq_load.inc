#ifndef ruby_internal_iseq_load__inc_
#define ruby_internal_iseq_load__inc_

#ifdef HAVE_TYPE_STRUCT_RTYPEDDATA

  static rb_data_type_t const * p_iseq_data_type;

  static void init_iseq_data_type()
  {
    rb_proc_t * p;
    VALUE body;

    VALUE binding = rb_binding_new();
    VALUE new_proc = rb_funcall(
        rb_cObject, rb_intern("eval"), 2, rb_str_new2("proc { }"), binding);
    GetProcPtr(new_proc, p);
    body = p->block.iseq->self;

    p_iseq_data_type = RTYPEDDATA_TYPE(body);
  }

#else

#endif
#

#endif

