#!/bin/sh
RUBYLIB=$RUBYLIB:..
ruby server.rb | ruby client.rb

