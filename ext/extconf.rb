require 'mkmf'

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
$CFLAGS << '-Wall -g'
create_makefile('nodewrap')

append_to_makefile = ''
rpp_files.each do |rpp_file|
append_to_makefile << <<END
#{rpp_file.sub(/\.rpp$/, '')}: #{rpp_file}
	ruby rubypp.rb $< $@
END
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

