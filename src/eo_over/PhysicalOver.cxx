/*
This file has been originally based on source code of SciDB (src/query/ops/build/PhysicalBuild.cpp)
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

#include "query/Operator.h"
#include "array/Metadata.h"
#include "OverArray.h"
//#include "query/ops/input/InputArray.h"

using namespace std;
using namespace boost;
using namespace scidb;

namespace scidb4geo
{
    class PhysicalOver: public PhysicalOperator
    {
    public:
        PhysicalOver ( const string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema ) :
            PhysicalOperator ( logicalName, physicalName, parameters, schema )
        { }

        virtual PhysicalBoundaries getOutputBoundaries(const std::vector<PhysicalBoundaries> & inputBoundaries, const std::vector< ArrayDesc> & inputSchemas) const
        {
            return inputBoundaries[0];
        }
        
 

        std::shared_ptr<Array> execute ( vector< std::shared_ptr<Array> > &inputArrays, std::shared_ptr<Query> query ) {
            assert ( inputArrays.size() == 2 );
            assert ( _parameters.size() == 0 );
            return std::shared_ptr<Array> ( new OverArray ( query, inputArrays[0]->getArrayDesc(), inputArrays[1]->getArrayDesc(), _schema ) );
        }
    };
    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalOver, "eo_over", "PhysicalOver" );
}
