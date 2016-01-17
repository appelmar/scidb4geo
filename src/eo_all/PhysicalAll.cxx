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

#include <string>

#include "query/Operator.h"
#include "array/TupleArray.h"
#include "system/SystemCatalog.h"

#include "../PostgresWrapper.h"
#include "../Utils.h"

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

        virtual RedistributeContext getOutputDistribution ( const std::vector<RedistributeContext> &inputDistributions,
                const std::vector< ArrayDesc> &inputSchemas ) const {
            return RedistributeContext ( psLocalInstance );
        }

        void preSingleExecute ( std::shared_ptr<Query> query ) {



            vector<ArrayDesc > arrays;
            //SystemCatalog::getInstance()->
            for ( uint16_t i = 0; i < _parameters.size(); ++i ) {

	       string _arrayName = ( ( std::shared_ptr<OperatorParamReference> & ) _parameters[i] )->getObjectName() ;
	      ArrayDesc arrayDesc;
	    SystemCatalog::getInstance()->getArrayDesc(_arrayName , query->getCatalogVersion(_arrayName ), LAST_VERSION, arrayDesc);
// 	       ArrayDesc arrayDesc;
// 	       string _arrayName = ( ( std::shared_ptr<OperatorParamReference> & ) _parameters[i] )->getObjectName() ;
// 	       SystemCatalog::getInstance()->getArrayDesc(_arrayName, query->getCatalogVersion(_arrayName), LAST_VERSION, arrayDesc);
	       arrays.push_back ( arrayDesc );
	      
//                 ArrayID id = SystemCatalog::getInstance()->findArrayByName ( ( ( std::shared_ptr<OperatorParamReference> & ) _parameters[i] )->getObjectName() );
//                 arrays.push_back ( SystemCatalog::getInstance()->getArrayDesc ( id ) );
            }


            if ( _parameters.empty() ) { // If no array is given, fetch all referenced arrays
                vector<EOArrayInfo> info =  PostgresWrapper::instance()->dbGetArrays();
                for ( uint16_t i = 0; i < info.size(); ++i ) {
		    ArrayDesc arrayDesc;
		    SystemCatalog::getInstance()->getArrayDesc(info[i].arrayname , query->getCatalogVersion(info[i].arrayname ), LAST_VERSION, arrayDesc); 
                    arrays.push_back ( arrayDesc );
                }
            }



            std::shared_ptr<TupleArray> tuples ( std::make_shared<TupleArray> ( _schema, _arena ) );
            for ( size_t i = 0; i < arrays.size(); ++i ) {

                int curcol = 0;

                Value tuple[6];
                tuple[curcol++].setString ( arrays[i].getName() );
                //tuple[1].setString ( info[i].setting );

                //ArrayDesc desc;
                //SystemCatalog::getInstance()->getArrayDesc ( info[i].arrayname, desc );
                //arrays[i] =
                //SystemCatalog::getInstance()->getArrayDesc (arrays[i].getName(), arrays[i] ); // Overwrite current array MD with complete MD
                //ArrayId id = SystemCatalog::getInstance()->findArrayByName(arrays[i].getName());
                Coordinates lowBoundary = SystemCatalog::getInstance()->getLowBoundary ( arrays[i].getId() );
                Coordinates highBoundary = SystemCatalog::getInstance()->getHighBoundary ( arrays[i].getId() );

                stringstream ss;


                // Get Dimensions
                for ( size_t iD = 0; iD < arrays[i].getDimensions().size(); ++iD ) {
                    DimensionDesc d = arrays[i].getDimensions() [iD];

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
                Attributes A = arrays[i].getAttributes ( true );
                for ( size_t iA = 0; iA < A.size(); ++iA ) {
                    ss << "<" << A[iA].getName() << SEP
                       << A[iA].getType() << SEP
                       << ( ( A[iA].isNullable() ) ? "null" : "" ) << ">" ;
                }
                tuple[curcol++].setString ( ss.str() );


                // GetSRS
                ss.str ( "" );
                SpatialArrayInfo srs =  PostgresWrapper::instance()->dbGetSpatialRefOrEmpty ( arrays[i].getName() );
                bool isSpatial = ( srs.arrayname.compare ( arrays[i].getName() ) == 0 );
                if ( isSpatial ) {
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


                // GetTRS
                ss.str ( "" );
                TemporalArrayInfo trs =  PostgresWrapper::instance()->dbGetTemporalRefOrEmpty ( arrays[i].getName() );
                bool isTemporal = ( trs.arrayname.compare ( arrays[i].getName() ) == 0 );
                if ( isTemporal ) {
                    ss << trs.tdim << SEP
                       << trs.tref->getStart().toStringISO() << SEP
                       << trs.tref->getCellsize().toStringISO();
                    tuple[curcol++].setString ( ss.str() );

                }
                else {
                    tuple[curcol++].setString ( "" );
                }






                // Get Extent
                stringstream sext;
                sext << " ";
                int xdim_idx = -1;
                int ydim_idx = -1;
                int tdim_idx = -1;
                // Add vertical...

                // Find dimensions
                DimensionDesc d;
                for ( size_t iD = 0; iD < arrays[i].getDimensions().size(); ++iD ) {
                    d = arrays[i].getDimensions() [iD];
                    if ( isSpatial ) {
                        if ( d.getBaseName().compare ( srs.xdim ) == 0 ) xdim_idx = iD;
                        else if ( d.getBaseName().compare ( srs.ydim ) == 0 ) ydim_idx = iD;
                    }
                    if ( isTemporal ) {
                        if ( d.getBaseName().compare ( trs.tdim ) == 0 ) tdim_idx = iD;
                    }
                    // Add vertical...
                }


                /* Array bounds might be unknown und thus equal maximum int64 values.
                The followoing loop tries to find better values based on dimension settings */
                for ( size_t iD = 0; iD < arrays[i].getDimensions().size(); ++iD ) {
                    if ( abs ( lowBoundary[iD] ) == SCIDB_MAXDIMINDEX ) {
                        lowBoundary[iD] = arrays[i].getDimensions() [iD].getCurrStart();
                        if ( abs ( lowBoundary[iD] ) == SCIDB_MAXDIMINDEX ) lowBoundary[iD] = arrays[i].getDimensions() [iD].getStartMin();
                    }
                    if ( abs ( highBoundary[iD] ) == SCIDB_MAXDIMINDEX ) {
                        highBoundary[iD] = arrays[i].getDimensions() [iD].getCurrEnd();
                        if ( abs ( highBoundary[iD] ) == SCIDB_MAXDIMINDEX ) highBoundary[iD] = arrays[i].getDimensions() [iD].getEndMax();
                    }
                }


                if ( isSpatial ) {
                    AffineTransform::double2 ps1_world = srs.A.f ( AffineTransform::double2 ( lowBoundary[xdim_idx],  lowBoundary[ydim_idx] ) );
                    AffineTransform::double2 ps2_world = srs.A.f ( AffineTransform::double2 ( lowBoundary[xdim_idx],  highBoundary[ydim_idx] ) );
                    AffineTransform::double2 ps3_world = srs.A.f ( AffineTransform::double2 ( highBoundary[xdim_idx], lowBoundary[ydim_idx] ) );
                    AffineTransform::double2 ps4_world = srs.A.f ( AffineTransform::double2 ( highBoundary[xdim_idx], highBoundary[ydim_idx] ) );

                    // Find minimum and maximum of transformed corners
                    double x[4] = {ps1_world.x, ps2_world.x, ps3_world.x, ps4_world.x};
                    double y[4] = {ps1_world.y, ps2_world.y, ps3_world.y, ps4_world.y};
                    AffineTransform::double2 lowleft ( utils::min ( x, 4 ), utils::min ( y, 4 ) );
                    AffineTransform::double2 upright ( utils::max ( x, 4 ), utils::max ( y, 4 ) );
                    sext << setprecision ( numeric_limits< double >::digits10 ) << lowleft.x << SEP << upright.x << SEP << lowleft.y << SEP << upright.y << SEP;
                }
                else {
                    sext <<  "0.0"  << SEP <<  "0.0"  << SEP   "0.0"  << SEP   "0.0"  << SEP ;
                }

                if ( isTemporal ) {
                    sext <<  trs.tref->datetimeAtIndex ( lowBoundary[tdim_idx] ).toStringISO() << SEP << ( trs.tref->datetimeAtIndex ( highBoundary[tdim_idx] ).toStringISO() ) << SEP;
                    delete trs.tref;

                }
                else {
                    sext <<  ""  << SEP <<  ""  << SEP  ;
                }
                // Add vertical...

                tuple[curcol++].setString ( sext.str() );








                tuples->appendTuple ( tuple );

            }

            _result = tuples;

        }

        std::shared_ptr<Array> execute ( vector< std::shared_ptr<Array> > &inputArrays, std::shared_ptr<Query> query ) {

            if ( !_result ) {
                _result = std::make_shared<MemArray> ( _schema, query );
            }
            return _result;
        }

    private:
        std::shared_ptr<Array> _result;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalAll, "eo_all", "PhysicalAll" );
} //namespace
