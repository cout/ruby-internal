$: << '../..'
require 'mkmf-ruby-internal'

have_header('ruby/node.h') or have_header('node.h')
have_header('internal.h')
have_struct_member('struct RClass', 'iv_index_tbl')
have_type('struct RTypedData')
have_func('rb_add_method')

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i
$CPPFLAGS << " -DRUBY_VERSION_CODE=#{ruby_version_code}"

create_ruby_internal_makefile 'internal/module/module'
