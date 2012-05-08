$: << '../..'
require 'mkmf-ruby-internal'
have_type('struct RTypedData')
create_ruby_internal_makefile 'internal/vm/vm'
