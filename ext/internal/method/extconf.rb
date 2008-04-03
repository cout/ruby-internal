$: << '../..'
require 'mkmf-ruby-internal'
have_var('rb_cMethod', 'ruby.h')
have_var('rb_cUnboundMethod', 'ruby.h')
create_ruby_internal_makefile 'internal/method'
