#ifndef global_entry__h_
#define global_entry__h_

st_table * rb_global_tbl;

struct trace_var {
    int removed;
    void (*func)();
    VALUE data;
    struct trace_var *next;
};
struct global_variable {
    int   counter;
    void *data;
    VALUE (*getter)();
    void  (*setter)();
    void  (*marker)();
    int block_trace;
    struct trace_var *trace;
};
struct global_entry {
    struct global_variable *var;
    ID id;
};
static VALUE undef_getter();
static void  undef_setter();
static void  undef_marker();
static VALUE val_getter();
static void  val_setter();
static void  val_marker();
struct global_entry*
rb_global_entry(id)
    ID id;
{
    struct global_entry *entry;
    st_data_t data;

    if (!st_lookup(rb_global_tbl, id, &data)) {
	struct global_variable *var;
	entry = ALLOC(struct global_entry);
	var = ALLOC(struct global_variable);
	entry->id = id;
	entry->var = var;
	var->counter = 1;
	var->data = 0;
	var->getter = undef_getter;
	var->setter = undef_setter;
	var->marker = undef_marker;

	var->block_trace = 0;
	var->trace = 0;
	st_add_direct(rb_global_tbl, id, (st_data_t)entry);
    }
    else {
	entry = (struct global_entry *)data;
    }
    return entry;
}
static VALUE
undef_getter(id)
    ID id;
{
    rb_warning("global variable `%s' not initialized", rb_id2name(id));

    return Qnil;
}
static void
undef_setter(val, id, data, var)
    VALUE val;
    ID id;
    void *data;
    struct global_variable *var;
{
    var->getter = val_getter;
    var->setter = val_setter;
    var->marker = val_marker;

    var->data = (void*)val;
}
static void
undef_marker()
{
}
static VALUE
val_getter(id, val)
    ID id;
    VALUE val;
{
    return val;
}
static void
val_setter(val, id, data, var)
    VALUE val;
    ID id;
    void *data;
    struct global_variable *var;
{
    var->data = (void*)val;
}
static void
val_marker(data)
    VALUE data;
{
    if (data) rb_gc_mark_maybe(data);
}

#endif

