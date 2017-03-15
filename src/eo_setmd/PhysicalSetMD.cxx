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

#include <boost/make_shared.hpp>
#include "array/Metadata.h"
#include "query/Operator.h"

#include "../ErrorCodes.h"
#include "../PostgresWrapper.h"

#include <boost/algorithm/string.hpp>

namespace scidb4geo {
    using namespace std;
    using namespace scidb;

    /*! @copydoc LogicalSetTRS
     */
    class PhysicalSetMD : public PhysicalOperator {
       public:
        PhysicalSetMD(const string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema) : PhysicalOperator(logicalName, physicalName, parameters, schema) {}

        virtual void preSingleExecute(shared_ptr<Query> query) {
            shared_ptr<OperatorParamArrayReference> &arrayRef = (shared_ptr<OperatorParamArrayReference> &)_parameters[0];
            query->getNamespaceArrayNames(arrayRef->getObjectName(), _namespaceName, _arrayName);
            ArrayID arrayID = query->getCatalogVersion(_namespaceName, _arrayName);
            ArrayDesc arrayDesc;
            SystemCatalog::getInstance()->getArrayDesc(arrayID, arrayDesc);

            string keys;
            string vals;

            if (_parameters.size() == 3) {
                // Set array metadata
                keys = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[1])->getExpression()->evaluate().getString();
                vals = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[2])->getExpression()->evaluate().getString();

            } else if (_parameters.size() == 4) {
                // Set attribute metadata
                keys = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[2])->getExpression()->evaluate().getString();
                vals = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[3])->getExpression()->evaluate().getString();
            }

            vector<string> k;
            vector<string> v;
            boost::split(k, keys, boost::is_any_of(",;"));
            boost::split(v, vals, boost::is_any_of(",;"));
            if (k.size() != v.size()) {
                SCIDB4GEO_ERROR("Different number of keys and values for metadata", SCIDB4GEO_ERR_UNKNOWN);
            }

            map<string, string> kv;
            for (unsigned int i = 0; i < k.size(); ++i) {
                kv.insert(std::pair<string, string>(k[i], v[i]));
            }

            if (_parameters.size() == 3) {
                PostgresWrapper::instance()->dbSetArrayMD(_arrayName, _namespaceName, kv);  // TODO: Add domain
            } else if (_parameters.size() == 4) {
                string attrname = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[1])->getExpression()->evaluate().getString();
                PostgresWrapper::instance()->dbSetAttributeMD(_arrayName, _namespaceName, attrname, kv);  // TODO: Add domain
            }
        }

        std::shared_ptr<Array> execute(std::vector<std::shared_ptr<Array> > &inputArrays,
                                       std::shared_ptr<Query> query) {
            return std::shared_ptr<Array>();
        }

       private:
        string _arrayName;
        string _namespaceName;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalSetMD, "eo_setmd", "PhysicalSetMD");
}
