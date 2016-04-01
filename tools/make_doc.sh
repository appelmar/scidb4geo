#!/bin/bash
# This file sets up the platform for automated documentation builds with travis ci.

# 1. install dependencies (pandoc)
wget https://github.com/jgm/pandoc/releases/download/1.17.0.2/pandoc-1.17.0.2-1-amd64.deb
yes | sudo gdebi pandoc-1.17.0.2-1-amd64.deb

# 2. Make documentation 
rm -rf doc/pub
mkdir -p doc/pub
mkdir -p doc/pub/operators


MDFILES=$( find doc/operators -iname "*.md" -type f )
for f in $MDFILES
do
  pandoc $f --to html --from markdown+autolink_bare_uris+ascii_identifiers+tex_math_single_backslash-implicit_figures  --output ${f/.md/.html} --smart --mathjax --email-obfuscation none --self-contained --standalone --section-divs
done
  
# copy public doc to gh-pages folder
cp doc/operators/*.html doc/pub/operators/
 
cd ../doc/pub
git init
git config user.name "Travis CI Documentation Builder"
git config user.email "marius.appel@uni-muenster.de"
git add .
git commit -m "Automated documentation build[ci skip]"
git push --force --quiet "https://${GH_TOKEN}@${GH_REF}" master:gh-pages



