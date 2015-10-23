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


#ifndef ARRAY_SCHEMA_UTILS_H
#define ARRAY_SCHEMA_UTILS_H

#include <string>
#include <array/Metadata.h>

namespace scidb4geo
{
    using namespace scidb;
    using namespace std;

    class ArraySchemaUtils
    {

        static Attributes arrayAttributesFromString ( string  str );

    };



}

#endif
