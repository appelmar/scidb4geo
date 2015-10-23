#
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

set(SciDB_KNOWN_VERSIONS ${SciDB_ADDITIONAL_VERSIONS} "14.12" "14.08")

if (DEFINED ENV{SCIDB_INSTALL_PATH} OR DEFINED SciDB_ROOT)
  set (SciDB_ROOT $ENV{SCIDB_INSTALL_PATH})
else (DEFINED ENV{SCIDB_INSTALL_PATH} OR DEFINED SciDB_ROOT)
  foreach (v ${SciDB_KNOWN_VERSIONS} )
    if (EXISTS "/opt/scidb/${v}")
      set(SciDB_ROOT "/opt/scidb/${v}" )
      break()
    endif (EXISTS "/opt/scidb/${v}")
  endforeach()
endif (DEFINED ENV{SCIDB_INSTALL_PATH} OR DEFINED SciDB_ROOT)



find_path(SciDB_INCLUDE_DIR 
  NAMES SciDBAPI.h 
  PATHS
    /usr /usr/local ${SciDB_ROOT}
  PATH_SUFFIXES
    include
)   
set(SciDB_INCLUDE_DIRS ${SciDB_INCLUDE_DIR})	   
find_library(SciDB_LIBRARY NAMES scidbclient
  PATHS  
    /usr /usr/local ${SciDB_ROOT}
  PATH_SUFFIXES
    lib
)   

set(SciDB_LIBRARIES ${SciDB_LIBRARY})	 
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SciDB DEFAULT_MSG
                                  SciDB_LIBRARY SciDB_INCLUDE_DIR)
								  
set(SciDB_FOUND  ${SCIDB_FOUND})
mark_as_advanced(SciDB_INCLUDE_DIR SciDB_LIBRARY )