#ifndef global_entry__h_
#define global_entry__h_

struct global_entry {
    ID id;
    void *data;
    VALUE (*getter)();
    void  (*setter)();
    void  (*marker)();
    int block_trace;
    struct trace_var *trace;
};

#endif

