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
#include "../Utils.h"
#include "../TemporalReference.h"

namespace scidb4geo
{
    using namespace std;
    using namespace boost;
    using namespace scidb;

    /*! @copydoc LogicalExtent
     */
    class PhysicalExtent: public PhysicalOperator
    {
    public:
        PhysicalExtent ( string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema ) :
            PhysicalOperator ( logicalName, physicalName, parameters, schema ) {
        }

        virtual ArrayDistribution getOutputDistribution ( const std::vector<ArrayDistribution> &inputDistributions,
                const std::vector< ArrayDesc> &inputSchemas ) const {
            return ArrayDistribution ( psLocalInstance );
        }




        void preSingleExecute ( boost::shared_ptr<Query> query ) {


            const string &arrayName = ( ( boost::shared_ptr<OperatorParamReference> & ) _parameters[0] )->getObjectName();

            vector<SpatialArrayInfo> info_s = PostgresWrapper::instance()->dbGetSpatialRef ( vector<string> ( 1, arrayName ) ) ;
            vector<TemporalArrayInfo> info_t = PostgresWrapper::instance()->dbGetTemporalRef ( vector<string> ( 1, arrayName ) ) ;
            // Add vertical...

            ArrayID arrayId = SystemCatalog::getInstance()->findArrayByName ( arrayName );
            boost::shared_ptr<ArrayDesc> arrayDesc = SystemCatalog::getInstance()->getArrayDesc ( arrayId );
            Dimensions dims = arrayDesc->getDimensions();

            int xdim_idx = -1;
            int ydim_idx = -1;
            int tdim_idx = -1;
            // Add vertical...


            if ( info_s.size() + info_t.size()  < 1 ) {
                throw PLUGIN_USER_EXCEPTION ( "libscidb4geo", SCIDB_SE_UDO, scidb4geo::SCIDB4GEO_ERR_UNKNOWN_SRS ) << " Array '" << arrayName << "' does not have any spatial, temporal, or vertical reference.";
            }

            // Find dimensions
            DimensionDesc d;
            for ( size_t i = 0; i < dims.size(); ++i ) {
                d = dims[i];
                if ( info_s.size() == 1 ) {
                    if ( d.getBaseName().compare ( info_s[0].xdim ) == 0 ) xdim_idx = i;
                    else if ( d.getBaseName().compare ( info_s[0].ydim ) == 0 ) ydim_idx = i;
                }
                if ( info_t.size() == 1 ) {
                    if ( d.getBaseName().compare ( info_t[0].tdim ) == 0 ) tdim_idx = i;
                }
                // Add vertical...
            }



            if ( ( ( xdim_idx < 0 || ydim_idx < 0 ) && info_s.size() > 0 ) ||
                    ( tdim_idx < 0  && info_t.size() > 0 ) ) { // Add vertical...
                stringstream serr;
                serr << "One or more referenced dimensions of array '" << arrayName << "' do not exist.";
                SCIDB4GEO_ERROR ( serr.str(), SCIDB4GEO_ERR_INCONSISTENT_DBSTATE );
                return;
            }

            Coordinates lowBoundary = SystemCatalog::getInstance()->getLowBoundary ( arrayDesc->getId() );
            Coordinates highBoundary = SystemCatalog::getInstance()->getHighBoundary ( arrayDesc->getId() );

            /* Array bounds might be unknown und thus equal maximum int64 values.
            The followoing loop tries to find better values based on dimension settings */
            for ( size_t i = 0; i < dims.size(); ++i ) {
                if ( abs ( lowBoundary[i] ) == SCIDB_MAXDIMINDEX ) {
                    lowBoundary[i] = dims[i].getCurrStart();
                    if ( abs ( lowBoundary[i] ) == SCIDB_MAXDIMINDEX ) lowBoundary[i] = dims[i].getStartMin();
                }
                if ( abs ( highBoundary[i] ) == SCIDB_MAXDIMINDEX ) {
                    highBoundary[i] = dims[i].getCurrEnd();
                    if ( abs ( highBoundary[i] ) == SCIDB_MAXDIMINDEX ) highBoundary[i] = dims[i].getEndMax();
                }
            }


            boost::shared_ptr<TupleArray> tuples ( boost::make_shared<TupleArray> ( _schema, _arena ) );
            Value tuple[10];
            tuple[0].setString ( arrayName );
            stringstream setting;


            if ( info_s.size() == 1 ) {
                setting << "s";
                // Transform all 4 corners
                stringstream sd;
                sd << "Bounds X:(" << lowBoundary[xdim_idx] << "," << highBoundary[xdim_idx] << ")\n";
                sd << "Bounds Y:(" << lowBoundary[ydim_idx] << "," << highBoundary[ydim_idx] << ")\n";
                SCIDB4GEO_DEBUG ( sd.str() );

                AffineTransform::double2 ps1_world = info_s[0].A.f ( AffineTransform::double2 ( lowBoundary[xdim_idx],  lowBoundary[ydim_idx] ) );
                AffineTransform::double2 ps2_world = info_s[0].A.f ( AffineTransform::double2 ( lowBoundary[xdim_idx],  highBoundary[ydim_idx] ) );
                AffineTransform::double2 ps3_world = info_s[0].A.f ( AffineTransform::double2 ( highBoundary[xdim_idx], lowBoundary[ydim_idx] ) );
                AffineTransform::double2 ps4_world = info_s[0].A.f ( AffineTransform::double2 ( highBoundary[xdim_idx], highBoundary[ydim_idx] ) );

                // Find minimum and maximum of transformed corners
                double x[4] = {ps1_world.x, ps2_world.x, ps3_world.x, ps4_world.x};
                double y[4] = {ps1_world.y, ps2_world.y, ps3_world.y, ps4_world.y};
                AffineTransform::double2 lowleft ( utils::min ( x, 4 ), utils::min ( y, 4 ) );
                AffineTransform::double2 upright ( utils::max ( x, 4 ), utils::max ( y, 4 ) );

                tuple[2].setDouble ( lowleft.x );
                tuple[3].setDouble ( upright.x );
                tuple[4].setDouble ( lowleft.y );
                tuple[5].setDouble ( upright.y );
            }

            if ( info_t.size() == 1 ) {
                setting << "t";
                stringstream sd;
                sd << "Bounds T:(" << lowBoundary[tdim_idx] << "," << highBoundary[tdim_idx] << ")";
                SCIDB4GEO_DEBUG ( sd.str() );
                tuple[6].setString ( info_t[0].tref->datetimeAtIndex ( lowBoundary[tdim_idx] ).toStringISO() );
                tuple[7].setString ( info_t[0].tref->datetimeAtIndex ( highBoundary[tdim_idx] ).toStringISO() );
                delete info_t[0].tref;

            }
            // Add vertical...

            tuple[1].setString ( setting.str() );
            tuples->appendTuple ( tuple );


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

    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalExtent, "eo_extent", "PhysicalExtent" );
    typedef PhysicalExtent PhysicalExtent_depr;
    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalExtent_depr, "st_extent", "PhysicalExtent_depr" ); // Backward compatibility
} //namespace

