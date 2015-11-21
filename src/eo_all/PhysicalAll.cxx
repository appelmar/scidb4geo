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

Modifications are copyright (C) 2015 Marius Appel <marius.appel@uni-muenster.de>

scidb4geo - A SciDB plugin for managing spatially referenced arrays

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

#include <string>

#include "query/Operator.h"
#include "array/TupleArray.h"
#include "system/SystemCatalog.h"

#include "../PostgresWrapper.h"


#define SEP ";;;"

namespace scidb4geo
{
    using namespace std;
    using namespace boost;
    using namespace scidb;

    /*! @copydoc LogicalAll
     */
    class PhysicalAll: public PhysicalOperator
    {
    public:
        PhysicalAll ( string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema ) :
            PhysicalOperator ( logicalName, physicalName, parameters, schema ) {
        }

        virtual ArrayDistribution getOutputDistribution ( const std::vector<ArrayDistribution> &inputDistributions,
                const std::vector< ArrayDesc> &inputSchemas ) const {
            return ArrayDistribution ( psLocalInstance );
        }

        void preSingleExecute ( boost::shared_ptr<Query> query ) {



            vector<boost::shared_ptr<ArrayDesc> > arrays;
            //SystemCatalog::getInstance()->
            for ( uint16_t i = 0; i < _parameters.size(); ++i ) {

                ArrayID id = SystemCatalog::getInstance()->findArrayByName ( ( ( boost::shared_ptr<OperatorParamReference> & ) _parameters[i] )->getObjectName() );
                arrays.push_back ( SystemCatalog::getInstance()->getArrayDesc ( id ) );
            }


            if ( _parameters.empty() ) { // If no array is given, fetch all referenced arrays
                vector<EOArrayInfo> info =  PostgresWrapper::instance()->dbGetArrays();
                for ( uint16_t i = 0; i < info.size(); ++i ) {

                    ArrayID id = SystemCatalog::getInstance()->findArrayByName ( info[i].arrayname );
                    arrays.push_back ( SystemCatalog::getInstance()->getArrayDesc ( id ) );
                }
            }



            boost::shared_ptr<TupleArray> tuples ( boost::make_shared<TupleArray> ( _schema, _arena ) );
            for ( size_t i = 0; i < arrays.size(); ++i ) {

                int curcol = 0;

                Value tuple[5];
                tuple[curcol++].setString ( arrays[i]->getName() );
                //tuple[1].setString ( info[i].setting );

                //ArrayDesc desc;
                //SystemCatalog::getInstance()->getArrayDesc ( info[i].arrayname, desc );
                //arrays[i] =
                //SystemCatalog::getInstance()->getArrayDesc (arrays[i].getName(), arrays[i] ); // Overwrite current array MD with complete MD
                //ArrayId id = SystemCatalog::getInstance()->findArrayByName(arrays[i].getName());
                Coordinates lowBoundary = SystemCatalog::getInstance()->getLowBoundary ( arrays[i]->getId() );
                Coordinates highBoundary = SystemCatalog::getInstance()->getHighBoundary ( arrays[i]->getId() );

                stringstream ss;


                // Get Dimensions
                for ( size_t iD = 0; iD < arrays[i]->getDimensions().size(); ++iD ) {
                    DimensionDesc d = arrays[i]->getDimensions() [iD];

                    ss << "[" << d.getBaseName() << SEP
                       << d.getStartMin() << SEP
                       << d.getLength() << SEP
                       << d.getChunkInterval() << SEP
                       << d.getChunkOverlap() << SEP
                       << d.getCurrStart() << SEP
                       << d.getCurrEnd() << SEP
                       << lowBoundary[iD] << SEP
                       << highBoundary[iD] << "]" ;
                }
                tuple[curcol++].setString ( ss.str() );
                ss.str ( "" );


                // Get Attributes
                Attributes A = arrays[i]->getAttributes ( true );
                for ( size_t iA = 0; iA < A.size(); ++iA ) {
                    ss << "<" << A[iA].getName() << SEP
                       << A[iA].getType() << SEP
                       << ( ( A[iA].isNullable() ) ? "null" : "" ) << ">" ;
                }
                tuple[curcol++].setString ( ss.str() );


                // GetSRS
                ss.str ( "" );
                SpatialArrayInfo srs =  PostgresWrapper::instance()->dbGetSpatialRefOrEmpty ( arrays[i]->getName() );
                if ( srs.arrayname.compare ( arrays[i]->getName() ) == 0 ) {
                    ss << srs.xdim << SEP
                       << srs.ydim << SEP
                       << srs.auth_name << SEP
                       << srs.auth_srid << SEP
                       << srs.A.toString() << SEP
                       << srs.srtext << SEP
                       << srs.proj4text;

                    tuple[curcol++].setString ( ss.str() );
                }
                else {
                    tuple[curcol++].setString ( "" );
                }


                // GetSRS
                ss.str ( "" );
                TemporalArrayInfo trs =  PostgresWrapper::instance()->dbGetTemporalRefOrEmpty ( arrays[i]->getName() );
                if ( trs.arrayname.compare ( arrays[i]->getName() ) == 0 ) {
                    ss << trs.tdim << SEP
                       << trs.tref->getStart().toStringISO() << SEP
                       << trs.tref->getCellsize().toStringISO();
                    tuple[curcol++].setString ( ss.str() );

                }
                else {
                    tuple[curcol++].setString ( "" );
                }


                tuples->appendTuple ( tuple );

            }

            _result = tuples;

        }

        boost::shared_ptr<Array> execute ( vector< boost::shared_ptr<Array> > &inputArrays, boost::shared_ptr<Query> query ) {

            if ( !_result ) {
                _result = boost::make_shared<MemArray> ( _schema, query );
            }
            return _result;
        }

    private:
        boost::shared_ptr<Array> _result;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalAll, "eo_all", "PhysicalAll" );
} //namespace
