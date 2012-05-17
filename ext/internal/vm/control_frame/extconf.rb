$: << '../../..'
require 'mkmf-ruby-internal'

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i
$CPPFLAGS << " -DRUBY_VERSION_CODE=#{ruby_version_code}"

have_type('struct RTypedData')

create_ruby_internal_makefile 'internal/vm/control_frame/control_frame'
