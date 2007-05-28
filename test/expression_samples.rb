require 'rbconfig'

class TEST_CLASS
  FOO = 10
end

EXPRESSION_SAMPLES = {
  :lit                     => '42',
  :vcall                   => 'foo',
  :fcall                   => 'foo()',
  :fcall_many              => 'foo(1, 2, 3)',
  :fcall_many2             => 'foo 1, 2, 3',
  :fcall_splat             => 'a = [2, 3]; foo(1, *a)',
  :fcall_block             => 'foo() { nil }',
  :fcall_block_with_args   => 'foo() { |x,y| [x, y] }',
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
  :if2                     => '42 if true',
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
  :cdecl                   => 'remove_foo; FOO = 1',
  :cvdecl                  => '@@foo = 1',
  # TODO: cvasgn
  :attrasgn                => 'h = {}; h.default = true; h',
  :const                   => 'FOO',
  :colon2                  => 'self::FOO',
  :colon3                  => '::TEST_CLASS::FOO',
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
  # TODO dregx with next set
  # TODO: dxstr
  # TODO: evstr
  :iter                    => 'loop { break }',
  :iter_x                  => 'loop { break 10 }',
  :while_1                 => 'while true; break; end',
  :while_1_x               => 'while true; break 7; end',
  :while_1_do              => 'while true do; break; end',
  :while_1_do_x            => 'while true do; break 7; end',
  :while_0                 => 'begin; break; end while true',
  :while_0_x               => 'begin; break 7; end while true',
  :until_1                 => 'until false; break; end',
  :until_1_x               => 'until false; break 7; end',
  :until_1_do              => 'until false do; break; end',
  :until_1_do_x            => 'until false do; break 7; end',
  :until_0                 => 'begin; break; end until false',
  :until_0_x               => 'begin; break 7; end until false',
  :break                   => 'break',
  :break_x                 => 'break 49',
  :catch                   => 'catch(:foo) { throw :foo; 42 }',
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


major = Config::CONFIG['MAJOR'].to_i
minor = Config::CONFIG['MINOR'].to_i
teeny = Config::CONFIG['TEENY'].to_i
ruby_version_code = major * 100 + minor * 10 + teeny

if ruby_version_code < 170 then
  EXPRESSION_SAMPLES.delete(:break_x)
  EXPRESSION_SAMPLES.delete(:iter_x)
  EXPRESSION_SAMPLES.delete(:while_1_x)
  EXPRESSION_SAMPLES.delete(:while_1_do_x)
  EXPRESSION_SAMPLES.delete(:while_0_x)
  EXPRESSION_SAMPLES.delete(:until_1_x)
  EXPRESSION_SAMPLES.delete(:until_1_do_x)
  EXPRESSION_SAMPLES.delete(:until_0_x)
end

