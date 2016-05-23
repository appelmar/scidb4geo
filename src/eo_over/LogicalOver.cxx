/*
This file has been originally based on source code of SciDB (src/query/ops/build/LogicalBuild.cpp)
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

#include "query/Operator.h"
#include "system/SystemCatalog.h"
#include "system/Exceptions.h"
#include "../ErrorCodes.h"


using namespace std;
using namespace scidb;

namespace scidb4geo
{

    class LogicalOver: public LogicalOperator
    {
    public:
        LogicalOver ( const string &logicalName, const std::string &alias ) :
            LogicalOperator ( logicalName, alias ) {
            ADD_PARAM_INPUT()
            ADD_PARAM_INPUT()
        }

        ArrayDesc inferSchema ( std::vector<ArrayDesc> schemas, std::shared_ptr<Query> query ) {
            assert ( schemas.size() == 2 );
            assert ( _parameters.size() == 0 );

            if ( ArrayDesc::makeUnversionedName ( schemas[0].getName() ).compare ( "" ) == 0 ||
                    ArrayDesc::makeUnversionedName ( schemas[1].getName() ).compare ( "" ) == 0 ||
                    ArrayDesc::makeUnversionedName ( schemas[0].getName() ).compare ( ArrayDesc::makeUnversionedName ( schemas[1].getName() ) ) == 0 ) {
                SCIDB4GEO_ERROR ( "Operator works on two different persistent input arrays", SCIDB4GEO_ERR_INVALIDINPUT );
            }

            // Check dimensions
            Dimensions const &dims = schemas[0].getDimensions();
	   
            for ( size_t i = 0, n = dims.size();  i < n; i++ ) {
                if ( dims[i].isMaxStar()) {
                    SCIDB4GEO_ERROR ( "Operator works only on bounded dimensions", SCIDB4GEO_ERR_INVALIDDIMENSION );
                }
            }

            Attributes outAttrs;

//             outAttrs.push_back ( AttributeDesc ( ( AttributeID ) 0, ArrayDesc::makeUnversionedName(schemas[1].getName()) + ".x", TID_INT64, AttributeDesc::IS_NULLABLE, 0 ) ); // TODO: Change name to target dimension name?!
//             outAttrs.push_back ( AttributeDesc ( ( AttributeID ) 1, ArrayDesc::makeUnversionedName(schemas[1].getName()) + ".y", TID_INT64, AttributeDesc::IS_NULLABLE, 0 ) ); // TODO: Change name to target dimension name?!
            outAttrs.push_back ( AttributeDesc ( ( AttributeID ) 0, "over_x", TID_INT64, AttributeDesc::IS_NULLABLE, 0 ) ); // TODO: Change name to target dimension name?!
            outAttrs.push_back ( AttributeDesc ( ( AttributeID ) 1, "over_y", TID_INT64, AttributeDesc::IS_NULLABLE, 0 ) ); // TODO: Change name to target dimension name?!
            outAttrs.push_back ( AttributeDesc ( ( AttributeID ) 2, "over_t", TID_INT64, AttributeDesc::IS_NULLABLE, 0 ) ); // TODO: Change name to target dimension name?!

            if ( schemas[0].getEmptyBitmapAttribute() ) {
                AttributeDesc const *emptyTag = schemas[0].getEmptyBitmapAttribute();
                outAttrs.push_back ( AttributeDesc ( ( AttributeID ) 3,
                                                     emptyTag->getName(),
                                                     emptyTag->getType(),
                                                     emptyTag->getFlags(),
                                                     emptyTag->getDefaultCompressionMethod(),
                                                     emptyTag->getAliases(),
                                                     emptyTag->getReserve(),
                                                     &emptyTag->getDefaultValue(),
                                                     emptyTag->getDefaultValueExpr(),
                                                     emptyTag->getVarSize() ) );
            }
            stringstream ss;
            ss << query->getInstanceID();
            ArrayDistPtr localDist = ArrayDistributionFactory::getInstance()->construct(psLocalInstance, DEFAULT_REDUNDANCY,ss.str());
            return ArrayDesc ( "OverArray", outAttrs, dims, localDist,  query->getDefaultArrayResidency());   


        }
    };
    REGISTER_LOGICAL_OPERATOR_FACTORY ( LogicalOver, "eo_over" );
}
