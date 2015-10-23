/*
scidb4geo - A SciDB plugin for managing spatially referenced arrays
Copyright (C) 2015 Marius Appel <marius.appel@uni-muenster.de>

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
     * @brief SciDB Operator eo_bbox().
     *
     * @par Synopsis:
     *   eo_bbox(nammed_array)
     * @par Summary:
     *   Computes the spatial bounding box of a spatially referenced array.
     * @par Input:
     *   - named_array: an existing array that has a spatial reference
     * @par Output array:
     *   A 2x2 array {(xmin,xmax),(ymin,ymax)}
     *
     * @par Examples:
     *   eo_bbox(a);
     *
     * @par Errors:
     *   n/a
     *
     * @par Notes:
     *   Bounding box coordinates are returned as the reference system's coordinates, i.e. after applying the affine transformation to dimension boundaries.
     *
     *
     */
    class LogicalBBOX: public LogicalOperator
    {
    public:
        LogicalBBOX ( const string &logicalName, const std::string &alias ) :
            LogicalOperator ( logicalName, alias ) {
            ADD_PARAM_IN_ARRAY_NAME()
        }




        ArrayDesc inferSchema ( std::vector< ArrayDesc> inputSchemas, boost::shared_ptr< Query> query ) {
            assert ( inputSchemas.size() == 0 );
            assert ( _parameters.size() == 1 );



            Attributes attributes ( 1 );
            attributes[0] = AttributeDesc ( ( AttributeID ) 0, "coord", TID_DOUBLE, 0, 0 );





            vector<DimensionDesc> dimensions ( 2 );


            dimensions[0] = DimensionDesc ( "coord", 0, 1, 2, 0 );
            dimensions[1] = DimensionDesc ( "min/max", 0, 1, 2, 0 );


            return ArrayDesc ( "BBOX", attributes, dimensions );
        }

    };


    REGISTER_LOGICAL_OPERATOR_FACTORY ( LogicalBBOX, "eo_bbox" );
    typedef LogicalBBOX LogicalBBOX_depr;
    REGISTER_LOGICAL_OPERATOR_FACTORY ( LogicalBBOX_depr, "st_bbox" ); // Backward compatibility
}

