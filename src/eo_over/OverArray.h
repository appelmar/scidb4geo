/*
This file has been originally based on source code of SciDB (src/query/ops/build/BuildArray.h)
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

#ifndef OVER_ARRAY_H
#define OVER_ARRAY_H

#include <string>
#include <vector>

#include "array/Metadata.h"
#include "array/DelegateArray.h"

#include "../PostgresWrapper.h"
#include "../ErrorCodes.h"

namespace scidb4geo
{

    using namespace std;
    using namespace scidb;
    using namespace boost;

    class OverArray;
    class OverArrayIterator;
    class OverChunkIterator;

    class OverChunk : public ConstChunk
    {
    public:
        virtual const ArrayDesc &getArrayDesc() const;
        virtual const AttributeDesc &getAttributeDesc() const;
        virtual Coordinates const &getFirstPosition ( bool withOverlap ) const;
        virtual Coordinates const &getLastPosition ( bool withOverlap ) const;
        virtual boost::shared_ptr<ConstChunkIterator> getConstIterator ( int iterationMode ) const;
        virtual int getCompressionMethod() const;
        virtual Array const &getArray() const;

        void setPosition ( Coordinates const &pos );

        OverChunk ( OverArray &array, AttributeID attrID );

    private:
        OverArray &array;
        Coordinates p_first;
        Coordinates p_last;
        Coordinates p_first_overlap;
        Coordinates p_last_overlap;
        AttributeID attr_id;
    };

    class OverChunkIterator : public ConstChunkIterator
    {
    public:
        virtual int getMode();
        virtual bool isEmpty();
        virtual Value &getItem();
        virtual void operator ++();
        virtual bool end();
        virtual Coordinates const &getPosition();
        virtual bool setPosition ( Coordinates const &pos );
        virtual void reset();
        ConstChunk const &getChunk();
        virtual boost::shared_ptr<Query> getQuery() {
            return _query;
        }

        OverChunkIterator ( OverArray &array, ConstChunk const *chunk, AttributeID attrID, int iterationMode );

    private:
        int iterationMode;
        OverArray &array;
        Coordinates const &p_first;
        Coordinates const &p_last;
        Coordinates p_cur;
        bool has_cur;
        AttributeID attrID;
        ConstChunk const *chunk;
        Value _value;
        Value _true_val;
        bool _nullable;
        boost::shared_ptr<Query> _query;

        bool _has_x;
        bool _has_y;
        bool _has_t;

        AffineTransform::double2 _p_in;
        AffineTransform::double2 _p_out;
    };

    class OverArrayIterator : public ConstArrayIterator
    {
        friend class OverChunkIterator;

    public:
        virtual ConstChunk const &getChunk();
        virtual bool end();
        virtual void operator ++();
        virtual Coordinates const &getPosition();
        virtual bool setPosition ( Coordinates const &pos );
        virtual void reset();

        OverArrayIterator ( OverArray &array, AttributeID id );

    private:
        void nextChunk();

        OverArray &array;
        bool hasCurrent;
        bool chunkInitialized;
        OverChunk chunk;
        Dimensions const &dims;
        Coordinates currPos;
    };





    class OverArray : public Array
    {
        friend class OverArrayIterator;
        friend class OverChunkIterator;
        friend class OverChunk;

    public:
        OverArray ( boost::shared_ptr<Query> &query, ArrayDesc const &descA, ArrayDesc const &descB, ArrayDesc const &descC );
        virtual ArrayDesc const &getArrayDesc() const;
        virtual boost::shared_ptr<ConstArrayIterator> getConstIterator ( AttributeID attr ) const;

    private:

        ArrayDesc _desc_A, _desc_B, _desc_C;

        int _xidx_A;
        int _xidx_B;
        int _yidx_A;
        int _yidx_B;
        int _tidx_A;
        int _tidx_B;

        SpatialArrayInfo  _srs_A;
        SpatialArrayInfo  _srs_B;
        TemporalArrayInfo _trs_A;
        TemporalArrayInfo _trs_B;

        uint32_t _ninstances;
        uint64_t _instance_id;

    };

}

#endif
