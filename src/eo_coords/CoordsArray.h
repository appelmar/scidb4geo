/*
This file has been originally based on source code of SciDB (src/query/ops/apply/ApplyArray.h)
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
#ifndef COORDS_ARRAY_H_
#define COORDS_ARRAY_H_

#include <string>
#include <vector>
#include "../PostgresWrapper.h"
#include "array/DelegateArray.h"
#include "array/Metadata.h"
#include "query/Expression.h"
#include "query/LogicalExpression.h"

namespace scidb4geo {

    using namespace std;
    using namespace scidb;

    class CoordsArray;
    class CoordsArrayIterator;
    class CoordsChunkIterator;

    class CoordsChunkIterator : public DelegateChunkIterator {
       public:
        virtual Value& getItem();
        virtual void operator++();
        virtual void reset();
        virtual bool setPosition(Coordinates const& pos);
        CoordsChunkIterator(CoordsArrayIterator const& arrayIterator, DelegateChunk const* chunk, int iterationMode);
        bool isNull();
        virtual std::shared_ptr<Query> getQuery() { return _query; }

       private:
        CoordsArray const& _array;
        AttributeID _outAttrId;
        //std::vector<BindInfo> const& _bindings;
        //std::vector< std::shared_ptr<ConstChunkIterator> > _iterators;
        //ExpressionContext _params;
        int _mode;
        Value _value;
        bool _applied;
        bool _nullable;
        Value _true_val;
        std::shared_ptr<Query> _query;

        AffineTransform::double2 _p_in;
        AffineTransform::double2 _p_out;
    };

    class CoordsArrayIterator : public DelegateArrayIterator {
        friend class CoordsChunkIterator;

       public:
        virtual void operator++();
        virtual void reset();
        virtual bool setPosition(Coordinates const& pos);
        CoordsArrayIterator(CoordsArray const& array, AttributeID attrID, AttributeID inputAttrID);

       private:
        std::vector<std::shared_ptr<ConstArrayIterator> > iterators;
        AttributeID inputAttrID;
    };

    class CoordsArray : public DelegateArray {
        friend class CoordsArrayIterator;
        friend class CoordsChunkIterator;

       public:
        virtual DelegateChunk* createChunk(DelegateArrayIterator const* iterator, AttributeID id) const;
        virtual DelegateChunkIterator* createChunkIterator(DelegateChunk const* chunk, int iterationMode) const;
        virtual DelegateArrayIterator* createArrayIterator(AttributeID id) const;

        /*CoordsArray(ArrayDesc const& desc, std::shared_ptr<Array> const& array,
            std::vector <std::shared_ptr<Expression> > expressions,
            const std::shared_ptr<Query>& query, bool tile);
      */

        CoordsArray(std::shared_ptr<Query>& query, std::shared_ptr<Array> const& array_in, ArrayDesc const& desc_out, bool tile);

       private:
        ArrayDesc _desc_out;

        int _xidx;
        int _yidx;
        int _tidx;
        bool _runInTileMode;
        SpatialArrayInfo _srs;
        TemporalArrayInfo _trs;

        uint32_t _ninstances;
        uint64_t _instance_id;
    };
}

#endif
