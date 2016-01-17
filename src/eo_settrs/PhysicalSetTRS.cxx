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

#include "../TemporalReference.h"

namespace scidb4geo
{
    using namespace std;
    using namespace scidb;

    static log4cxx::LoggerPtr logger ( log4cxx::Logger::getLogger ( "scidb4geo.eo_settrs" ) );


    /*! @copydoc LogicalSetTRS
     */
    class PhysicalSetTRS : public PhysicalOperator
    {
    public:
        PhysicalSetTRS ( const string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema ) :
            PhysicalOperator ( logicalName, physicalName, parameters, schema ) {
            _arrayName = ( ( std::shared_ptr<OperatorParamReference> & ) parameters[0] )->getObjectName();
        }




        virtual void preSingleExecute ( shared_ptr<Query> query ) {


            if ( _schema.isTransient() ) {
                shared_ptr<const InstanceMembership> membership ( Cluster::getInstance()->getInstanceMembership() );

                if ( ( membership->getViewId() != query->getCoordinatorLiveness()->getViewId() ) ||
                        ( membership->getInstances().size() != query->getInstancesCount() ) ) {

                    throw USER_EXCEPTION ( SCIDB_SE_EXECUTION, SCIDB_LE_NO_QUORUM2 );
                }
            }

            // Construct SRS object out of parameters

            string dim_t = ( ( std::shared_ptr<OperatorParamPhysicalExpression> & ) _parameters[1] )->getExpression()->evaluate().getString();
            string t0in = ( ( std::shared_ptr<OperatorParamPhysicalExpression> & ) _parameters[2] )->getExpression()->evaluate().getString();
            string dtin = ( ( std::shared_ptr<OperatorParamPhysicalExpression> & ) _parameters[3] )->getExpression()->evaluate().getString();


            // Check whether dimension exists
	    ArrayDesc arrayDesc;
	    SystemCatalog::getInstance()->getArrayDesc(_arrayName , query->getCatalogVersion(_arrayName ), LAST_VERSION, arrayDesc);
//             ArrayID arrayId = SystemCatalog::getInstance()->findArrayByName ( _arrayName );
//             std::shared_ptr<ArrayDesc> arrayDesc = SystemCatalog::getInstance()->getArrayDesc ( arrayId );
            Dimensions dims = arrayDesc.getDimensions();
            bool dimOK = false;
            for ( size_t i = 0; i < dims.size(); ++i ) {
                if ( dims[i].getBaseName().compare ( dim_t ) == 0 ) {
                    dimOK = true;
                    break;
                }
            }

            if ( !dimOK ) {
                stringstream serr;
                serr << "Dimension " << dim_t << " does not exist in target array.";
                SCIDB4GEO_ERROR ( serr.str() , SCIDB4GEO_ERR_UNDECLARED_DIMENSION );
            }



            TReference tref ( t0in, dtin );

            //TODO: Add some checks

            // Add to system catalog
            string t0 =  tref.getStart().toStringISO();
            string dt = tref.getCellsize().toStringISO();
            PostgresWrapper::instance()->dbSetTemporalRef ( _arrayName, dim_t, t0, dt );



        }

        std::shared_ptr< Array> execute ( std::vector< std::shared_ptr< Array> > &inputArrays,
                                            std::shared_ptr<Query> query ) {
            return std::shared_ptr< Array>();
        }


    private:
        string _arrayName;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalSetTRS, "eo_settrs", "PhysicalSetTRS" );
    typedef PhysicalSetTRS PhysicalSetTRS_depr;
    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalSetTRS_depr, "st_settrs", "PhysicalSetTRS_depr" ); // Backward compatibility
}

