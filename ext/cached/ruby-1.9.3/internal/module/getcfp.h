#ifndef getcfp__h
#define getcfp__h

#include "ruby.h"

#ifdef RUBY_VM

#include "vm_core.h"

static rb_control_frame_t *
getcfp(rb_thread_t *th, rb_control_frame_t *cfp)
{
    while (!RUBY_VM_CONTROL_FRAME_STACK_OVERFLOW_P(th, cfp)) {
	if (RUBY_VM_NORMAL_ISEQ_P(cfp->iseq)) {
	    return cfp;
	}
	cfp = RUBY_VM_PREVIOUS_CONTROL_FRAME(cfp);
    }
    return 0;
}


#endif

#endif
