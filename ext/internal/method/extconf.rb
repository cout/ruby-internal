$: << '../..'
require 'mkmf-ruby-internal'

have_var('rb_cMethod', 'ruby.h')
have_var('rb_cUnboundMethod', 'ruby.h')
have_header('ruby/node.h') or have_header('node.h')

create_ruby_internal_makefile 'internal/method/method'
