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


# Script for installing scidb4geo plugin from binary. Must be started from the directory of the script.
# Pars:
# - $1 scidbconfig file for extracting postgres connection information, plugins dir, etc.

if [ -z "$1" ]; then
  echo -e "ERROR: Path to SciDB configuration file not provided\nUsage: setup config_file\n";
  exit;
fi


echo -e "-----------------"
echo -e "Installation script for installing scidb4geo plugin from binaries. This script must run under scidb user."
echo -e "-----------------"



SCIDB_CONFIG=$1
if [ ! -f $SCIDB_CONFIG ]; then
    echo -e "\nWARNING: SciDB config file '${SCIDB_CONFIG}' not found. "
    : ${SCIDB_INSTALL_PATH:=/opt/scidb/14.12}
    if [ ! -f ${SCIDB_INSTALL_PATH}/etc/config.ini ]; then
      exit;
    else SCIDB_CONFIG=${SCIDB_INSTALL_PATH}/etc/config.ini
    fi
    echo "Using '${SCIDB_CONFIG}' instead. "
fi


echo -e "\nInstallation removes any previous installation including  existing geographic reference metadata."
read -p "Are you sure you want to continue now? Type y or n: " -n 1 -r REPLY
echo    # (optional) move to a new line
if [[ ! $REPLY =~ ^[Yy]$ ]]
then
    exit 1
fi



echo -e "Create installation directory under ${HOME}/.scidb4geo ..."
mkdir ${HOME}/.scidb4geo &> /dev/null # create directory for configuration files
echo -e "Copying configuration and macro..."
cp -f ${SCIDB_CONFIG} ${HOME}/.scidb4geo/config.ini # copy scidb configuration file
cp -f scidb4geo_macro.afl ${HOME}/.scidb4geo/scidb4geo_macro.afl # copy macro

echo -e "Extracting SciDB configuration parameters ..."
# Extract database connection information out of config file # TODO: extract dbname...
dbname=$(grep -Po '(?<=\[).*?(?=\])'   $SCIDB_CONFIG)
dbuser=$(awk -F "=" '/db_user/ {print $2}' $SCIDB_CONFIG)
dbpasswd=$(awk -F "=" '/db_passwd/ {print $2}' $SCIDB_CONFIG)
pluginsdir=$(awk -F "=" '/pluginsdir/ {print $2}' $SCIDB_CONFIG)


# Loading hstore extension
echo -e "Checking Postgres version and try to load hstore extension..."
PGVERSION="$(PGPASSWORD=${dbpasswd} psql -h localhost -p 5432 -U ${dbuser} -d ${dbname} -t -c 'SHOW server_version_num')" 
if [ "$PGVERSION" -gt "90099" ]; # create extension support as of 9.1
then
  PGPASSWORD=${dbpasswd} psql -h localhost -p 5432 -U ${dbuser} -d ${dbname} -c "create extension if not exists hstore" 1> /dev/null
elif [ "$PGVERSION" -lt "80500" ] && [ "$PGVERSION" -gt "80399" ]; # Postgres 8.4
then
  echo -e "WARNING: To use the hstore extension for storing additional earth-observation metadata, please make sure that the extension is installed and loaded in the system catalog. For that, run /home/share/postgres/8.4/contrib/hstore.sql as a database superuser and rerun this script if table scidb4geo_array_gdalmd cannot be created."
else 
  echo -e "ERROR: Postgres version ${PGVERSION} not supported."
  exit
fi


# Uninstall previous installation
echo -e "Removing previous installations..."
PGPASSWORD=${dbpasswd} psql -h localhost -p 5432 -U ${dbuser} -d ${dbname} -f dbuninstall.sql 1> /dev/null

# Create system catalog tables
echo -e "Creating schemas..."
PGPASSWORD=${dbpasswd} psql -h localhost -p 5432 -U ${dbuser} -d ${dbname} -f dbinstall.sql 1> /dev/null 

# Fill spatial_ref_sys table (data can be used from PostGIS)
echo -e "Inserting EPSG data..."
PGPASSWORD=${dbpasswd} psql -h localhost -p 5432 -U ${dbuser} -d ${dbname} -f spatial_ref_sys.sql 1> /dev/null

# For binary distribution, copy shared library to pluginsdir
if [ -f libscidb4geo.so ]; then
     echo -e "Copying plugin binary to ${pluginsdir}..."
     cp libscidb4geo.so ${pluginsdir}/
else 
     echo -e "No binary libscidb4geo.so found. You need to copy it to the SciDB plugin directory manually!"
fi

echo -e "\n...DONE. Please now restart SciDB and run load_library('scidb4geo')."