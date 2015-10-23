/*
This file has been originally based on source code of SciDB (src/query/ops/create_array/LogicalCreateArray.cpp)
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

Modifications are copyright (C) 2015 Marius Appel <marius.appel@uni-muenster.de>

scidb4geo - A SciDB plugin for managing spatially referenced arrays

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

#include <query/Operator.h>
#include "ArraySchemaUtils.h"
#include "../ErrorCodes.h"

namespace scidb4geo
{

    using namespace std;
    using namespace scidb;

    struct LogicalCreateS : LogicalOperator {
        LogicalCreateS ( const string &logicalName, const string &alias )
            : LogicalOperator ( logicalName, alias ) {

            _properties.ddl       = true;

            ADD_PARAM_OUT_ARRAY_NAME()        // The array name
            ADD_PARAM_CONSTANT ( TID_STRING ) // Definition of attributes
            ADD_PARAM_CONSTANT ( TID_STRING ) // Definition of SRS
            ADD_PARAM_CONSTANT ( TID_STRING ) // Definition of coverage
            ADD_PARAM_CONSTANT ( TID_STRING ) // Definition of affine transformation
            ADD_PARAM_VARIES()        // Additional parameters (like TEMP flag)

        }

        vector<boost::shared_ptr<OperatorParamPlaceholder> > nextVaryParamPlaceholder ( const vector< ArrayDesc> &schemas ) {
            vector<boost::shared_ptr<OperatorParamPlaceholder> > res;

            if ( _parameters.size() == 4 ) {
                //res.push_back(PARAM_CONSTANT(TID_STRING));
                res.push_back ( PARAM_CONSTANT ( TID_BOOL ) );
            }

            else  res.push_back ( END_OF_VARIES_PARAMS() );

            return res;
        }

        ArrayDesc inferSchema ( vector<ArrayDesc>, shared_ptr<Query> query ) {
            assert ( param<OperatorParam> ( 0 )->getParamType() == PARAM_ARRAY_REF );
            string arrayname ( param<OperatorParamArrayReference> ( 0 )->getObjectName() );

            if ( SystemCatalog::getInstance()->containsArray ( arrayname ) ) {
                SCIDB4GEO_ERROR ( "Array with the same name already exists", SCIDB4GEO_ERR_ARRAYEXISTS );
            }

            // TODO: Derive schema, check affine transform and SRS






            return ArrayDesc();
        }

        void inferArrayAccess ( shared_ptr<Query> &query ) {
            LogicalOperator::inferArrayAccess ( query );

            assert ( param<OperatorParam> ( 0 )->getParamType() == PARAM_ARRAY_REF );

            string arrayname ( param<OperatorParamArrayReference> ( 0 )->getObjectName() );

            assert ( !arrayname.empty() && arrayname.find ( '@' ) == string::npos ); // no version number

            shared_ptr<SystemCatalog::LockDesc> lock ( make_shared<SystemCatalog::LockDesc> ( arrayname,
                    query->getQueryID(),
                    Cluster::getInstance()->getLocalInstanceId(),
                    SystemCatalog::LockDesc::COORD,
                    SystemCatalog::LockDesc::CRT ) );
            shared_ptr<SystemCatalog::LockDesc> resLock = query->requestLock ( lock );
            assert ( resLock );
            assert ( resLock->getLockMode() >= SystemCatalog::LockDesc::CRT );
        }

        template<class t>
        shared_ptr<t> &param ( size_t i ) const {
            assert ( i < _parameters.size() );

            return ( shared_ptr<t> & ) _parameters[i];
        }
    };


    DECLARE_LOGICAL_OPERATOR_FACTORY ( LogicalCreateS,     "eo_create_s" )

} //namespace

