/*
This file has been originally based on source code of SciDB (src/query/ops/apply/ApplyArray.cpp)
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
#include "array/Metadata.h"
#include "array/Array.h"
#include "CoordsArray.h"
#include "../PostgresWrapper.h"
#include "../AffineTransform.h"

namespace scidb4geo
{

using namespace boost;
using namespace std;
using namespace scidb;

//
// Coords chunk iterator methods
//
inline bool CoordsChunkIterator::isNull()
{
    //SCIDB4GEO_DEBUG("CoordsChunkIterator::isNull()");
    return false;
}

void CoordsChunkIterator::reset()
{
    //SCIDB4GEO_DEBUG("CoordsChunkIterator::reset()");
    _applied = false;
    inputIterator->reset();
    if (!inputIterator->end())
    {
//         for (size_t i = 0, n = _iterators.size(); i < n; i++)
//         {
//             if (_iterators[i] && _iterators[i] != inputIterator)
//             {
//                 _iterators[i]->reset();
//             }
//         }
        if (isNull())
        {
            ++(*this);
        }
    }
}

bool CoordsChunkIterator::setPosition(Coordinates const& pos)
{
     //SCIDB4GEO_DEBUG("CoordsChunkIterator::setPosition()");
    _applied = false;
    if (inputIterator->setPosition(pos))
    {
//         for (size_t i = 0, n = _iterators.size(); i < n; i++)
//         {
//             if (_iterators[i])
//             {
//                 if (!_iterators[i]->setPosition(pos))
//                     throw USER_EXCEPTION(SCIDB_SE_EXECUTION, SCIDB_LE_OPERATION_FAILED) << "setPosition";
//             }
//         }
        return !isNull();
    }
    return false;
}

Value& CoordsChunkIterator::getItem()
{
//      SCIDB4GEO_DEBUG("CoordsChunkIterator::getItem()");
//      SCIDB4GEO_DEBUG("attrid:"); 
//      stringstream ss;
//      ss <<  _outAttrId;
//      
//      SCIDB4GEO_DEBUG(ss.str().c_str()); 
     
     if (!_applied)
     {

        AttributeID attrID = _outAttrId;
        
        
        
        Coordinates p_cur = inputIterator->getPosition();
        
        // X
        if ( attrID ==  ( _array.getArrayDesc().getAttributes().size()-3)) {
            if ( _array._xidx >=  0) {
                 AffineTransform::double2 p( p_cur[_array._xidx],   p_cur[_array._yidx]);
                 _array._srs.A.f ( p);
                _value.setDouble( p.x );
                
            }
            else {
                _value.setNull();
            }
        }

        // Y
        else if ( attrID == ( _array.getArrayDesc().getAttributes().size()-2 )) {
             if ( _array._yidx >=  0) {
                AffineTransform::double2 p( p_cur[_array._xidx],   p_cur[_array._yidx]);
                 _array._srs.A.f ( p);
                _value.setDouble( p.y );
            }
            else {
                _value.setNull();
            }
        }

        // T
        else if (  attrID ==  ( _array.getArrayDesc().getAttributes().size()-1 )) {
            if ( _array._tidx >=  0) {
                TPoint ptin = _array._trs.tref->datetimeAtIndex ( p_cur[_array._tidx] );
                _value.setString( ptin.toStringISO() );
            }
            else {
                _value.setNull();
            }
        }
        
//         else {
//              _value = _true_val;      
//         }
        _applied = true;

     }
    return _value;
}

void CoordsChunkIterator::operator ++()
{
    //SCIDB4GEO_DEBUG("CoordsChunkIterator::operator ++()");
    do
    {
        _applied = false;
        ++(*inputIterator);
        if (!inputIterator->end())
        {
//             for (size_t i = 0, n = _iterators.size(); i < n; i++)
//             {
//                 if (_iterators[i] && _iterators[i] != inputIterator)
//                 {
//                     ++(*_iterators[i]);
//                 }
//             }
        }
        else
        {
            break;
        }
    } while (isNull());
}

CoordsChunkIterator::CoordsChunkIterator(CoordsArrayIterator const& arrayIterator, DelegateChunk const* chunk, int iterationMode) :
    DelegateChunkIterator(chunk, iterationMode & ~(INTENDED_TILE_MODE | IGNORE_NULL_VALUES | IGNORE_DEFAULT_VALUES)),
    _array((CoordsArray&) arrayIterator.array),
    _outAttrId(arrayIterator.attr),
    //_bindings(_array._bindingSets[_outAttrId]),
   // _iterators(_bindings.size()),
    //_params(*_array._expressions[_outAttrId]),
    _mode(iterationMode),
    _applied(false),
    //_nullable(_array._attributeNullable[_outAttrId]),
    _query(Query::getValidQueryPtr(_array._query))
{
    SCIDB4GEO_DEBUG("CoordsChunkIterator::CoordsChunkIterator()");
    _true_val.setBool(true);
    
    stringstream ss;
    ss <<  "Attributes: " <<  _array.getArrayDesc().getAttributes().size();
    ss <<  " xidx=" <<  _array._xidx <<  " yidx=" <<  _array._yidx << " tidx=" <<  _array._tidx;
    SCIDB4GEO_DEBUG(ss.str().c_str());
    
    
    if (_outAttrId == _array._desc_out.getAttributes().size()-3 || _outAttrId == _array._desc_out.getAttributes().size()-2)
        _value = Value(TypeLibrary::getType(TID_DOUBLE));
    else if (_outAttrId == _array._desc_out.getAttributes().size()-1)
        _value = Value(TypeLibrary::getType(TID_STRING));
        
       //reset();
    
//     for (size_t i = 0, n = _bindings.size(); i < n; i++)
//     {
//         switch (_bindings[i].kind)
//         {
//             case BindInfo::BI_COORDINATE:
//                 if (_mode & TILE_MODE)
//                 {
//                     if (arrayIterator.iterators[i] == arrayIterator.getInputIterator())
//                     {
//                         _iterators[i] = inputIterator;
//                     }
//                     else
//                     {
//                         _iterators[i] = arrayIterator.iterators[i]->getChunk().getConstIterator(TILE_MODE | IGNORE_EMPTY_CELLS);
//                     }
//                 }
//                 break;
//             case BindInfo::BI_ATTRIBUTE:
//                 if ((AttributeID) _bindings[i].resolvedId == arrayIterator.inputAttrID)
//                 {
//                     _iterators[i] = inputIterator;
//                 }
//                 else
//                 {
//                     _iterators[i] = arrayIterator.iterators[i]->getChunk().getConstIterator(inputIterator->getMode());
//                 }
//                 break;
//             case BindInfo::BI_VALUE:
//                 _params[i] = _bindings[i].value;
//                 break;
//             default:
//                 break;
//         }
//     }
    if (isNull())
    {
        ++(*this);
    }
}

//
// Coords array iterator methods
//

bool CoordsArrayIterator::setPosition(Coordinates const& pos)
{
    SCIDB4GEO_DEBUG("CoordsArrayIterator::setPosition()");
    if (inputIterator->setPosition(pos))
    {
//         for (size_t i = 0, n = iterators.size(); i < n; i++)
//         {
//             if (iterators[i])
//             {
//                 if (!iterators[i]->setPosition(pos))
//                     throw USER_EXCEPTION(SCIDB_SE_EXECUTION, SCIDB_LE_OPERATION_FAILED) << "setPosition";
//             }
//         }
        return true;
    }
    return false;
}

void CoordsArrayIterator::reset()
{
    SCIDB4GEO_DEBUG("CoordsArrayIterator::reset()");
    inputIterator->reset();
//     for (size_t i = 0, n = iterators.size(); i < n; i++)
//     {
//         if (iterators[i] && iterators[i] != inputIterator)
//         {
//             iterators[i]->reset();
//         }
//     }
}

void CoordsArrayIterator::operator ++()
{
    SCIDB4GEO_DEBUG("CoordsArrayIterator::operator ++()");
    ++(*inputIterator);
//     for (size_t i = 0, n = iterators.size(); i < n; i++)
//     {
//         if (iterators[i] && iterators[i] != inputIterator)
//         {
//             ++(*iterators[i]);
//         }
//     }
}

CoordsArrayIterator::CoordsArrayIterator(CoordsArray const& array, AttributeID outAttrID, AttributeID inAttrID) :
    DelegateArrayIterator(array, outAttrID, array.getInputArray()->getConstIterator(inAttrID)),
    inputAttrID(inAttrID)
{
     SCIDB4GEO_DEBUG("CoordsArrayIterator::CoordsArrayIterator()");
}

//
// Coords array methods
//

DelegateChunkIterator* CoordsArray::createChunkIterator(DelegateChunk const* chunk, int iterationMode) const
{
   // StatisticsScope sScope(_statistics);
    CoordsArrayIterator const& arrayIterator = (CoordsArrayIterator const&) chunk->getArrayIterator();
    AttributeDesc const& attr = chunk->getAttributeDesc();
    AttributeID attId = attr.getId();

    if (chunk->inTileMode())
    {
        if ((iterationMode & ChunkIterator::INTENDED_TILE_MODE))
        {
            iterationMode |= ChunkIterator::TILE_MODE;
        }
    }
    else
    {
        iterationMode &= ~ChunkIterator::TILE_MODE;
    }
    DelegateChunkIterator* res = (attId >= inputArray->getArrayDesc().getAttributes().size()) ? (DelegateChunkIterator*) new CoordsChunkIterator(arrayIterator, chunk, iterationMode)
                                                             : DelegateArray::createChunkIterator(chunk, iterationMode);
    return res;
}

DelegateArrayIterator* CoordsArray::createArrayIterator(AttributeID attrID) const
{
    AttributeID inputAttrID;

    if ( attrID >= inputArray->getArrayDesc().getAttributes().size() )
    {
        //vector<BindInfo> const& bindings = _bindingSets[attrID];
        inputAttrID = 0;

//         for (size_t i = 0, n = bindings.size(); i < n; i++)
//         {
//             if (bindings[i].kind == BindInfo::BI_ATTRIBUTE)
//             {
//                 inputAttrID = (AttributeID) bindings[i].resolvedId;
//                 break;
//             }
//         }
    }
    else if (desc.getEmptyBitmapAttribute() && attrID == desc.getEmptyBitmapAttribute()->getId() )
    {
        inputAttrID = inputArray->getArrayDesc().getEmptyBitmapAttribute()->getId();
    }
    else
    {
        inputAttrID = attrID;
    }

    return new CoordsArrayIterator(*this, attrID, inputAttrID);
}




DelegateChunk* CoordsArray::createChunk(DelegateArrayIterator const* iterator, AttributeID attrID) const
{
    // A chunk is a clone if the attribute is already in the input array
    bool isClone = attrID < inputArray->getArrayDesc().getAttributes().size();
    DelegateChunk* chunk = new DelegateChunk(*this, *iterator, attrID, isClone);
    chunk->overrideTileMode(_runInTileMode);
    return chunk;
}



CoordsArray::CoordsArray( std::shared_ptr<Query> &query,  std::shared_ptr<Array> const& array_in, ArrayDesc const &desc_out, bool tile ) :
DelegateArray(desc_out, array_in), 
_xidx ( -1 ),  _yidx ( -1 ), _tidx ( -1 ),  _runInTileMode(tile)
{
     _query = query;
     _ninstances = query->getInstancesCount();
     _instance_id = query->getInstanceID();
     
     _srs = PostgresWrapper::instance()->dbGetSpatialRefOrEmpty ( ArrayDesc::makeUnversionedName(array_in->getName())  );
     if (_srs.arrayname.empty()) {
         SCIDB4GEO_DEBUG ( "eo_coords: array is not spatially referenced, ignoring space." );
     }
     else {
         DimensionDesc d;
        for ( uint32_t i = 0; i < array_in->getArrayDesc().getDimensions().size(); ++i ) {
            d = array_in->getArrayDesc().getDimensions() [i];
            if ( d.getBaseName().compare ( _srs.xdim ) == 0 ) _xidx = i;
            else if ( d.getBaseName().compare ( _srs.ydim ) == 0 ) _yidx = i;
        } 
     }
     
     
      _trs = PostgresWrapper::instance()->dbGetTemporalRefOrEmpty( ArrayDesc::makeUnversionedName(array_in->getName()) );
      if (_trs.arrayname.empty()) {
          SCIDB4GEO_DEBUG ( "eo_coords: array is not temporally referenced, ignoring date and time." );
      }
      else {
          DimensionDesc d;
          for ( uint32_t i = 0; i < array_in->getArrayDesc().getDimensions().size(); ++i ) {
              d = array_in->getArrayDesc().getDimensions() [i];
              if ( d.getBaseName().compare ( _trs.tdim ) == 0 ) _tidx = i;
          } 
      }
 
 
 
    if (_trs.arrayname.empty() && _srs.arrayname.empty()) {
        SCIDB4GEO_ERROR("Array is neither spatial nor temporal, cancel eo_coords()",  SCIDB4GEO_ERR_UNKNOWN_SRS);
    }
 
 
}




}
