spec = Gem::Specification.new do |s|
  s.name = 'nodewrap'
  s.version = '0.5.0'
  s.summary = 'A library that provides acccess to ruby\'s ' + \
              'internal node tree, among other things'
  s.homepage = 'http://rubystuff.org/nodewrap/'
  s.rubyforge_project = 'nodewrap'
  s.author = 'Paul Brannan'
  s.email = 'curlypaul924@gmail.com'

  s.description = <<-END
Nodewrap is Ruby module that provides direct access to Ruby's internal
node structure. Originally this started as a proof-of-concept to allow
Node objects to be dumped and loaded using Ruby's builtin marshalling
mechanism. Methods to dump and load classes and modules are were also
added, and with a little work, nodewrap can be used to dump entire class
hierarchies from one Ruby process and load them into another.
  END

  patterns = [
    'lib/*.rb',
    'ext/*.c',
    'ext/*.h',
    'ext/*.rpp',
    'ext/*.rb',
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
    'test/*.rb',
    '*.rb',
    'COPYING',
    'LEGAL',
    'LGPL',
    'LICENSE',
    'README',
    'TODO',
    'metaconfig',
    'ruby.supp',
  ]
  s.files = patterns.collect { |p| Dir.glob(p) }.flatten

  s.test_files = Dir.glob('test/*.rb')

  s.extensions = 'ext/extconf.rb'

  s.extra_rdoc_files = [ 'README' ]

  spec.autorequire = 'nodewrap'
end

