ruby_source_path = @config['ruby-source-path']
ruby_include_path = File.join(ruby_source_path, 'include', 'ruby')
puts "Looking for #{ruby_include_path}"
if not File.exist?(ruby_include_path) or \
   not File.directory?(ruby_include_path) then
  # pre-YARV
  puts "Could not find #{ruby_include_path}"
  ruby_include_path = ruby_source_path
end

@config['ruby-include-path'] = ruby_include_path

@options['config-opt'] << "--ruby-source-path=#{@config['ruby-source-path']}"
@options['config-opt'] << "--ruby-include-path=#{@config['ruby-include-path']}"
