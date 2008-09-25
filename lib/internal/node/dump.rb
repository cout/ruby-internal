# This is a script similar to the Pragmatic Programmers' NodeDump.so.
# You can use it similarly, e.g.:
#
#   $ cat test.rb
#   def foo
#     1+1
#   end
#
#   $ ruby -rinternal/node/dump test.rb
#   NODE_NEWLINE at test.rb:1
#   |-nth = 1
#   +-next = NODE_DEFN at test.rb:1
#     |-defn = NODE_SCOPE at test.rb:3
#     | |-rval = false
#     | |-tbl = nil
#     | +-next = NODE_BLOCK at test.rb:1
#     |   |-next = NODE_BLOCK at test.rb:3
#     |   | |-next = false
#     |   | +-head = NODE_NEWLINE at test.rb:2
#     |   |   |-nth = 2
#     |   |   +-next = NODE_CALL at test.rb:2
#     |   |     |-recv = NODE_LIT at test.rb:2
#     |   |     | +-lit = 1
#     |   |     |-args = NODE_ARRAY at test.rb:2
#     |   |     | |-alen = 1
#     |   |     | |-head = NODE_LIT at test.rb:2
#     |   |     | | +-lit = 1
#     |   |     | +-next = false
#     |   |     +-mid = :+
#     |   +-head = NODE_ARGS at test.rb:1
#     |     |-cnt = 0
#     |     |-rest = -1
#     |     +-opt = false
#     |-mid = :foo
#     +-noex = PRIVATE

require 'internal/node/pp'

set_trace_func proc {
  begin
    n = $ruby_eval_tree
    if defined?($ruby_eval_tree_begin) and $ruby_eval_tree_begin then
      pp $ruby_eval_tree_begin, $stderr
    end
    if n then
      PP.pp n, $stderr
      exit!
    end
  rescue Exception
    p $!, $!.backtrace
  end
}

