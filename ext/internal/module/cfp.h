#ifndef ruby_internal__module_cfp__h
#define ruby_internal__module_cfp__h

#include "vm_core.h"
#include "eval_intern.h"

rb_control_frame_t *
vm_get_ruby_level_cfp(rb_thread_t *th, rb_control_frame_t *cfp);

rb_control_frame_t *
rb_vm_get_ruby_level_next_cfp(rb_thread_t *th, rb_control_frame_t *cfp);

#endif // ruby_internal__module_cfp__h
