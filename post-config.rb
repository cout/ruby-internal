if not @config['ruby-source-path'] then
  $stderr.puts "Source code for the ruby interpreter could not be found (perhaps you forgot --ruby-source-path?)"
  exit 1
end

dive_into('ext') do
  command('make clean')
end

