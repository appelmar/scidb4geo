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

    static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("scidb4geo.setSpatialRef"));

    /**
     * @brief SciDB Operator eo_cpsrs().
     *
     * @par Synopsis:
     *   eo_cpsrs(source,target)
     * @par Summary:
     *   Copies spatial reference from source to target array. Both arrays must be named.
     * @par Input:
     *   - source: source array with spatial reference
     *   - target: target array to be updated with source's spatial reference information
     * @par Output array:
     *   Returns the target array
     *
     * @par Examples:
     *  eo_cpsrs(a,b);
     *
     * @par Errors:
     *   n/a
     *
     * @par Notes:
     *   The target array must contain two dimensions equal to the source's spatial dimensions. Matching is done by name, i.e. the total number of dimensions can be different.
     *
     *
     */
    class LogicalCpSRS : public LogicalOperator {
       public:
        LogicalCpSRS(const string &logicalName, const string &alias) : LogicalOperator(logicalName, alias) {
            _properties.tile = true;

            ADD_PARAM_IN_ARRAY_NAME2(PLACEHOLDER_ARRAY_NAME_VERSION | PLACEHOLDER_ARRAY_NAME_INDEX_NAME)  // Arrayname will be stored in _parameters[0]
            ADD_PARAM_IN_ARRAY_NAME2(PLACEHOLDER_ARRAY_NAME_VERSION | PLACEHOLDER_ARRAY_NAME_INDEX_NAME)  // Arrayname will be stored in _parameters[1]
        }

        void inferArrayAccess(std::shared_ptr<Query> &query) {
            cout << "CALLED SetSpatialRefLogical::inferArrayAccess" << endl;
            LogicalOperator::inferArrayAccess(query);

            assert(_parameters.size() == 2);
            assert(_parameters[0]->getParamType() == PARAM_ARRAY_REF && _parameters[1]->getParamType() == PARAM_ARRAY_REF);

            const string &arrayNameFrom = ((std::shared_ptr<OperatorParamReference> &)_parameters[0])->getObjectName();
            const string &arrayNameTo = ((std::shared_ptr<OperatorParamReference> &)_parameters[1])->getObjectName();

            assert(arrayNameFrom.find('@') == std::string::npos);
            assert(arrayNameTo.find('@') == std::string::npos);

            ArrayDesc descFrom;
            ArrayDesc descTo;

            //             SystemCatalog::getInstance()->getArrayDesc ( arrayNameFrom, descFrom );
            //             SystemCatalog::getInstance()->getArrayDesc ( arrayNameTo, descTo );

            SystemCatalog::getInstance()->getArrayDesc(arrayNameFrom, query->getCatalogVersion(arrayNameFrom), LAST_VERSION, descFrom);
            SystemCatalog::getInstance()->getArrayDesc(arrayNameTo, query->getCatalogVersion(arrayNameTo), LAST_VERSION, descTo);

            if (descTo.isTransient()) {
                std::shared_ptr<SystemCatalog::LockDesc> lock(std::make_shared<SystemCatalog::LockDesc>(arrayNameTo,
                                                                                                        query->getQueryID(),
                                                                                                        Cluster::getInstance()->getLocalInstanceId(),
                                                                                                        SystemCatalog::LockDesc::COORD,
                                                                                                        SystemCatalog::LockDesc::WR));
                std::shared_ptr<SystemCatalog::LockDesc> resLock(query->requestLock(lock));

                assert(resLock);
                assert(resLock->getLockMode() >= SystemCatalog::LockDesc::WR);
            } else {
                LogicalOperator::inferArrayAccess(query);  // take read lock as per usual
            }
        }

        ArrayDesc inferSchema(std::vector<ArrayDesc> schemas, std::shared_ptr<Query> query) {
            cout << "CALLED SetSpatialRefLogical::inferSchema" << endl;
            assert(schemas.size() == 0);
            assert(_parameters.size() == 2);
            assert(_parameters[0]->getParamType() == PARAM_ARRAY_REF && _parameters[1]->getParamType() == PARAM_ARRAY_REF);

            shared_ptr<OperatorParamArrayReference> &arrayFromRef = (shared_ptr<OperatorParamArrayReference> &)_parameters[0];
            shared_ptr<OperatorParamArrayReference> &arrayToRef = (shared_ptr<OperatorParamArrayReference> &)_parameters[1];

            assert(arrayFromRef->getArrayName().find('@') == string::npos);
            assert(arrayFromRef->getObjectName().find('@') == string::npos);
            assert(arrayToRef->getArrayName().find('@') == string::npos);
            assert(arrayToRef->getObjectName().find('@') == string::npos);

            ArrayDesc schema;

            return schema;
        }
    };

    REGISTER_LOGICAL_OPERATOR_FACTORY(LogicalCpSRS, "eo_cpsrs");
    typedef LogicalCpSRS LogicalCpSRS_depr;
    REGISTER_LOGICAL_OPERATOR_FACTORY(LogicalCpSRS_depr, "st_cpsrs");  // Backward compatibility
}
