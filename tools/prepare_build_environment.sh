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

# This files installs dependencies from binary packages for Ubuntu 14.04, SciDB 14.12


echo -e "This script installs dependencies for building the scidb4geo plugin without compiling SciDB from scratch. Prebuilt binaries of dependent packages will be downloaded and installed. This might take sime time. \n"
read -p "Are you sure you want to continue now? Type y or n: " -n 1 -r REPLY
echo    # (optional) move to a new line
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    exit 1
fi

wget -O- https://downloads.paradigm4.com/key | sudo apt-key add -
touch /etc/apt/sources.list.d/scidb.list
echo "deb https://downloads.paradigm4.com/ ubuntu14.04/14.12/" >> /etc/apt/sources.list.d/scidb.list
echo "deb-src https://downloads.paradigm4.com/ ubuntu14.04/14.12/" >> /etc/apt/sources.list.d/scidb.list
apt-get update
apt-cache search scidb
apt-get install --no-install-recommends --fix-missing -y --force-yes scidb-14.12-dev scidb-14.12-libcsv scidb-14.12-libmpich2-dev scidb-14.12-libboost1.54-all-dev scidb-14.12-libboost1.54-dev liblog4cxx10-dev libpqxx-dev cmake

: ${SCIDB_ROOT:=/opt/scidb/14.12} # set default root directory for SciDB headers, libraries, 3rd party dependencies

mkdir -p ${SCIDB_ROOT}/extern/MurmurHash 2>&1> /dev/null
wget -P ${SCIDB_ROOT}/extern/MurmurHash/ https://raw.githubusercontent.com/parkerabercrombie/SciDB-GDAL/master/extern/MurmurHash/MurmurHash3.h

