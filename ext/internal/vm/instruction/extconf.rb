$: << '../../..'
require 'mkmf-ruby-internal'

have_header('vm.h')

create_ruby_internal_makefile 'internal/vm/instruction/instruction'
