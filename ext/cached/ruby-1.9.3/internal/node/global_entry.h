#ifndef global_entry__h_
#define global_entry__h_

st_table * rb_global_tbl;

#define global_entry rb_global_entry
#define gvar_getter_t rb_gvar_getter_t
#define gvar_setter_t rb_gvar_setter_t
#define gvar_marker_t rb_gvar_marker_t

struct global_variable {
    int   counter;
    void *data;
    gvar_getter_t *getter;
    gvar_setter_t *setter;
    gvar_marker_t *marker;
    int block_trace;
    struct trace_var *trace;
};
#define undef_getter	rb_gvar_undef_getter
#define undef_setter	rb_gvar_undef_setter
#define undef_marker	rb_gvar_undef_marker
struct global_entry*
rb_global_entry(ID id)
{
    struct global_entry *entry;
    st_data_t data;

    if (!st_lookup(rb_global_tbl, (st_data_t)id, &data)) {
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

#endif

