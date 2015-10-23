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

#include <string>

#include "query/Operator.h"
#include "array/TupleArray.h"
#include "system/SystemCatalog.h"

#include "../PostgresWrapper.h"

namespace scidb4geo
{
    using namespace std;
    using namespace boost;
    using namespace scidb;

    /*! @copydoc LogicalBBOX
     */
    class PhysicalBBOX: public PhysicalOperator
    {
    public:
        PhysicalBBOX ( string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema ) :
            PhysicalOperator ( logicalName, physicalName, parameters, schema ) {
        }

        virtual ArrayDistribution getOutputDistribution ( const std::vector<ArrayDistribution> &inputDistributions,
                const std::vector< ArrayDesc> &inputSchemas ) const {
            return ArrayDistribution ( psLocalInstance );
        }


        boost::shared_ptr<Array> execute ( vector< boost::shared_ptr<Array> > &inputArrays, boost::shared_ptr<Query> query ) {
            assert ( _parameters.size() == 1 ); // TODO: Add parameter for WGS84 transform, would require proj4



            const string &arrayName = ( ( boost::shared_ptr<OperatorParamReference> & ) _parameters[0] )->getObjectName();
            vector<string> v;
            string s ( arrayName );
            v.push_back ( s );
            vector<SpatialArrayInfo> info = PostgresWrapper::instance()->dbGetSpatialRef ( v ) ;

            ArrayID arrayId = SystemCatalog::getInstance()->findArrayByName ( arrayName );
            boost::shared_ptr<ArrayDesc> arrayDesc = SystemCatalog::getInstance()->getArrayDesc ( arrayId );
            Dimensions dims = arrayDesc->getDimensions();
            DimensionDesc d;
            int xdim_idx = -1;
            int ydim_idx = -1;

            // Find spatial dimensions
            for ( size_t i = 0; i < dims.size(); ++i ) {
                d = dims[i];
                if ( d.getBaseName().compare ( info[0].xdim ) == 0 ) xdim_idx = i;
                if ( d.getBaseName().compare ( info[0].ydim ) == 0 ) ydim_idx = i;
            }

            if ( xdim_idx < 0 || ydim_idx < 0 ) {
                throw PLUGIN_USER_EXCEPTION ( "libscidb4geo", SCIDB_SE_UDO, scidb4geo::SCIDB4GEO_ERR_INCONSISTENT_DBSTATE ) << " One or both spatial dimensions of array '" << arrayName << "' not existing";

            }

            /*
             cout << "X: " << dims[xdim_idx].getCurrStart() << "    " << dims[xdim_idx].getStartMin() << "    " << dims[xdim_idx].getCurrEnd() << "    " << dims[xdim_idx].getEndMax() << endl;
             cout << "Y: " << dims[ydim_idx].getCurrStart() << "    " << dims[ydim_idx].getStartMin() << "    " << dims[ydim_idx].getCurrEnd() << "    " << dims[ydim_idx].getEndMax() << endl;
             */

            EOExtentInfo bbox;
            // apply affine transform
            // TODO: This must be tested: getCurrStart vs getStartMin etc, assert xmin<xmax, ...
            // Is it enough to take only 2 points (xmin,ymin) and (xmax,ymax) or must (xmin,ymax), (xmax,ymin) also be included and aggregated by min afterwards?
            bbox.xmin = info[0].A._x0 +  info[0].A._a11 *  dims[xdim_idx].getStartMin()  +  info[0].A._a12 *  dims[ydim_idx].getStartMin();
            bbox.ymin = info[0].A._y0 +  info[0].A._a21 *  dims[xdim_idx].getStartMin()  +  info[0].A._a22 *  dims[ydim_idx].getStartMin();
            bbox.xmax = info[0].A._x0 +  info[0].A._a11 *  dims[xdim_idx].getEndMax()  +  info[0].A._a12 *  dims[ydim_idx].getEndMax();
            bbox.ymax = info[0].A._y0 +  info[0].A._a21 *  dims[xdim_idx].getEndMax()  +  info[0].A._a22 *  dims[ydim_idx].getEndMax();


            boost::shared_ptr<MemArray> out ( boost::make_shared<MemArray> ( _schema, query ) );
            boost::shared_ptr<ArrayIterator> outputArrayIter = out->getIterator ( 0 );
            Coordinates chunkCoord ( 2 ); // -> vector [0,0]
            chunkCoord[0] = 0;
            chunkCoord[1] = 0;

            boost::shared_ptr<ChunkIterator> outputChunkIter = outputArrayIter->newChunk ( chunkCoord ).getIterator ( query, ChunkIterator::SEQUENTIAL_WRITE );
            outputChunkIter->setPosition ( chunkCoord );

            Value value[4];

            value[0].setDouble ( bbox.xmin );
            value[1].setDouble ( bbox.xmax );
            value[2].setDouble ( bbox.ymin );
            value[3].setDouble ( bbox.ymax );


            outputChunkIter->writeItem ( value[0] );
            outputChunkIter->operator++();
            outputChunkIter->writeItem ( value[1] );
            outputChunkIter->operator++();
            outputChunkIter->writeItem ( value[2] );
            outputChunkIter->operator++();
            outputChunkIter->writeItem ( value[3] );


            /* Finish writing the chunk. After this call, outputChunkIter is invalidated. */
            outputChunkIter->flush();

            return out;
        }


    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalBBOX, "eo_bbox", "PhysicalBBOX" );
    typedef PhysicalBBOX PhysicalBBOX_depr;
    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalBBOX_depr, "st_bbox", "PhysicalBBOX_depr" ); // Backward compatibility
} //namespace

