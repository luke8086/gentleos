#!/bin/sh

# Remove comments from PBM files and convert to ASCII

set -e

for file in "$@"; do
    perl -ni -e 'print unless /^#/' "$file"
    magick "$file" -compress none "$file"
done
