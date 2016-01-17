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

#include "../plugin.h" // Must be first to define PROJECT_ROOT

#include "log4cxx/logger.h"

#include "query/Operator.h"
#include "system/SystemCatalog.h"
#include "system/Exceptions.h"

#include "boost/date_time/posix_time/posix_time.hpp"

namespace scidb4geo
{


    using namespace std;
    using namespace scidb;

    static log4cxx::LoggerPtr logger ( log4cxx::Logger::getLogger ( "scidb4geo.eo_settrs" ) );

    /**
     * @brief SciDB Operator eo_settrs().
     *
     * @par Synopsis:
     *   eo_settrs(named_array, tdim_name, t0_iso, dt_iso)
     * @par Summary:
     *   Adds spatial reference system information to an existing array.
     * @par Input:
     *   - named_array: an existing (named) array.
     *   - tdim_name: name of the array's temporal dimension as string
     *   - t0_iso: ISO 8601 date/time point string
     *   - dt_iso: ISO 8601 date/time period string
     * @par Output array:
     *   Returns the original input array
     *
     * @par Examples:
     *   eo_settrs(named_array, 't', '2015-02-01', 'P1D');
     *
     * @par Errors:
     *   n/a
     *
     * @par Notes:
     *
     *
     */
    class LogicalSetTRS: public LogicalOperator
    {
    public:
        LogicalSetTRS ( const string &logicalName, const string &alias ) :
            LogicalOperator ( logicalName, alias ) {



            ADD_PARAM_IN_ARRAY_NAME2 ( PLACEHOLDER_ARRAY_NAME_VERSION | PLACEHOLDER_ARRAY_NAME_INDEX_NAME ) // Arrayname will be stored in _parameters[0]
            //ADD_PARAM_IN_DIMENSION_NAME()
            //ADD_PARAM_IN_DIMENSION_NAME()
            ADD_PARAM_CONSTANT ( TID_STRING ) // tdim as string
            ADD_PARAM_CONSTANT ( TID_STRING ) // t0 as string
            ADD_PARAM_CONSTANT ( TID_STRING ) // dt as string
        }






        ArrayDesc inferSchema ( std::vector<ArrayDesc> schemas, std::shared_ptr<Query> query ) {
            assert ( schemas.size() == 0 );
            assert ( _parameters.size() == 4 );
            assert ( _parameters[0]->getParamType() == PARAM_ARRAY_REF );
            shared_ptr<OperatorParamArrayReference> &arrayRef = ( shared_ptr<OperatorParamArrayReference> & ) _parameters[0];
            assert ( arrayRef->getArrayName().find ( '@' ) == string::npos );
            assert ( arrayRef->getObjectName().find ( '@' ) == string::npos );
            if ( arrayRef->getVersion() == ALL_VERSIONS ) {
                throw USER_QUERY_EXCEPTION ( SCIDB_SE_INFER_SCHEMA, SCIDB_LE_WRONG_ASTERISK_USAGE2, _parameters[0]->getParsingContext() );
            }


            ArrayDesc schema;
            return schema;
        }

    };



    REGISTER_LOGICAL_OPERATOR_FACTORY ( LogicalSetTRS, "eo_settrs" );
    typedef LogicalSetTRS LogicalSetTRS_depr;
    REGISTER_LOGICAL_OPERATOR_FACTORY ( LogicalSetTRS_depr, "st_settrs" ); // Backward compatibility
}

