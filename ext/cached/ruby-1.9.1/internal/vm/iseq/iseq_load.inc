#ifndef ruby_internal_iseq_load__inc_
#define ruby_internal_iseq_load__inc_

#include "internal/yarv-headers/gc.h"
#include "internal/yarv-headers/iseq.h"

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
	    /* It's possible that strings are freed
	     * GC_INFO("%s @ %s\n", RSTRING_PTR(iseq->name),
	     *                      RSTRING_PTR(iseq->filename));
	     */
	    if (iseq->iseq != iseq->iseq_encoded) {
		RUBY_FREE_UNLESS_NULL(iseq->iseq_encoded);
	    }

	    RUBY_FREE_UNLESS_NULL(iseq->iseq);
	    RUBY_FREE_UNLESS_NULL(iseq->insn_info_table);
	    RUBY_FREE_UNLESS_NULL(iseq->local_table);
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
    rb_iseq_t *iseq;
    RUBY_MARK_ENTER("iseq");

    if (ptr) {
	iseq = ptr;
	RUBY_GC_INFO("%s @ %s\n", RSTRING_PTR(iseq->name), RSTRING_PTR(iseq->filename));
	RUBY_MARK_UNLESS_NULL(iseq->mark_ary);
	RUBY_MARK_UNLESS_NULL(iseq->name);
	RUBY_MARK_UNLESS_NULL(iseq->filename);
	RUBY_MARK_UNLESS_NULL((VALUE)iseq->cref_stack);
	RUBY_MARK_UNLESS_NULL(iseq->klass);
	RUBY_MARK_UNLESS_NULL(iseq->coverage);
/* 	RUBY_MARK_UNLESS_NULL((VALUE)iseq->node); */
/*	RUBY_MARK_UNLESS_NULL(iseq->cached_special_block); */
	RUBY_MARK_UNLESS_NULL(iseq->orig);

	if (iseq->compile_data != 0) {
	    RUBY_MARK_UNLESS_NULL(iseq->compile_data->mark_ary);
	    RUBY_MARK_UNLESS_NULL(iseq->compile_data->err_info);
	    RUBY_MARK_UNLESS_NULL(iseq->compile_data->catch_table_ary);
	}
    }
    RUBY_MARK_LEAVE("iseq");
}
static VALUE
iseq_alloc(VALUE klass)
{
    VALUE volatile obj;
    rb_iseq_t *iseq;

    obj = Data_Make_Struct(klass, rb_iseq_t, iseq_mark, iseq_free, iseq);
    MEMZERO(iseq, rb_iseq_t, 1);
    return obj;
}
static VALUE
prepare_iseq_build(rb_iseq_t *iseq,
		   VALUE name, VALUE filename,
		   VALUE parent, VALUE type, VALUE block_opt,
		   const rb_compile_option_t *option)
{
    OBJ_FREEZE(name);
    OBJ_FREEZE(filename);

    iseq->name = name;
    iseq->filename = filename;
    iseq->defined_method_id = 0;
    iseq->mark_ary = rb_ary_new();
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
    iseq->compile_data->mark_ary = rb_ary_new();
    RBASIC(iseq->compile_data->mark_ary)->klass = 0;

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
    VALUE name, filename;
    VALUE type, body, locals, args, exception;

    VALUE iseq_type;
    struct st_table *type_map = 0;
    rb_iseq_t *iseq;
    rb_compile_option_t option;
    int i = 0;

    /* [magic, major_version, minor_version, format_type, misc,
     *  name, filename,
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
    prepare_iseq_build(iseq, name, filename,
		       parent, iseq_type, 0, &option);

    rb_iseq_build_from_ary(iseq, locals, args, exception, body);

    cleanup_iseq_build(iseq);
    return iseqval;
}

#endif

