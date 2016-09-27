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

#include "../plugin.h"  // Must be first to define PROJECT_ROOT

#include <string>

#include "array/TupleArray.h"
#include "query/Operator.h"
#include "system/SystemCatalog.h"

#include "../PostgresWrapper.h"

namespace scidb4geo {
    using namespace std;
    using namespace boost;
    using namespace scidb;

    /*! @copydoc LogicalRegNewSRS
     */
    class PhysicalRegNewSRS : public PhysicalOperator {
       public:
        PhysicalRegNewSRS(string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema) : PhysicalOperator(logicalName, physicalName, parameters, schema) {
        }

        virtual RedistributeContext getOutputDistribution(const std::vector<RedistributeContext> &inputDistributions,
                                                          const std::vector<ArrayDesc> &inputSchemas) const {
            return RedistributeContext(psLocalInstance);
        }

        void preSingleExecute(std::shared_ptr<Query> query) {
            assert(_parameters.size() == 4);

            SRSInfo info;
            info.auth_name = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[0])->getExpression()->evaluate().getString();
            info.auth_srid = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[1])->getExpression()->evaluate().getInt32();
            info.srtext = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[2])->getExpression()->evaluate().getString();
            info.proj4text = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[3])->getExpression()->evaluate().getString();

            PostgresWrapper::instance()->dbRegNewSRS(info);
            std::shared_ptr<TupleArray> tuples(std::make_shared<TupleArray>(_schema, _arena));

            Value tuple[4];
            tuple[0].setString(info.auth_name);
            tuple[1].setInt32((int32_t)info.auth_srid);
            tuple[2].setString(info.srtext);
            tuple[3].setString(info.proj4text);
            tuples->appendTuple(tuple);
            _result = tuples;
        }

        std::shared_ptr<Array> execute(vector<std::shared_ptr<Array> > &inputArrays, std::shared_ptr<Query> query) {
            // Returns an in-memory array
            assert(inputArrays.size() == 0);
            if (!_result) {
                _result = std::make_shared<MemArray>(_schema, query);
            }
            return _result;
        }

       private:
        std::shared_ptr<Array> _result;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalRegNewSRS, "eo_regnewsrs", "PhysicalRegNewSRS");
    typedef PhysicalRegNewSRS PhysicalRegNewSRS_depr;
    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalRegNewSRS_depr, "st_regnewsrs", "PhysicalRegNewSRS_depr");  // Backward compatibility
}
