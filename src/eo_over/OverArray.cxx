/*
This file has been originally based on source code of SciDB (src/query/ops/build/BuildArray.cpp)
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

#include "OverArray.h"


namespace scidb4geo
{

    using namespace boost;
    using namespace std;
    using namespace scidb;


    int OverChunkIterator::getMode() const
    {
        return iterationMode;
    }

    Value &OverChunkIterator::getItem()
    {
        if ( !has_cur ) throw USER_EXCEPTION ( SCIDB_SE_EXECUTION, SCIDB_LE_NO_CURRENT_ELEMENT );

        if ( attrID >= 3 )  return _true_val; //TODO: Optionally set cell empty if cell is outside target array

        // X
        if ( attrID == 0 ) {
            if ( _has_x ) {
                _p_in.x = p_cur[array._xidx_A];
                _p_in.y = p_cur[array._yidx_A];
                array._srs_A.A.f ( _p_in );
                array._srs_B.A.fInv ( _p_in, _p_out );
                _value.setInt64 ( round ( _p_out.x ) );
            }
            else {
                _value.setNull();
            }
        }

        // Y
        else if ( attrID == 1 ) {
            if ( _has_y ) {
                _p_in.x = p_cur[array._xidx_A];
                _p_in.y = p_cur[array._yidx_A];
                array._srs_A.A.f ( _p_in );
                array._srs_B.A.fInv ( _p_in, _p_out );
                _value.setInt64 ( round ( _p_out.y ) );
            }
            else {
                _value.setNull();
            }
        }

        // T
        else if ( attrID == 2 ) {
            if ( _has_t ) {
                TPoint ptin = array._trs_A.tref->datetimeAtIndex ( p_cur[array._tidx_A] );
                _value.setInt64 ( round ( array._trs_B.tref->indexAtDatetime ( ptin ) ) );
            }
            else {
                _value.setNull();
            }
        }
        return _value;
    }

    void OverChunkIterator::operator ++()
    {
        if ( !has_cur )
            throw USER_EXCEPTION ( SCIDB_SE_EXECUTION, SCIDB_LE_NO_CURRENT_ELEMENT );
        for ( int i = p_cur.size(); --i >= 0; ) {
            if ( ++p_cur[i] > p_last[i] ) {
                p_cur[i] = p_first[i];
            }
            else {
                has_cur = true;
                return;
            }
        }
        has_cur = false;
    }

    bool OverChunkIterator::end()
    {
        return !has_cur;
    }

    bool OverChunkIterator::isEmpty() const
    {
        return false;
    }

    Coordinates const &OverChunkIterator::getPosition()
    {
        return p_cur;
    }

    bool OverChunkIterator::setPosition ( Coordinates const &pos )
    {
        for ( size_t i = 0, n = p_cur.size(); i < n; i++ ) {
            if ( pos[i] < p_first[i] || pos[i] > p_last[i] ) {
                return has_cur = false;
            }
        }
        p_cur = pos;
        return has_cur = true;
    }

    void OverChunkIterator::reset()
    {
        p_cur = p_first;
        has_cur = true;
    }

    ConstChunk const &OverChunkIterator::getChunk()
    {
        return *chunk;
    }

    OverChunkIterator::OverChunkIterator ( OverArray &outputArray,
                                           ConstChunk const *aChunk,
                                           AttributeID attr, int mode )
        : iterationMode ( mode ),
          array ( outputArray ),
          p_first ( aChunk->getFirstPosition ( ( mode &IGNORE_OVERLAPS ) == 0 ) ),
          p_last ( aChunk->getLastPosition ( ( mode &IGNORE_OVERLAPS ) == 0 ) ),
          p_cur ( p_first.size() ),
          attrID ( attr ),
          chunk ( aChunk ),
          _nullable ( true ),
          _query ( Query::getValidQueryPtr ( array._query ) ),
          _has_x ( outputArray._xidx_A >= 0 && outputArray._xidx_B >= 0 ),
          _has_y ( outputArray._yidx_A >= 0 && outputArray._yidx_B >= 0 ),
          _has_t ( outputArray._tidx_A >= 0 && outputArray._tidx_B >= 0 )
    {
        _true_val.setBool ( true );
        if ( attrID == 0 || attrID == 1 || attrID == 2 ) _value = Value ( TypeLibrary::getType ( TID_INT64 ) );
        if ( attrID == 3 ) _value = Value ( TypeLibrary::getType ( TID_BOOL ) );
        reset();
    }

    // array._xidx_A >= 0 && array._yidx_A >= 0 &&  && array._yidx_B >= 0

    Array const &OverChunk::getArray() const
    {
        return array;
    }

    const ArrayDesc &OverChunk::getArrayDesc() const
    {
        return array._desc_C;
    }

    const AttributeDesc &OverChunk::getAttributeDesc() const
    {
        return array._desc_C.getAttributes() [attr_id];
    }

    Coordinates const &OverChunk::getFirstPosition ( bool withOverlap ) const
    {
        return withOverlap ? p_first_overlap : p_first;
    }

    Coordinates const &OverChunk::getLastPosition ( bool withOverlap ) const
    {
        return withOverlap ? p_last_overlap : p_last;
    }

    std::shared_ptr<ConstChunkIterator> OverChunk::getConstIterator ( int iterationMode ) const
    {
        return std::shared_ptr<ConstChunkIterator> ( new OverChunkIterator ( array, this, attr_id, iterationMode ) );
    }

    int OverChunk::getCompressionMethod() const
    {
        return array._desc_C.getAttributes() [attr_id].getDefaultCompressionMethod();
    }

    void OverChunk::setPosition ( Coordinates const &pos )
    {
        p_first = pos;
        Dimensions const &dims = array._desc_C.getDimensions();
        for ( size_t i = 0, n = dims.size(); i < n; i++ ) {
            p_first_overlap[i] = p_first[i] - dims[i].getChunkOverlap();
            if ( p_first_overlap[i] < dims[i].getStartMin() ) {
                p_first_overlap[i] = dims[i].getStartMin();
            }
            p_last[i] = p_first[i] + dims[i].getChunkInterval() - 1;
            p_last_overlap[i] = p_last[i] + dims[i].getChunkOverlap();
            if ( p_last[i] > dims[i].getEndMax() ) {
                p_last[i] = dims[i].getEndMax();
            }
            if ( p_last_overlap[i] > dims[i].getEndMax() ) {
                p_last_overlap[i] = dims[i].getEndMax();
            }
        }
    }

    OverChunk::OverChunk ( OverArray &arr, AttributeID attr )
        : array ( arr ),
          p_first ( arr._desc_C.getDimensions().size() ),
          p_last ( p_first.size() ),
          p_first_overlap ( p_first.size() ),
          p_last_overlap ( p_first.size() ),
          attr_id ( attr )
    { }


    void OverArrayIterator::operator ++()
    {
        if ( !hasCurrent ) {
            throw USER_EXCEPTION ( SCIDB_SE_EXECUTION, SCIDB_LE_NO_CURRENT_ELEMENT );
        }
        Query::getValidQueryPtr ( array._query );
        nextChunk();
    }

    bool OverArrayIterator::end()
    {
        return !hasCurrent;
    }

    Coordinates const &OverArrayIterator::getPosition()
    {
        if ( !hasCurrent ) {
            throw USER_EXCEPTION ( SCIDB_SE_EXECUTION, SCIDB_LE_NO_CURRENT_ELEMENT );
        }
        return currPos;
    }

    void OverArrayIterator::nextChunk()
    {
      
      chunkInitialized = false;
        while (true) {
            int i = dims.size() - 1;
            while ((currPos[i] += dims[i].getChunkInterval()) > dims[i].getEndMax()) {
                if (i == 0) {
                    hasCurrent = false;
                    return;
                }
                currPos[i] = dims[i].getStartMin();
                i -= 1;
            }
            if (array._desc_C.getPrimaryInstanceId(currPos,array._ninstances) == array._instance_id) {
                hasCurrent = true;
                return;
            }
        } 
    }

    bool OverArrayIterator::setPosition ( Coordinates const &pos )
    {
        Query::getValidQueryPtr ( array._query );
        for ( size_t i = 0, n = currPos.size(); i < n; i++ ) {
            if ( pos[i] < dims[i].getStartMin() || pos[i] > dims[i].getEndMax() ) {
                return hasCurrent = false;
            }
        }
        	
	currPos = pos;
        array._desc_C.getChunkPositionFor(currPos);
        chunkInitialized = false;
        return hasCurrent = array._desc_C.getPrimaryInstanceId(currPos, array._ninstances) == array._instance_id;
	
    }

    void OverArrayIterator::reset()
    {
        Query::getValidQueryPtr ( array._query );
        size_t nDims = currPos.size();
        for ( size_t i = 0; i < nDims; i++ ) {
            currPos[i] = dims[i].getStartMin();
        }
        currPos[nDims - 1] -= dims[nDims - 1].getChunkInterval();
        nextChunk();
    }

    ConstChunk const &OverArrayIterator::getChunk()
    {
        if ( !hasCurrent ) {
            throw USER_EXCEPTION ( SCIDB_SE_EXECUTION, SCIDB_LE_NO_CURRENT_ELEMENT );
        }
        Query::getValidQueryPtr ( array._query );
        if ( !chunkInitialized ) {
            chunk.setPosition ( currPos );
            chunkInitialized = true;
        }
        return chunk;
    }


    OverArrayIterator::OverArrayIterator ( OverArray &arr, AttributeID attrID )
        : array ( arr ),
          chunk ( arr, attrID ),
          dims ( arr._desc_C.getDimensions() ),
          currPos ( dims.size() )
    {
        reset();
    }




    ArrayDesc const &OverArray::getArrayDesc() const
    {
        return _desc_C;
    }

    std::shared_ptr<ConstArrayIterator> OverArray::getConstIterator ( AttributeID attr ) const
    {
        return std::shared_ptr<ConstArrayIterator> ( new OverArrayIterator ( * ( OverArray * ) this, attr ) );
    }



    OverArray::OverArray ( std::shared_ptr<Query> &query, ArrayDesc const &descA, ArrayDesc const &descB, ArrayDesc const &descC )
        :  _desc_A ( descA ),  _desc_B ( descB ),  _desc_C ( descC ),
           _xidx_A ( -1 ), _xidx_B ( -1 ), _yidx_A ( -1 ), _yidx_B ( -1 ), _tidx_A ( -1 ), _tidx_B ( -1 ),
           _ninstances ( 0 ), _instance_id ( INVALID_INSTANCE )
    {
        assert ( query );
        _query = query;
        _ninstances = query->getInstancesCount();
        _instance_id = query->getInstanceID();

        assert ( _ninstances > 0 && _instance_id < _ninstances );

        // Get reference information
        vector<string> nameAB;
        nameAB.push_back ( ArrayDesc::makeUnversionedName ( _desc_A.getName() ) );
        nameAB.push_back ( ArrayDesc::makeUnversionedName ( _desc_B.getName() ) );
        vector <SpatialArrayInfo> srsAB = PostgresWrapper::instance()->dbGetSpatialRef ( nameAB );
        if ( srsAB.size() != 2 ) {
            SCIDB4GEO_DEBUG ( "eo_over: at least one of both arrays is not spatially referenced, ignoring space in overlay operation." );
        }
        else {
            // Result is in arbitrary order, so check whether A or B is first
            if ( srsAB[0].arrayname.compare ( nameAB[0] ) == 0 ) {
                _srs_A = srsAB[0];
                _srs_B = srsAB[1];
            }
            else {
                _srs_A = srsAB[1];
                _srs_B = srsAB[0];
            }

            DimensionDesc d;
            for ( uint32_t i = 0; i < _desc_A.getDimensions().size(); ++i ) {
                d = _desc_A.getDimensions() [i];
                if ( d.getBaseName().compare ( _srs_A.xdim ) == 0 ) _xidx_A = i;
                else if ( d.getBaseName().compare ( _srs_A.ydim ) == 0 ) _yidx_A = i;
            }
            for ( uint32_t i = 0; i < _desc_B.getDimensions().size(); ++i ) {
                d = _desc_B.getDimensions() [i];
                if ( d.getBaseName().compare ( _srs_B.xdim ) == 0 ) _xidx_B = i;
                else if ( d.getBaseName().compare ( _srs_B.ydim ) == 0 ) _yidx_B = i;
            }

            /*  TODO: The following check assumes that both reference systems must have equal authority names and ids. Checking the parameters of a projection
            would be more convinient (e.g. see OGRSpatialReference::IsSame() in GDAL). */
            if ( _srs_A.auth_name.compare ( _srs_B.auth_name ) != 0 || _srs_A.auth_srid != _srs_B.auth_srid ) {
                SCIDB4GEO_ERROR ( "Arrays have different spatial reference systems, this plugin is currently not able to apply reprojection.", SCIDB4GEO_ERR_SRSMISMATCH );
            }
        }


        vector <TemporalArrayInfo> trsAB = PostgresWrapper::instance()->dbGetTemporalRef ( nameAB );
        if ( trsAB.size() != 2 ) {
            SCIDB4GEO_DEBUG ( "eo_over: at least one of both arrays is not temporally referenced, ignoring date and time in overlay operation." );
        }
        else {
            // Result is in arbitrary order, so check whether A or B is first
            if ( trsAB[0].arrayname.compare ( nameAB[0] ) == 0 ) {
                _trs_A = trsAB[0];
                _trs_B = trsAB[1];
            }
            else {
                _trs_A = trsAB[1];
                _trs_B = trsAB[0];
            }

            DimensionDesc d;
            for ( uint32_t i = 0; i < _desc_A.getDimensions().size(); ++i ) {
                d = _desc_A.getDimensions() [i];
                if ( d.getBaseName().compare ( _trs_A.tdim ) == 0 ) _tidx_A = i;
            }
            for ( uint32_t i = 0; i < _desc_B.getDimensions().size(); ++i ) {
                d = _desc_B.getDimensions() [i];
                if ( d.getBaseName().compare ( _trs_B.tdim ) == 0 ) _tidx_B = i;
            }
        }





    }
}
