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

