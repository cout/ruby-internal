#ifndef nodewrap__h_
#define nodewrap__h_

VALUE node_id(NODE * n);
NODE * id_to_node(VALUE id);
void dump_node_to_hash(NODE * n, VALUE node_hash);
void load_node_from_hash(NODE * n, VALUE node_id, VALUE node_hash, VALUE id_hash);

#endif

