require 'rbconfig'

class File_Output
  def initialize(file)
    @file = file
    @indent = 0
  end

  def indent
    @indent += 1
    begin
      yield
    ensure
      @indent -= 1
    end
  end

  def c_block(&block)
    puts '{'
    begin
      indent(&block)
    ensure
      puts '}'
    end
  end

  def puts(line)
    @file.puts "#{'  ' * @indent}#{line}"
  end

  def _()
    @file.puts
  end
end

File.open(hfile_name, 'w') do |outfile|
  f = File_Output.new(outfile)
  f.puts '#ifndef nodeinfo_h'
  f.puts '#define nodeinfo_h'
  f._
  f.puts '#include "ruby.h"'
  f.puts '#include "node.h"'
  f.puts '#include "nodewrap.h"'
  f._
  f.puts 'enum Node_Elem_Name'
  f.puts '{'
  f.indent do
    f.puts "NEN_NONE,"
    nodeinfo.sort.each do |node_elem_name, node_elem_ref|
      f.puts "NEN_#{node_elem_name.upcase},"
    end
    f.puts "NEN_COUNT_"
  end
  f.puts '};'
  f._
  f.puts 'VALUE dump_node_elem(enum Node_Elem_Name nen, NODE * n, VALUE node_hash);'
  f.puts 'void load_node_elem(enum Node_Elem_Name nen, VALUE v, NODE * n, VALUE node_hash, VALUE id_hash);'
  f._
  f.puts '#endif'
end

File.open(cfile_name, 'w') do |outfile|
  f = File_Output.new(outfile)

  f.puts "#include \"#{hfile_name}\""
  f.puts ''

  # -------------------------------------------------------------------
  # dump_node_elem
  f.puts 'VALUE dump_node_elem(enum Node_Elem_Name nen, NODE * n, VALUE node_hash)'
  f.c_block do
    f.puts 'switch(nen)'
    f.c_block do
      f.puts 'case NEN_NONE:'
      f.puts '  return Qnil;'
      nodeinfo.sort.each do |node_elem_name, node_elem_ref|
        f.puts "case NEN_#{node_elem_name.upcase}:"
        f.indent do
          case node_elem_name
          when 'rval'
            f.puts('return dump_node_elem(NEN_HEAD, n, node_hash);')
          else
            case node_elem_ref
            when /\.node$/
              f.puts "if(n->nd_#{node_elem_name})"
              f.c_block do
                f.puts "dump_node_to_hash(n->nd_#{node_elem_name}, node_hash);"
                f.puts "return node_id(n->nd_#{node_elem_name});"
              end
              f.puts 'else'
              f.c_block do
                f.puts 'return Qnil;'
              end
            when /\.id$/
              f.puts "return ID2SYM(n->nd_#{node_elem_name});"
            when /\.value$/
              f.puts "return n->nd_#{node_elem_name};"
            when /\.(argc|state|cnt)$/
              f.puts "return INT2NUM(n->nd_#{node_elem_name});"
            when /\.(tbl)$/
              f.puts "if(n->nd_#{node_elem_name})"
              f.c_block do
                f.puts 'size_t j;'
                f.puts 'VALUE arr = rb_ary_new();'
                f.puts "for(j = 1; j < n->nd_#{node_elem_name}[0] + 1; ++j)"
                f.c_block do
                  f.puts "rb_ary_push(arr, ID2SYM(n->nd_#{node_elem_name}[j]));"
                end
                f.puts 'return arr;'
              end
              f.puts 'else'
              f.c_block do
                f.puts 'return Qnil;'
              end
            when /\.(cfunc)$/,
                 /\.(argc|state|cnt)$/,
                 /\.(entry)$/
              f.puts "rb_raise(rb_eArgError, \"Cannot dump #{$1}\");"
            end
          end
        end
      end
    end
    f.puts 'rb_raise(rb_eArgError, "Invalid Node_Elem_Name %d", nen);'
  end
  f._

  # -------------------------------------------------------------------
  # load_node_elem
  f.puts 'void load_node_elem(enum Node_Elem_Name nen, VALUE v, NODE * n, VALUE node_hash, VALUE id_hash)'
  f.c_block do
    f.puts 'switch(nen)'
    f.c_block do
      f.puts 'case NEN_NONE:'
      f.puts '  return;'
      nodeinfo.sort.each do |node_elem_name, node_elem_ref|
        outfile.puts "    case NEN_#{node_elem_name.upcase}:"
        f.indent do
          case node_elem_name
          when 'rval'
            f.puts('return load_node_elem(NEN_HEAD, v, n, node_hash, id_hash);')
          else
            case node_elem_ref
            when /\.node$/
              f.c_block do
                f.puts 'VALUE nid = rb_hash_aref(id_hash, v);'
                f.puts 'if(RTEST(nid))'
                f.c_block do
                  f.puts "n->nd_#{node_elem_name} = id_to_node(nid);"
                end
                f.puts 'else if(v == Qnil)'
                f.c_block do
                  f.puts "n->nd_#{node_elem_name} = 0;"
                end
                f.puts 'else'
                f.c_block do
                  f.puts "NODE * new_node = NEW_NIL();"
                  f.puts "load_node_from_hash(new_node, v, node_hash, id_hash);"
                  f.puts "n->nd_#{node_elem_name} = new_node;"
                end
              end
              f.puts 'return;'
            when /\.id$/
              f.puts "n->nd_#{node_elem_name} = SYM2ID(v);"
              f.puts 'return;'
            when /\.value$/
              f.puts "n->nd_#{node_elem_name} = v;"
              f.puts 'return;'
            when /\.(argc|state|cnt)$/
              f.puts "n->nd_#{node_elem_name} = NUM2INT(v);"
              f.puts 'return;'
            when /\.(tbl)$/
              f.puts 'if(v == Qnil)'
              f.c_block do
                f.puts "n->nd_#{node_elem_name} = 0;"
              end
              f.puts 'else'
              f.c_block do
                f.puts "Check_Type(v, T_ARRAY);"
                f.puts "size_t len = RARRAY(v)->len;"
                f.puts "ID * tmp_tbl = ALLOCA_N(ID, len);"
                f.puts "size_t j;"
                f.puts "for(j = 0; j < len; ++j)"
                f.c_block do
                  f.puts "tmp_tbl[j] = SYM2ID(RARRAY(v)->ptr[j]);"
                end
                f.puts "n->nd_#{node_elem_name} = ALLOC_N(ID, len);"
                f.puts "memcpy(n->nd_#{node_elem_name}, tmp_tbl, len);"
              end
            when /\.(cfunc)$/,
                 /\.(argc|state|cnt)$/,
                 /\.(entry)$/
              f.puts "rb_raise(rb_eRuntimeError, \"Cannot load #{$1}\");"
              f.puts 'return;'
            end
          end
        end
      end
    end
    f.puts 'rb_raise(rb_eRuntimeError, "Internal error: invalid Node_Elem_Name %d", nen);'
  end
  f._
end

