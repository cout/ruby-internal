# This is a script similar to the Pragmatic Programmers' NodeDump.so.
# You can use it similarly, e.g.:
#
#   $ cat test.rb
#   def foo
#     1+1
#   end
#
#   $ ruby -rnwdump test.rb
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
#     +-noex = 2

require 'nodepp'

set_trace_func proc {
  n = $ruby_eval_tree
  if $ruby_eval_tree_begin then
    $ruby_eval_tree_begin.pretty_print($stderr)
  end
  if n then
    n.pretty_print($stderr)
  end
  exit if n
}

