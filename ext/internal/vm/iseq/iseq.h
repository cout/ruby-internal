#ifndef ruby_internal_vm__iseq_h_
#define ruby_internal_vm__iseq_h_

void dump_iseq_to_hash(VALUE iseq, VALUE node_hash);
VALUE load_iseq_from_hash(VALUE iseq, VALUE orig_node_id, VALUE node_hash, VALUE id_hash);

#endif
