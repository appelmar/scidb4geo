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
#include "array/DBArray.h"
#include "array/Metadata.h"
#include "array/TransientCache.h"
#include "query/Operator.h"
#include "system/SystemCatalog.h"

#include <log4cxx/logger.h>

#include "../ErrorCodes.h"
#include "../PostgresWrapper.h"

#include "../TemporalReference.h"

namespace scidb4geo {
    using namespace std;
    using namespace scidb;

    static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("scidb4geo.eo_settrs"));

    /*! @copydoc LogicalSetTRS
     */
    class PhysicalSetTRS : public PhysicalOperator {
       public:
        PhysicalSetTRS(const string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema) : PhysicalOperator(logicalName, physicalName, parameters, schema) {
            _arrayName = ((std::shared_ptr<OperatorParamReference> &)parameters[0])->getObjectName();
        }

        virtual void preSingleExecute(shared_ptr<Query> query) {
            // Construct SRS object out of parameters

            TemporalArrayInfo trs;
            shared_ptr<OperatorParamArrayReference> &arrayRef = (shared_ptr<OperatorParamArrayReference> &)_parameters[0];
            query->getNamespaceArrayNames(arrayRef->getObjectName(), _namespaceName, _arrayName);

            if (_parameters.size() == 2) {
                string namespace2;
                string array2;
                shared_ptr<OperatorParamArrayReference> &arrayRef2 = (shared_ptr<OperatorParamArrayReference> &)_parameters[1];
                query->getNamespaceArrayNames(arrayRef2->getObjectName(), namespace2, array2);
                trs = PostgresWrapper::instance()->dbGetTemporalRefOrEmpty(ArrayDesc::makeUnversionedName(array2),namespace2);
            } else {
                trs.tdim = ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[1])->getExpression()->evaluate().getString();
                trs.tref = new TReference(((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[2])->getExpression()->evaluate().getString(),
                                          ((std::shared_ptr<OperatorParamPhysicalExpression> &)_parameters[3])->getExpression()->evaluate().getString());
            }

            // Check whether dimension exists
            ArrayID arrayID = query->getCatalogVersion(_namespaceName, _arrayName);
            ArrayDesc arrayDesc;
            SystemCatalog::getInstance()->getArrayDesc(arrayID, arrayDesc);
            //             ArrayID arrayId = SystemCatalog::getInstance()->findArrayByName ( _arrayName );
            //             std::shared_ptr<ArrayDesc> arrayDesc = SystemCatalog::getInstance()->getArrayDesc ( arrayId );
            Dimensions dims = arrayDesc.getDimensions();
            bool dimOK = false;
            for (size_t i = 0; i < dims.size(); ++i) {
                if (dims[i].getBaseName().compare(trs.tdim) == 0) {
                    dimOK = true;
                    break;
                }
            }

            if (!dimOK) {
                stringstream serr;
                serr << "Dimension " << trs.tdim << " does not exist in target array.";
                SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_UNDECLARED_DIMENSION);
            }

            //TODO: Add some checks

            PostgresWrapper::instance()->dbSetTemporalRef(_arrayName,_namespaceName, trs.tdim, trs.tref->getStart().toStringISO(), trs.tref->getCellsize().toStringISO());

            delete trs.tref;
        }

        std::shared_ptr<Array> execute(std::vector<std::shared_ptr<Array> > &inputArrays,
                                       std::shared_ptr<Query> query) {
            return std::shared_ptr<Array>();
        }

       private:
        string _arrayName;
        string _namespaceName;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalSetTRS, "eo_settrs", "PhysicalSetTRS");
    typedef PhysicalSetTRS PhysicalSetTRS_depr;
    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalSetTRS_depr, "st_settrs", "PhysicalSetTRS_depr");  // Backward compatibility
}
