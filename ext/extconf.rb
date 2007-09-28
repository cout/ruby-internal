require 'mkmf'
require 'ruby_version_code'

cmdline_ruby_source_path = arg_config('--ruby-source-path')
configured_ruby_source_dir = nil
begin
  require 'ruby_source_dir'
  configured_ruby_source_dir = RUBY_SOURCE_DIR
rescue InstallError
end

rb_files = Dir['*.rb']

rpp_files = Dir['*.rpp']
generated_files = rpp_files.map { |f| f.sub(/\.rpp$/, '') }

srcs = Dir['*.c']
generated_files.each do |f|
  if f =~ /\.c$/ then
    srcs << f
  end
end
srcs.uniq!
$objs = srcs.map { |f| f.sub(/\.c$/, ".#{$OBJEXT}") }
$CFLAGS << ' -Wall -g'
if RUBY_VERSION_CODE >= 190 then
$CPPFLAGS << " -I#{RUBY_SOURCE_DIR}"
end
create_makefile('nodewrap')

append_to_makefile = ''

# (nil and not configured => cached
# '' => cached
# not configured => cached

if (not cmdline_ruby_source_path and not configured_ruby_source_dir) or \
   cmdline_ruby_source_path == '' then

rpp_files.each do |rpp_file|
dest_file = rpp_file.sub(/\.rpp$/, '')
append_to_makefile << <<END
#{dest_file}: #{rpp_file} #{rb_files.join(' ')}
	@$(RUBY) -rftools -e 'File.copy("cached/ruby-#{RUBY_VERSION}/#{dest_file}", ".", true)'
END
end

else

rpp_files.each do |rpp_file|
dest_file = rpp_file.sub(/\.rpp$/, '')
append_to_makefile << <<END
#{dest_file}: #{rpp_file} #{rb_files.join(' ')}
	$(RUBY) rubypp.rb #{rpp_file} #{dest_file}
END
end


end

generated_headers = generated_files.select { |x| x =~ /\.h$/ }
append_to_makefile << <<END
$(OBJS): #{generated_headers.join(' ')}
clean: clean_generated_files
clean_generated_files:
	@$(RM) #{generated_files.join(' ')}
END

File.open('Makefile', 'a') do |makefile|
  makefile.puts(append_to_makefile)
end

