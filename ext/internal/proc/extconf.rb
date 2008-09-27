$: << '../..'
require 'mkmf-ruby-internal'

have_header('iseq.h')

have_header('ruby/node.h') or have_header('node.h')

create_ruby_internal_makefile 'internal/proc/proc'

