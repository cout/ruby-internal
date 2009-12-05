require 'generate_rdoc'

generate_rdoc
system("scp -r doc/* cout@rubyforge.org:/var/www/gforge-projects/ruby-internal")
