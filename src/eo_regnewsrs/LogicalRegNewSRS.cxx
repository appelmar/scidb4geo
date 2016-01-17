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

#include "query/Operator.h"
#include "system/Exceptions.h"
#include "array/Metadata.h"
#include "system/SystemCatalog.h"

#include "../PostgresWrapper.h"
namespace scidb4geo
{


    using namespace std;
    using namespace boost;
    using namespace scidb;


    /**
     * @brief SciDB Operator eo_regnewsrs().
     *
     * @par Synopsis:
     *   eo_regnewsrs(auth_name, auth_srid, wktext, proj4text)
     * @par Summary:
     *   Adds a custom spatial reference system to the system catalog
     * @par Input:
     *   - auth_name: authority name, can be defined arbitrarily but each combination auth_name,auth_id must be unique
     *   - auth_srid: integer authority identifying number, can be defined arbitrarily but each combination auth_name,auth_id must be unique
     *   - wktext: WKT representation of the coordinate system
     *   - proj4text: proj4 string of the coordinate system
     * @par Output array:
     *   NULL
     *
     * @par Examples:
     *   eo_regnewsrs('XXXX',4326, 'GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]]','+proj=longlat +datum=WGS84 +no_defs ');
     *
     * @par Errors:
     *   n/a
     *
     * @par Notes:
     *   n/a
     *
     *
     */
    class LogicalRegNewSRS: public LogicalOperator
    {
    public:
        LogicalRegNewSRS ( const string &logicalName, const std::string &alias ) :
            LogicalOperator ( logicalName, alias ) {
            ADD_PARAM_CONSTANT ( TID_STRING )
            ADD_PARAM_CONSTANT ( TID_INT32 )
            ADD_PARAM_CONSTANT ( TID_STRING )
            ADD_PARAM_CONSTANT ( TID_STRING )
        }




        ArrayDesc inferSchema ( std::vector< ArrayDesc> inputSchemas, std::shared_ptr< Query> query ) {
            assert ( inputSchemas.size() == 0 );
            assert ( _parameters.size() == 4 );

            Attributes attributes ( 4 );
            attributes[0] = AttributeDesc ( ( AttributeID ) 0, "auth_name", TID_STRING, 0, 0 );
            attributes[1] = AttributeDesc ( ( AttributeID ) 1, "auth_id", TID_INT32, 0, 0 );
            attributes[2] = AttributeDesc ( ( AttributeID ) 2, "srtext", TID_STRING, 0, 0 );
            attributes[3] = AttributeDesc ( ( AttributeID ) 3, "proj4text", TID_STRING, 0, 0 );

            size_t nArrays;
            nArrays = 1;


            vector<DimensionDesc> dimensions ( 1 );
            size_t end    = nArrays > 0 ? nArrays - 1 : 0;

            dimensions[0] = DimensionDesc ( "srs", 0, 0, end, end, nArrays, 0 );
            return ArrayDesc ( "SRS", attributes, dimensions, defaultPartitioning()  );
        }

    };


    REGISTER_LOGICAL_OPERATOR_FACTORY ( LogicalRegNewSRS, "eo_regnewsrs" );
    typedef LogicalRegNewSRS LogicalRegNewSRS_depr;
    REGISTER_LOGICAL_OPERATOR_FACTORY ( LogicalRegNewSRS_depr, "st_regnewsrs" ); // Backward compatibility

}

