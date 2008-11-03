#include <ruby.h>

#ifdef RUBY_VM
#include "eval_intern.h"
#endif

#define TAG_DST() (_tag.dst == (VALUE)ruby_frame->uniq)
#define TAG_RETURN	0x1
#define TAG_BREAK	0x2
#define TAG_NEXT	0x3
#define TAG_RETRY	0x4
#define TAG_REDO	0x5
#define TAG_RAISE	0x6
#define TAG_THROW	0x7
#define TAG_FATAL	0x8
#define TAG_MASK	0xf
