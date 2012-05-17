$: << '../../'
require 'mkmf-ruby-internal'
have_type('struct RTypedData')
create_ruby_internal_makefile 'internal/thread/thread'
