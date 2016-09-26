/*
This file has been originally based on source code of SciDB (src/query/ops/apply/LogicalApply.cpp)
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
Modification date: (2016-09-26)

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

    class LogicalCoords: public LogicalOperator
    {
    public:
        LogicalCoords ( const string &logicalName, const std::string &alias ) :
            LogicalOperator ( logicalName, alias ) {
            _properties.tile = false;
            ADD_PARAM_INPUT()
        }

        ArrayDesc inferSchema ( std::vector<ArrayDesc> schemas, std::shared_ptr<Query> query ) {
            assert ( schemas.size() == 1 );
            assert ( _parameters.size() == 0 );

//             if ( ArrayDesc::makeUnversionedName ( schemas[0].getName() ).compare ( "" ) == 0 ||
//                     ArrayDesc::makeUnversionedName ( schemas[1].getName() ).compare ( "" ) == 0 ||
//                     ArrayDesc::makeUnversionedName ( schemas[0].getName() ).compare ( ArrayDesc::makeUnversionedName ( schemas[1].getName() ) ) == 0 ) {
//                 SCIDB4GEO_ERROR ( "Operator works on two different persistent input arrays", SCIDB4GEO_ERR_INVALIDINPUT );
//             }

            // Check dimensions
            Dimensions const &dims = schemas[0].getDimensions();

            Attributes outAttrs;
            AttributeID nextAttrId =0;
            for (size_t i=0; i<schemas[0].getAttributes().size(); i++)
            {
                AttributeDesc const& attr = schemas[0].getAttributes()[i];
                if(attr.getType()!=TID_INDICATOR)
                {
                    outAttrs.push_back( AttributeDesc(nextAttrId++,
                                                    attr.getName(),
                                                    attr.getType(),
                                                    attr.getFlags(),
                                                    attr.getDefaultCompressionMethod(),
                                                    attr.getAliases(),
                                                    attr.getReserve(),
                                                    &attr.getDefaultValue(),
                                                    attr.getDefaultValueExpr(),
                                                    attr.getVarSize()));
                }
            }
            
            
             if ( schemas[0].getEmptyBitmapAttribute() ) {
                AttributeDesc const *emptyTag = schemas[0].getEmptyBitmapAttribute();
                outAttrs.push_back ( AttributeDesc ( ( AttributeID ) nextAttrId++,
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
            
            outAttrs.push_back ( AttributeDesc ( ( AttributeID ) nextAttrId++, "eo_x", TID_DOUBLE, AttributeDesc::IS_NULLABLE, 0 ) ); 
            outAttrs.push_back ( AttributeDesc ( ( AttributeID ) nextAttrId++, "eo_y", TID_DOUBLE, AttributeDesc::IS_NULLABLE, 0 ) ); 
            outAttrs.push_back ( AttributeDesc ( ( AttributeID ) nextAttrId++, "eo_t", TID_STRING, AttributeDesc::IS_NULLABLE, 0 ) );

           

            return ArrayDesc ( "CoordsArray", outAttrs, dims, defaultPartitioning()  );

        }
    };
    REGISTER_LOGICAL_OPERATOR_FACTORY ( LogicalCoords, "eo_coords" );
}
