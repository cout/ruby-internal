#ifndef ruby_internal_node__h_
#define ruby_internal_node__h_

#include <ruby.h>

#ifdef RUBY_VM
#include "ruby/node.h"
#else
#include "node.h"
#endif

VALUE wrap_node(NODE * n);
VALUE wrap_node_as(NODE * n, VALUE klass);

NODE * unwrap_node(VALUE r);

VALUE node_id(NODE * n);
NODE * id_to_node(VALUE id);

VALUE eval_ruby_node(NODE * node, VALUE self, VALUE cref);

void dump_node_or_iseq_to_hash(VALUE n, int node_type, VALUE node_hash);
VALUE load_node_or_iseq_from_hash(VALUE node_id, VALUE node_hash, VALUE id_hash);

void dump_node_to_hash(NODE * n, int node_type, VALUE node_hash);
NODE * load_node_from_hash(VALUE arr, VALUE orig_node_id, VALUE node_hash, VALUE id_hash);

extern VALUE rb_cNodeSubclass[NODE_LAST];

#endif

