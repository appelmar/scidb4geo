#!/bin/bash

# scidb4geo - A SciDB plugin for managing spatially referenced arrays
# Copyright (C) 2015 Marius Appel <marius.appel@uni-muenster.de>
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


# Script for deployment of scidb4geo plugin on development environment
#${SCIDB_INSTALL_PATH:="/home/scidb/scidbtrunk/stage/install"}
cd /home/scidb/scidbtrunk
./run.py stop
cp /home/scidb/scidbtrunk/build/bin/plugins/libscidb4geo.so /home/scidb/scidbtrunk/stage/install/lib/scidb/plugins/
cp /home/scidb/scidbtrunk/examples/scidb4geo/install/scidb4geo_macro.afl ~/.scidb4geo/scidb4geo_macro.afl # copy macro
./run.py start
iquery -aq "load_library('scidb4geo');"
iquery -aq "load_module('/home/scidb/.scidb4geo/scidb4geo_macro.afl');"
