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
static void
compile_data_free(struct iseq_compile_data *compile_data)
{
    if (compile_data) {
	struct iseq_compile_data_storage *cur, *next;
	cur = compile_data->storage_head;
	while (cur) {
	    next = cur->next;
	    ruby_xfree(cur);
	    cur = next;
	}
	ruby_xfree(compile_data);
    }
}
static void
iseq_free(void *ptr)
{
    rb_iseq_t *iseq;
    RUBY_FREE_ENTER("iseq");

    if (ptr) {
	iseq = ptr;
	if (!iseq->orig) {
	    /* It's possible that strings are freed */
	    if (0) {
		RUBY_GC_INFO("%s @ %s\n", RSTRING_PTR(iseq->name),
					  RSTRING_PTR(iseq->filename));
	    }

	    if (iseq->iseq != iseq->iseq_encoded) {
		RUBY_FREE_UNLESS_NULL(iseq->iseq_encoded);
	    }

	    RUBY_FREE_UNLESS_NULL(iseq->iseq);
	    RUBY_FREE_UNLESS_NULL(iseq->insn_info_table);
	    RUBY_FREE_UNLESS_NULL(iseq->local_table);
	    RUBY_FREE_UNLESS_NULL(iseq->ic_entries);
	    RUBY_FREE_UNLESS_NULL(iseq->catch_table);
	    RUBY_FREE_UNLESS_NULL(iseq->arg_opt_table);
	    compile_data_free(iseq->compile_data);
	}
	ruby_xfree(ptr);
    }
    RUBY_FREE_LEAVE("iseq");
}
static void
iseq_mark(void *ptr)
{
    RUBY_MARK_ENTER("iseq");

    if (ptr) {
	rb_iseq_t *iseq = ptr;

	RUBY_GC_INFO("%s @ %s\n", RSTRING_PTR(iseq->name), RSTRING_PTR(iseq->filename));
	RUBY_MARK_UNLESS_NULL(iseq->mark_ary);
	RUBY_MARK_UNLESS_NULL(iseq->name);
	RUBY_MARK_UNLESS_NULL(iseq->filename);
	RUBY_MARK_UNLESS_NULL(iseq->filepath);
	RUBY_MARK_UNLESS_NULL((VALUE)iseq->cref_stack);
	RUBY_MARK_UNLESS_NULL(iseq->klass);
	RUBY_MARK_UNLESS_NULL(iseq->coverage);
#if 0
	RUBY_MARK_UNLESS_NULL((VALUE)iseq->node);
	RUBY_MARK_UNLESS_NULL(iseq->cached_special_block);
#endif
	RUBY_MARK_UNLESS_NULL(iseq->orig);

	if (iseq->compile_data != 0) {
	    struct iseq_compile_data *const compile_data = iseq->compile_data;
	    RUBY_MARK_UNLESS_NULL(compile_data->mark_ary);
	    RUBY_MARK_UNLESS_NULL(compile_data->err_info);
	    RUBY_MARK_UNLESS_NULL(compile_data->catch_table_ary);
	}
    }
    RUBY_MARK_LEAVE("iseq");
}
static size_t
iseq_memsize(const void *ptr)
{
    size_t size = sizeof(rb_iseq_t);
    const rb_iseq_t *iseq;

    if (ptr) {
	iseq = ptr;
	if (!iseq->orig) {
	    if (iseq->iseq != iseq->iseq_encoded) {
		size += iseq->iseq_size * sizeof(VALUE);
	    }

	    size += iseq->iseq_size * sizeof(VALUE);
	    size += iseq->insn_info_size * sizeof(struct iseq_insn_info_entry);
	    size += iseq->local_table_size * sizeof(ID);
	    size += iseq->catch_table_size * sizeof(struct iseq_catch_table_entry);
	    size += iseq->arg_opts * sizeof(VALUE);
	    size += iseq->ic_size * sizeof(struct iseq_inline_cache_entry);

	    if (iseq->compile_data) {
		struct iseq_compile_data_storage *cur;

		cur = iseq->compile_data->storage_head;
		while (cur) {
		    size += cur->size + sizeof(struct iseq_compile_data_storage);
		    cur = cur->next;
		}
		size += sizeof(struct iseq_compile_data);
	    }
	}
    }

    return size;
}
static VALUE
iseq_alloc(VALUE klass)
{
    rb_iseq_t *iseq;
    return TypedData_Make_Struct(klass, rb_iseq_t, p_iseq_data_type, iseq);
}
static void
set_relation(rb_iseq_t *iseq, const VALUE parent)
{
    const VALUE type = iseq->type;
    rb_thread_t *th = GET_THREAD();

    /* set class nest stack */
    if (type == ISEQ_TYPE_TOP) {
	/* toplevel is private */
	iseq->cref_stack = NEW_BLOCK(rb_cObject);
	iseq->cref_stack->nd_file = 0;
	iseq->cref_stack->nd_visi = NOEX_PRIVATE;
	if (th->top_wrapper) {
	    NODE *cref = NEW_BLOCK(th->top_wrapper);
	    cref->nd_file = 0;
	    cref->nd_visi = NOEX_PRIVATE;
	    cref->nd_next = iseq->cref_stack;
	    iseq->cref_stack = cref;
	}
    }
    else if (type == ISEQ_TYPE_METHOD || type == ISEQ_TYPE_CLASS) {
	iseq->cref_stack = NEW_BLOCK(0); /* place holder */
	iseq->cref_stack->nd_file = 0;
    }
    else if (RTEST(parent)) {
	rb_iseq_t *piseq;
	GetISeqPtr(parent, piseq);
	iseq->cref_stack = piseq->cref_stack;
    }

    if (type == ISEQ_TYPE_TOP ||
	type == ISEQ_TYPE_METHOD || type == ISEQ_TYPE_CLASS) {
	iseq->local_iseq = iseq;
    }
    else if (RTEST(parent)) {
	rb_iseq_t *piseq;
	GetISeqPtr(parent, piseq);
	iseq->local_iseq = piseq->local_iseq;
    }

    if (RTEST(parent)) {
	rb_iseq_t *piseq;
	GetISeqPtr(parent, piseq);
	iseq->parent_iseq = piseq;
    }
}
static VALUE
prepare_iseq_build(rb_iseq_t *iseq,
		   VALUE name, VALUE filename, VALUE filepath, VALUE line_no,
		   VALUE parent, VALUE type, VALUE block_opt,
		   const rb_compile_option_t *option)
{
    OBJ_FREEZE(name);
    OBJ_FREEZE(filename);

    iseq->name = name;
    iseq->filename = filename;
    iseq->filepath = filepath == Qnil ? Qnil : rb_realpath_internal(Qnil, filepath, 1);
    iseq->line_no = line_no;
    iseq->defined_method_id = 0;
    iseq->mark_ary = rb_ary_tmp_new(3);
    OBJ_UNTRUST(iseq->mark_ary);
    RBASIC(iseq->mark_ary)->klass = 0;

    iseq->type = type;
    iseq->arg_rest = -1;
    iseq->arg_block = -1;
    iseq->klass = 0;

    /*
     * iseq->special_block_builder = GC_GUARDED_PTR_REF(block_opt);
     * iseq->cached_special_block_builder = 0;
     * iseq->cached_special_block = 0;
     */

    iseq->compile_data = ALLOC(struct iseq_compile_data);
    MEMZERO(iseq->compile_data, struct iseq_compile_data, 1);
    iseq->compile_data->mark_ary = rb_ary_tmp_new(3);

    iseq->compile_data->storage_head = iseq->compile_data->storage_current =
      (struct iseq_compile_data_storage *)
	ALLOC_N(char, INITIAL_ISEQ_COMPILE_DATA_STORAGE_BUFF_SIZE +
		sizeof(struct iseq_compile_data_storage));

    iseq->compile_data->catch_table_ary = rb_ary_new();
    iseq->compile_data->storage_head->pos = 0;
    iseq->compile_data->storage_head->next = 0;
    iseq->compile_data->storage_head->size =
      INITIAL_ISEQ_COMPILE_DATA_STORAGE_BUFF_SIZE;
    iseq->compile_data->storage_head->buff =
      (char *)(&iseq->compile_data->storage_head->buff + 1);
    iseq->compile_data->option = option;
    iseq->compile_data->last_coverable_line = -1;

    set_relation(iseq, parent);

    iseq->coverage = Qfalse;
    if (!GET_THREAD()->parse_in_eval) {
	extern VALUE rb_get_coverages(void);
	VALUE coverages = rb_get_coverages();
	if (RTEST(coverages)) {
	    iseq->coverage = rb_hash_lookup(coverages, filename);
	    if (NIL_P(iseq->coverage)) iseq->coverage = Qfalse;
	}
    }

    return Qtrue;
}
static VALUE
cleanup_iseq_build(rb_iseq_t *iseq)
{
    struct iseq_compile_data *data = iseq->compile_data;
    VALUE err = data->err_info;
    iseq->compile_data = 0;
    compile_data_free(data);

    if (RTEST(err)) {
	rb_funcall2(err, rb_intern("set_backtrace"), 1, &iseq->filename);
	rb_exc_raise(err);
    }
    return Qtrue;
}
static rb_compile_option_t COMPILE_OPTION_DEFAULT = {
    OPT_INLINE_CONST_CACHE, /* int inline_const_cache; */
    OPT_PEEPHOLE_OPTIMIZATION, /* int peephole_optimization; */
    OPT_TAILCALL_OPTIMIZATION, /* int tailcall_optimization */
    OPT_SPECIALISED_INSTRUCTION, /* int specialized_instruction; */
    OPT_OPERANDS_UNIFICATION, /* int operands_unification; */
    OPT_INSTRUCTIONS_UNIFICATION, /* int instructions_unification; */
    OPT_STACK_CACHING, /* int stack_caching; */
    OPT_TRACE_INSTRUCTION, /* int trace_instruction */
};
static const rb_compile_option_t COMPILE_OPTION_FALSE = {0};
static void
make_compile_option(rb_compile_option_t *option, VALUE opt)
{
    if (opt == Qnil) {
	*option = COMPILE_OPTION_DEFAULT;
    }
    else if (opt == Qfalse) {
	*option = COMPILE_OPTION_FALSE;
    }
    else if (opt == Qtrue) {
	memset(option, 1, sizeof(rb_compile_option_t));
    }
    else if (CLASS_OF(opt) == rb_cHash) {
	*option = COMPILE_OPTION_DEFAULT;

#define SET_COMPILE_OPTION(o, h, mem) \
  { VALUE flag = rb_hash_aref(h, ID2SYM(rb_intern(#mem))); \
      if (flag == Qtrue)  { o->mem = 1; } \
      else if (flag == Qfalse)  { o->mem = 0; } \
  }
#define SET_COMPILE_OPTION_NUM(o, h, mem) \
  { VALUE num = rb_hash_aref(opt, ID2SYM(rb_intern(#mem))); \
      if (!NIL_P(num)) o->mem = NUM2INT(num); \
  }
	SET_COMPILE_OPTION(option, opt, inline_const_cache);
	SET_COMPILE_OPTION(option, opt, peephole_optimization);
	SET_COMPILE_OPTION(option, opt, tailcall_optimization);
	SET_COMPILE_OPTION(option, opt, specialized_instruction);
	SET_COMPILE_OPTION(option, opt, operands_unification);
	SET_COMPILE_OPTION(option, opt, instructions_unification);
	SET_COMPILE_OPTION(option, opt, stack_caching);
	SET_COMPILE_OPTION(option, opt, trace_instruction);
	SET_COMPILE_OPTION_NUM(option, opt, debug_level);
#undef SET_COMPILE_OPTION
#undef SET_COMPILE_OPTION_NUM
    }
    else {
	rb_raise(rb_eTypeError, "Compile option must be Hash/true/false/nil");
    }
}
static VALUE
make_compile_option_value(rb_compile_option_t *option)
{
    VALUE opt = rb_hash_new();
#define SET_COMPILE_OPTION(o, h, mem) \
  rb_hash_aset(h, ID2SYM(rb_intern(#mem)), o->mem ? Qtrue : Qfalse)
#define SET_COMPILE_OPTION_NUM(o, h, mem) \
  rb_hash_aset(h, ID2SYM(rb_intern(#mem)), INT2NUM(o->mem))
    {
	SET_COMPILE_OPTION(option, opt, inline_const_cache);
	SET_COMPILE_OPTION(option, opt, peephole_optimization);
	SET_COMPILE_OPTION(option, opt, tailcall_optimization);
	SET_COMPILE_OPTION(option, opt, specialized_instruction);
	SET_COMPILE_OPTION(option, opt, operands_unification);
	SET_COMPILE_OPTION(option, opt, instructions_unification);
	SET_COMPILE_OPTION(option, opt, stack_caching);
	SET_COMPILE_OPTION_NUM(option, opt, debug_level);
    }
#undef SET_COMPILE_OPTION
#undef SET_COMPILE_OPTION_NUM
    return opt;
}
#define CHECK_ARRAY(v)   rb_convert_type(v, T_ARRAY, "Array", "to_ary")
#define CHECK_STRING(v)  rb_convert_type(v, T_STRING, "String", "to_str")
#define CHECK_SYMBOL(v)  rb_convert_type(v, T_SYMBOL, "Symbol", "to_sym")
static inline VALUE CHECK_INTEGER(VALUE v) {NUM2LONG(v); return v;}
static VALUE
iseq_load(VALUE self, VALUE data, VALUE parent, VALUE opt)
{
    VALUE iseqval = iseq_alloc(self);

    VALUE magic, version1, version2, format_type, misc;
    VALUE name, filename, filepath, line_no;
    VALUE type, body, locals, args, exception;

    VALUE iseq_type;
    struct st_table *type_map = 0;
    rb_iseq_t *iseq;
    rb_compile_option_t option;
    int i = 0;

    /* [magic, major_version, minor_version, format_type, misc,
     *  name, filename, line_no,
     *  type, locals, args, exception_table, body]
     */

    data        = CHECK_ARRAY(data);

    magic       = CHECK_STRING(rb_ary_entry(data, i++));
    version1    = CHECK_INTEGER(rb_ary_entry(data, i++));
    version2    = CHECK_INTEGER(rb_ary_entry(data, i++));
    format_type = CHECK_INTEGER(rb_ary_entry(data, i++));
    misc        = rb_ary_entry(data, i++); /* TODO */

    name        = CHECK_STRING(rb_ary_entry(data, i++));
    filename    = CHECK_STRING(rb_ary_entry(data, i++));
    filepath    = rb_ary_entry(data, i++);
    filepath    = NIL_P(filepath) ? Qnil : CHECK_STRING(filepath);
    line_no     = CHECK_INTEGER(rb_ary_entry(data, i++));

    type        = CHECK_SYMBOL(rb_ary_entry(data, i++));
    locals      = CHECK_ARRAY(rb_ary_entry(data, i++));

    args        = rb_ary_entry(data, i++);
    if (FIXNUM_P(args) || (args = CHECK_ARRAY(args))) {
	/* */
    }

    exception   = CHECK_ARRAY(rb_ary_entry(data, i++));
    body        = CHECK_ARRAY(rb_ary_entry(data, i++));

    GetISeqPtr(iseqval, iseq);
    iseq->self = iseqval;

    if (type_map == 0) {
	type_map = st_init_numtable();
	st_insert(type_map, ID2SYM(rb_intern("top")), ISEQ_TYPE_TOP);
	st_insert(type_map, ID2SYM(rb_intern("method")), ISEQ_TYPE_METHOD);
	st_insert(type_map, ID2SYM(rb_intern("block")), ISEQ_TYPE_BLOCK);
	st_insert(type_map, ID2SYM(rb_intern("class")), ISEQ_TYPE_CLASS);
	st_insert(type_map, ID2SYM(rb_intern("rescue")), ISEQ_TYPE_RESCUE);
	st_insert(type_map, ID2SYM(rb_intern("ensure")), ISEQ_TYPE_ENSURE);
	st_insert(type_map, ID2SYM(rb_intern("eval")), ISEQ_TYPE_EVAL);
	st_insert(type_map, ID2SYM(rb_intern("main")), ISEQ_TYPE_MAIN);
	st_insert(type_map, ID2SYM(rb_intern("defined_guard")), ISEQ_TYPE_DEFINED_GUARD);
    }

    if (st_lookup(type_map, type, &iseq_type) == 0) {
	const char *typename = rb_id2name(type);
	if (typename)
	    rb_raise(rb_eTypeError, "unsupport type: :%s", typename);
	else
	    rb_raise(rb_eTypeError, "unsupport type: %p", (void *)type);
    }

    if (parent == Qnil) {
	parent = 0;
    }

    make_compile_option(&option, opt);
    prepare_iseq_build(iseq, name, filename, filepath, line_no,
		       parent, iseq_type, 0, &option);

    rb_iseq_build_from_ary(iseq, locals, args, exception, body);

    cleanup_iseq_build(iseq);
    return iseqval;
}

#endif

