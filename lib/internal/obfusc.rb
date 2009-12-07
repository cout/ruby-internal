# A code obfuscator for Ruby.  It works by getting a dump of the node
# tree, then wrapping it with a lightweight loader.
#
# NOTE: This code is for demonstration purposes only!  It has not been
# extensively tested, and I cannot guarantee that it works in all cases
# (I don't even know what the corner cases are).  If you wish to use
# this in a commercial product, please verify that the code is correctly
# loaded.
#
# Usage:
#
#   $ cat test.rb
#   def foo
#     1+1
#   end
#   
#   puts foo()
#
#   $ ruby -rinternal/obfusc test.rb > test2.rb
#   $ ruby test2.rb
#   2

require 'internal/node/obfusc'

begin_nodes = []
set_trace_func proc {
  n = $ruby_eval_tree
  if defined?($ruby_eval_tree_begin) and $ruby_eval_tree_begin then
    n.pretty_print($stderr)
    begin_nodes << $ruby_eval_tree_begin
  end
  if n then
    # TODO: Do I need to include ruby_dyna_vars?
    str = n.obfusc(begin_nodes)
    puts str
    exit
  end
}

