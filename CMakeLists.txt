
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



set (scidb4geo_VERSION_MAJOR 0)
set (scidb4geo_VERSION_MINOR 1)

configure_file( ${CMAKE_CURRENT_LIST_DIR}/src/plugin.h.in ${CMAKE_CURRENT_LIST_DIR}/src/plugin.h )


# Make sure that logical operators are always first!

set (scidb4geo_src
    src/plugin.cxx
    
    src/eo_all/LogicalAll.cxx
    src/eo_all/PhysicalAll.cxx
    
    src/eo_setsrs/LogicalSetSRS.cxx
    src/eo_setsrs/PhysicalSetSRS.cxx
    
    src/eo_getsrs/LogicalGetSRS.cxx
    src/eo_getsrs/PhysicalGetSRS.cxx
    
    src/eo_settrs/LogicalSetTRS.cxx
    src/eo_settrs/PhysicalSetTRS.cxx
    
    
    src/eo_gettrs/LogicalGetTRS.cxx
    src/eo_gettrs/PhysicalGetTRS.cxx
    
    src/eo_arrays/LogicalArrays.cxx
    src/eo_arrays/PhysicalArrays.cxx
    
    src/eo_regnewsrs/LogicalRegNewSRS.cxx
    src/eo_regnewsrs/PhysicalRegNewSRS.cxx
    
    src/eo_extent/LogicalExtent.cxx
    src/eo_extent/PhysicalExtent.cxx
    
    
    src/eo_over/OverArray.cxx
    src/eo_over/LogicalOver.cxx
    src/eo_over/PhysicalOver.cxx
    
    src/eo_setmd/LogicalSetMD.cxx
    src/eo_setmd/PhysicalSetMD.cxx
    
    src/eo_getmd/LogicalGetMD.cxx
    src/eo_getmd/PhysicalGetMD.cxx
    
    src/eo_cpsrs/LogicalCpSRS.cxx
    src/eo_cpsrs/PhysicalCpSRS.cxx 
	
    src/PostgresWrapper.cxx
    src/AffineTransform.cxx
    src/TemporalReference.cxx
    
    src/ErrorCodes.cxx
)

file(GLOB scidb4geo_include "*.h")

add_library(scidb4geo SHARED ${scidb4geo_src} ${scidb4geo_include} )

extractDebugInfo("${GENERAL_OUTPUT_DIRECTORY}/plugins" "libscidb4geo${CMAKE_SHARED_LIBRARY_SUFFIX}" scidb4geo)
set_target_properties(scidb4geo PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${GENERAL_OUTPUT_DIRECTORY}/plugins)
get_target_property(OUTPUT_SCIDB scidb4geo LOCATION)
add_custom_command(TARGET scidb4geo POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT_SCIDB} ${CMAKE_CURRENT_LIST_DIR}/install/.  COMMAND ${CMAKE_COMMAND} -E chdir ${CMAKE_CURRENT_LIST_DIR} tar "cfz" "scidb4geo_bin.tar.gz" install/ ) # copy shared library not only to scidb plugins path but also to the plugins dev path for binary packaging
