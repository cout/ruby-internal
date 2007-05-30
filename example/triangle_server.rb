require 'nodewrap'

module M
  class Triangle
    def initialize(a, b, c)
      @a = a
      @b = b
      @c = c
    end

    def area
      s = (@a + @b + @c) / 2.0
      return Math.sqrt(s*(s-@a)*(s-@b)*(s-@c))
    end

    def to_s
      return "a #{@a}-#{@b}-#{@c} triangle"
    end
  end
end

Marshal.dump(M, $stdout)
$stdout.flush

