/*
This file has been originally based on source code of SciDB (examples/example_udos/PhysicalHelloInstances.cpp)
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

#include <boost/make_shared.hpp>
#include "query/Operator.h"
#include "array/TransientCache.h"
#include "array/DBArray.h"
#include "array/Metadata.h"
#include "system/SystemCatalog.h"
#include <log4cxx/logger.h>

#include "../PostgresWrapper.h"
#include "../ErrorCodes.h"

namespace scidb4geo
{
    using namespace std;
    using namespace scidb;

    static log4cxx::LoggerPtr logger ( log4cxx::Logger::getLogger ( "scidb4geo.eo_setsrs" ) );


    /*! @copydoc LogicalSetSRS
     */
    class PhysicalSetSRS : public PhysicalOperator
    {
    public:
        PhysicalSetSRS ( const string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema ) :
            PhysicalOperator ( logicalName, physicalName, parameters, schema ) {
                
        }


        

        virtual void preSingleExecute ( shared_ptr<Query> query ) {
            
                 
            shared_ptr<OperatorParamArrayReference> &arrayRef = ( shared_ptr<OperatorParamArrayReference> & ) _parameters[0];
            query->getNamespaceArrayNames(arrayRef->getObjectName(), _namespaceName, _arrayName);
            
            // Construct SRS object out of parameters
            AffineTransform A ( ( ( std::shared_ptr<OperatorParamPhysicalExpression> & ) _parameters[_parameters.size() - 1] )->getExpression()->evaluate().getString() ); // create affine transform based on string
            string auth_name = ( ( std::shared_ptr<OperatorParamPhysicalExpression> & ) _parameters[3] )->getExpression()->evaluate().getString();
            int auth_srid = ( ( std::shared_ptr<OperatorParamPhysicalExpression> & ) _parameters[4] )->getExpression()->evaluate().getInt32();
            string dim_x = ( ( std::shared_ptr<OperatorParamPhysicalExpression> & ) _parameters[1] )->getExpression()->evaluate().getString();
            string dim_y = ( ( std::shared_ptr<OperatorParamPhysicalExpression> & ) _parameters[2] )->getExpression()->evaluate().getString();



	   
            ArrayID arrayID = query->getCatalogVersion(_namespaceName, _arrayName);
            ArrayDesc arrayDesc;
            SystemCatalog::getInstance()->getArrayDesc(arrayID, arrayDesc);
   

            Dimensions dims = arrayDesc.getDimensions();
            bool dimOK_x = false;
            bool dimOK_y = false;
            for ( size_t i = 0; i < dims.size(); ++i ) {
                if ( dims[i].getBaseName().compare ( dim_x ) == 0 ) dimOK_x = true;
                else if ( dims[i].getBaseName().compare ( dim_y ) == 0 ) dimOK_y = true;
                if ( dimOK_x && dimOK_y ) break;
            }
            if ( !dimOK_x ) {
                if ( !dimOK_y ) {
                    stringstream serr;
                    serr << "Dimensions " << dim_x << " and " << dim_y  << " do not exist in target array.";
                    SCIDB4GEO_ERROR ( serr.str() , SCIDB4GEO_ERR_UNDECLARED_DIMENSION );
                }
                else {
                    stringstream serr;
                    serr << "Dimension " << dim_x << " does not exist in target array.";
                    SCIDB4GEO_ERROR ( serr.str()  , SCIDB4GEO_ERR_UNDECLARED_DIMENSION );
                }

            }
            else if ( !dimOK_y ) {
                stringstream serr;
                serr << "Dimension " << dim_y << " does not exist in target array.";
                SCIDB4GEO_ERROR ( serr.str() , SCIDB4GEO_ERR_UNDECLARED_DIMENSION );
            }



            // Add to system catalog
            PostgresWrapper::instance()->dbSetSpatialRef ( _arrayName, dim_x, dim_y, auth_name, auth_srid, A );


        }

        std::shared_ptr< Array> execute ( std::vector< std::shared_ptr< Array> > &inputArrays,
                                            std::shared_ptr<Query> query ) {
            return std::shared_ptr< Array>();
        }


    private:
        string _arrayName;
        string _namespaceName;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalSetSRS, "eo_setsrs", "PhysicalSetSRS" );
    typedef PhysicalSetSRS PhysicalSetSRS_depr;
    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalSetSRS_depr, "st_setsrs", "PhysicalSetSRS_depr" ); // Backward compatibility
}

