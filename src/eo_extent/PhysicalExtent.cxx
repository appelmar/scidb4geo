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
#include "../TemporalReference.h"
#include "../Utils.h"

namespace scidb4geo {
    using namespace std;
    using namespace boost;
    using namespace scidb;

    /*! @copydoc LogicalExtent
     */
    class PhysicalExtent : public PhysicalOperator {
       public:
        PhysicalExtent(string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema) : PhysicalOperator(logicalName, physicalName, parameters, schema) {
        }

        virtual RedistributeContext getOutputDistribution(const std::vector<RedistributeContext> &inputDistributions,
                                                          const std::vector<ArrayDesc> &inputSchemas) const {
            return RedistributeContext(psLocalInstance);
        }

        void preSingleExecute(std::shared_ptr<Query> query) {
            vector<string> arrays(_parameters.size());
            for (uint16_t i = 0; i < _parameters.size(); ++i) {
                arrays.push_back(ArrayDesc::makeUnversionedName(((std::shared_ptr<OperatorParamReference> &)_parameters[i])->getObjectName()));
            }

            vector<SpatialArrayInfo> info_s = PostgresWrapper::instance()->dbGetSpatialRef(arrays);
            vector<TemporalArrayInfo> info_t = PostgresWrapper::instance()->dbGetTemporalRef(arrays);

            // Add vertical...
            map<string, SpatialArrayInfo> srs_map;
            map<string, TemporalArrayInfo> trs_map;

            set<string> array_set;
            for (int i = 0; i < info_s.size(); ++i) {
                array_set.insert(info_s[i].arrayname);
                srs_map.insert(std::pair<string, SpatialArrayInfo>(info_s[i].arrayname, info_s[i]));
            }
            for (int i = 0; i < info_t.size(); ++i) {
                array_set.insert(info_t[i].arrayname);
                trs_map.insert(std::pair<string, TemporalArrayInfo>(info_t[i].arrayname, info_t[i]));
            }

            std::shared_ptr<TupleArray> tuples(std::make_shared<TupleArray>(_schema, _arena));

            std::set<string>::iterator it;
            for (it = array_set.begin(); it != array_set.end(); ++it) {
                ArrayDesc arrayDesc;
                SystemCatalog::getInstance()->getArrayDesc(*it, query->getCatalogVersion(*it), LAST_VERSION, arrayDesc);

                int xdim_idx = -1;
                int ydim_idx = -1;
                int tdim_idx = -1;
                // Add vertical...

                Dimensions dims = arrayDesc.getDimensions();

                // Find dimensions
                DimensionDesc d;
                for (size_t i = 0; i < dims.size(); ++i) {
                    d = dims[i];
                    if (srs_map.find(*it) != srs_map.end()) {
                        if (d.getBaseName().compare(srs_map[*it].xdim) == 0)
                            xdim_idx = i;
                        else if (d.getBaseName().compare(srs_map[*it].ydim) == 0)
                            ydim_idx = i;
                    }
                    if (trs_map.find(*it) != trs_map.end()) {
                        if (d.getBaseName().compare(trs_map[*it].tdim) == 0) tdim_idx = i;
                    }
                    // Add vertical...
                }

                Coordinates lowBoundary = SystemCatalog::getInstance()->getLowBoundary(arrayDesc.getId());
                Coordinates highBoundary = SystemCatalog::getInstance()->getHighBoundary(arrayDesc.getId());

                /* Array bounds might be unknown und thus equal maximum int64 values.
            The followoing loop tries to find better values based on dimension settings */
                for (size_t i = 0; i < dims.size(); ++i) {
                    if (abs(lowBoundary[i]) == SCIDB_MAXDIMINDEX) {
                        lowBoundary[i] = dims[i].getCurrStart();
                        if (abs(lowBoundary[i]) == SCIDB_MAXDIMINDEX) lowBoundary[i] = dims[i].getStartMin();
                    }
                    if (abs(highBoundary[i]) == SCIDB_MAXDIMINDEX) {
                        highBoundary[i] = dims[i].getCurrEnd();
                        if (abs(highBoundary[i]) == SCIDB_MAXDIMINDEX) highBoundary[i] = dims[i].getEndMax();
                    }
                }

                Value tuple[10];
                tuple[0].setString(*it);
                stringstream setting;

                if (srs_map.find(*it) != srs_map.end()) {
                    setting << "s";
                    // Transform all 4 corners
                    //stringstream sd;
                    //sd << "Bounds X:(" << lowBoundary[xdim_idx] << "," << highBoundary[xdim_idx] << ")\n";
                    //sd << "Bounds Y:(" << lowBoundary[ydim_idx] << "," << highBoundary[ydim_idx] << ")\n";
                    //SCIDB4GEO_DEBUG ( sd.str() );

                    AffineTransform::double2 ps1_world, ps2_world, ps3_world, ps4_world;
                    AffineTransform::double2 ps1(lowBoundary[xdim_idx], lowBoundary[ydim_idx]);
                    AffineTransform::double2 ps2(lowBoundary[xdim_idx], highBoundary[ydim_idx]);
                    AffineTransform::double2 ps3(highBoundary[xdim_idx], lowBoundary[ydim_idx]);
                    AffineTransform::double2 ps4(highBoundary[xdim_idx], highBoundary[ydim_idx]);
                    srs_map[*it].A.f(ps1, ps1_world);
                    srs_map[*it].A.f(ps2, ps2_world);
                    srs_map[*it].A.f(ps3, ps3_world);
                    srs_map[*it].A.f(ps4, ps4_world);

                    // Find minimum and maximum of transformed corners
                    double x[4] = {ps1_world.x, ps2_world.x, ps3_world.x, ps4_world.x};
                    double y[4] = {ps1_world.y, ps2_world.y, ps3_world.y, ps4_world.y};
                    AffineTransform::double2 lowleft(utils::min(x, 4), utils::min(y, 4));
                    AffineTransform::double2 upright(utils::max(x, 4), utils::max(y, 4));

                    tuple[2].setDouble(lowleft.x);
                    tuple[3].setDouble(upright.x);
                    tuple[4].setDouble(lowleft.y);
                    tuple[5].setDouble(upright.y);
                }

                if (trs_map.find(*it) != trs_map.end()) {
                    setting << "t";
                    stringstream sd;
                    sd << "Bounds T:(" << lowBoundary[tdim_idx] << "," << highBoundary[tdim_idx] << ")";
                    SCIDB4GEO_DEBUG(sd.str());
                    tuple[6].setString(trs_map[*it].tref->datetimeAtIndex(lowBoundary[tdim_idx]).toStringISO());
                    tuple[7].setString(trs_map[*it].tref->datetimeAtIndex(highBoundary[tdim_idx]).toStringISO());
                    //delete info_t[0].tref;
                }
                // Add vertical...

                tuple[1].setString(setting.str());
                tuples->appendTuple(tuple);
            }

            _result = tuples;
        }

        std::shared_ptr<Array> execute(vector<std::shared_ptr<Array> > &inputArrays, std::shared_ptr<Query> query) {
            if (!_result) {
                _result = std::make_shared<MemArray>(_schema, query);
            }
            return _result;
        }

       private:
        std::shared_ptr<Array> _result;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalExtent, "eo_extent", "PhysicalExtent");
    typedef PhysicalExtent PhysicalExtent_depr;
    REGISTER_PHYSICAL_OPERATOR_FACTORY(PhysicalExtent_depr, "st_extent", "PhysicalExtent_depr");  // Backward compatibility
}  //namespace
