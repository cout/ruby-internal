spec = Gem::Specification.new do |s|
  s.name = 'ruby-internal'
  s.version = '0.8.1'
  s.summary = 'A library that provides acccess to the ' + \
              'internals of the ruby interpreter'
  s.homepage = 'http://rubystuff.org/nodewrap/'
  s.rubyforge_project = 'nodewrap'
  s.author = 'Paul Brannan'
  s.email = 'curlypaul924@gmail.com'

  s.add_dependency 'rubypp', '>= 0.0.1'

  s.description = <<-END
Ruby-Internal is Ruby module that provides direct access to Ruby's
internal data structures.
END

  patterns = [
    'lib/**/*.rb',
    'ext/mkmf-ruby-internal.rb',
    'ext/ruby_source_dir.rb',
    'ext/internal/**/*.rpp',
    'ext/internal/**/*.c',
    'ext/internal/**/*.h',
    'ext/internal/**/*.rb',
    'ext/cached/ruby-*/**/*',
    'example/*.rb',
    'example/README',
    'test/*.rb',
    '*.rb',
    'COPYING',
    'LEGAL',
    'LGPL',
    'LICENSE',
    'README',
    'TODO',
    'metaconfig',
    'setup.rb',
    'Rakefile',
    'bin/ruby-internal-node-dump',
    'bin/ruby-internal-obfuscate',
    'sample/dump_class.rb',
    'sample/irbrc',
  ]
  s.files = patterns.collect { |p| Dir.glob(p) }.flatten

  s.executables = [
    'ruby-internal-node-dump',
    'ruby-internal-obfuscate',
  ]

  s.extensions = 'Rakefile'
  s.test_files = Dir.glob('test/test_*.rb')

  s.extra_rdoc_files = [ 'README' ]

  s.require_paths << 'ext'
end

