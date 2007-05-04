#!/bin/sh

dir="$1"

for f in "$dir"/ruby-?.?.?; do
  ruby generate_cached.rb "$f"
done

