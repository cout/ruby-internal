require 'nodewrap'

class Object
  def classtree(s = '', prefix = '', obj=self, graphed={})
    # if Module == obj.class.class then
    #   s << "included module "
    # elsif obj.singleton? then
    #   s << "singleton class "
    # elsif Class === obj then
    #   s << "class "
    # end
    s << "#{obj}"
    if graphed[obj] then
      s << " (*)\n"
      return s
    end
    s << "\n"
    graphed[obj] = true
    return if Kernel == obj
    subtree = (Module === obj) && (obj.real_superclass)
    s << "#{prefix}#{subtree ? '|-' : '+-'}class = "
    classtree(s, prefix + (subtree ? '| ' : '  '), obj.real_class, graphed)
    if subtree then
      s << "#{prefix}+-super = "
      classtree(s, prefix + '  ', obj.real_superclass, graphed)
    end
    return s
  end
end

if __FILE__ == $0 then
  module M
  end

  class Foo
    class << self; end
    include M
  end

  class Bar < Foo
  end

  class Baz
    class << self; @foo = 1; end
    def self.foo; end
  end

  b = Bar.new
  b.classtree($stderr)

  b = Baz.new
  b.classtree($stderr)
end

