require 'nodewrap'

class Foo
  def foo
    puts 'foo!'
    p self
  end

  def self.foo
    puts 'self.foo'
  end

  FOO = 1

  class << self
    def foo
      puts "singleton foo"
    end

    FOO = 2
  end
end

d = Marshal.dump(Foo)
p d

Foo2 = Marshal.load(d)
p Foo2

class Foo2
  p FOO

  class << self
    p FOO
  end
end

Foo2.foo
f = Foo2.new
f.foo

