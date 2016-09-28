/*
This file has been originally based on source code of SciDB (examples/example_udos/LogicalHelloInstances.cpp)
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

#include "../plugin.h"  // Must be first to define PROJECT_ROOT

#include "log4cxx/logger.h"

#include "query/Operator.h"
#include "system/Exceptions.h"
#include "system/SystemCatalog.h"

namespace scidb4geo {

    using namespace std;
    using namespace scidb;

    class LogicalSetMD : public LogicalOperator {
       public:
        LogicalSetMD(const string &logicalName, const string &alias) : LogicalOperator(logicalName, alias) {
            _properties.exclusive = true;
            _properties.ddl = true;

            ADD_PARAM_IN_ARRAY_NAME()  // Arrayname will be stored in _parameters[0]
            ADD_PARAM_CONSTANT(TID_STRING)
            ADD_PARAM_CONSTANT(TID_STRING)
            ADD_PARAM_VARIES()
        }

        vector<std::shared_ptr<OperatorParamPlaceholder> > nextVaryParamPlaceholder(const vector<ArrayDesc> &schemas) {
            vector<std::shared_ptr<OperatorParamPlaceholder> > res;

            // Par 4: String or integer (either affine_transform_string or auth_srid)
            if (_parameters.size() == 3) {
                res.push_back(PARAM_CONSTANT(TID_STRING));
            }
            res.push_back(END_OF_VARIES_PARAMS());

            return res;
        }

        ArrayDesc inferSchema(std::vector<ArrayDesc> schemas, std::shared_ptr<Query> query) {
            assert(schemas.size() == 0);
            assert(_parameters.size() == 3 || _parameters.size() == 4);
            assert(_parameters[0]->getParamType() == PARAM_ARRAY_REF);

            ArrayDesc schema;
            schema.setDistribution(defaultPartitioning());
            schema.setResidency(query->getDefaultArrayResidency());
            return schema;
        }
    };

    REGISTER_LOGICAL_OPERATOR_FACTORY(LogicalSetMD, "eo_setmd");
}
