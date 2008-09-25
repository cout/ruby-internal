$: << '../..'
require 'mkmf-ruby-internal'

have_header('iseq.h')

create_ruby_internal_makefile 'internal/proc/proc'

