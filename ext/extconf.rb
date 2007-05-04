require 'mkmf'

wd = Dir.pwd()
Dir.chdir('..')
require 'install.rb'
install_config = ConfigTable.load
Dir.chdir(wd)

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
create_makefile('nodewrap')

append_to_makefile = ''

if install_config['ruby-source-path'] then

rpp_files.each do |rpp_file|
dest_file = rpp_file.sub(/\.rpp$/, '')
append_to_makefile << <<END
#{dest_file}: #{rpp_file} #{rb_files.join(' ')}
	$(ruby) rubypp.rb #{rpp_file} #{dest_file}
END
end

else

rpp_files.each do |rpp_file|
dest_file = rpp_file.sub(/\.rpp$/, '')
append_to_makefile << <<END
#{dest_file}: #{rpp_file} #{rb_files.join(' ')}
	@$(ruby) -rftools -e 'File.copy("cached/ruby-#{RUBY_VERSION}/#{dest_file}", ".", true)'
END
end

end

append_to_makefile << <<END
$(OBJS): #{generated_files.join(' ')}
clean: clean_generated_files
clean_generated_files:
	@$(RM) #{generated_files.join(' ')}
END

File.open('Makefile', 'a') do |makefile|
  makefile.puts(append_to_makefile)
end

