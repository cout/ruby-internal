require 'rbconfig'

major = Config::CONFIG['MAJOR'].to_i
minor = Config::CONFIG['MINOR'].to_i
teeny = Config::CONFIG['TEENY'].to_i
RUBY_VERSION_CODE = major * 100 + minor * 10 + teeny

