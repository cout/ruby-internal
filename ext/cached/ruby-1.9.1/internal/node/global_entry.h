#ifndef global_entry__h_
#define global_entry__h_

st_table * rb_global_tbl;

#define global_variable rb_global_variable
#define gvar_getter_t rb_gvar_getter_t
#define gvar_setter_t rb_gvar_setter_t
#define gvar_marker_t rb_gvar_marker_t
struct trace_var {
    int removed;
    void (*func)(VALUE arg, VALUE val);
    VALUE data;
    struct trace_var *next;
};
struct global_variable {
    int   counter;
    void *data;
    gvar_getter_t *getter;
    gvar_setter_t *setter;
    gvar_marker_t *marker;
    int block_trace;
    struct trace_var *trace;
};
struct global_entry {
    struct global_variable *var;
    ID id;
};
#define undef_getter	rb_gvar_undef_getter
#define undef_setter	rb_gvar_undef_setter
#define undef_marker	rb_gvar_undef_marker
#define val_getter	rb_gvar_val_getter
#define val_setter	rb_gvar_val_setter
#define val_marker	rb_gvar_val_marker
struct global_entry*
rb_global_entry(ID id)
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
VALUE
undef_getter(ID id, void *data, struct global_variable *var)
{
    rb_warning("global variable `%s' not initialized", rb_id2name(id));

    return Qnil;
}
void
undef_setter(VALUE val, ID id, void *data, struct global_variable *var)
{
    var->getter = val_getter;
    var->setter = val_setter;
    var->marker = val_marker;

    var->data = (void*)val;
}
void
undef_marker(VALUE *var)
{
}
VALUE
val_getter(ID id, void *data, struct global_variable *var)
{
    return (VALUE)data;
}
void
val_setter(VALUE val, ID id, void *data, struct global_variable *var)
{
    var->data = (void*)val;
}
void
val_marker(VALUE *var)
{
    VALUE data = (VALUE)var;
    if (data) rb_gc_mark_maybe(data);
}

#endif

