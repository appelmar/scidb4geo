/*
This file has been originally based on source code of SciDB (examples/example_udos/LogicalHelloInstances.cpp)
which is copyright (C) 2008-2014 SciDB, Inc.

SciDB is free software: you can redistribute it and/or modify
it under the terms of the AFFERO GNU General Public License as published by
the Free Software Foundation.

SciDB is distributed "AS-IS" AND WITHOUT ANY WARRANTY OF ANY KIND,
INCLUDING ANY IMPLIED WARRANTY OF MERCHANTABILITY,
NON-INFRINGEMENT, OR FITNESS FOR A PARTICULAR PURPOSE. See
the AFFERO GNU General Public License for the complete license terms.

You should have received a copy of the AFFERO GNU General Public License
along with SciDB.  If not, see <http://www.gnu.org/licenses/agpl-3.0.html>

-----------------------------------------------------------------------------
Modification date: (2015-08-01)

Modifications are copyright (C) 2016 Marius Appel <marius.appel@uni-muenster.de>

scidb4geo - A SciDB plugin for managing spacetime earth-observation arrays

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#include "../plugin.h"  // Must be first to define PROJECT_ROOT

#include "log4cxx/logger.h"

#include "../ErrorCodes.h"
#include "query/Operator.h"
#include "system/Exceptions.h"
#include "system/SystemCatalog.h"

namespace scidb4geo {

    using namespace std;
    using namespace scidb;

    static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("scidb4geo.setSpatialRef"));

    /**
     * @brief SciDB Operator eo_setsrs().
     *
     * @par Synopsis:
     *   eo_setsrs(named_array, xdim_name, ydim_name, auth_name, auth_srid, affine_transform_string)
     * @par Summary:
     *   Adds spatial reference system information to an existing array.
     * @par Input:
     *   - named_array: an existing (named) array.
     *   - xdim: name of the array's east-west dimension as string
     *   - ydim: name of the array's north-south dimension as string
     *   - auth_name: authority name, e.g. EPSG
     *   - auth_srid: ID of the reference system, e.g. 4326
     *   - proj4text: proj4 string
     *   - affine_transform_string: String representation of an affine transformation for relating integer array coordinates to SRS coordinates (see notes for details)
     * @par Output array:
     *   Returns the original input array
     *
     * @par Examples:
     *   eo_setsrs(named_array, 'j', 'i', 'EPSG', 4326, 'x0=7.01 y0=52.1 a11=0.03 a22=0.03');
     *
     * @par Errors:
     *   n/a
     *
     * @par Notes:
     *   An affine transformation is used to convert array integer coordinates to the spatial reference system coordinates. In two dimensions, 6 parameters are used. x0 and y0 specify an offset
     *   i.e. coordinates at the array's origin while a11, a12, a21, a22 specify a transformation matrix.
     *   Ignoring rotation and shear, i.e. a12 = a21 = 0, a11 and a22 can be interpreted as pixel size in x and y
     *   dimensions respectively (see also the GDAL documentation http://www.gdal.org/gdal_datamodel.html for further details). Default values are x0=y0=a12=a21=0 and a11=a22=1.
     *
     *
     *
     */
    class LogicalSetSRS : public LogicalOperator {
       public:
        LogicalSetSRS(const string &logicalName, const string &alias) : LogicalOperator(logicalName, alias) {
            ADD_PARAM_IN_ARRAY_NAME2(PLACEHOLDER_ARRAY_NAME_VERSION | PLACEHOLDER_ARRAY_NAME_INDEX_NAME)  // Arrayname will be stored in _parameters[0]
            //ADD_PARAM_IN_DIMENSION_NAME()
            //ADD_PARAM_IN_DIMENSION_NAME()
            //ADD_PARAM_CONSTANT ( TID_STRING )              // xdim as string
            // ADD_PARAM_CONSTANT ( TID_STRING )              // ydim as string
            ADD_PARAM_VARIES()
        }

        vector<std::shared_ptr<OperatorParamPlaceholder> > nextVaryParamPlaceholder(const vector<ArrayDesc> &schemas) {
            vector<std::shared_ptr<OperatorParamPlaceholder> > res;

            if (_parameters.size() == 1) {
                //  2nd argument is either dim name or array reference
                res.push_back(PARAM_CONSTANT(TID_STRING));
                res.push_back(PARAM_IN_ARRAY_NAME2(PLACEHOLDER_ARRAY_NAME_VERSION | PLACEHOLDER_ARRAY_NAME_INDEX_NAME));
            } else if (_parameters.size() == 2) {
                //  3rd argument must be dim name if present
                res.push_back(END_OF_VARIES_PARAMS());
                res.push_back(PARAM_CONSTANT(TID_STRING));
            }

            // Par 4: String or integer (either affine_transform_string or auth_srid)
            else if (_parameters.size() == 3) {
                res.push_back(PARAM_CONSTANT(TID_STRING));
            } else if (_parameters.size() == 4) {
                //res.push_back(PARAM_CONSTANT(TID_STRING));
                res.push_back(PARAM_CONSTANT(TID_INT32));
            }
            // Par 6: String or empty (either affine_transform_string or nothing)
            else if (_parameters.size() == 5) {
                res.push_back(PARAM_CONSTANT(TID_STRING));
                res.push_back(END_OF_VARIES_PARAMS());
            } else
                res.push_back(END_OF_VARIES_PARAMS());

            return res;
        }

        ArrayDesc inferSchema(std::vector<ArrayDesc> schemas, std::shared_ptr<Query> query) {
            //assert ( schemas.size() == 0 );

            bool valid = false;
            valid = valid || (_parameters.size() == 5);
            //                 && _parameters[1]->getParamType() == PARAM_CONSTANT ( TID_STRING )
            //                 && _parameters[2]->getParamType() == PARAM_CONSTANT ( TID_STRING )
            //                 && _parameters[2]->getParamType() == PARAM_CONSTANT ( TID_STRING )
            //                 && _parameters[2]->getParamType() == PARAM_CONSTANT ( TID_INT32 );

            valid = valid || (_parameters.size() == 6);
            //                 && _parameters[1]->getParamType() == PARAM_CONSTANT ( TID_STRING )
            //                 && _parameters[2]->getParamType() == PARAM_CONSTANT ( TID_STRING )
            //                 && _parameters[2]->getParamType() == PARAM_CONSTANT ( TID_STRING )
            //                 && _parameters[2]->getParamType() == PARAM_CONSTANT ( TID_INT32 )
            //                 && _parameters[2]->getParamType() == PARAM_CONSTANT ( TID_STRING );

            valid = valid || (_parameters.size() == 2 && _parameters[1]->getParamType() == PARAM_ARRAY_REF);
            valid = valid && _parameters[0]->getParamType() == PARAM_ARRAY_REF;

            if (!valid) {
                SCIDB4GEO_ERROR("Invalid call of eo_setsrs()", SCIDB4GEO_ERR_INVALIDINPUT);
            }

            assert(valid);

            shared_ptr<OperatorParamArrayReference> &arrayRef = (shared_ptr<OperatorParamArrayReference> &)_parameters[0];
            assert(arrayRef->getArrayName().find('@') == string::npos);
            assert(arrayRef->getObjectName().find('@') == string::npos);
            if (arrayRef->getVersion() == ALL_VERSIONS) {
                throw USER_QUERY_EXCEPTION(SCIDB_SE_INFER_SCHEMA, SCIDB_LE_WRONG_ASTERISK_USAGE2, _parameters[0]->getParsingContext());
            }
            ArrayDesc schema;
            return schema;
        }
    };

    REGISTER_LOGICAL_OPERATOR_FACTORY(LogicalSetSRS, "eo_setsrs");
    typedef LogicalSetSRS LogicalSetSRS_depr;
    REGISTER_LOGICAL_OPERATOR_FACTORY(LogicalSetSRS_depr, "st_setsrs");  // Backward compatibility
}
