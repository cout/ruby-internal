spec = Gem::Specification.new do |s|
  s.name = 'ruby-internal'
  s.version = '0.5.0'
  s.summary = 'A library that provides acccess to the ' + \
              'internals of the ruby interpreter'
  s.homepage = 'http://rubystuff.org/nodewrap/'
  s.rubyforge_project = 'nodewrap'
  s.author = 'Paul Brannan'
  s.email = 'curlypaul924@gmail.com'

  s.description = <<-END
Nodewrap is Ruby module that provides direct access to Ruby's internal
data structures.i
END

  patterns = [
    'lib/*.rb',
    'ext/*.c',
    'ext/*.h',
    'ext/*.rpp',
    'ext/*.rb',
    'ext/*.yaml',
    'ext/MANIFEST',
    'ext/cached/ruby-*/*.c',
    'ext/cached/ruby-*/*.c',
    'ext/cached/ruby-*/*.h',
    'ext/cached/ruby-*/COPYING',
    'ext/cached/ruby-*/GPL',
    'ext/cached/ruby-*/LEGAL',
    'ext/cached/ruby-*/LGPL',
    'ext/cached/ruby-*/README',
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
  ]
  s.files = patterns.collect { |p| Dir.glob(p) }.flatten

  s.test_files = Dir.glob('test/test_*.rb')

  s.extensions = 'ext/extconf.rb'

  s.extra_rdoc_files = [ 'README' ]
end

