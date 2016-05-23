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

#include "../plugin.h" // Must be first to define PROJECT_ROOT

#include <string>

#include "query/Operator.h"
#include "array/TupleArray.h"
#include "system/SystemCatalog.h"
#include <map>
#include "../PostgresWrapper.h"

namespace scidb4geo
{
    using namespace std;
    using namespace boost;
    using namespace scidb;

    /*! @copydoc LogicalOperator
     */
    class PhysicalGetMD: public PhysicalOperator
    {
    public:
        PhysicalGetMD ( string &logicalName, const string &physicalName, const Parameters &parameters, const ArrayDesc &schema ) :
            PhysicalOperator ( logicalName, physicalName, parameters, schema ) {
        }

        virtual RedistributeContext getOutputDistribution ( const std::vector<RedistributeContext> &inputDistributions,
                const std::vector< ArrayDesc> &inputSchemas ) const {
            return RedistributeContext(_schema.getDistribution(),_schema.getResidency());
        }

        void preSingleExecute ( std::shared_ptr<Query> query ) {

            std::shared_ptr<TupleArray> tuples ( std::make_shared<TupleArray> ( _schema, _arena ) );

            string arrayname;
            string namespacename;
            std::shared_ptr<OperatorParamArrayReference> &arrayRef = ( std::shared_ptr<OperatorParamArrayReference> & ) _parameters[0];
            query->getNamespaceArrayNames(arrayRef->getObjectName(), namespacename, arrayname);
            
            ArrayID arrayID = query->getCatalogVersion(namespacename, arrayname);
            ArrayDesc arrayDesc;
            SystemCatalog::getInstance()->getArrayDesc(arrayID, arrayDesc);


            map<string, string> arraymd = PostgresWrapper::instance()->dbGetArrayMD ( arrayname ); // TODO: Add domain

            for ( map<string, string>::iterator it = arraymd.begin(); it != arraymd.end(); ++it ) {
                Value tuple[5];
                tuple[0].setString ( arrayname );
                tuple[1].setString ( "" );
                tuple[2].setString ( "" ); // TODO: add domain
                tuple[3].setString ( it->first );
                tuple[4].setString ( it->second );

                tuples->appendTuple ( tuple );
            }

            for ( int i = 0; i < arrayDesc.getAttributes ( true ).size(); ++i ) {
                map<string, string> attrmd = PostgresWrapper::instance()->dbGetAttributeMD ( arrayname, arrayDesc.getAttributes ( true ) [i].getName() ); // TODO: Add domain
                for ( map<string, string>::iterator it = attrmd.begin(); it != attrmd.end(); ++it ) {
                    Value tuple[5];
                    tuple[0].setString ( arrayname );
                    tuple[1].setString ( arrayDesc.getAttributes ( true ) [i].getName() ); // bitmap empty indicator will be removed
                    tuple[2].setString ( "" ); // TODO: add domain
                    tuple[3].setString ( it->first );
                    tuple[4].setString ( it->second );

                    tuples->appendTuple ( tuple );
                }
            }

            if ( tuples->getNumberOfTuples() > 0 ) {
                tuples->truncate();
                _result = tuples;
            }

        }

        std::shared_ptr<Array> execute ( vector< std::shared_ptr<Array> > &inputArrays, std::shared_ptr<Query> query ) {
            if ( !_result ) {
                _result = std::make_shared<MemArray> ( _schema, query );
            }
            return _result;
        }

    private:
        std::shared_ptr<Array> _result;
    };

    REGISTER_PHYSICAL_OPERATOR_FACTORY ( PhysicalGetMD, "eo_getmd", "PhysicalGetMD" );

}

