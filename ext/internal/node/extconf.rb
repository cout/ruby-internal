$: << '../..'
require 'mkmf-ruby-internal'

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i

have_type('st_data_t', [ 'ruby.h', 'st.h' ]) or
have_type('st_data_t', [ 'ruby.h', 'ruby/st.h'])

have_const('NODE_ALLOCA', [ 'ruby.h', 'node.h' ]) or
have_const('NODE_ALLOCA', [ 'ruby.h', 'ruby/node.h' ])

have_func('rb_protect_inspect', 'ruby.h')
have_func('rb_obj_respond_to', 'ruby.h')
have_func('rb_define_alloc_func', 'ruby.h')
have_func('rb_is_local_id', 'ruby.h')
have_func('rb_source_filename', 'ruby.h')

checking_for("ruby_top_cref") do
  if try_link(<<-END) then
#include <stdio.h>
extern void * ruby_top_cref;
int main() { printf("%p\\n", ruby_top_cref); return 0; }
  END
    $defs.push "-DHAVE_RUBY_TOP_CREF"
  end
end

checking_for("ruby_cref") do
  if try_link(<<-END) then
#include <stdio.h>
extern void * ruby_cref;
int main() { printf("%p\\n", ruby_cref); return 0; }
  END
    $defs.push "-DHAVE_RUBY_CREF"
  end
end

have_header('iseq.h')

ruby_version_code = RUBY_VERSION.gsub(/\./, '').to_i
$CPPFLAGS << " -DRUBY_VERSION_CODE=#{ruby_version_code}"

have_header('ruby/node.h') or have_header('node.h')

create_ruby_internal_makefile 'internal/node/node'

