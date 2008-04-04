$: << '../..'
require 'mkmf-ruby-internal'

module Kernel
  if not method_defined?(:try_const)
    alias_method :try_const, :try_constant
  end

  if not method_defined?(:have_const)
    def have_const(const, headers = nil, opt = "", &b)
      checking_for checking_message([*const].compact.join(' '), headers, opt) do
        try_const(const, headers, opt, &b)
      end
    end
  end
end

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i

if ruby_version_code >= 190
  have_type('st_data_t', 'ruby/st.h')
else
  have_type('st_data_t', 'st.h')
end

if ruby_version_code >= 190
  have_const('NODE_ALLOCA', 'ruby/node.h')
else
  have_const('NODE_ALLOCA', 'node.h')
end

have_func('rb_protect_inspect', 'ruby.h')
have_func('rb_obj_respond_to', 'ruby.h')
have_func('rb_define_alloc_func', 'ruby.h')
have_func('rb_is_local_id', 'ruby.h')

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i
$CPPFLAGS << " -DRUBY_VERSION_CODE=#{ruby_version_code}"

create_ruby_internal_makefile 'internal/node/node'

