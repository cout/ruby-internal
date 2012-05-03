$: << '../../..'
require 'mkmf-ruby-internal'
create_ruby_internal_makefile 'internal/vm/constants/constants'

have_const('VM_CALL_SEND_BIT', [ 'ruby.h', 'vm_core.h' ])
