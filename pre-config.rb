begin
  require 'fileutils'
rescue LoadError
  require 'ftools'
  FileUtils = File
end

FileUtils.rm_f 'ext/Makefile'

ruby_source_path = config('ruby-source-path')
ruby_include_path = config('ruby-include-path')

if ruby_source_path != '' then
  if ruby_include_path == '' then
    ruby_include_path = File.join(ruby_source_path, 'include', 'ruby')
    if not File.exist?(ruby_include_path) or \
       not File.directory?(ruby_include_path) then
      # pre-YARV
      ruby_include_path = ruby_source_path
    end

    set_config('ruby-include-path', ruby_include_path)
  end

  @config.config_opt << "--ruby-source-path=#{@config['ruby-source-path']}"
  @config.config_opt << "--ruby-include-path=#{@config['ruby-include-path']}"
end

