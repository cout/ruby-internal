$: << '../..'
require 'mkmf-ruby-internal'

have_func('vm_get_ruby_level_cfp')
have_func('vm_get_ruby_level_next_cfp')
have_header('ruby/node.h') or have_header('node.h')

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i
$CPPFLAGS << " -DRUBY_VERSION_CODE=#{ruby_version_code}"

create_ruby_internal_makefile 'internal/module/module'
