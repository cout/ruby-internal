$: << '../../..'
require 'mkmf-ruby-internal'

have_header('ruby/node.h') or have_header('node.h')

create_ruby_internal_makefile 'internal/vm/inline_cache/inline_cache'
