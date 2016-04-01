#!/bin/bash

# scidb4geo - A SciDB plugin for managing spatially referenced arrays
# Copyright (C) 2016 Marius Appel <marius.appel@uni-muenster.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# -----------------------------------------------------------------------------


# This file makes HTML documentation files and pushes changes to gh-pages branch


# 1. clean up
rm -rf doc/pub
mkdir -p doc/pub
mkdir -p doc/pub/operators

# 2. call pandoc for all Markdown files
MDFILES=$( find doc/operators -iname "*.md" -type f )
for f in $MDFILES
do
  pandoc $f --to html --from markdown+autolink_bare_uris+ascii_identifiers+tex_math_single_backslash-implicit_figures  --output ${f/.md/.html} --smart --mathjax --email-obfuscation none --self-contained --standalone --section-divs
done
  
# 3. copy public doc to local gh-pages folder
cp doc/operators/*.html doc/pub/operators/
cd doc/pub

# 4. make this folder as the root of gh-pages branch
git init
git config user.name "Travis CI Documentation Builder"
git config user.email "marius.appel@uni-muenster.de"
git add .
git add operators
git commit -m "Automated documentation build[ci skip]"
git push --force --quiet "https://${GH_TOKEN}@${GH_REF}" master:gh-pages



