/*
scidb4geo - A SciDB plugin for managing spatially referenced arrays
Copyright (C) 2015 Marius Appel <marius.appel@uni-muenster.de>

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

#include "../plugin.h" // Must be first

#include "ArraySchemaUtils.h"
#include "../ErrorCodes.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include "query/TypeSystem.h"

namespace scidb4geo
{
    using namespace scidb;
    using namespace std;

    Attributes ArraySchemaUtils::arrayAttributesFromString ( string str )
    {

        boost::trim ( str );
        if ( str[0] != '<' || str[str.length() - 1] != '>' ) {
            //SCIDB4GEO_ERROR("Cannot derive attribute definitions from schema string",SCIDB4GEO_ERR_UNKNOWN);
        }

        string s = str.substr ( 1, str[str.length() - 2] ); // remove < >
        vector<string> single_attr_strings;
        boost::split ( single_attr_strings, s, boost::is_any_of ( "," ) );
        if ( single_attr_strings.size() == 0 ) {
            //SCIDB4GEO_ERROR("Cannot derive attribute definitions from schema string",SCIDB4GEO_ERR_UNKNOWN);
        }

        Attributes outAttrs;
        int i = 0;
        for ( vector<string>::iterator attr_str = single_attr_strings.begin(); attr_str != single_attr_strings.end(); ++attr_str ) {
            bool attr_nullable = false;

            string attr_default = "";
            string attr_typeid = "";
            vector<string> name_def;
            boost::split ( name_def, *attr_str, boost::is_any_of ( ":" ) );
            if ( name_def.size() != 2 ) {
                //SCIDB4GEO_ERROR("Cannot derive attribute definitions from schema string",SCIDB4GEO_ERR_UNKNOWN);
            }
            boost::trim ( name_def[0] );
            boost::trim ( name_def[1] );
            string attr_name = name_def[0];
            string def =  name_def[1];
            attr_typeid = def.substr ( 0, def.find_first_of ( " " ) - 1 );
            int attr_flags = 0;
            if ( attr_nullable ) attr_flags = AttributeDesc::IS_NULLABLE; // TODO: Change to &=

            if ( boost::contains ( def, "NOT NULL" ) ) attr_nullable = false;
            else if ( boost::contains ( def, "NULL" ) ) attr_nullable = true;

            //if (boost::contains(def, "DEFAULT"))  // TODO: Take next word as default value

            outAttrs.push_back ( AttributeDesc ( ( AttributeID ) i++, attr_name , attr_typeid, attr_flags, 0 ) );


        }

        return outAttrs;

    }






}

