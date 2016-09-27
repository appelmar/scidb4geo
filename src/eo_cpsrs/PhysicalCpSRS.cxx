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

#include "../ErrorCodes.h"
#include "../PostgresWrapper.h"

namespace scidb4geo {
    using namespace std;
    using namespace scidb;

    /*! @copydoc LogicalCpSRS
     */
    class PhysicalCpSRS : public PhysicalOperator {
       public:
        PhysicalCpSRS(const string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema) : PhysicalOperator(logicalName, physicalName, parameters, schema) {
            _arrayNameFrom = ((std::shared_ptr<OperatorParamReference> &)parameters[0])->getObjectName();
            _arrayNameTo = ((std::shared_ptr<OperatorParamReference> &)parameters[1])->getObjectName();
        }

        virtual void preSingleExecute(shared_ptr<Query> query) {
            if (_schema.isTransient()) {
                shared_ptr<const InstanceMembership> membership(Cluster::getInstance()->getInstanceMembership());

                if ((membership->getViewId() != query->getCoordinatorLiveness()->getViewId()) ||
                    (membership->getInstances().size() != query->getInstancesCount())) {
                    throw USER_EXCEPTION(SCIDB_SE_EXECUTION, SCIDB_LE_NO_QUORUM2);
                }
            }

            // Get SRS info from existing array
            SpatialArrayInfo info = PostgresWrapper::instance()->dbGetSpatialRef(_arrayNameFrom);

            // Array dimensions are currently matched by its name. They could also by matched by index number but
            // currently we think of the former to be more useful in practice, think of lat / lon as dimension names.
            // This means that the number of dimensions does NOT need to match, which is useful for e.g. aggregation
            ArrayDesc descFrom;
            ArrayDesc descTo;

            //             SystemCatalog::getInstance()->getArrayDesc ( _arrayNameFrom, descFrom );
            //             SystemCatalog::getInstance()->getArrayDesc ( _arrayNameTo, descTo );
            //

            SystemCatalog::getInstance()->getArrayDesc(_arrayNameFrom, query->getCatalogVersion(_arrayNameFrom), LAST_VERSION, descFrom);
            SystemCatalog::getInstance()->getArrayDesc(_arrayNameTo, query->getCatalogVersion(_arrayNameTo), LAST_VERSION, descTo);

            Dimensions dimFrom = descFrom.getDimensions();
            Dimensions dimTo = descTo.getDimensions();

            // Check whether dimension with equal names exist in target array
            int xdim_idx_to = -1, ydim_idx_to = -1;
            for (uint16_t i = 0; i < dimTo.size(); ++i) {
                if (info.xdim.compare(dimTo[i].getBaseName()) == 0) xdim_idx_to = i;
                if (info.ydim.compare(dimTo[i].getBaseName()) == 0) ydim_idx_to = i;
            }
            if (xdim_idx_to < 0 || ydim_idx_to < 0) {
                stringstream serr;
                serr << "Cannot find matching array dimensions '" << info.xdim << "','" << info.ydim << "' in target array";
                SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_UNKNOWN_DIMENSIONNAME);
                return;
            }

            // Check whether dimension have equal boundaries, currently we only output a warning if not1
            int xdim_idx_src = -1, ydim_idx_src = -1;
            for (uint16_t i = 0; i < dimTo.size(); ++i) {
                if (info.xdim == dimTo[i].getBaseName()) xdim_idx_src = i;
                if (info.ydim == dimTo[i].getBaseName()) ydim_idx_src = i;
            }
            if (xdim_idx_src < 0 || ydim_idx_src < 0) {
                stringstream serr;
                serr << "Cannot find array dimensions in source array";
                SCIDB4GEO_ERROR(serr.str(), SCIDB4GEO_ERR_INCONSISTENT_DBSTATE);
                return;
            }

            if (dimFrom[xdim_idx_src].getStartMin() != dimTo[xdim_idx_to].getStartMin() ||
                dimFrom[xdim_idx_src].getEndMax() != dimTo[xdim_idx_to].getEndMax() ||
                dimFrom[ydim_idx_src].getStartMin() != dimTo[ydim_idx_to].getStartMin() ||
                dimFrom[ydim_idx_src].getEndMax() != dimTo[ydim_idx_to].getEndMax()) {
                SCIDB4GEO_WARN("Array dimensions have different boundaries. Please make sure that the reference system in the target array is correct");
            }

            // Set SRS of target array
            PostgresWrapper::instance()->dbSetSpatialRef(_arrayNameTo, info.xdim, info.ydim, info.auth_name, info.auth_srid, info.A);
        }

        std::shared_ptr<Array> execute(std::vector<std::shared_ptr<Array> > &inputArrays,
                                       std::shared_ptr<Query> query) {
            return std::shared_ptr<Array>();
        }

       private:
        string _arrayNameFrom;
        string _arrayNameTo;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalCpSRS, "eo_cpsrs", "PhysicalCpSRS");
    typedef PhysicalCpSRS PhysicalCpSRS_depr;
    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalCpSRS_depr, "st_cpsrs", "PhysicalCpSRS_depr");  // Backward compatibility
}
