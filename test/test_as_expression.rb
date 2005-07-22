require 'test/unit'
require 'test/unit/ui/console/testrunner'
require 'nodewrap'
require 'as_expression'

dir = File.dirname(__FILE__)
require "#{dir}/node_samples"

$stdout.sync = true
$stderr.sync = true

class TC_As_Expression < Test::Unit::TestCase
  CODE_SAMPLES = {
    :lit                     => '42',
    :vcall                   => 'foo',
    :fcall                   => 'foo()',
    :fcall_many              => 'foo(1, 2, 3)',
    :fcall_many2             => 'foo 1, 2, 3',
    :plus                    => '1+1',
    :minus                   => '54-2',
    :times                   => '12*6',
    :div                     => '16/7',
    :less                    => '8 < 5',
    :less_eq                 => '8 <= 5',
    :greater                 => '8 > 5',
    :greater_eq              => '8 >= 5',
    :equal                   => '8 == 5',
    :threequal               => '8 === 5',
    :spaceship               => "8 <=\> 5", # the \> is for Align.vim
    :lshift                  => '8 << 5 ',
    :rshift                  => '8 >> 5',
    :bit_and                 => '8 & 5',
    :bit_or                  => '8 | 5',
    :bit_xor                 => '8 ^ 5',
    :mod                     => '8 % 5',
    :not_equal               => '8 != 5',
    # TODO: :match           => ''foo' =~ /foo|bar/',
    # TODO: :not_match       => ''foo' !~ /foo|bar/',
    :call                    => 'self.foo',
    # TODO: super
    # TODO: zsuper
    # TODO: redo
    # TODO: retry
    :sym_not                 => '!true',
    :sym_and                 => 'true && false',
    :sym_or                  => 'true || false',
    :lit_not                 => 'not true',
    :lit_and                 => 'true and false',
    :lit_or                  => 'true or false',
    :array                   => '[1, 2, 3]',
    :zarray                  => '[]',
    # TODO: block
    :hash                    => '{ 1 => 2, 2 => 3 }',
    :zhash                   => '{}',
    :ternary                 => 'true ? false : nil',
    :if                      => 'if true then false; else nil; end',
    :true                    => 'true',
    :false                   => 'false',
    :nil                     => 'nil',
    :self                    => 'self',
    :dot2                    => '5..8',
    :dot3                    => '5...8',
    :gvar                    => '$stdout',
    :ivar                    => '@foo',
    :cvar                    => '@@foo',
    :dvar                    => 'foo = 1; foo',
    :nth_ref                 => '$1',
    :back_ref                => '$`',
    :dasgn_curr              => 'foo = 1',
    # TODO: dasgn
    :iasgn                   => '@foo = 6',
    # TODO: lasgn
    # TODO: masgn
    :cdecl                   => 'TC_As_Expression.class_eval { remove_const(:FOO) }; FOO = 1',
    :cvdecl                  => '@@foo = 1',
    # TODO: cvasgn
    :attrasgn                => 'h = {}; h.default = true; h',
    :const                   => 'FOO',
    :colon2                  => 'TC_As_Expression::FOO',
    :colon3                  => '::TC_As_Expression::FOO',
    # TODO: lvar
    # TODO: newline
    :str                     => '"foo"',
    :regx                    => '/foo/',
    # TODO: :regx_cflag      => '/foo/i',
    :regx_once               => '/foo/o',
    # :regx_once_cflag       => '/foo/oi',
    # TODO: xstr
    :dstr                    => 'a = 1; b = 2; "#{a}#{b}"',
    :dregx                   => 'a = 1; b = 2; /#{a}#{b}/',
    # :dregx_cflag           => 'a = 1; b = 2; /#{a}#{b}/i',
    :dregx_once              => 'a = 1; b = 2; /#{a}#{b}/',
    # :dregx_once_cflag      => 'a = 1; b = 2; /#{a}#{b}/oi',
    # TODO: dxstr
    # TODO: evstr
    :iter                    => 'loop { break 10 }',
    :while_1                 => 'while true; break 7; end',
    :while_1_do              => 'while true do; break 7; end',
    :while_0                 => 'begin; break 7; end while true',
    :until_1                 => 'until false; break 7; end',
    :until_1_do              => 'until false do; break 7; end',
    :until_0                 => 'begin; break 7; end until false',
    :break                   => 'break 49',
    # TODO: yield
    :begin                   => 'begin; 42; end',
    :begin_empty             => 'begin; end',
    :begin_ensure            => 'begin; 7; ensure; 42; end',
    :begin_ensure_empty_head => 'begin; ensure; 42; end',
    :begin_ensure_empty_body => 'begin; ensure; end',
    :rescue                  => 'raise "foo" rescue 7',
    :rescue2                 => 'raise "foo"; 42 rescue 7',
    :begin_rescue            => 'begin; raise "foo"; rescue; 42; end',
    :begin_rescue_multi      => 'begin; raise "foo"; 6; rescue; 42; end',
    :begin_rescue_empty_head => 'begin; rescue; end',
    :begin_rescue_empty_resq => 'begin; 42; rescue; end',
    :resbody                 => 'begin; raise "foo"; rescue; 42; end',
    :case_when               => 'case 42; when TrueClass then 6; when Fixnum then 42; end',
    :case_when_multi_arg     => 'case 42; when TrueClass, Fixnum then 42; end',
    :case_when_empty_body    => 'case 42; when TrueClass; when Fixnum then 42; end',
    # TODO: alias
    # TODO: valias
    # TODO: undef
    # TODO: class
    # TODO: sclass
    # TODO: scope
    # TODO: defn
    # TODO: defs
    :defined                 => 'defined?(NilClass)',
    # TODO: match3
  }

  CODE_SAMPLES.each do |name, code|
    p = proc {
      p_orig = eval("proc { #{code} }")
      code_new = p_orig.body.as_expression
      p_new = eval("proc { #{code_new} }")
      result_orig = result_new = nil
      exc_orig = exc_new = nil
      begin; result_orig = p_orig.call; rescue; exc_orig = $!; end
      begin; result_new = p_new.call; rescue; exc_new = $!; end
      assert_equal(result_orig, result_new)
      assert_equal(exc_orig.class, exc_new.class)
      if exc_orig and exc_new then
        assert_equal(exc_orig.message, exc_new.message)
      end
    }
    define_method "test_#{name}", p
  end

  def initialize(test_method_name)
    # TODO: This seems to be the only way to get tests defined with #
    # define_method to run on 1.8.1 and earlier.
    @method_name = test_method_name
    @test_passed = true
  end

  def setup
    @foo = 42
  end

  def foo(*args)
    return args
  end

  @@foo = 10

  FOO = 57
end

