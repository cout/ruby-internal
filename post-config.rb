if not @config['ruby-source-path'] then
  $stderr.puts "ERROR: Source code for the ruby interpreter could not be found (perhaps you forgot --ruby-source-path?)"
  exit 1
end

path=@config['ruby-source-path']
ruby_h=File.join(path, 'ruby.h')
if not File.exist?(path) or
   not File.directory?(path) or
   not File.exist?(ruby_h) then
   $stderr.puts "ERROR: Could not find the ruby source code at #{path}"
   exit 1
end

dive_into('ext') do
  command('make clean')
end

