require 'mkmf'

$CFLAGS << '-Wall -g'
create_makefile('nodewrap')

rpp_files = Dir['*.rpp']
generated_files = rpp_files.map { |f| f.sub(/\.rpp$/, '') }

# TODO: I think this syntax is gmake-specific
# TODO: Add some extra rules to "make clean"
append_to_makefile = ''
rpp_files.each do |rpp_file|
append_to_makefile << <<END
#{rpp_file.sub(/\.rpp$/, '')}: #{rpp_file}
	ruby-1.7 rubypp.rb $< $@
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

