$: << '../..'
require 'mkmf-ruby-internal'

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i
$CPPFLAGS << " -DRUBY_VERSION_CODE=#{ruby_version_code}"

have_var('rb_cMethod', 'ruby.h')
have_var('rb_cUnboundMethod', 'ruby.h')
have_header('ruby/node.h') or have_header('node.h')
have_type('struct RTypedData')

create_ruby_internal_makefile 'internal/method/method'
