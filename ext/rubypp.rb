class Preprocessor
  def initialize(input, output, filename)
    @input = input
    @output = output
    @filename = filename
    @linenum = 1
  end

  def getline
    line = @input.gets
    @linenum += 1 if not line.nil?
    return line
  end

  def preprocess
    success = false
    begin
      loop do
        line = getline
        break if line.nil?
        case line
        when /(.*[^\\]|^)\#\{(.*?)\}(.*)/
          puts "#{$1}#{evaluate($2, @linenum)}#{$3}"
        when /^\#ruby\s+<<(.*)/
          marker = $1
          str = ''
          evalstart = @linenum
          loop do
            line = getline
            if line.nil? then
              raise "End of input without #{marker}"
            end
            break if line.chomp == marker
            str << line
          end
          result = evaluate(str, evalstart)
          puts result if not result.nil?
        when /^\#ruby\s+(.*)/
          result = evaluate($1, @linenum)
          puts result if not result.nil?
        else
          puts line
        end
      end
      success = true
    ensure
      if not success then
        $stderr.puts "Error on line #{@linenum}:"
      end
    end
  end

  def evaluate(str, linenum)
    result = eval(str, TOPLEVEL_BINDING, @filename, linenum).to_s
    success = true
    return result
  end

  def puts(line)
    @output.puts(line)
  end
end

def puts(line)
  $preprocessor.puts(line)
end

if __FILE__ == $0 then
  input_file = ARGV[0]
  output_file = ARGV[1]
  File.open(input_file, 'r') do |input|
    sucess = false
    begin
      File.open(output_file, 'w') do |output|
        $preprocessor = Preprocessor.new(input, output, input_file)
        $preprocessor.preprocess()
      end
      success = true
    ensure
      if not success then
        File.unlink(output_file)
      end
    end
  end
end

